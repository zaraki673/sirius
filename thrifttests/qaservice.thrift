# thrift compiler translates each service into .cpp/.h files
# clients include these files so that they can serialize their args in the correct form
# servers include these files so that they can serialize return values in the correct form
namespace java tutorial

service QAService
{
	void AskQuestion(1:list<string> question)
}
