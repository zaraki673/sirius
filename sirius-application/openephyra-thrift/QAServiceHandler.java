// This handler implements the services provided to the client.

// Open Ephyra packages
import info.ephyra.OpenEphyra;
//import info.ephyra.search.Result;
//import info.ephyra.io.MsgPrinter;

// DEBUG: Open Ephyra additional testing packages
import info.ephyra.answerselection.AnswerSelection;
import info.ephyra.answerselection.filters.AnswerPatternFilter;
import info.ephyra.answerselection.filters.AnswerTypeFilter;
import info.ephyra.answerselection.filters.DuplicateFilter;
import info.ephyra.answerselection.filters.FactoidSubsetFilter;
import info.ephyra.answerselection.filters.FactoidsFromPredicatesFilter;
import info.ephyra.answerselection.filters.PredicateExtractionFilter;
import info.ephyra.answerselection.filters.QuestionKeywordsFilter;
import info.ephyra.answerselection.filters.ScoreCombinationFilter;
import info.ephyra.answerselection.filters.ScoreNormalizationFilter;
import info.ephyra.answerselection.filters.ScoreSorterFilter;
import info.ephyra.answerselection.filters.StopwordFilter;
import info.ephyra.answerselection.filters.TruncationFilter;
import info.ephyra.answerselection.filters.WebDocumentFetcherFilter;
import info.ephyra.io.Logger;
import info.ephyra.io.MsgPrinter;
import info.ephyra.nlp.LingPipe;
import info.ephyra.nlp.NETagger;
import info.ephyra.nlp.OpenNLP;
import info.ephyra.nlp.SnowballStemmer;
import info.ephyra.nlp.StanfordNeTagger;
import info.ephyra.nlp.StanfordParser;
import info.ephyra.nlp.indices.FunctionWords;
import info.ephyra.nlp.indices.IrregularVerbs;
import info.ephyra.nlp.indices.Prepositions;
import info.ephyra.nlp.indices.WordFrequencies;
import info.ephyra.nlp.semantics.ontologies.Ontology;
import info.ephyra.nlp.semantics.ontologies.WordNet;
import info.ephyra.querygeneration.Query;
import info.ephyra.querygeneration.QueryGeneration;
import info.ephyra.querygeneration.generators.BagOfTermsG;
import info.ephyra.querygeneration.generators.BagOfWordsG;
import info.ephyra.querygeneration.generators.PredicateG;
import info.ephyra.querygeneration.generators.QuestionInterpretationG;
import info.ephyra.querygeneration.generators.QuestionReformulationG;
import info.ephyra.questionanalysis.AnalyzedQuestion;
import info.ephyra.questionanalysis.QuestionAnalysis;
import info.ephyra.questionanalysis.QuestionInterpreter;
import info.ephyra.questionanalysis.QuestionNormalizer;
import info.ephyra.search.Result;
import info.ephyra.search.Search;
import info.ephyra.search.searchers.BingKM;
import info.ephyra.search.searchers.IndriKM;

import java.util.ArrayList;

// Interface definition
import qastubs.QAService;
import java.util.List;

public class QAServiceHandler implements QAService.Iface {
	public void askQuestion(List<String> arguments)
	{
		System.out.println("askQuestion():");
		String[] argString = arguments.toArray(new String[arguments.size()]);
		OpenEphyra.main(argString);
	}

	public String askFactoidThrift(String question)
	{
		System.out.println("askFactoidThrift():");
		String dir = "";
		
		// enable msg printing to screen (logging not enabled)
		MsgPrinter.enableStatusMsgs(true);
		MsgPrinter.enableErrorMsgs(true);
		MsgPrinter.printStatusMsg("askFactoidThrift(): Arg = " + question);

		OpenEphyra oe  = new OpenEphyra(dir);
		Result result = oe.askFactoid(question);
		String answer = result.getAnswer();
		return answer;
	}

	public String askFactoidThriftDebug(String question)
	{
		System.out.println("askFactoidThriftDebug():");
		//String dir = "~/sirius/sirius-application/question-answer/";
		String dir = "";
		// enable msg printing to screen (logging not enabled)
		MsgPrinter.enableStatusMsgs(true);
		MsgPrinter.enableErrorMsgs(true);
		MsgPrinter.printStatusMsg("askFactoidThrift(): Arg = " + question);

		OpenEphyra oe  = new OpenEphyra(dir);
		// FUNCTION: initFactoid():
		// question analysis
		Ontology wordNet = new WordNet();
		// - dictionaries for term extraction
		QuestionAnalysis.clearDictionaries();
		QuestionAnalysis.addDictionary(wordNet);
		// - ontologies for term expansion
		QuestionAnalysis.clearOntologies();
		QuestionAnalysis.addOntology(wordNet);
		
		// query generation
		QueryGeneration.clearQueryGenerators();
		QueryGeneration.addQueryGenerator(new BagOfWordsG());
		QueryGeneration.addQueryGenerator(new BagOfTermsG());
		QueryGeneration.addQueryGenerator(new PredicateG());
		QueryGeneration.addQueryGenerator(new QuestionInterpretationG());
		QueryGeneration.addQueryGenerator(new QuestionReformulationG());
		
		// search
		// - knowledge miners for unstructured knowledge sources
		Search.clearKnowledgeMiners();
		for (String[] indriIndices : IndriKM.getIndriIndices())
			Search.addKnowledgeMiner(new IndriKM(indriIndices, false));
		
		// - knowledge annotators for (semi-)structured knowledge sources
		Search.clearKnowledgeAnnotators();
		
		// answer extraction and selection
		// (the filters are applied in this order)
		AnswerSelection.clearFilters();
		// - answer extraction filters
		AnswerSelection.addFilter(new AnswerTypeFilter());
		AnswerSelection.addFilter(new AnswerPatternFilter());
		AnswerSelection.addFilter(new PredicateExtractionFilter());
		AnswerSelection.addFilter(new FactoidsFromPredicatesFilter());
		AnswerSelection.addFilter(new TruncationFilter());
		// - answer selection filters
		AnswerSelection.addFilter(new StopwordFilter());
		AnswerSelection.addFilter(new QuestionKeywordsFilter());
		AnswerSelection.addFilter(new ScoreNormalizationFilter(OpenEphyra.NORMALIZER));
		AnswerSelection.addFilter(new ScoreCombinationFilter());
		AnswerSelection.addFilter(new FactoidSubsetFilter());
		AnswerSelection.addFilter(new DuplicateFilter());
		AnswerSelection.addFilter(new ScoreSorterFilter());

		// FUNCTION: askFactoid():
		// analyze question
		MsgPrinter.printAnalyzingQuestion();
		AnalyzedQuestion aq = QuestionAnalysis.analyze(question);
		
		// FUNCTION: runPipeline():
		// query generation
		MsgPrinter.printGeneratingQueries();
		Query[] queries = QueryGeneration.getQueries(aq);
		System.out.println("#queries: " + queries.length);
		
		// search
		MsgPrinter.printSearching();
		Result[] results = Search.doSearch(queries);
		System.out.println("#results: " + results.length);	
	
		// answer selection
		MsgPrinter.printSelectingAnswers();
		results = AnswerSelection.getResults(results, 1, 0);

		String answer = results[0].getAnswer();
		System.out.println("Java handler says: your answer is " + answer);
		return answer;
	}
}

