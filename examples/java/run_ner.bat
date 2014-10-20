setlocal
set PATH=../../mitielib;%PATH%
set CLASSPATH=../../mitielib/javamitie.jar;. 


javac NerExample.java

java NerExample ../../sample_text.txt  ../../MITIE-models/english/ner_model.dat ../../MITIE-models/english/binary_relations/rel_classifier_people.person.place_of_birth.svm

pause
