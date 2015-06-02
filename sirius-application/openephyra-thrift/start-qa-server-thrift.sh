# NOTE ABOUT CLASSPATHS:
# Classpaths contain jar files and paths to the TOP of package hierarchies.
# For example, say a program imports the class info.ephyra.OpenEphyra
# Now javac knows to look for the class info.ephyra.OpenEphyra in the directory info/ephyra/
# However, javac still needs the classpath to the package.

# Add thrift libraries to class path
export CLASSPATH=/home/tollben/thrift-0.9.2/lib/java/build/libthrift-0.9.2.jar:/home/tollben/thrift-0.9.2/lib/java/build/lib/slf4j-api-1.5.8.jar:/home/tollben/thrift-0.9.2/lib/java/build/lib/slf4j-log4j12-1.5.8.jar:/home/tollben/thrift-0.9.2/lib/java/build/lib/log4j-1.2.14.jar

# Add open ephyra libraries to class path
# export CLASSPATH=$CLASSPATH:/home/tollben/sirius/sirius-application/question-answer/bin

# Add my classes to class path
# NOTE: these class paths are only necessary when running the program
export CLASSPATH=$CLASSPATH:/home/tollben/sirius/sirius-application/question-answer/openephyra-thrift:/home/tollben/sirius/sirius-application/question-answer/openephyra-thrift/gen-java

# Add open ephyra libraries to class path; start from top directory (question-answer)
cd ..;

export CLASSPATH=$CLASSPATH:bin:lib/ml/maxent.jar:lib/ml/minorthird.jar:lib/nlp/jwnl.jar:lib/nlp/lingpipe.jar:lib/nlp/opennlp-tools.jar:lib/nlp/plingstemmer.jar:lib/nlp/snowball.jar:lib/nlp/stanford-ner.jar:lib/nlp/stanford-parser.jar:lib/nlp/stanford-postagger.jar:lib/qa/javelin.jar:lib/search/bing-search-java-sdk.jar:lib/search/googleapi.jar:lib/search/indri.jar:lib/search/yahoosearch.jar:lib/util/commons-logging.jar:lib/util/gson.jar:lib/util/htmlparser.jar:lib/util/log4j.jar:lib/util/trove.jar:lib/util/servlet-api.jar:lib/util/jetty-all.jar:lib/util/commons-codec-1.9.jar

export INDRI_INDEX=`pwd`/wiki_indri_index/
#export THREADS=$(cat /proc/cpuinfo | grep processor | wc -l)
#if [ $THREADS -lt 8 ]; then
#  export THREADS=8
#fi

java -Djava.library.path=lib/search/ -server -Xms1024m -Xmx2048m QADaemon

#java QADaemon
#java -agentlib:jdwp=transport=dt_socket,address=8000,server=y,suspend=n QADaemon
