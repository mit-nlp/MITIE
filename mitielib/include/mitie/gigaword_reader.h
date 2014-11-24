// Copyright (C) 2012 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_GIGAWoRD_READER_H_
#define MIT_LL_GIGAWoRD_READER_H_

#include "dlib/xml_parser.h"
#include "dlib/string.h"
#include "dlib/dir_nav.h"
#include <list>
#include <fstream>

namespace mitie
{

// ----------------------------------------------------------------------------------------

    struct gigaword_document
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a parsed gigaword document.  It corresponds to a <DOC> field
                out of the gigaword corpus.
        !*/

        std::string id;
        std::string type;
        std::string headline;
        std::string dateline;
        std::string text;
    };

// ----------------------------------------------------------------------------------------

    class gigaword_file_reader 
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a tool for reading gigaword_document objects out of a single
                gigaword file.  
        !*/

    public:

        gigaword_file_reader (
        ) : in(0) {}
        /*!
            ensures
                - any attempts to get a document will return false.  I.e. this will look like a 
                  reader that has run out of documents.
        !*/

        gigaword_file_reader (
            std::istream& in_
        ) : in(&in_) {}
        /*!
            ensures
                - will read documents from the supplied input stream
        !*/

        bool operator() (
            gigaword_document& doc
        ) 
        /*!
            ensures
                - if (the input stream given to this object's constructor has another <DOC> record) then
                    - parses the next document and puts it into #doc
                    - returns true
                - else
                    - returns false
        !*/
        {
            if (!in || !in->good())
                return false;

            dlib::xml_parser::kernel_1a_c parser;
            doc_handler dh(doc);
            xml_error_handler eh;
            parser.add_document_handler(dh);
            parser.add_error_handler(eh);
            parser.parse(*in);

            if (!eh.no_errors)
            {
                using namespace std;
                cout << "doc.id:       " << doc.id << endl;
                cout << "doc.type:     " << doc.type << endl;
                cout << "doc.headline: " << doc.headline << endl;
                cout << "Are there &AMP; references in the text?  Per the XML standard, these should be lowercase.\n" << endl;
            }
            return eh.no_errors && in->good();
        }

    private:

        class doc_handler : public dlib::document_handler
        {
        public:

            gigaword_document& doc;
            enum zone { HEADLINE, DATELINE, TEXT, NONE};
            zone current_zone;

            doc_handler(
                gigaword_document& doc_
            ) : doc(doc_)
            {
                doc.id.clear();
                doc.type.clear();
                doc.headline.clear();
                doc.dateline.clear();
                doc.text.clear();

                current_zone = NONE;
            }

            virtual void start_document (
            )
            {
            }

            virtual void end_document (
            )
            {
            }

            virtual void start_element ( 
                const unsigned long ,
                const std::string& name,
                const dlib::attribute_list& atts
            )
            {
                if (name == "DOC")
                {
                    if (atts.is_in_list("id"))
                        doc.id = dlib::trim(atts["id"]);
                    if (atts.is_in_list("type"))
                        doc.type = dlib::trim(atts["type"]);
                }
                else if (name == "TEXT")
                {
                    current_zone = TEXT;
                }
                else if (name == "HEADLINE")
                {
                    current_zone = HEADLINE;
                }
                else if (name == "DATELINE")
                {
                    current_zone = DATELINE;
                }
            }

            virtual void end_element ( 
                const unsigned long ,
                const std::string& name
            )
            {
                switch (current_zone)
                {
                    case TEXT: 
                        if (name == "TEXT") 
                            current_zone = NONE;
                        break;

                    case HEADLINE: 
                        if (name == "HEADLINE") 
                            current_zone = NONE;
                        break;

                    case DATELINE: 
                        if (name == "DATELINE") 
                            current_zone = NONE;
                        break;

                    default:
                        break;
                }
            }

            virtual void characters ( 
                const std::string& data
            )
            {
                switch (current_zone)
                {
                    case TEXT:
                        doc.text += data;
                        break;
                    case HEADLINE:
                        doc.headline = dlib::trim(data);
                        break;
                    case DATELINE:
                        doc.dateline = dlib::trim(data);
                        break;

                    default:
                        break;
                }

            }

            virtual void processing_instruction (
                const unsigned long ,
                const std::string& ,
                const std::string& 
            )
            {
            }
        };

        class xml_error_handler : public dlib::error_handler
        {

        public:
            bool no_errors;

            xml_error_handler()
            {
                no_errors = true;
            }

            virtual void error (
                const unsigned long line_number
            )
            {
                std::cout << "There is a non-fatal error on line " << line_number << " in the file we are parsing." << std::endl;
                no_errors = false;
            }

            virtual void fatal_error (
                const unsigned long line_number
            )
            {
                std::cout << "There is a fatal error on line " << line_number << " so parsing will now halt" << std::endl;
                no_errors = false;
            }
        };


        std::istream* in;
    };

// ----------------------------------------------------------------------------------------

    class gigaword_reader 
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a tool that wraps the gigaword_file_reader and makes it easer
                to read from a set of files or individual files.
        !*/
    public:

        gigaword_reader (
            const char* filename 
        )
        /*!
            ensures
                - This object will read gigaword_documents from the file with the 
                  given filename.
        !*/
        {
            file_list.push_back(filename);
            reset();
        }

        gigaword_reader (
            const std::string& filename 
        )
        /*!
            ensures
                - This object will read gigaword_documents from the file with the 
                  given filename.
        !*/
        {
            file_list.push_back(filename);
            reset();
        }

        gigaword_reader (
            const dlib::file& filename
        )
        /*!
            ensures
                - This object will read gigaword_documents from the file with the 
                  given filename.
        !*/
        {
            file_list.push_back(filename.full_name());
            reset();
        }

        gigaword_reader (
            const std::vector<dlib::file>& files
        )
        /*!
            ensures
                - This object will read gigaword_documents out of the list of supplied
                  files.
        !*/
        {
            for (unsigned long i = 0; i < files.size(); ++i)
            {
                file_list.push_back(files[i].full_name());
            }
            reset();
        }

        void reset(
        ){ next_file = 0; reader = gigaword_file_reader();  }
        /*!
            ensures
                - puts the reader back at the start of the document sequence.  Therefore,
                  calling reset() will allow you to do another pass over the documents.
        !*/

        bool operator() (
            gigaword_document& doc
        )
        /*!
            ensures
                - if (the file/files given to this object's constructor have another <DOC> record) then
                    - parses the next document and puts it into #doc
                    - returns true
                - else
                    - returns false
        !*/
        {
            while (true)
            {
                // try to get the next doc
                if (reader(doc))
                    return true;

                if (next_file < file_list.size())
                {
                    // go to the next gigaword file if we are out of docs from the current one
                    fin.close();
                    fin.clear();
                    fin.open(file_list[next_file].c_str());
                    reader = fin;
                    ++next_file;
                }
                else
                {
                    return false;
                }
            }
        }

        bool operator() (
            std::string& doc
        )
        /*!
            ensures
                - if (the file/files given to this object's constructor have another <DOC> record) then
                    - parses the next document and puts its text into #doc. 
                    - returns true
                - else
                    - returns false
        !*/
        {
            gigaword_document temp;
            bool result = (*this)(temp);
            doc.swap(temp.text);
            return result;
        }

    private:

        unsigned long next_file;
        std::vector<std::string> file_list;
        std::ifstream fin;
        gigaword_file_reader reader;
    };

// ----------------------------------------------------------------------------------------

    template <
        typename basic_tokenizer
        >
    class gigaword_tokenizer 
    {
        /*!
            REQUIREMENTS ON basic_tokenizer
                This must be an object with an interface compatible with the unigram_tokenizer.

            WHAT THIS OBJECT REPRESENTS
                This object is a tool for converting a folder or file of gigaword data into
                a stream of tokens.
        !*/

    public:

        typedef std::string token_type;

        gigaword_tokenizer (
            const char* filename 
        ) : reader(filename) {}
        /*!
            ensures
                - This object will read tokens from the file with the given filename.
        !*/

        gigaword_tokenizer (
            const std::string& filename 
        ) : reader(filename) {}
        /*!
            ensures
                - This object will read tokens from the file with the given filename.
        !*/

        gigaword_tokenizer (
            const dlib::file& filename
        ) : reader(filename) {}
        /*!
            ensures
                - This object will read tokens from the file with the given filename.
        !*/

        gigaword_tokenizer (
            const std::vector<dlib::file>& files
        ) : reader(files) {}
        /*!
            requires
                - files.size() > 0
            ensures
                - This object will read tokens out of the list of supplied files.
        !*/

        void reset(
        ){ reader.reset(); tok = basic_tokenizer(); }
        /*!
            ensures
                - puts the tokenizer back at the start of the token sequence.  Therefore,
                  calling reset() will allow you to do another pass over the tokens.
        !*/

        bool operator() (
            std::string& token 
        )
        /*!
            ensures
                - reads the next token from the dataset given to this object's constructor
                  and stores it in #token.
                - if (there is not a next token) then
                    - #token.size() == 0
                    - returns false
                - else
                    - #token.size() != 0
                    - returns true
        !*/
        {
            while (true)
            {
                // if there is a token then return it
                if (tok(token))
                {
                    return true;
                }

                // no tokens, so try and go to the next gigaword file
                std::string doc;
                if (reader(doc))
                {
                    sin.clear();
                    sin.str(doc);
                    tok = basic_tokenizer(sin);
                }
                else
                {
                    return false;
                }
            }
        }

    private:
        gigaword_reader reader;
        std::istringstream sin;
        basic_tokenizer tok;
    };

// ----------------------------------------------------------------------------------------

}

#endif // MIT_LL_GIGAWoRD_READER_H_

