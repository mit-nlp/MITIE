import ctypes, os, time

def _last_modified_time(filename):
    if os.path.isfile(filename):
        return os.path.getmtime(filename)
    else:
        return 0


# Load the mitie shared library.  We will look in a few places to see if we can find it.
# What we do depends on our platform
parent = os.path.dirname(os.path.realpath(__file__))
if os.name == 'nt': #if on windows just look in the same folder as the mitie.py file
    _f = ctypes.CDLL(parent+'/mitie')
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

_f.mitie_ner_get_num_detections.restype = ctypes.c_ulong
_f.mitie_ner_get_num_detections.argtypes = ctypes.c_void_p,

_f.mitie_entities_overlap.restype = ctypes.c_int
_f.mitie_entities_overlap.argtypes = ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_ulong


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
        self.__obj = _f.mitie_load_named_entity_extractor(filename)
        self.__mitie_free = _f.mitie_free
        if (self.__obj == None):
            raise Exception("Unable to load named entity extractor from " + filename)

    def __del__(self):
        self.__mitie_free(self.__obj)

    def get_possible_ner_tags(self):
        num = _f.mitie_get_num_possible_ner_tags(self.__obj)
        return [_f.mitie_get_named_entity_tagstr(self.__obj, i) for i in xrange(num)]


    def extract_entities(self, tokens):
        tags = self.get_possible_ner_tags()
        # Now extract the entities and return the results
        dets = _f.mitie_extract_entities(self.__obj, python_to_mitie_str_array(tokens))
        if (dets == None):
            raise Exception("Unable to create entity detections.")
        num = _f.mitie_ner_get_num_detections(dets)
        temp = ([(xrange(_f.mitie_ner_get_detection_position(dets,i),
            _f.mitie_ner_get_detection_position(dets,i)+_f.mitie_ner_get_detection_length(dets,i)),
            tags[_f.mitie_ner_get_detection_tag(dets,i)]
            ) for i in xrange(num)])
        _f.mitie_free(dets)
        return temp

    def _get_windowed_range(self, tokens, arg1, arg2):
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



    def extract_binary_relation(self, tokens, arg1, arg2):
        arg1_start  = min(arg1)
        arg1_length = len(arg1)
        arg2_start  = min(arg2)
        arg2_length = len(arg2)
        if (_f.mitie_entities_overlap(arg1_start, arg1_length, arg2_start, arg2_length) == 1):
            raise Exception("Error, extract_binary_relation() called with overlapping entities: " + arg1 + ", " + arg2)

        # we are going to crop out a window of tokens around the entities
        r = self._get_windowed_range(tokens, arg1, arg2)
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


class binary_relation:
    def __init__(self, obj):
        self.__obj =  obj 
        self.__mitie_free = _f.mitie_free

    def get_obj(self):
        return self.__obj

    def __del__(self):
        self.__mitie_free(self.__obj)


class binary_relation_detector:
    def __init__(self, filename):
        self.__obj = _f.mitie_load_binary_relation_detector(filename)
        self.__mitie_free = _f.mitie_free
        if (self.__obj == None):
            raise Exception("Unable to load binary relation detector from " + filename)

    def __del__(self):
        self.__mitie_free(self.__obj)

    def __str__(self):
        return "binary_relation_detector: " + _f.mitie_binary_relation_detector_name_string(self.__obj)

    def __repr__(self):
        return "<binary_relation_detector: " + _f.mitie_binary_relation_detector_name_string(self.__obj) + ">"

    def get_name_string(self):
        return _f.mitie_binary_relation_detector_name_string(self.__obj)
    
    def __call__(self, relation):
        score = ctypes.c_double()
        if (_f.mitie_classify_binary_relation(self.__obj, relation.get_obj(), ctypes.byref(score)) != 0):
            raise Exception("Unable to classify binary relation.  The detector is incompatible with the NER object used for extraction.")
        return score.value

