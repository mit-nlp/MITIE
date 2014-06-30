
SRC = ner_example.cpp
SRC += ../../mitielib/src/mitie.cpp
SRC += ../../mitielib/src/named_entity_extractor.cpp
SRC += ../../mitielib/src/ner_feature_extraction.cpp
SRC += ../../mitielib/src/binary_relation_detector.cpp
SRC += ../../mitielib/src/stem.c
SRC += ../../mitielib/src/stemmer.cpp


CFLAGS = -fPIC -Wall -W -O3 -I../../mitielib/include -I../../dlib
LDFLAGS = 
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	LDFLAGS += -static
endif
#ifeq ($(UNAME_S),Darwin)
#	LDFLAGS += 
#endif
CC = g++

TARGET = ner_example

####################################################

TMP = $(SRC:.cpp=.o)
OBJ = $(TMP:.c=.o)

$(TARGET): $(OBJ) 
	@echo Linking $@ with flags: $(LDFLAGS)
	@$(CC) $(OBJ) -o $@ $(LDFLAGS) 
	@echo Build Complete

.cpp.o: $<
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $< -o $@

.c.o: $<
	@echo Compiling $<
	@gcc -c $(CFLAGS) $< -o $@

clean:
	@rm -f $(OBJ) $(TARGET)
	@echo All object files and binaries removed

dep: 
	@echo Running makedepend
	@makedepend -- $(CFLAGS) -- $(SRC) 2> /dev/null 
	@echo Completed makedepend

################################################
##########  Stuff from makedepend  #############
################################################

