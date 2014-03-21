examples: ner_example ner_stream
ner_example:
	(cd examples/C; make)
	cp examples/C/mitie_ner .

ner_stream:
	(cd tools/ner_stream; make)
	cp tools/ner_stream/ner_stream .
clean:
	(cd tools/ner_stream; make clean)
	(cd examples/C; make clean)
