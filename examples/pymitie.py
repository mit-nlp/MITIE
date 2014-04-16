"""
This is a Python wrapper around MITie. It's a minimal implementation that does one thing only -- 
take a Python string and return a list of tagged tokens. Not much but it's a start. 

Consideration: MITie models are large and take a while to load. I load them at module-loading time, and suggest that
a reference to the pymitie module be made global in an application. Speeds things up a lot!

Requirements:
* Build mitielib first and put the .so file somewhere on the path. 
* Install libffi
    apt-get install libffi-devel
* Install cffi
    pip install cffi

**NOTE: This only works on Linux for now. MacOS throws strange errors that I haven't figured out yet. HELP?**

Usage:

  from pymitie import tag
  
  text = "Jack and Jill went up the hill"
  tag(text)
  
  [(Jack, PERSON), (Jill, PERSON)]
  
** I'll be working on this wrapper for the next few weeks -- will appreciate any help! **

"""


from cffi import FFI


ffi = FFI()

## Definitions copied from <mitie.h> file and stripped to the minimum. CFFI parser is finicky, no fancy stuff!
ffi.cdef("""typedef struct mitie_named_entity_extractor  mitie_named_entity_extractor;
	typedef struct mitie_named_entity_detections mitie_named_entity_detections;
	char** mitie_tokenize (
        const char* text
    );
	mitie_named_entity_extractor* mitie_load_named_entity_extractor (const char* filename);
	unsigned long mitie_get_num_possible_ner_tags (const mitie_named_entity_extractor* ner);
    const char* mitie_get_named_entity_tagstr (const mitie_named_entity_extractor* ner, unsigned long idx);
    mitie_named_entity_detections* mitie_extract_entities (const mitie_named_entity_extractor* ner, char** tokens );
    unsigned long mitie_ner_get_num_detections (const mitie_named_entity_detections* dets);
    unsigned long mitie_ner_get_detection_position (const mitie_named_entity_detections* dets, unsigned long idx);
    unsigned long mitie_ner_get_detection_length (const mitie_named_entity_detections* dets, unsigned long idx);
    unsigned long mitie_ner_get_detection_tag ( const mitie_named_entity_detections* dets, unsigned long idx);
    const char* mitie_ner_get_detection_tagstr (const mitie_named_entity_detections* dets, unsigned long idx);
	""",override=True)

C = ffi.dlopen("../mitielib/libmitie.so")
models = ffi.new("char[]", "../MITIE-models/ner_model.dat")

ner = C.mitie_load_named_entity_extractor(models)
	#C.mitie_get_num_possible_ner_tags(ner)

def tag(text):

  """ Tag text using MITie entity extraction models """
	
	data = ffi.new("char[]", text)
	tokens = C.mitie_tokenize(data)

	dets = C.mitie_extract_entities(ner, tokens)
	num_dets = C.mitie_ner_get_num_detections(dets);

	print num_dets

	tags = []
	for i in range(num_dets):
		tpos = C.mitie_ner_get_detection_position(dets, i);
		tlen = C.mitie_ner_get_detection_length(dets, i);

		tag = C.mitie_ner_get_detection_tag(dets,i)
		tagstr = ffi.string(C.mitie_ner_get_detection_tagstr(dets,i))

		token = ffi.string(tokens[tpos])

		tags.append((token, tagstr))

	return tags
		
