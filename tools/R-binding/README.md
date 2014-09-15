MITIE R Package
=====

MITIE's functionality can be used in the [R](http://www.r-project.org/) statistical computing environment.

Linux users can download (or build) the [MITIE source package](http://sourceforge.net/projects/mitie/files/R-package/MITIE_0.2.tar.gz), and then install the package into R from source.

Windows users should download the [pre-built binary package](http://sourceforge.net/projects/mitie/files/R-package/MITIE_0.2.zip) and install it through an R graphical user interface.

### Building the Source Package

The [MITIE source package](http://sourceforge.net/projects/mitie/files/R-package/MITIE_0.2.tar.gz) is available for download. However, it can be packaged with the following steps.

Starting in the top level MITIE folder, run the commands:
```
cd tools/R-binding
./copy_source.sh
./build_source_package.sh
```
This will create a `MITIE_{version}.tar.gz` source package.

### Installing the MITIE Source Package (Linux/Mac)

Note: The following requires the R development tools to be installed, e.g., the `r-base-dev` package on ubuntu.

Step 1: Install MITIE dependencies.

MITIE depends on the [Rcpp](http://www.rcpp.org/) package for integrating C++ with R. Rcpp can be installed using a GUI package manager or using the R command:
```
install.packages("Rcpp")
```

Step 2: Build or download latest [MITIE source package](http://sourceforge.net/projects/mitie/files/R-package/MITIE_0.2.tar.gz).

Step 3: From the linux command line, run:
```
R CMD INSTALL MITIE_{version}.tar.gz
```

### Installing the MITIE Binary Package (Windows)

The MITIE package can be installed from source on Windows if [Rtools](http://cran.r-project.org/bin/windows/Rtools/) is installed.
However, pre-compiled Windows binaries are also available (and more convenient).

Step 1: Install MITIE dependencies.

MITIE depends on the [Rcpp](http://www.rcpp.org/) package for integrating C++ with R. Rcpp can be installed using a GUI package manager or using the R command:
```
install.packages("Rcpp")
```

Step 2: Download the latest [Windows binary package](http://sourceforge.net/projects/mitie/files/R-package/MITIE_0.2.zip), which was built with R version 3.1. 

Step 3: Use GUI's package manager to install package from local zip file. For example, in RGui, select "Packages" / "Install package(s) from local zip files..." and navigate to MITIE_{version}.zip to install the package.

### Using MITIE from R

MITIE requires trained model files to do named entity extraction, binary relation extraction, etc. Models for English and Spanish are currently available:
[MITIE-models-v0.2.tar.bz2](http://sourceforge.net/projects/mitie/files/binaries/MITIE-models-v0.2.tar.bz2) and
[MITIE-models-v0.2-Spanish.zip](http://sourceforge.net/projects/mitie/files/binaries/MITIE-models-v0.2-Spanish.zip).

The following R session demonstrates how to perform named entity extraction.
```
library(MITIE)
help(MITIE)

# Load named entity extractor from disk
# NOTE: models can be downloaded from http://sourceforge.net/projects/mitie/files/binaries/
# NOTE: change this path to point to where your model files are

ner_model_path <- "C:/MITIE-models/english/ner_model.dat"
ner <- NamedEntityExtractor$new(ner_model_path)

# Print out what kind of tags this tagger can predict

tag_names <- ner$get_possible_ner_tags()
tag_names

#  [1] "PERSON"       "LOCATION"     "ORGANIZATION" "MISC"

# Prepare some data

tokens <- mitie_tokenize("Bill Gates was born in Seattle, Washington. Bill used to be the CEO of Microsoft.")
tokens

#  [1] "Bill"       "Gates"      "was"        "born"       "in"        
#  [6] "Seattle"    ","          "Washington" "."          "Bill"      
# [11] "used"       "to"         "be"         "the"        "CEO"       
# [16] "of"         "Microsoft"  "."  

# Extract entities

entities <- ner$extract_entities(tokens)
for (i in 1:length(entities)) {
    entity = entities[[i]]
    position = paste("(", entity$start, ",", entity$end, ")", sep="")
    text = paste(tokens[entity$start:entity$end], collapse=" ")
    print(paste(text, "/", tag_names[entity$tag], "@", position))
}

# [1] "Bill Gates / PERSON @ (1,2)"
# [1] "Seattle / LOCATION @ (6,6)"
# [1] "Washington / LOCATION @ (8,8)"
# [1] "Bill / PERSON @ (10,10)"
# [1] "Microsoft / ORGANIZATION @ (17,17)"

```


