MITIE: MIT Information Extraction
=====

This project provides free (even for commercial use)
[state-of-the-art](../../wiki/Evaluation) information extraction
tools. The current release includes tools for performing named entity
extraction and binary relation detection as well as tools for training
custom extractors and relation detectors.  

MITIE comes with trained models for English and Spanish.  The English named entity recognition model is 
trained based on data from the English Gigaword news corpus, the CoNLL 2003 named entity recognition task,
and ACE data.  The Spanish model is based on the Spanish Gigaword corpus and CoNLL 2002 named entity 
recognition task.  There are also 21 English binary relation extraction models provided which
were trained on a 
[combination of Wikipedia and Freebase data](https://sourceforge.net/projects/mitie/files/freebase_wikipedia_binary_relation_training_data_v1.0.tar.bz2/download).

Additionally, the core library provides APIs in C, C++, and Python 2.7.  Outside
projects have created bindings for [OCaml](https://github.com/travisbrady/omitie) and 
[.NET](https://github.com/BayardRock/MITIE-Dot-Net).  Future releases will 
add bindings in Java, R, and possibly other languages.

# Using MITIE

MITIE's primary API is a C API which is documented in the
[mitie.h](mitielib/include/mitie.h) header file.  Beyond this, there are many
[example programs](examples/) showing how to use MITIE from C, C++, or Python 2.7.

### Initial Setup

Before you can run the provided examples you will need to download the trained
model files which you can do by running:
```
make MITIE-models
```
or by simply downloading the [MITIE-models-v0.2.tar.bz2](http://sourceforge.net/projects/mitie/files/binaries/MITIE-models-v0.2.tar.bz2)
file and extracting it in your MITIE folder.  Note that the Spanish models are supplied in 
a separate download.  So if you want to use the Spanish NER model then download [MITIE-models-v0.2-Spanish.zip](http://sourceforge.net/projects/mitie/files/binaries/MITIE-models-v0.2-Spanish.zip) and
extract it into your MITIE folder.

### Using MITIE from the command line

MITIE comes with a basic streaming NER tool.  So you can tell MITIE to
process each line of a text file independently and output marked up text with the command:

```
cat sample_text.txt | ./ner_stream MITIE-models/english/ner_model.dat  
```

The ner_stream executable can be compiled by running `make` in the top level MITIE folder or
by navigating to the [tools/ner_stream](tools/ner_stream) folder and running `make` or using 
CMake to build it which can be done with the following commands:
```
cd tools/ner_stream
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Compiling MITIE as a shared library

On a UNIX like system, this can be accomplished by running `make` in the top level MITIE folder or
by running:
```
cd mitielib
make
```
This produces shared and static library files in the mitielib folder.  On a non-UNIX system you can use
CMake to compile a shared library by typing:
```
cd mitielib
mkdir build
cd build
cmake ..
cmake --build . --config Release --target install
```

Either of these methods will create a MITIE shared library in the mitielib folder.

### Using MITIE from a Python 2.7 program

Once you have built the MITIE shared library, you can go to the [examples/python](examples/python) folder
and simply run any of the Python scripts.  Each script is a tutorial explaining some aspect of
MITIE: [named entity recognition and relation extraction](examples/python/ner.py), 
[training a custom NER tool](examples/python/train_ner.py), or 
[training a custom relation extractor](examples/python/train_ner.py).

### Using MITIE from R

MITIE can be installed as an R package. See the [README](tools/R-binding) for more details.

### Using MITIE from a C program

There are example C programs in the [examples/C](examples/C) folder.  To compile of them you simply
go into those folders and run `make`.  Or use CMake like so:
```
cd examples/C/ner
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Using MITIE from a C++ program

There are example C++ programs in the [examples/cpp](examples/cpp) folder.  To compile any of them you simply
go into those folders and run `make`.  Or use CMake like so:
```
cd examples/cpp/ner
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Using MITIE from a Java program

There is an example Java program in the [examples/java](examples/java) folder.  Before you can run it you
must compile MITIE's java interface which you can do like so:
```
cd mitielib/java
mkdir build
cd build
cmake ..
cmake --build . --config Release --target install
```
That will place a javamitie shared library and jar file into the mitielib folder.  Once you have those
two files you can run the example program in examples/java by running run_ner.bat if you are on Windows or
run_ner.sh if you are on a POSIX system like Linux or OS X.

Also note that you must have Swig and the Java JDK installed to compile the MITIE interface.

### Running MITIE's unit tests

You can run a simple regression test to validate your build.  Do this by running
the following command from the top level MITIE folder:

```
make test
```

`make test` builds both the example programs and downloads required
example models.  If you require a non-standard C++ compiler, change
`CC` in `examples/C/makefile` and in `tools/ner_stream/makefile`.


# Precompiled Python 2.7 binaries

We have built Python 2.7 binaries packaged with sample models for 64bit Linux and Windows (both 32 and 64 bit version of Python).  You can download the precompiled package here: [Precompiled MITIE 0.2](http://sourceforge.net/projects/mitie/files/binaries/mitie-v0.2-python-2.7-windows-or-linux64.zip)

# License

MITIE is licensed under the Boost Software License - Version 1.0 - August 17th, 2003.  

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
