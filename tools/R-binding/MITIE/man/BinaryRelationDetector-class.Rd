\name{BinaryRelationDetector-class}
\Rdversion{1.1}
\docType{class}
\alias{BinaryRelationDetector-class}
\alias{BinaryRelationDetector}

\title{Class \code{"BinaryRelationDetector"}}
\description{
Tool for detecting relationships between entities such as person / place_of_birth.
}
\section{Extends}{

All reference classes extend and inherit methods from \code{"\linkS4class{envRefClass}"}.

}

\examples{
\dontrun{
# Get detailed help for reference class methods

BinaryRelationDetector$methods()
BinaryRelationDetector$help(get_relation_name)
BinaryRelationDetector$help(save_to_disk)
BinaryRelationDetector$help(score)

# Load named entity extractor from disk

ner_model_path <- "/path/MITIE-models/english/ner_model.dat"
ner <- NamedEntityExtractor$new(ner_model_path)
tag_names <- ner$get_possible_ner_tags()

# Tokenize sample text

sample_text <- "
A Pegasus Airlines plane landed at an Istanbul airport Friday after a 
passenger \"said that there was a bomb on board\" and wanted the plane to 
land in Sochi, Russia, the site of the Winter Olympics, said officials 
with Turkey's Transportation Ministry.

Meredith Vieira will become the first woman to host Olympics primetime 
coverage on her own when she fills on Friday night for the ailing Bob 
Costas, who is battling a continuing eye infection.  \"It's an honor to 
fill in for him,\" Vieira said on TODAY Friday. \"You think about the 
Olympics, and you think the athletes and then Bob Costas.\" \"Bob's eye 
issue has improved but he's not quite ready to do the show,\" NBC 
Olympics Executive Producer Jim Bell told TODAY.com from Sochi on 
Thursday.

From wikipedia we learn that Josiah Franklin's son, Benjamin Franklin 
was born in Boston.  Since wikipedia allows anyone to edit it, you could 
change the entry to say that Philadelphia is the birthplace of Benjamin 
Franklin.  However, that would be a bad edit since Benjamin Franklin was 
definitely born in Boston.
"

tokens <- mitie_tokenize(sample_text)

# Extract entities

entities <- ner$extract_entities(tokens)
for (i in 1:length(entities)) {
    entity <- entities[[i]]
    position <- paste("(", entity$start, ",", entity$end, ")", sep="")
    text <- paste(tokens[entity$start:entity$end], collapse=" ")
    print(paste(text, "/", tag_names[entity$tag], "@", position))
}
# [1] "Pegasus Airlines / ORGANIZATION @ (2,3)"
# [1] "Istanbul / LOCATION @ (8,8)"
# [1] "Sochi / LOCATION @ (31,31)"
# [1] "Russia / LOCATION @ (33,33)"
# [1] "Turkey / LOCATION @ (45,45)"
# [1] "Transportation Ministry / ORGANIZATION @ (47,48)"
# [1] "Meredith Vieira / PERSON @ (50,51)"
# [1] "Olympics / MISC @ (59,59)"
# [1] "Bob Costas / PERSON @ (74,75)"
# [1] "Vieira / PERSON @ (97,97)"
# [1] "Olympics / MISC @ (108,108)"
# [1] "Bob Costas / PERSON @ (117,118)"
# [1] "Bob / PERSON @ (122,122)"
# [1] "NBC Olympics / ORGANIZATION @ (140,141)"
# [1] "Jim Bell / PERSON @ (144,145)"
# [1] "Sochi / LOCATION @ (151,151)"
# [1] "Josiah Franklin / PERSON @ (160,161)"
# [1] "Benjamin Franklin / PERSON @ (165,166)"
# [1] "Boston / LOCATION @ (170,170)"
# [1] "Philadelphia / LOCATION @ (188,188)"
# [1] "Benjamin Franklin / PERSON @ (193,194)"
# [1] "Benjamin Franklin / PERSON @ (205,206)"
# [1] "Boston / LOCATION @ (211,211)"

# Load "person born in place" binary relation detector from disk
# Note: relation detector models can be downloaded from http://sourceforge.net/projects/mitie/files/

brd_model_path <- "/path/MITIE-models/english/binary_relations/rel_classifier_people.person.place_of_birth.svm"
brd <- BinaryRelationDetector$new(brd_model_path)
brd$get_relation_name()
# [1] "people.person.place_of_birth"

# Make a list of neighboring entities. Once we have this list we will ask the
# relation detector if any of these entity pairs is an example of the "person 
# born in place" relation. 

# NOTE: We also swap neighboring entities because "person born in place" 
# mentions can appear in the text in as "place is birthplace of person". So we 
# must consider both possible orderings of the arguments.

neighbors = list()
for (i in 1:(length(entities) - 1)) {
    neighbors[[length(neighbors) + 1]] <- list(i, i + 1)
    neighbors[[length(neighbors) + 1]] <- list(i + 1, i)
}

# Check each entity pair and see which ones the detector selects.

for (pair in neighbors) {
    person <- entities[[ pair[[1]] ]]
    place <- entities[[ pair[[2]] ]]
    
    # Detection has two steps in MITIE. First, you convert a pair of entities
    # into a special representation.
    rel <- ner$create_binary_relation(tokens, person, place)
    
    # Then you ask the detector to classify that pair of entities.  If the
    # score value is > 0 then it is saying that it has found a relation.  The
    # larger the score the more confident it is.  Finally, the reason we do
    # detection in two parts is so you can reuse the intermediate rel in many
    # calls to different relation detectors without needing to redo the
    # processing done in NamedEntityExtractor$create_binary_relation().
    score = brd$score(rel)
    
    # Print out any matching relations.
    if (score > 0) {
        person_text <- paste(tokens[person$start:person$end], collapse=" ")
        place_text <- paste(tokens[place$start:place$end], collapse=" ")
        print(paste(person_text, "BORN IN", place_text))
    }
}
# [1] "Benjamin Franklin BORN IN Boston"
# [1] "Benjamin Franklin BORN IN Philadelphia"
# [1] "Benjamin Franklin BORN IN Boston"

# The code above shows the basic details of MITIE's relation detection API.
# However, it is important to note that real world data is noisy any confusing.
# Not all detected relations will be correct. Therefore, it's important to
# aggregate many relation detections together to get the best signal out of
# your data. A good way to do this is to pick an entity you are in interested
# in (e.g. Benjamin Franklin) and then find all the relations that mention him
# and order them by most frequent to least frequent. We show how to do this in
# the code below.

query <- "Benjamin Franklin"
hits <- list()
for (pair in neighbors) {
    person <- entities[[ pair[[1]] ]]
    place <- entities[[ pair[[2]] ]]
    rel <- ner$create_binary_relation(tokens, person, place)
    score <- brd$score(rel)
    if (score > 0) {
        person_text <- paste(tokens[person$start:person$end], collapse=" ")
        place_text <- paste(tokens[place$start:place$end], collapse=" ")
        if (person_text == query) {
            if (is.null(hits[[place_text]])) {
                hits[[place_text]] <- 0
            }
            hits[[place_text]] <- hits[[place_text]] + 1
        }
    }    
}

# Convert hits to data frame, sort, and show top hits

places <- names(hits)
counts <- numeric(length(places))
for (i in 1:length(places)) {
    counts[i] <- hits[[ places[i] ]]
}

df = data.frame(places, counts)
print("Top most common relations:")
df[order(-counts)]
#         places counts
# 1       Boston      2
# 2 Philadelphia      1
}
}
\keyword{classes}
\section{Fields}{
  \describe{
    \item{\code{.brd}:}{Object of class \code{externalptr} pointer to binary relation detector C++ object. }
  }
}
\section{Methods}{
  \describe{
    \item{\code{get_relation_name()}:}{ Returns type of relation that this object can classify. }
    \item{\code{save_to_disk(filename)}:}{ Saves binary relation detector object to disk. }
    \item{\code{score(binary_relation)}:}{ Classifies a relation object. }
%%    \item{\code{initialize(filename, ...)}:}{ Construct new \code{BinaryRelationDetector} object from saved model (using \code{BinaryRelationDetector$new(filename)}). }
  }
}
