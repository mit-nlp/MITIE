import ctypes, os, time, platform

def _last_modified_time(filename):
    if os.path.isfile(filename):
        return os.path.getmtime(filename)
    else:
        return 0


# Load the mitie shared library.  We will look in a few places to see if we can find it.
# What we do depends on our platform
parent = os.path.dirname(os.path.realpath(__file__))
if os.name == 'nt': 
    #if on windows just look in the same folder as the mitie.py file and also in any 
    #subfolders that might have the appropriate 32 or 64 bit dlls, whichever is right for
    #the version of python we are using.
    arch = platform.architecture()
    files = []
    files.append(parent+'/mitie')
    if (arch[0] == "32bit"):
        files.append(parent+'/win32/mitie')
    else:
        files.append(parent+'/win64/mitie')

    times = [(_last_modified_time(f+".dll"),f) for f in files]
    most_recent = max(times, key=lambda x:x[0])[1]
    _f = ctypes.CDLL(most_recent)
else:
    # On UNIX like platforms MITIE might be in any number of places.  Check them all and
    # pick the one with the most recent timestamp.
    files = ([parent+'/libmitie.so', 'libmitie.so', 'libmitie.dylib',
            parent+'/libmitie.dylib', '/usr/local/lib/libmitie.so',
            '/usr/local/lib/libmitie.dylib'])
    times = [(_last_modified_time(f),f) for f in files]
    most_recent = max(times, key=lambda x:x[0])[1]
    _f = ctypes.CDLL(most_recent)
    

    

_f.mitie_free.restype = None
_f.mitie_free.argtypes = ctypes.c_void_p,

_f.mitie_get_named_entity_tagstr.restype = ctypes.c_char_p
_f.mitie_get_named_entity_tagstr.argtypes = ctypes.c_void_p, ctypes.c_ulong

_f.mitie_get_num_possible_ner_tags.restype = ctypes.c_ulong
_f.mitie_get_num_possible_ner_tags.argtypes = ctypes.c_void_p,

_f.mitie_extract_entities.restype = ctypes.c_void_p
_f.mitie_extract_entities.argtypes = ctypes.c_void_p, ctypes.c_void_p

_f.mitie_load_named_entity_extractor.restype = ctypes.c_void_p
_f.mitie_load_named_entity_extractor.argtypes = ctypes.c_char_p,

_f.mitie_load_entire_file.restype = ctypes.c_void_p
_f.mitie_load_entire_file.argtypes = ctypes.c_char_p,

_f.mitie_ner_get_detection_position.restype = ctypes.c_ulong
_f.mitie_ner_get_detection_position.argtypes = ctypes.c_void_p, ctypes.c_ulong

_f.mitie_ner_get_detection_length.restype = ctypes.c_ulong
_f.mitie_ner_get_detection_length.argtypes = ctypes.c_void_p, ctypes.c_ulong

_f.mitie_ner_get_detection_tag.restype = ctypes.c_ulong
_f.mitie_ner_get_detection_tag.argtypes = ctypes.c_void_p, ctypes.c_ulong

_f.mitie_ner_get_detection_score.restype = ctypes.c_double
_f.mitie_ner_get_detection_score.argtypes = ctypes.c_void_p, ctypes.c_ulong

_f.mitie_ner_get_num_detections.restype = ctypes.c_ulong
_f.mitie_ner_get_num_detections.argtypes = ctypes.c_void_p,

_f.mitie_entities_overlap.restype = ctypes.c_int
_f.mitie_entities_overlap.argtypes = ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong

_f.mitie_save_named_entity_extractor.restype = ctypes.c_int
_f.mitie_save_named_entity_extractor.argtypes = ctypes.c_char_p, ctypes.c_void_p

_f.mitie_extract_binary_relation.restype = ctypes.c_void_p
_f.mitie_extract_binary_relation.argtypes = ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong

def _get_windowed_range(tokens, arg1, arg2):
    """returns an xrange that spans a range that includes the arg1 and arg2 ranges
    along with an additional 5 tokens on each side, subject to the constraint that
    the returned xrange does not go outside of tokens, where tokens is a list."""
    winsize = 5
    begin = min(min(arg1), min(arg2))
    end   = max(max(arg1), max(arg2))+1
    if (begin > winsize):
        begin -= winsize 
    else:
        begin = 0
    end = min(end+winsize, len(tokens))
    r = xrange(begin, end)
    return r


def python_to_mitie_str_array(tokens, r = None):
    """Convert from a Python list of strings into MITIE's NULL terminated char** array type.  
    Note that the memory returned by this object is managed by Python and doesn't need to be 
    freed by the user.

    r should be a range that indicates which part of tokens to convert.  If r is not given
    then it defaults to xrange(len(tokens)) which selects the entirety of tokens to convert.
    """
    if (r == None):
        r = xrange(len(tokens))

    ctokens = (ctypes.c_char_p*(len(r)+1))()
    i = 0
    for j in r:
        if (isinstance(tokens[j], tuple)):
            ctokens[i] = tokens[j][0]
        else:
            ctokens[i] = tokens[j]
        i = i + 1
    ctokens[i] = None
    return ctokens

def _range_is_valid (list, range):
    """checks if each element of the range is a valid element of the list and returns True if this is the case."""
    return (0 <= min(range) and max(range) < len(list))

def load_entire_file(filename):
    x = _f.mitie_load_entire_file(filename)
    if (x == None):
        raise Exception("Unable to load file " + filename)
    res = ctypes.string_at(x) 
    _f.mitie_free(x)
    return res

def tokenize(str):
    """ Split str into tokens and return them as a list. """
    mitie_tokenize = _f.mitie_tokenize
    mitie_tokenize.restype = ctypes.POINTER(ctypes.c_char_p)
    mitie_tokenize.argtypes = ctypes.c_char_p,
    tok = mitie_tokenize(str)
    if (tok == None):
        raise Exception("Unable to tokenize string.")
    i = 0
    res = []
    while(tok[i] != None):
        res.append(tok[i])
        i = i + 1
    _f.mitie_free(tok)
    return res

def tokenize_with_offsets(str):
    """ Split str into tokens and return them as a list.  Also, each element of the list
    contains a tuple of the token text and the byte offset which indicates the position of the
    first character in the token within the input str. """
    mitie_tokenize = _f.mitie_tokenize_with_offsets
    mitie_tokenize.restype = ctypes.POINTER(ctypes.c_char_p)
    mitie_tokenize.argtypes = ctypes.c_char_p, ctypes.POINTER(ctypes.POINTER(ctypes.c_ulong))
    token_offsets = ctypes.POINTER(ctypes.c_ulong)()
    tok = mitie_tokenize(str, ctypes.byref(token_offsets))
    if (tok == None):
        raise Exception("Unable to tokenize string.")
    i = 0
    res = []
    while(tok[i] != None):
        res.append((tok[i], token_offsets[i]))
        i = i + 1
    _f.mitie_free(tok)
    _f.mitie_free(token_offsets)
    return res


class named_entity_extractor:
    def __init__(self, filename):
        self.__mitie_free = _f.mitie_free
        if (isinstance(filename, ctypes.c_void_p)):
            # If we get here then it means we are using the "private" constructor used by
            # the training tools to create a named_entity_extractor.  In this case,
            # filename is a pointer to a ner object.
            self.__obj = filename
        else:
            self.__obj = _f.mitie_load_named_entity_extractor(filename)
        if (self.__obj == None):
            raise Exception("Unable to load named entity extractor from " + filename)

    def __del__(self):
        self.__mitie_free(self.__obj)

    @property
    def _obj(self):
        return self.__obj

    def get_possible_ner_tags(self):
        num = _f.mitie_get_num_possible_ner_tags(self.__obj)
        return [_f.mitie_get_named_entity_tagstr(self.__obj, i) for i in xrange(num)]

    def save_to_disk(self, filename):
        """Save this object to disk.  You recall it from disk with the following Python
        code: 
            ner = named_entity_extractor(filename)"""
        if (_f.mitie_save_named_entity_extractor(filename, self.__obj) != 0):
            raise Exception("Unable to save named_entity_extractor to the file " + filename);

    def extract_entities(self, tokens):
        tags = self.get_possible_ner_tags()
        # Now extract the entities and return the results
        dets = _f.mitie_extract_entities(self.__obj, python_to_mitie_str_array(tokens))
        if (dets == None):
            raise Exception("Unable to create entity detections.")
        num = _f.mitie_ner_get_num_detections(dets)
        temp = ([(xrange(_f.mitie_ner_get_detection_position(dets,i),
            _f.mitie_ner_get_detection_position(dets,i)+_f.mitie_ner_get_detection_length(dets,i)),
            tags[_f.mitie_ner_get_detection_tag(dets,i)],
            _f.mitie_ner_get_detection_score(dets,i)
            ) for i in xrange(num)])
        _f.mitie_free(dets)
        return temp

    def extract_binary_relation(self, tokens, arg1, arg2):
        """
        requires
            - arg1 and arg2 are range objects and they don't go outside the
              range xrange(len(tokens)).
            - arg1 and arg2 do not overlap
        ensures
            - returns a processed binary relation that describes the relation
              given by the two relation argument positions arg1 and arg2.  You
              can pass the returned object to a binary_relation_detector to see
              if it is an instance of a known relation type."""
        arg1_start  = min(arg1)
        arg1_length = len(arg1)
        arg2_start  = min(arg2)
        arg2_length = len(arg2)
        if (_f.mitie_entities_overlap(arg1_start, arg1_length, arg2_start, arg2_length) == 1):
            raise Exception("Error, extract_binary_relation() called with overlapping entities: " + arg1 + ", " + arg2)

        # we are going to crop out a window of tokens around the entities
        r = _get_windowed_range(tokens, arg1, arg2)
        arg1_start -= min(r)
        arg2_start -= min(r)
        ctokens = python_to_mitie_str_array(tokens, r)
        rel = _f.mitie_extract_binary_relation(self.__obj, ctokens, arg1_start, arg1_length, arg2_start, arg2_length)
        if (rel == None):
            raise Exception("Unable to create binary relation.")
        return binary_relation(rel)



####################################################################################################

_f.mitie_load_binary_relation_detector.restype = ctypes.c_void_p
_f.mitie_load_binary_relation_detector.argtypes = ctypes.c_char_p, 

_f.mitie_binary_relation_detector_name_string.restype = ctypes.c_char_p
_f.mitie_binary_relation_detector_name_string.argtypes = ctypes.c_void_p, 

_f.mitie_classify_binary_relation.restype = ctypes.c_int
_f.mitie_classify_binary_relation.argtypes = ctypes.c_void_p, ctypes.c_void_p, ctypes.POINTER(ctypes.c_double)

_f.mitie_save_binary_relation_detector.restype = ctypes.c_int
_f.mitie_save_binary_relation_detector.argtypes = ctypes.c_char_p, ctypes.c_void_p


class binary_relation:
    def __init__(self, obj):
        self.__obj =  obj 
        self.__mitie_free = _f.mitie_free

    @property
    def _obj(self):
        return self.__obj

    def __del__(self):
        self.__mitie_free(self.__obj)


class binary_relation_detector:
    def __init__(self, filename):
        self.__mitie_free = _f.mitie_free
        if (isinstance(filename, ctypes.c_void_p)):
            # If we get here then it means we are using the "private" constructor used by
            # the training tools to create a binary_relation_detector.  In this case,
            # filename is a pointer to a ner object.
            self.__obj = filename
        else:
            self.__obj = _f.mitie_load_binary_relation_detector(filename)
        if (self.__obj == None):
            raise Exception("Unable to load binary relation detector from " + filename)

    def __del__(self):
        self.__mitie_free(self.__obj)

    def save_to_disk(self, filename):
        """Save this object to disk.  You recall it from disk with the following Python
        code: 
            ner = binary_relation_detector(filename)"""
        if (_f.mitie_save_binary_relation_detector(filename, self.__obj) != 0):
            raise Exception("Unable to save binary_relation_detector to the file " + filename);

    def __str__(self):
        return "binary_relation_detector: " + _f.mitie_binary_relation_detector_name_string(self.__obj)

    def __repr__(self):
        return "<binary_relation_detector: " + _f.mitie_binary_relation_detector_name_string(self.__obj) + ">"

    @property
    def name_string(self):
        return _f.mitie_binary_relation_detector_name_string(self.__obj)
    
    def __call__(self, relation):
        """Classify a relation object.  The input should have been produced by 
        named_entity_extractor.extract_binary_relation().  This function returns a classification score
        and if this number is > 0 then the relation detector is indicating that the input relation
        is a true instance of the type of relation this object detects."""
        score = ctypes.c_double()
        if (_f.mitie_classify_binary_relation(self.__obj, relation._obj, ctypes.byref(score)) != 0):
            raise Exception("Unable to classify binary relation.  The detector is incompatible with the NER object used for extraction.")
        return score.value

##############################################################################
####                          TRAINING API                                 ###
##############################################################################

_f.mitie_add_ner_training_entity.restype = ctypes.c_int
_f.mitie_add_ner_training_entity.argtypes = ctypes.c_void_p, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_char_p

_f.mitie_add_ner_training_instance.restype = ctypes.c_int
_f.mitie_add_ner_training_instance.argtypes = ctypes.c_void_p, ctypes.c_void_p 

_f.mitie_create_ner_trainer.restype = ctypes.c_void_p
_f.mitie_create_ner_trainer.argtypes = ctypes.c_char_p,

_f.mitie_create_ner_training_instance.restype = ctypes.c_void_p
_f.mitie_create_ner_training_instance.argtypes = ctypes.c_void_p,

_f.mitie_ner_trainer_get_beta.restype = ctypes.c_double
_f.mitie_ner_trainer_get_beta.argtypes = ctypes.c_void_p,

_f.mitie_ner_trainer_get_num_threads.restype = ctypes.c_ulong
_f.mitie_ner_trainer_get_num_threads.argtypes = ctypes.c_void_p,

_f.mitie_ner_trainer_set_beta.restype = None
_f.mitie_ner_trainer_set_beta.argtypes = ctypes.c_void_p, ctypes.c_double

_f.mitie_ner_trainer_set_num_threads.restype = None
_f.mitie_ner_trainer_set_num_threads.argtypes = ctypes.c_void_p, ctypes.c_ulong

_f.mitie_ner_trainer_size.restype = ctypes.c_ulong
_f.mitie_ner_trainer_size.argtypes = ctypes.c_void_p,

_f.mitie_ner_training_instance_num_entities.restype = ctypes.c_ulong
_f.mitie_ner_training_instance_num_entities.argtypes = ctypes.c_void_p,

_f.mitie_ner_training_instance_num_tokens.restype = ctypes.c_ulong
_f.mitie_ner_training_instance_num_tokens.argtypes = ctypes.c_void_p,

_f.mitie_overlaps_any_entity.restype = ctypes.c_int
_f.mitie_overlaps_any_entity.argtypes = ctypes.c_void_p, ctypes.c_ulong, ctypes.c_ulong

_f.mitie_train_named_entity_extractor.restype = ctypes.c_void_p
_f.mitie_train_named_entity_extractor.argtypes = ctypes.c_void_p,

class ner_training_instance:
    def __init__(self, tokens):
        self.__obj = _f.mitie_create_ner_training_instance(python_to_mitie_str_array(tokens))
        self.__mitie_free = _f.mitie_free
        if (self.__obj == None):
            raise Exception("Unable to create ner_training_instance.  Probably ran out of RAM.")

    def __del__(self):
        self.__mitie_free(self.__obj)

    @property
    def _obj(self):
        return self.__obj

    @property
    def num_tokens(self):
        return _f.mitie_ner_training_instance_num_tokens(self.__obj)

    @property
    def num_entities(self):
        return _f.mitie_ner_training_instance_num_entities(self.__obj)

    def overlaps_any_entity(self, range):
        """Takes a xrange and reports if the range overlaps any entities already in this object."""
        if (len(range) == 0 or max(range) >= self.num_tokens):
            raise Exception("Invalid range given to ner_training_instance.overlaps_any_entity()")
        return _f.mitie_overlaps_any_entity(self.__obj, min(range), len(range)) == 1

    def add_entity(self, range, label):
        if (len(range) == 0 or max(range) >= self.num_tokens or min(range) < 0):
            raise Exception("Invalid range given to ner_training_instance.overlaps_any_entity()")
        if (self.overlaps_any_entity(range)):
            raise Exception("Invalid range given to ner_training_instance.overlaps_any_entity().  It overlaps an entity given to a previous call to add_entity().")
        if (_f.mitie_add_ner_training_entity(self.__obj, min(range), len(range), label) != 0):
            raise Exception("Unable to add entity to training instance.  Probably ran out of RAM.");
        

class ner_trainer(object):
    def __init__(self, filename):
        self.__obj = _f.mitie_create_ner_trainer(filename)
        self.__mitie_free = _f.mitie_free
        if (self.__obj == None):
            raise Exception("Unable to create ner_trainer based on " + filename)

    def __del__(self):
        self.__mitie_free(self.__obj)
    
    @property
    def size(self):
        return _f.mitie_ner_trainer_size(self.__obj)

    def add(self, instance):
        if (_f.mitie_add_ner_training_instance(self.__obj, instance._obj) != 0):
            raise Exception("Unable to add training instance to ner_trainer.  Probably ran out of RAM.");

    @property
    def beta(self):
        return _f.mitie_ner_trainer_get_beta(self.__obj)

    @beta.setter
    def beta(self, value):
        if (value < 0):
            raise Exception("Invalid beta value given.  beta can't be negative.")
        _f.mitie_ner_trainer_set_beta(self.__obj, value)
    
    @property
    def num_threads(self):
        return _f.mitie_ner_trainer_get_num_threads(self.__obj)

    @num_threads.setter
    def num_threads(self, value):
        _f.mitie_ner_trainer_set_num_threads(self.__obj, value)
    
    def train(self):
        if (self.size == 0):
            raise Exception("You can't call train() on an empty trainer.")
        # Make the type be a c_void_p so the named_entity_extractor constructor will know what to do.
        obj = ctypes.c_void_p(_f.mitie_train_named_entity_extractor(self.__obj))
        if (obj == None):
            raise Exception("Unable to create named_entity_extractor.  Probably ran out of RAM")
        return named_entity_extractor(obj)


##############################################################################

_f.mitie_create_binary_relation_trainer.restype = ctypes.c_void_p
_f.mitie_create_binary_relation_trainer.argtypes = ctypes.c_char_p, ctypes.c_void_p

_f.mitie_binary_relation_trainer_num_positive_examples.restype = ctypes.c_ulong
_f.mitie_binary_relation_trainer_num_positive_examples.argtypes = ctypes.c_void_p,

_f.mitie_binary_relation_trainer_num_negative_examples.restype = ctypes.c_ulong
_f.mitie_binary_relation_trainer_num_negative_examples.argtypes = ctypes.c_void_p,

_f.mitie_add_positive_binary_relation.restype = ctypes.c_int
_f.mitie_add_positive_binary_relation.argtypes = ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong

_f.mitie_add_negative_binary_relation.restype = ctypes.c_int
_f.mitie_add_negative_binary_relation.argtypes = ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong

_f.mitie_binary_relation_trainer_get_beta.restype = ctypes.c_double
_f.mitie_binary_relation_trainer_get_beta.argtypes = ctypes.c_void_p,

_f.mitie_binary_relation_trainer_get_num_threads.restype = ctypes.c_ulong
_f.mitie_binary_relation_trainer_get_num_threads.argtypes = ctypes.c_void_p,

_f.mitie_binary_relation_trainer_set_beta.restype = None
_f.mitie_binary_relation_trainer_set_beta.argtypes = ctypes.c_void_p, ctypes.c_double

_f.mitie_binary_relation_trainer_set_num_threads.restype = None
_f.mitie_binary_relation_trainer_set_num_threads.argtypes = ctypes.c_void_p, ctypes.c_ulong

_f.mitie_train_binary_relation_detector.restype = ctypes.c_void_p
_f.mitie_train_binary_relation_detector.argtypes = ctypes.c_void_p,



class binary_relation_detector_trainer(object):
    def __init__(self, relation_name, ner):
        self.__obj = _f.mitie_create_binary_relation_trainer(relation_name, ner._obj)
        self.__mitie_free = _f.mitie_free
        if (self.__obj == None):
            raise Exception("Unable to create binary_relation_detector_trainer")

    def __del__(self):
        self.__mitie_free(self.__obj)

    @property
    def num_positive_examples(self):
        return _f.mitie_binary_relation_trainer_num_positive_examples(self.__obj)

    @property
    def num_negative_examples(self):
        return _f.mitie_binary_relation_trainer_num_negative_examples(self.__obj)

    def add_positive_binary_relation(self, tokens, arg1, arg2):
        if (len(arg1) == 0 or len(arg2) == 0 or not _range_is_valid(tokens,arg1) or not _range_is_valid(tokens,arg2)):
            raise Exception("One of the ranges given to this function was invalid.")
        arg1_start  = min(arg1)
        arg1_length = len(arg1)
        arg2_start  = min(arg2)
        arg2_length = len(arg2)
        if (_f.mitie_entities_overlap(arg1_start, arg1_length, arg2_start, arg2_length) == 1):
            raise Exception("Error, add_positive_binary_relation() called with overlapping entities: " + arg1 + ", " + arg2)
        r = _get_windowed_range(tokens, arg1, arg2)
        arg1_start -= min(r)
        arg2_start -= min(r)
        ctokens = python_to_mitie_str_array(tokens, r)
        if (_f.mitie_add_positive_binary_relation(self.__obj, ctokens, arg1_start, arg1_length, arg2_start, arg2_length) != 0):
            raise Exception("Unable to add positive binary relation to binary_relation_detector_trainer.")

    def add_negative_binary_relation(self, tokens, arg1, arg2):
        if (len(arg1) == 0 or len(arg2) == 0 or not _range_is_valid(tokens,arg1) or not _range_is_valid(tokens,arg2)):
            raise Exception("One of the ranges given to this function was invalid.")
        arg1_start  = min(arg1)
        arg1_length = len(arg1)
        arg2_start  = min(arg2)
        arg2_length = len(arg2)
        if (_f.mitie_entities_overlap(arg1_start, arg1_length, arg2_start, arg2_length) == 1):
            raise Exception("Error, add_negative_binary_relation() called with overlapping entities: " + arg1 + ", " + arg2)
        r = _get_windowed_range(tokens, arg1, arg2)
        arg1_start -= min(r)
        arg2_start -= min(r)
        ctokens = python_to_mitie_str_array(tokens, r)
        if (_f.mitie_add_negative_binary_relation(self.__obj, ctokens, arg1_start, arg1_length, arg2_start, arg2_length) != 0):
            raise Exception("Unable to add negative binary relation to binary_relation_detector_trainer.")

    @property
    def beta(self):
        return _f.mitie_binary_relation_trainer_get_beta(self.__obj)

    @beta.setter
    def beta(self, value):
        if (value < 0):
            raise Exception("Invalid beta value given.  beta can't be negative.")
        _f.mitie_binary_relation_trainer_set_beta(self.__obj, value)
    
    @property
    def num_threads(self):
        return _f.mitie_binary_relation_trainer_get_num_threads(self.__obj)

    @num_threads.setter
    def num_threads(self, value):
        _f.mitie_binary_relation_trainer_set_num_threads(self.__obj, value)
    
    def train(self):
        if (self.num_positive_examples == 0 or self.num_negative_examples == 0):
            raise Exception("You must give both positive and negative training examples before you call train().")
        # Make the type be a c_void_p so the binary_relation_detector constructor will know what to do.
        obj = ctypes.c_void_p(_f.mitie_train_binary_relation_detector(self.__obj))
        if (obj == None):
            raise Exception("Unable to create binary_relation_detector.  Probably ran out of RAM")
        return binary_relation_detector(obj)


