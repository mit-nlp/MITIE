
# A list of all the folders that have makefiles in them.  Running make all builds all these things
SUBDIRS = tools/ner_stream examples/C/ner examples/C/relation_extraction examples/cpp/ner examples/cpp/train_ner \
	  examples/cpp/train_relation_extraction examples/cpp/relation_extraction examples/cpp/text_categorizer \
	  examples/cpp/train_text_categorizer examples/cpp/train_text_categorizer_BoW

examples: tools/ner_stream examples/C/ner examples/C/relation_extraction examples/cpp/train_text_categorizer_BoW
	cp examples/C/ner/ner_example .
	cp examples/C/relation_extraction/relation_extraction_example .
	cp tools/ner_stream/ner_stream .
	cp examples/cpp/train_text_categorizer_BoW/train_text_categorizer_BoW_example .

MITIE-models-v0.2.tar.bz2:
	curl -LO https://github.com/mit-nlp/MITIE/releases/download/v0.4/MITIE-models-v0.2.tar.bz2

MITIE-models: MITIE-models-v0.2.tar.bz2
	tar -xjf MITIE-models-v0.2.tar.bz2
	
test: all examples MITIE-models
	./ner_stream MITIE-models/english/ner_model.dat < sample_text.txt > /tmp/MITIE_test.out
	diff /tmp/MITIE_test.out sample_text.reference-output
	./relation_extraction_example MITIE-models/english/ner_model.dat MITIE-models/english/binary_relations/rel_classifier_location.location.contains.svm sample_text.txt > /tmp/MITIE_test_rel.out
	diff /tmp/MITIE_test_rel.out sample_text.reference-output-relations
	./train_text_categorizer_BoW_example > /tmp/MITIE_test_bow.out
	@echo Testing completed successfully



.PHONY: mitie mitielib $(SUBDIRS)
all: $(SUBDIRS)
mitie: mitielib
$(SUBDIRS): mitie 
	$(MAKE) -C $@
mitielib: dlib/dlib
	$(MAKE) -C $@
clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	@rm -rf ner_stream ner_example relation_extraction_example train_text_categorizer_BoW_example
