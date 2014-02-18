
SRC = main.c
SRC += ../../mitielib/src/mitie.cpp
SRC += ../../mitielib/src/named_entity_extractor.cpp
SRC += ../../mitielib/src/ner_feature_extraction.cpp
SRC += ../../mitielib/src/stem.c
SRC += ../../mitielib/src/stemmer.cpp


CFLAGS = -Wall -W -O3 -I../../mitielib/include -I../../dlib
LFLAGS = 
CC = g++

TARGET = mitie_example 

####################################################

TMP = $(SRC:.cpp=.o)
OBJ = $(TMP:.c=.o)

$(TARGET): $(OBJ) 
	@echo Linking $@
	@$(CC) $(OBJ) -o $@ $(LFLAGS) 
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

