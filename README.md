MITIE: MIT Information Extraction
=====

This project provides free (even for commercial use)
[state-of-the-art](../../wiki/Evaluation) information extraction
tools. The current release includes tools for performing [named entity
extraction](http://blog.dlib.net/2014/04/mitie-completely-free-and-state-of-art.html) 
and [binary relation detection](http://blog.dlib.net/2014/07/mitie-v02-released-now-includes-python.html) 
as well as tools for training custom extractors and relation detectors.  

MITIE is built on top of [dlib](http://dlib.net), a high-performance machine-learning library[1], MITIE makes use of several state-of-the-art techniques including the use of distributional word embeddings[2] and Structural Support Vector Machines[3].  MITIE offers several pre-trained models providing varying levels of support for both English, Spanish, and German trained using a variety of linguistic resources (e.g., CoNLL 2003, ACE, [Wikipedia, Freebase](https://github.com/mit-nlp/MITIE/releases/download/v0.4/freebase_wikipedia_binary_relation_training_data_v1.0.tar.bz2), and Gigaword). The core MITIE software is written in C++, but bindings for several other software languages including Python, R, Java, C, and MATLAB allow a user to quickly integrate MITIE into his/her own applications.

Outside projects have created API bindings for [OCaml](https://github.com/travisbrady/omitie), 
[.NET](https://github.com/BayardRock/MITIE-Dot-Net), [.NET Core](https://github.com/slamj1/MitieNetCore), and
[Ruby](https://github.com/ankane/mitie).  There is also an [interactive tool](https://github.com/Sotera/mitie-trainer) for labeling data and training MITIE.

# Using MITIE

MITIE's primary API is a C API which is documented in the
[mitie.h](mitielib/include/mitie.h) header file.  Beyond this, there are many
[example programs](examples/) showing how to use MITIE from C, C++, Java, R, or Python 2.7.

### Initial Setup

Before you can run the provided examples you will need to download the trained
model files which you can do by running:
```
make MITIE-models
```
or by simply downloading the [MITIE-models-v0.2.tar.bz2](https://github.com/mit-nlp/MITIE/releases/download/v0.4/MITIE-models-v0.2.tar.bz2)
file and extracting it in your MITIE folder.  Note that the Spanish and German models are supplied in 
separate downloads.  So if you want to use the Spanish NER model then download [MITIE-models-v0.2-Spanish.zip](https://github.com/mit-nlp/MITIE/releases/download/v0.4/MITIE-models-v0.2-Spanish.zip) and
extract it into your MITIE folder.  Similarly for the German model: [MITIE-models-v0.2-German.tar.bz2](https://github.com/mit-nlp/MITIE/releases/download/v0.4/MITIE-models-v0.2-German.tar.bz2)

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
This produces shared and static library files in the mitielib folder.  Or you can use
CMake to compile a shared library by typing:
```
cd mitielib
mkdir build
cd build
cmake ..
cmake --build . --config Release --target install
```

Either of these methods will create a MITIE shared library in the mitielib folder. 

### Compiling MITIE using OpenBLAS

If you compile MITIE using cmake then it will automatically find and use any optimized BLAS
libraries on your machine.  However, if you compile using regular make then you have
to manually locate your BLAS libaries or DLIB will default to its built in, but slower, BLAS
implementation.   Therefore, to use OpenBLAS when compiling without cmake, locate `libopenblas.a` and `libgfortran.a`, then
run `make` as follows:
```
cd mitielib 
make BLAS_PATH=/path/to/openblas.a LIBGFORTRAN_PATH=/path/to/libfortran.a
```
Note that if your BLAS libraries are not in standard locations cmake will fail to find them.  However,
you can tell it what folder to look in by replacing `cmake ..` with a statement such as:
```
cmake -DCMAKE_LIBRARY_PATH=/home/me/place/i/put/blas/lib ..
```

### Using MITIE from a Python 2.7 program

Once you have built the MITIE shared library, you can go to the [examples/python](examples/python) folder
and simply run any of the Python scripts.  Each script is a tutorial explaining some aspect of
MITIE: [named entity recognition and relation extraction](examples/python/ner.py), 
[training a custom NER tool](examples/python/train_ner.py), or 
[training a custom relation extractor](examples/python/train_relation_extraction.py).

You can also install ``mitie`` direcly from github with this command:
``pip install git+https://github.com/mit-nlp/MITIE.git``.


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

Also note that you must have Swig 1.3.40 or newer, CMake 2.8.4 or newer, and the Java JDK installed to compile the MITIE interface.  Finally, note that if you are using 64bit Java on Windows then you will need to use a command like:
```
cmake -G "Visual Studio 10 Win64" ..
```
instead of  `cmake ..` so that Visual Studio knows to make a 64bit library.

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

We have built Python 2.7 binaries packaged with sample models for 64bit Linux and Windows (both 32 and 64 bit version of Python).  You can download the precompiled package here: [Precompiled MITIE 0.2](https://github.com/mit-nlp/MITIE/releases/download/v0.4/mitie-v0.2-python-2.7-windows-or-linux64.zip)


# Precompiled Java 64bit binaries

We have built Java binaries for the 64bit JVM which work on Linux and Windows.  You can download the precompiled package here: [Precompiled Java MITIE 0.3](https://github.com/mit-nlp/MITIE/releases/download/v0.4/mitie-java-v0.3-windows64-or-linux64.zip).  In the file is an examples/java folder.  You can run the example by executing the provided .bat or .sh file.

# Citing MITIE

There isn't any paper specifically about MITIE. However, since MITIE is
basically just a thin wrapper around dlib please cite dlib's JMLR paper if you
use MITIE in your research:

```
Davis E. King. Dlib-ml: A Machine Learning Toolkit. Journal of Machine Learning Research 10, pp. 1755-1758, 2009

@Article{dlib09,
  author = {Davis E. King},
  title = {Dlib-ml: A Machine Learning Toolkit},
  journal = {Journal of Machine Learning Research},
  year = {2009},
  volume = {10},
  pages = {1755-1758},
}
```

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

# References

[1] Davis E. King. Dlib-ml: A Machine Learning Toolkit. Journal of Machine Learning Research 10, pp. 1755-1758, 2009.

[2] Paramveer Dhillon, Dean Foster and Lyle Ungar, Eigenwords: Spectral Word Embeddings, Journal of Machine Learning Research (JMLR), 16, 2015.

[3] T. Joachims, T. Finley, Chun-Nam Yu, Cutting-Plane Training of Structural SVMs, Machine Learning, 77(1):27-59, 2009.
