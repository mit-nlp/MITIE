# temporarily add the path to the mitie shared library to the proper linux and
# mac os environment variables respectively
export LD_LIBRARY_PATH=../../mitielib 
export DYLD_LIBRARY_PATH=../../mitielib 

export CLASSPATH=../../mitielib/javamitie.jar:. 

javac TrainSeparateDocCategorizerExample.java

java -Djava.library.path="../../mitielib/" TrainSeparateDocCategorizerExample
