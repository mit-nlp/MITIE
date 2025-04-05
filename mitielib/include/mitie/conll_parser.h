// Copyright (C) 2013 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_CONLL_PaRSER_H_
#define MIT_LL_CONLL_PaRSER_H_

#include <mitie/mitie_api_prefix.h>
#include <vector>
#include <string>

namespace mitie
{

// ----------------------------------------------------------------------------------------

    typedef unsigned long BIO_label;

    constexpr unsigned long I_PER  = 0;
    constexpr unsigned long B_PER  = 1;
    constexpr unsigned long O      = 2;
    constexpr unsigned long B_LOC  = 3;
    constexpr unsigned long B_ORG  = 4;
    constexpr unsigned long B_MISC = 5;
    constexpr unsigned long I_ORG  = 6;
    constexpr unsigned long I_LOC  = 7;
    constexpr unsigned long I_MISC = 8;

    // BILOU extension
    constexpr unsigned long L_PER  = 9;
    constexpr unsigned long L_ORG  = 10;
    constexpr unsigned long L_LOC  = 11;
    constexpr unsigned long L_MISC = 12;
    constexpr unsigned long U_PER  = 13;
    constexpr unsigned long U_ORG  = 14;
    constexpr unsigned long U_LOC  = 15;
    constexpr unsigned long U_MISC = 16;

    // chunk labels
    constexpr unsigned long PER  = 0;
    constexpr unsigned long LOC  = 1;
    constexpr unsigned long ORG  = 2;
    constexpr unsigned long MISC = 3;
    constexpr unsigned long NOT_ENTITY = 4;

// ----------------------------------------------------------------------------------------

    typedef std::vector<std::pair<std::string, BIO_label> > labeled_sentence;

// ----------------------------------------------------------------------------------------

    MITIE_API void parse_conll_data (
        const std::string& filename,
        std::vector<std::vector<std::string> >& sentences,
        std::vector<std::vector<std::pair<unsigned long, unsigned long> > >& chunks,
        std::vector<std::vector<unsigned long> >& chunk_labels
    );
    /*!
        ensures
            - reads the given file and parses it as a CONLL 2003 NER data file.  The output
              is the set of sentences where each has been annotated with its named entity
              chunk boundaries and chunk labels.  
            - #sentences.size() == chunks.size() == chunk_labels.size()
            - for all valid i:
                - #sentences[i] == The tokens in the i-th sentence in the CONLL dataset file.
                - #chunks[i].size() == #chunk_labels[i].size()
                - #chunks[i] == the named entity chunks in #sentences[i].  Moreover,
                  #chunks[i][j] has a label of #chunk_labels[i][j].  
                - #chunks[i][j] specifies a half open range of words within sentences[i].
                  This range contains a named entity.  The range starts with sentences[i][chunks[i][j].first]
                  and ends at sentences[i][chunks[i][j].second-1].
                - #chunk_labels[i][j] is equal to either PER, ORG, LOC, or MISC.
    !*/

    MITIE_API void parse_conll_data (
        const std::string& filename,
        std::vector<std::vector<std::string> >& sentences,
        std::vector<std::vector<std::pair<unsigned long, unsigned long> > >& chunks,
        std::vector<std::vector<std::string> >& chunk_labels
    );
    /*!
        ensures
            - This function is identical to the version above except instead of using
              integer chunk labels (i.e. PER, LOC, ORG, and MISC) it uses strings of
              "PERSON", "LOCATION", "ORGANIZATION", and "MISC".
    !*/

// ----------------------------------------------------------------------------------------

    MITIE_API std::vector<labeled_sentence> parse_conll_data (
        const std::string& filename
    );
    /*!
        ensures
            - reads the given file and parses it as a CONLL 2003 NER data file.  We
              return the set of sentences with each token labeled with it's NER label.
    !*/

// ----------------------------------------------------------------------------------------

    MITIE_API void print_conll_data (
        const std::vector<labeled_sentence>& data
    );
    /*!
        ensures
            - prints the given data in the CONLL 2003 format.  Since data only has tokens
              and NER tags the POS and Chunk tags are filled in with X.
    !*/

// ----------------------------------------------------------------------------------------

    MITIE_API void print_conll_data (
        const std::vector<labeled_sentence>& data,
        const std::vector<std::vector<BIO_label> >& extra_labels
    );
    /*!
        requires
            - data.size() == extra_labels.size()
            - for all valid i:
                - data[i].size() == extra_labels[i].size()
        ensures
            - prints the given data int he CONLL 2003 format.  Since data only has tokens
              and NER tags the POS and Chunk tags are filled in with X.  We also print extra_labels
              as the 5th column.  Therefore, extra_labels should contain predicted labels for
              each token.
    !*/

// ----------------------------------------------------------------------------------------

    MITIE_API void separate_labels_from_tokens (
        const std::vector<labeled_sentence>& data,
        std::vector<std::vector<std::string> >& tokens,
        std::vector<std::vector<BIO_label> >& labels
    );
    /*!
        ensures
            - Splits the labeled data given to this function into two vectors, one containing
              the tokens and another the labels
            - #tokens.size() == data.size()
            - #labels.size() == data.size()
            - for all valid i:
                - #tokens[i].size() == data[i].size()
                - #labels[i].size() == data[i].size()
    !*/

// ----------------------------------------------------------------------------------------

    MITIE_API std::string lookup_conll_label (
        const BIO_label& label 
    );

    MITIE_API void convert_from_BIO_to_BILOU (
        std::vector<BIO_label>& labels
    );

    MITIE_API void convert_from_BILOU_to_BIO (
        std::vector<BIO_label>& labels
    );

    MITIE_API void convert_from_BIO_to_BILOU (
        std::vector<std::vector<BIO_label> >& labels
    );

    MITIE_API void convert_from_BILOU_to_BIO (
        std::vector<std::vector<BIO_label> >& labels
    );

// ----------------------------------------------------------------------------------------

}

#endif // MIT_LL_CONLL_PaRSER_H_

