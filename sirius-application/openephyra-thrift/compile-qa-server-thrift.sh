# NOTE ABOUT CLASSPATHS:
# Classpaths contain jar files and paths to the TOP of package hierarchies.
# For example, say a program imports the class info.ephyra.OpenEphyra
# Now javac knows to look for the class info.ephyra.OpenEphyra in the directory info/ephyra/
# However, javac still needs the classpath to the package.

# Add thrift libraries to class path
export CLASSPATH=~/thrift-0.9.2/lib/java/build/libthrift-0.9.2.jar:~/thrift-0.9.2/lib/java/build/lib/slf4j-api-1.5.8.jar:~/thrift-0.9.2/lib/java/build/lib/slf4j-log4j12-1.5.8.jar:~/thrift-0.9.2/lib/java/build/lib/log4j-1.2.14.jar

# Add command center to class path
export CLASSPATH=$CLASSPATH:~/sirius/sirius-application/command-center/gen-java

# Add open ephyra libraries to class path
export CLASSPATH=$CLASSPATH:~/sirius/sirius-application/question-answer/bin

javac QADaemon.java QAServiceHandler.java gen-java/qastubs/QAService.java
