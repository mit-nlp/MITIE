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

_f.mitie_ner_get_detection_position.restype = ctypes.c_void_p
_f.mitie_ner_get_detection_position.argtypes = ctypes.c_void_p, ctypes.c_ulong

_f.mitie_ner_get_detection_length.restype = ctypes.c_void_p
_f.mitie_ner_get_detection_length.argtypes = ctypes.c_void_p, ctypes.c_ulong

_f.mitie_ner_get_detection_tag.restype = ctypes.c_ulong
_f.mitie_ner_get_detection_tag.argtypes = ctypes.c_void_p, ctypes.c_ulong

_f.mitie_ner_get_num_detections.restype = ctypes.c_ulong
_f.mitie_ner_get_num_detections.argtypes = ctypes.c_void_p,

def load_entire_file(filename):
    x = _f.mitie_load_entire_file(filename)
    res = ctypes.string_at(x) 
    _f.mitie_free(x)
    return res

def tokenize(str):
    mitie_tokenize = _f.mitie_tokenize
    mitie_tokenize.restype = ctypes.POINTER(ctypes.c_char_p)
    tok = mitie_tokenize(str)
    i = 0
    res = []
    while(tok[i] != None):
        res.append(tok[i])
        i = i + 1
    _f.mitie_free(tok)
    return res


class named_entity_extractor:
    def __init__(self, filename):
        self.__obj = _f.mitie_load_named_entity_extractor(filename)
        self.__mitie_free = _f.mitie_free

    def __del__(self):
        self.__mitie_free(self.__obj)

    def get_possible_ner_tags(self):
        num = _f.mitie_get_num_possible_ner_tags(self.__obj)
        return [_f.mitie_get_named_entity_tagstr(self.__obj, i) for i in xrange(num)]


    def extract_entities(self, tokens):
        # convert the python style token array into one we can pass to the C API
        ctokens = (ctypes.c_char_p*(len(tokens)+1))()
        i = 0
        for str in tokens:
            ctokens[i] = str
            i = i + 1
        ctokens[i] = None

        tags = self.get_possible_ner_tags()
        # Now extract the entities and return the results
        dets = _f.mitie_extract_entities(self.__obj, ctokens)
        num = _f.mitie_ner_get_num_detections(dets)
        return ([(xrange(_f.mitie_ner_get_detection_position(dets,i),
            _f.mitie_ner_get_detection_position(dets,i)+_f.mitie_ner_get_detection_length(dets,i)),
            tags[_f.mitie_ner_get_detection_tag(dets,i)]
            ) for i in xrange(num)])
        _f.mitie_free(dets)

