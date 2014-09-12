
mitie_load_entire_file <- function(filename) {
    .Call("mitie_load_entire_file_R", filename, PACKAGE = "MITIE")
}

mitie_tokenize <- function(text) {
    .Call("mitie_tokenize_R", text, PACKAGE = "MITIE")
}

mitie_range_overlaps <- function(arg1, arg2) {
    # --|arg1|----
    # ----|arg2|--
    # ----L------- (rightMostLeft)
    # -------R---- (leftMostRight)
    rightMostLeft = max(arg1$start, arg2$start)
    leftMostRight = min(arg1$end, arg2$end)
    return (rightMostLeft <= leftMostRight)
}

NamedEntityExtractor <- setRefClass("NamedEntityExtractor",
    fields = list(.ner = "externalptr"),
    methods = list(
        initialize = function(filename_or_ptr, ...) {
            if (class(filename_or_ptr) == "externalptr") {
                # construct new object trained extractor already in memory
                .ner <<- filename_or_ptr
            } else {
                .ner <<- .Call("mitie_load_named_entity_extractor_R", filename_or_ptr, PACKAGE = "MITIE")
            }
            callSuper(...)
        },
        get_possible_ner_tags = function() {
            "Returns character vector of tag labels that this extractor can recognize.
            
             This vector can be used to find the label corresponding to a tag index returned
             by extract_entities().
             "
            .Call("mitie_get_tag_name_strings_R", .ner, PACKAGE = "MITIE")
        },
        extract_entities = function(tokens) {
            "Extract named entities from text.
            
             Args:
                 tokens: character vector containing words from a sentence or other piece of text
             
             Returns:
                 List of entities, each of which is a named list containing members:
                     start: index of first token belonging to the entity
                     end: index of last token belonging to the entity
                     tag: index into character vector returned by get_possible_ner_tags()
             "
            .Call("mitie_extract_entities_R", .ner, tokens, PACKAGE = "MITIE")
        },
        create_binary_relation = function(tokens, arg1, arg2) {
            "Converts a raw relation mention pair into an object that can be scored.
            
             Args:
                 tokens: character vector containing words from a piece of text
                 arg1: named list containing members:
                     start: index of argument 1's first token
                     end: index of argument 1's last token
                 arg2: named list containing members:
                     start: index of argument 2's first token
                     end: index of argument 2's last token
             
             Returns:
                 External pointer to a binary relation object that can 
                 be used as an argument to BinaryRelationDetector$score() method.
             "
            # check arguments
            if (arg1$start < 1 | arg1$end > length(tokens) | arg1$end < arg1$start) {
                stop("arg1 has bad range")
            } 
            if (arg2$start < 1 | arg2$end > length(tokens) | arg2$end < arg2$start) {
                stop("arg2 has bad range")
            }
            if (mitie_range_overlaps(arg1, arg2)) {
                stop("arguments overlap")
            }
            .Call("mitie_extract_binary_relation_R", .ner, tokens, arg1, arg2, PACKAGE = "MITIE")
        },
        save_to_disk = function(filename) {
            "Saves entity extractor object to disk.
            
             An object that's been saved to a file can be recalled from 
             disk with the following code: 
                 
                 ner = NamedEntityExtractor$new(filename)
             
             Args:
                 filename: name of file to save object to
             "
            .Call("mitie_save_named_entity_extractor_R", .ner, filename, PACKAGE = "MITIE")
        }
    )
)

BinaryRelationDetector <- setRefClass("BinaryRelationDetector",
    fields = list(.brd = "externalptr"),
    methods = list(
        initialize = function(filename_or_ptr, ...) {
            if (class(filename_or_ptr) == "externalptr") {
                # construct new object trained detector already in memory
                .brd <<- filename_or_ptr
            } else {
                .brd <<- .Call("mitie_load_binary_relation_detector_R", filename_or_ptr, PACKAGE = "MITIE")
            }
            callSuper(...)
        },
        get_relation_name = function() {
            "Returns type of relation that this object can classify."
            .Call("mitie_get_binary_relation_name_R", .brd, PACKAGE = "MITIE")
        },
        score = function(binary_relation) {
            "Classifies a relation object.
            
             Args:
                 binary_relation: an object produced by NamedEntityExtractor$extract_binary_relation()
                 
             Returns:
                 A classification score. If this number is > 0, then the relation detector
                 is indicating that the input relation is a true instance of the type of relation this object detects.
             "
            .Call("mitie_score_binary_relation_R", .brd, binary_relation, PACKAGE = "MITIE")
        },
        save_to_disk = function(filename) {
            "Saves binary relation detector object to disk.
            
             An object that's been saved to a file can be recalled from 
             disk with the following code: 
             
                 brd = BinaryRelationDetector$new(filename)
             
             Args:
                 filename: name of file to save object to
             "
            .Call("mitie_save_binary_relation_detector_R", .brd, filename, PACKAGE = "MITIE")
        }
    )
)

NERTrainingInstance <- setRefClass("NERTrainingInstance",
    fields = list(
        tokens = "character",
        .entities = "list"
    ),
    methods = list(
        initialize = function(...) {
            callSuper(...)
        },
        add_entity = function(start, end, tag) {
            "Adds the given entity to this training instance.
            
             NOTE: The range of this entity must not overlap with previously added entities.
             
             Args:
                 start: index of entity's first token in tokens vector
                 end: index of entity's last token in tokens vector
                 tag: character (string) representing entity's type (e.g., 'PER')
             "
            # check range
            if (start < 1 | end > length(tokens) | end < start) {
                stop("bad range")
            }
            # check doesn't overlap with previous entities
            entity <- list(start = start, end = end, tag = tag)
            for (existing in .entities) {
                if (mitie_range_overlaps(entity, existing)) {
                    stop("new entity overlaps existing entity")
                }
            }
            .entities <<- c(.entities, list(entity))
        }
    )
)

NERTrainer <- setRefClass("NERTrainer",
    fields = list(
        .trainer = "externalptr"
    ),
    methods = list(
        initialize = function(wordrep_filename, ...) {
            .trainer <<- .Call("mitie_create_ner_trainer_R", wordrep_filename, PACKAGE = "MITIE")
            callSuper(...)
        },
        add_training_instance = function(instance) {
            "Adds the given training instance into this object.
            
            It will be used to create a NamedEntityExtractor when train() is called.
            
            Args:
                instance: reference object of class NERTrainingInstance
            "
            .Call("mitie_add_ner_training_instance_R", .trainer, instance$tokens, instance$.entities, PACKAGE = "MITIE")
        },
        get_size = function() {
            "Returns number of training instances that have been added to this object."
            .Call("mitie_ner_trainer_get_size_R", .trainer, PACKAGE = "MITIE")
        },
        get_num_threads = function() {
            "Returns number of threads that will be used to perform training."
            .Call("mitie_ner_trainer_get_num_threads_R", .trainer, PACKAGE = "MITIE")
        },
        set_num_threads = function(num_threads) {
            "Sets number of threads that will be used for training.
             
             This should set be set to the number of processing cores on your computer.
             "
            .Call("mitie_ner_trainer_set_num_threads_R", .trainer, num_threads, PACKAGE = "MITIE")
        },
        get_beta = function() {
            "Returns value of beta parameter (see set_beta)."
            .Call("mitie_ner_trainer_get_beta_R", .trainer, PACKAGE = "MITIE")
        },
        set_beta = function(beta) {
            "Sets value of parameter that controls trade-off between trying to avoid false alarms but also detecting everything.
            
             Different values of beta have the following interpretations:
                 - beta < 1 indicates that you care more about avoiding false alarms
                   than missing detections.  The smaller you make beta the more the
                   trainer will try to avoid false alarms.
                 - beta == 1 indicates that you don't have a preference between avoiding
                   false alarms or not missing detections.  That is, you care about
                   these two things equally.
                 - beta > 1 indicates that care more about not missing detections than
                   avoiding false alarms.
             "
            if (beta < 0) {
                stop(paste("Invalid beta value: ", beta, " (beta can't be negative)"))
            }
            .Call("mitie_ner_trainer_set_beta_R", .trainer, beta, PACKAGE = "MITIE")
        },
        train = function() {
            "Trains a named entity extractor based on the training instances added with add_training_instance().
            
             Returns:
                 Object of class NamedEntityExtractor.
             "
            if (get_size() == 0) {
                stop("Can't call train() on an empty trainer.")
            }
            ner = .Call("mitie_train_named_entity_extractor_R", .trainer, PACKAGE = "MITIE")
            NamedEntityExtractor$new(ner)
        }
    )
)

BinaryRelationDetectorTrainer <- setRefClass("BinaryRelationDetectorTrainer",
    fields = list(
        .trainer = "externalptr"
    ),
    methods = list(
        initialize = function(relation_name, ner, ...) {
            .trainer <<- .Call("mitie_create_binary_relation_trainer_R", relation_name, ner$.ner, PACKAGE = "MITIE")
            callSuper(...)
        },
        add_positive_binary_relation = function(tokens, arg1, arg2) {
            "Adds a positive training instance into the trainer.
            
             This function tells the trainer that the given tokens contain an
             example of the binary relation we are trying to learn.  Moreover,
             the first argument of the relation is specified by arg1 and
             the second argument of the relation specified by arg2.

             NOTE: The ranges of arguments 1 and 2 must not overlap with each other.
             
             Args:
                 tokens: character vector containing words from a piece of text
                 arg1: named list containing members:
                     start: index of first token belonging to arg1
                     end: index of last token belonging to arg1
                 arg2: named list containing members:
                     start: index of first token belonging to arg2
                     end: index of last token belonging to arg2
             "
            # check arguments
            if (arg1$start < 1 | arg1$end > length(tokens) | arg1$end < arg1$start) {
                stop("arg1 has bad range")
            } 
            if (arg2$start < 1 | arg2$end > length(tokens) | arg2$end < arg2$start) {
                stop("arg2 has bad range")
            }
            if (mitie_range_overlaps(arg1, arg2)) {
                stop("arguments overlap")
            }

            .Call("mitie_add_positive_binary_relation_R", .trainer, tokens, arg1, arg2, PACKAGE = "MITIE")
        },
        add_negative_binary_relation = function(tokens, arg1, arg2) {
            "Adds a negative training instance into the trainer.
            
             This function tells the trainer that the given tokens and argument
             combination is not an example of the binary relation we are trying to learn.

             NOTE: The ranges of arguments 1 and 2 must not overlap with each other.
             
             Args:
                 tokens: character vector containing words from a piece of text
                 arg1: named list containing members:
                     start: index of first token belonging to arg1
                     end: index of last token belonging to arg1
                 arg2: named list containing members:
                     start: index of first token belonging to arg2
                     end: index of last token belonging to arg2
             "
            # check arguments
            if (arg1$start < 1 | arg1$end > length(tokens) | arg1$end < arg1$start) {
                stop("arg1 has bad range")
            } 
            if (arg2$start < 1 | arg2$end > length(tokens) | arg2$end < arg2$start) {
                stop("arg2 has bad range")
            }
            if (mitie_range_overlaps(arg1, arg2)) {
                stop("arguments overlap")
            }
             
            .Call("mitie_add_negative_binary_relation_R", .trainer, tokens, arg1, arg2, PACKAGE = "MITIE")
        },
        get_relation_name = function() {
            "Returns type of relation that this object will classify."
            .Call("mitie_binary_relation_trainer_get_relation_name_R", .trainer, PACKAGE = "MITIE")
        },
        get_num_positive_examples = function() {
            "Returns number of positive examples added with add_positive_binary_relation()."
            .Call("mitie_binary_relation_trainer_get_num_positive_examples_R", .trainer, PACKAGE = "MITIE")
        },
        get_num_negative_examples = function() {
            "Returns number of negative examples added with add_negative_binary_relation()."
            .Call("mitie_binary_relation_trainer_get_num_negative_examples_R", .trainer, PACKAGE = "MITIE")
        },
        get_num_threads = function() {
            "Returns number of threads that will be used to perform training."
            .Call("mitie_binary_relation_trainer_get_num_threads_R", .trainer, PACKAGE = "MITIE")
        },
        set_num_threads = function(num_threads) {
            "Sets number of threads that will be used for training.
             
             This should set be set to the number of processing cores on your computer.
             "
            .Call("mitie_binary_relation_trainer_set_num_threads_R", .trainer, num_threads, PACKAGE = "MITIE")
        },
        get_beta = function() {
            "Returns value of beta parameter (see set_beta)."
            .Call("mitie_binary_relation_trainer_get_beta_R", .trainer, PACKAGE = "MITIE")
        },
        set_beta = function(beta) {
            "Sets value of parameter that controls trade-off between trying to avoid false alarms but also detecting everything.
            
             Beta must be non-negative. Different values of beta have the following interpretations:
                 - beta < 1 indicates that you care more about avoiding false alarms
                   than missing detections.  The smaller you make beta the more the
                   trainer will try to avoid false alarms.
                 - beta == 1 indicates that you don't have a preference between avoiding
                   false alarms or not missing detections.  That is, you care about
                   these two things equally.
                 - beta > 1 indicates that care more about not missing detections than
                   avoiding false alarms.
             "
            if (beta < 0) {
                stop(paste("Invalid beta value: ", beta, " (beta can't be negative)"))
            }
            .Call("mitie_binary_relation_trainer_set_beta_R", .trainer, beta, PACKAGE = "MITIE")
        },
        train = function() {
            "Trains a binary relation detector based on the postive and negative training instances.
            
             Returns:
                 Object of class BinaryRelationDetector.
             "
            if (get_num_positive_examples() == 0 | get_num_negative_examples() == 0) {
                stop("You must give both positive and negative training examples before you call train().")
            }
            brd = .Call("mitie_train_binary_relation_detector_R", .trainer, PACKAGE = "MITIE")
            BinaryRelationDetector$new(brd)
        }
    )
)

