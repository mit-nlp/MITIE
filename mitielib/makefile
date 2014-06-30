
INSTALL_PREFIX=/usr/local

SRC = src/mitie.cpp
SRC += src/named_entity_extractor.cpp
SRC += src/ner_feature_extraction.cpp
SRC += src/binary_relation_detector.cpp
SRC += src/stem.c
SRC += src/stemmer.cpp


CFLAGS =  -fpic  -Wall -W  -O3   -Iinclude -I../dlib 
LFLAGS = -shared  
CC = g++

HEADERS=include/mitie.h

####################################################

TMP = $(SRC:.cpp=.o)
OBJ = $(TMP:.c=.o)


all: libmitie.so libmitie.a

libmitie.so: $(OBJ) 
	@echo Linking $@
	@$(CC) $(OBJ) -o $@ $(LFLAGS) 

libmitie.a: libmitie.so
	@echo Making $@
	@ar -rsc $@ $(OBJ)
	@echo Build Complete

.cpp.o: $<
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $< -o $@

.c.o: $<
	@echo Compiling $<
	@gcc -c $(CFLAGS) $< -o $@

install: all 
	@echo copying libmitie.so and libmitie.a to $(INSTALL_PREFIX)/lib
	@cp libmitie.so libmitie.a $(INSTALL_PREFIX)/lib/
	@echo copying $(HEADERS) to $(INSTALL_PREFIX)/include
	@cp $(HEADERS) $(INSTALL_PREFIX)/include/

clean:
	@rm -f $(OBJ) libmitie.so libmitie.a 
	@echo All object files and binaries removed

dep: 
	@echo Running makedepend
	@makedepend -- $(CFLAGS) -- $(SRC) 2> /dev/null 
	@echo Completed makedepend

################################################
##########  Stuff from makedepend  #############
################################################

