#BLAS_PATH=../../OpenBLAS-0.2.14/libopenblas.a
#LIBGFORTRAN_PATH=/usr/local/gfortran/lib/libgfortran.a

CFLAGS=
ifeq ($(BLAS_PATH),)
	BLAS=
	CFLAGS=
else
	BLAS=$(BLAS_PATH) $(LIBGFORTRAN_PATH)
	CFLAGS += -DDLIB_USE_BLAS
#	BLAS=-L$(BLAS_PATH) -lopenblas -L/usr/local/gfortran/lib -lgfortran
endif
INSTALL_PREFIX=/usr/local

SRC = src/mitie.cpp
SRC += src/named_entity_extractor.cpp
SRC += src/ner_feature_extraction.cpp
SRC += src/binary_relation_detector.cpp
SRC += src/binary_relation_detector_trainer.cpp
SRC += src/stem.c
SRC += src/stemmer.cpp
SRC += src/conll_parser.cpp
SRC += src/ner_trainer.cpp
SRC += ../dlib/dlib/threads/multithreaded_object_extension.cpp
SRC += ../dlib/dlib/threads/threaded_object_extension.cpp
SRC += ../dlib/dlib/threads/threads_kernel_1.cpp
SRC += ../dlib/dlib/threads/threads_kernel_2.cpp
SRC += ../dlib/dlib/threads/threads_kernel_shared.cpp
SRC += ../dlib/dlib/threads/thread_pool_extension.cpp
SRC += ../dlib/dlib/misc_api/misc_api_kernel_1.cpp
SRC += ../dlib/dlib/misc_api/misc_api_kernel_2.cpp

CFLAGS +=  -fpic  -Wall -W  -O3   -Iinclude -I../dlib 
LDFLAGS = -shared
CC = g++

HEADERS=include/mitie.h

####################################################

TMP = $(SRC:.cpp=.o)
OBJ = $(TMP:.c=.o)


all: libmitie.so libmitie.a

libmitie.so: $(OBJ) 
	@echo Linking $@
	@$(CC) -shared $(OBJ) $(BLAS) -o $@

libmitie.a: libmitie.so
	@echo Making $@
	@ar -rsc $@ $(OBJ) $(BLAS)
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

