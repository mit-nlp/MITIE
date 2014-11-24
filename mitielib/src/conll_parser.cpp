// Copyright (C) 2013 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#include <mitie/conll_parser.h>

#include <fstream>
#include <dlib/string.h>

using namespace std;
using namespace dlib;

namespace mitie
{

// ----------------------------------------------------------------------------------------

    std::string lookup_conll_label (
        const BIO_label& label 
    )
    {
        switch (label)
        {
            case B_PER:  return "B-PER";
            case B_ORG:  return "B-ORG";
            case B_MISC: return "B-MISC";
            case B_LOC:  return "B-LOC";
            case I_PER:  return "I-PER";
            case I_ORG:  return "I-ORG";
            case I_MISC: return "I-MISC";
            case I_LOC:  return "I-LOC";
            case L_PER:  return "L-PER";
            case L_ORG:  return "L-ORG";
            case L_MISC: return "L-MISC";
            case L_LOC:  return "L-LOC";
            case U_PER:  return "U-PER";
            case U_ORG:  return "U-ORG";
            case U_MISC: return "U-MISC";
            case U_LOC:  return "U-LOC";

            case O:      return "O";
            default: throw dlib::error("invalid label given to lookup_conll_label()!");
        }
    }

// ----------------------------------------------------------------------------------------

    BIO_label lookup_conll_label (
        const std::string& str
    )
    {
        if (str == "O")
            return O;
        else if (str == "I-PER")
            return I_PER;
        else if (str == "I-ORG")
            return I_ORG;
        else if (str == "I-LOC")
            return I_LOC;
        else if (str == "I-MISC")
            return I_MISC;
        else if (str == "B-PER")
            return B_PER;
        else if (str == "B-ORG")
            return B_ORG;
        else if (str == "B-LOC")
            return B_LOC;
        else if (str == "B-MISC")
            return B_MISC;
        else
            throw dlib::error("INVALID CONLL LABEL FOUND: " + str);
    }

// ----------------------------------------------------------------------------------------

    std::vector<labeled_sentence> parse_conll_data (
        const std::string& filename
    )
    {
        std::vector<labeled_sentence> result;
        
        labeled_sentence sentence;

        ifstream fin(filename.c_str());
        std::string line;
        unsigned long line_number = 0;
        while(getline(fin, line))
        {
            ++line_number;
            const std::vector<std::string>& toks = split(line," \t\n");
            if (toks.size() == 4)
            {
                sentence.push_back(make_pair(toks[0], lookup_conll_label(toks[3])));
            }
            else if (toks.size() == 0)
            {
                // this is the end of the sentence
                result.push_back(sentence);
                sentence.clear();
            }
            else
            {
                throw dlib::error("CONNLL PARSE ERROR, wrong number of tokens in line " + cast_to_string(line_number));
            }
        }

        return result;
    }

// ----------------------------------------------------------------------------------------

    void print_conll_data (
        const std::vector<labeled_sentence>& data
    )
    {
        for (unsigned long i = 0; i < data.size(); ++i)
        {
            for (unsigned long j = 0; j < data[i].size(); ++j)
            {
                cout << data[i][j].first << " X X " << lookup_conll_label(data[i][j].second) << "\n";
            }
            cout << "\n";
        }
    }

// ----------------------------------------------------------------------------------------

    void print_conll_data (
        const std::vector<labeled_sentence>& data,
        const std::vector<std::vector<BIO_label> >& extra_labels
    )
    {
        for (unsigned long i = 0; i < data.size(); ++i)
        {
            for (unsigned long j = 0; j < data[i].size(); ++j)
            {
                cout << data[i][j].first << " X X " <<
                    lookup_conll_label(data[i][j].second) << " " <<
                    lookup_conll_label(extra_labels[i][j]) << "\n";
            }
            cout << "\n";
        }
    }

// ----------------------------------------------------------------------------------------

    void separate_labels_from_tokens (
        const std::vector<labeled_sentence>& data,
        std::vector<std::vector<std::string> >& tokens,
        std::vector<std::vector<BIO_label> >& labels
    )
    {
        tokens.clear();
        labels.clear();
        tokens.resize(data.size());
        labels.resize(data.size());
        for (unsigned long i = 0; i < data.size(); ++i)
        {
            for (unsigned long j = 0; j < data[i].size(); ++j)
            {
                tokens[i].push_back(data[i][j].first);
                labels[i].push_back(data[i][j].second);
            }
        }
    }

// ----------------------------------------------------------------------------------------

    namespace
    {
        bool is_B ( const BIO_label& val) { return val == B_PER || val == B_ORG || val == B_LOC || val == B_MISC; }
        bool is_I ( const BIO_label& val) { return val == I_PER || val == I_ORG || val == I_LOC || val == I_MISC; }
        bool is_L ( const BIO_label& val) { return val == L_PER || val == L_ORG || val == L_LOC || val == L_MISC; }
        bool is_U ( const BIO_label& val) { return val == U_PER || val == U_ORG || val == U_LOC || val == U_MISC; }
        bool is_O ( const BIO_label& val) { return val == O; }

        BIO_label to_chunk_label (const BIO_label& val)
        {
            switch (val)
            {
                case B_PER: return PER;
                case I_PER: return PER;
                case L_PER: return PER;
                case U_PER: return PER;

                case B_LOC: return LOC;
                case I_LOC: return LOC;
                case L_LOC: return LOC;
                case U_LOC: return LOC;

                case B_MISC: return MISC;
                case I_MISC: return MISC;
                case L_MISC: return MISC;
                case U_MISC: return MISC;

                case B_ORG: return ORG;
                case I_ORG: return ORG;
                case L_ORG: return ORG;
                case U_ORG: return ORG;

                default: throw dlib::error("bad label");
            }
        }

        BIO_label to_B(const BIO_label& val) 
        {
            switch (val)
            {
                case B_PER: return B_PER;
                case I_PER: return B_PER;
                case L_PER: return B_PER;
                case U_PER: return B_PER;

                case B_LOC: return B_LOC;
                case I_LOC: return B_LOC;
                case L_LOC: return B_LOC;
                case U_LOC: return B_LOC;

                case B_MISC: return B_MISC;
                case I_MISC: return B_MISC;
                case L_MISC: return B_MISC;
                case U_MISC: return B_MISC;

                case B_ORG: return B_ORG;
                case I_ORG: return B_ORG;
                case L_ORG: return B_ORG;
                case U_ORG: return B_ORG;

                default: throw dlib::error("bad label");
            }
        }

        BIO_label to_L(const BIO_label& val) 
        {
            switch (val)
            {
                case B_PER: return L_PER;
                case I_PER: return L_PER;
                case L_PER: return L_PER;
                case U_PER: return L_PER;

                case B_LOC: return L_LOC;
                case I_LOC: return L_LOC;
                case L_LOC: return L_LOC;
                case U_LOC: return L_LOC;

                case B_MISC: return L_MISC;
                case I_MISC: return L_MISC;
                case L_MISC: return L_MISC;
                case U_MISC: return L_MISC;

                case B_ORG: return L_ORG;
                case I_ORG: return L_ORG;
                case L_ORG: return L_ORG;
                case U_ORG: return L_ORG;

                default: throw dlib::error("bad label");
            }
        }

        BIO_label to_U(const BIO_label& val) 
        {
            switch (val)
            {
                case B_PER: return U_PER;
                case I_PER: return U_PER;
                case L_PER: return U_PER;
                case U_PER: return U_PER;

                case B_LOC: return U_LOC;
                case I_LOC: return U_LOC;
                case L_LOC: return U_LOC;
                case U_LOC: return U_LOC;

                case B_MISC: return U_MISC;
                case I_MISC: return U_MISC;
                case L_MISC: return U_MISC;
                case U_MISC: return U_MISC;

                case B_ORG: return U_ORG;
                case I_ORG: return U_ORG;
                case L_ORG: return U_ORG;
                case U_ORG: return U_ORG;

                default: throw dlib::error("bad label");
            }
        }

        BIO_label to_I(const BIO_label& val) 
        {
            switch (val)
            {
                case B_PER: return I_PER;
                case I_PER: return I_PER;
                case L_PER: return I_PER;
                case U_PER: return I_PER;

                case B_LOC: return I_LOC;
                case I_LOC: return I_LOC;
                case L_LOC: return I_LOC;
                case U_LOC: return I_LOC;

                case B_MISC: return I_MISC;
                case I_MISC: return I_MISC;
                case L_MISC: return I_MISC;
                case U_MISC: return I_MISC;

                case B_ORG: return I_ORG;
                case I_ORG: return I_ORG;
                case L_ORG: return I_ORG;
                case U_ORG: return I_ORG;

                default: throw dlib::error("bad label");
            }
        }
    }

// ----------------------------------------------------------------------------------------

    void convert_from_BIO_to_BILOU (
        std::vector<BIO_label>& labels
    )
    {
        for (unsigned long i = 0; i < labels.size(); ++i)
        {
            BIO_label last = O, cur = O, next = O;
            if (i != 0)
                last = labels[i-1];
            if (i+1 < labels.size())
                next = labels[i+1];
            cur = labels[i];


            if (is_O(cur))
                continue;

            if (is_B(cur))
            {
                if (next != to_I(cur))
                    labels[i] = to_U(cur);
            }
            else if (is_I(cur))
            {
                if (last == cur || last == to_B(cur))
                {
                    if (next == cur)
                    {
                        // do nothing, this is really an I
                    }
                    else
                    {
                        labels[i] = to_L(cur);
                    }
                }
                else
                {
                    if (next == cur)
                    {
                        labels[i] = to_B(cur);
                    }
                    else
                    {
                        labels[i] = to_U(cur);
                    }
                }
            }
        }
    }

// ----------------------------------------------------------------------------------------

    void convert_from_BILOU_to_BIO (
        std::vector<BIO_label>& labels
    )
    {
        for (unsigned long i = 0; i < labels.size(); ++i)
        {
            BIO_label last = O, cur = O;
            if (i != 0)
                last = labels[i-1];
            cur = labels[i];


            if (is_O(cur))
                continue;

            if (is_U(cur))
            {
                if (last != to_I(cur) && last != to_B(cur))
                    labels[i] = to_I(cur);
                else
                    labels[i] = to_B(cur);
                continue;
            }

            if (is_B(cur))
            {
                if (last != to_I(cur) && last != to_B(cur))
                    labels[i] = to_I(cur);
                continue;
            }

            if (is_L(cur))
            {
                labels[i] = to_I(cur);
                continue;
            }
        }
    }

// ----------------------------------------------------------------------------------------

    void convert_from_BIO_to_BILOU (
        std::vector<std::vector<BIO_label> >& labels
    )
    {
        for (unsigned long i = 0; i < labels.size(); ++i)
        {
            convert_from_BIO_to_BILOU(labels[i]);
        }
    }

// ----------------------------------------------------------------------------------------

    void convert_from_BILOU_to_BIO (
        std::vector<std::vector<BIO_label> >& labels
    )
    {
        for (unsigned long i = 0; i < labels.size(); ++i)
        {
            convert_from_BILOU_to_BIO(labels[i]);
        }
    }

// ----------------------------------------------------------------------------------------

    static void convert_sentence (
        const labeled_sentence& sent,
        std::vector<std::string>& tokens,
        std::vector<std::pair<unsigned long, unsigned long> >& chunks,
        std::vector<unsigned long>& chunk_labels
    )
    {
        // copy the tokens over
        tokens.resize(sent.size());
        for (unsigned long i = 0; i < tokens.size(); ++i)
            tokens[i] = sent[i].first;

        chunks.clear();
        chunk_labels.clear();
        // now find the chunks
        for (unsigned long i = 0; i < sent.size(); ++i)
        {
            BIO_label label = sent[i].second;
            if (is_O(label))
                continue;

            if (is_B(label) || is_I(label))
            {
                chunk_labels.push_back(to_chunk_label(label));
                // figure out the chunk boundaries
                const unsigned long begin = i;
                ++i;
                while (i < sent.size() && is_I(sent[i].second) && to_chunk_label(sent[i].second)==chunk_labels.back())
                    ++i;
                const unsigned long end = i;
                --i;
                chunks.push_back(make_pair(begin,end));
            }
            else
            {
                throw dlib::error("invalid labels found in conll data");
            }
        }
    }

// ----------------------------------------------------------------------------------------

    void parse_conll_data (
        const std::string& filename,
        std::vector<std::vector<std::string> >& sentences,
        std::vector<std::vector<std::pair<unsigned long, unsigned long> > >& chunks,
        std::vector<std::vector<unsigned long> >& chunk_labels
    )
    {
        std::vector<labeled_sentence> data = parse_conll_data (filename);

        sentences.resize(data.size());
        chunks.resize(data.size());
        chunk_labels.resize(data.size());

        for (unsigned long i = 0; i < data.size(); ++i)
        {
            convert_sentence(data[i], sentences[i], chunks[i], chunk_labels[i]);
        }
    }

// ----------------------------------------------------------------------------------------

    void parse_conll_data (
        const std::string& filename,
        std::vector<std::vector<std::string> >& sentences,
        std::vector<std::vector<std::pair<unsigned long, unsigned long> > >& chunks,
        std::vector<std::vector<std::string> >& chunk_labels
    )
    {
        std::vector<std::vector<unsigned long> > int_chunk_labels;
        parse_conll_data(filename, sentences, chunks, int_chunk_labels);
        // now convert the chunk labels
        chunk_labels.clear();
        chunk_labels.resize(int_chunk_labels.size());
        for (unsigned long i = 0; i < int_chunk_labels.size(); ++i)
        {
            chunk_labels[i].resize(int_chunk_labels[i].size());
            for (unsigned long j = 0; j < int_chunk_labels[i].size(); ++j)
            {
                if (int_chunk_labels[i][j] == PER)
                    chunk_labels[i][j] = "PERSON";
                else if (int_chunk_labels[i][j] == ORG)
                    chunk_labels[i][j] = "ORGANIZATION";
                else if (int_chunk_labels[i][j] == LOC)
                    chunk_labels[i][j] = "LOCATION";
                else if (int_chunk_labels[i][j] == MISC)
                    chunk_labels[i][j] = "MISC";
                else
                    DLIB_CASSERT(false, "");
            }
        }
    }

// ----------------------------------------------------------------------------------------

}
