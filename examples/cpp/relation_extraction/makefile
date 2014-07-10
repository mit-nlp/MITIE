
SRC = relation_extraction_example.cpp
TARGET = relation_extraction_example

MITIEDIR = ../../../mitielib

CFLAGS = -fPIC -Wall -W -O3 -I$(MITIEDIR)/include -I../../../dlib
LDFLAGS = $(MITIEDIR)/libmitie.a -lpthread
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	#LDFLAGS += -static
endif
#ifeq ($(UNAME_S),Darwin)
#	LDFLAGS += 
#endif
CC = g++


####################################################

TMP = $(SRC:.cpp=.o)
OBJ = $(TMP:.c=.o)

$(TARGET): $(OBJ) $(MITIEDIR)
	@echo Linking $@ with flags: $(LDFLAGS)
	@$(CC) $(OBJ) -o $@ $(LDFLAGS) 
	@echo Build Complete

.PHONY: $(MITIEDIR)
$(MITIEDIR):
	@$(MAKE) -C $(MITIEDIR)

.cpp.o: $<
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $< -o $@

.c.o: $<
	@echo Compiling $<
	@gcc -c $(CFLAGS) $< -o $@

clean:
	@rm -f $(OBJ) $(TARGET)
	@$(MAKE) -C $(MITIEDIR) clean
	@echo All object files and binaries removed

dep: 
	@echo Running makedepend
	@makedepend -- $(CFLAGS) -- $(SRC) 2> /dev/null 
	@echo Completed makedepend

################################################
##########  Stuff from makedepend  #############
################################################

