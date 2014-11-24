// Copyright (C) 2013 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_CONLL_PaRSER_H_
#define MIT_LL_CONLL_PaRSER_H_

#include <vector>
#include <string>

namespace mitie
{

// ----------------------------------------------------------------------------------------

    typedef unsigned long BIO_label;

    const unsigned long I_PER  = 0;
    const unsigned long B_PER  = 1;
    const unsigned long O      = 2;
    const unsigned long B_LOC  = 3;
    const unsigned long B_ORG  = 4;
    const unsigned long B_MISC = 5;
    const unsigned long I_ORG  = 6;
    const unsigned long I_LOC  = 7;
    const unsigned long I_MISC = 8;

    // BILOU extension
    const unsigned long L_PER  = 9;
    const unsigned long L_ORG  = 10;
    const unsigned long L_LOC  = 11;
    const unsigned long L_MISC = 12;
    const unsigned long U_PER  = 13;
    const unsigned long U_ORG  = 14;
    const unsigned long U_LOC  = 15;
    const unsigned long U_MISC = 16;

    // chunk labels
    const unsigned long PER  = 0;
    const unsigned long LOC  = 1;
    const unsigned long ORG  = 2;
    const unsigned long MISC = 3;
    const unsigned long NOT_ENTITY = 4;

// ----------------------------------------------------------------------------------------

    typedef std::vector<std::pair<std::string, BIO_label> > labeled_sentence;

// ----------------------------------------------------------------------------------------

    void parse_conll_data (
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

    void parse_conll_data (
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

    std::vector<labeled_sentence> parse_conll_data (
        const std::string& filename
    );
    /*!
        ensures
            - reads the given file and parses it as a CONLL 2003 NER data file.  We
              return the set of sentences with each token labeled with it's NER label.
    !*/

// ----------------------------------------------------------------------------------------

    void print_conll_data (
        const std::vector<labeled_sentence>& data
    );
    /*!
        ensures
            - prints the given data in the CONLL 2003 format.  Since data only has tokens
              and NER tags the POS and Chunk tags are filled in with X.
    !*/

// ----------------------------------------------------------------------------------------

    void print_conll_data (
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

    void separate_labels_from_tokens (
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

    std::string lookup_conll_label (
        const BIO_label& label 
    );

    void convert_from_BIO_to_BILOU (
        std::vector<BIO_label>& labels
    );

    void convert_from_BILOU_to_BIO (
        std::vector<BIO_label>& labels
    );

    void convert_from_BIO_to_BILOU (
        std::vector<std::vector<BIO_label> >& labels
    );

    void convert_from_BILOU_to_BIO (
        std::vector<std::vector<BIO_label> >& labels
    );

// ----------------------------------------------------------------------------------------

}

#endif // MIT_LL_CONLL_PaRSER_H_

