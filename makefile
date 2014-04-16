examples: ner_example ner_stream mitie

MITIE-models:
	wget -O - http://sourceforge.net/projects/mitie/files/binaries/example-models.zip > example-models.zip
	unzip example-models.zip

ner_example:
	(cd examples/C; make)
	cp examples/C/ner_example .

ner_stream:
	(cd tools/ner_stream; make)
	cp tools/ner_stream/ner_stream .

mitie: 
	(cd mitielib; make)

test: ner_stream MITIE-models
	./ner_stream MITIE-models/ner_model.dat < sample_text.txt > /tmp/test.out
	diff /tmp/test.out sample_text.reference-output
clean:
	(cd tools/ner_stream; make clean)
	(cd examples/C; make clean)
	(cd mitielib; make clean)
	rm -rf ner_stream ner_example 
clean-models:
	rm -rf MITIE-models
