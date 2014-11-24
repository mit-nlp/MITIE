// Copyright (C) 2013 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_TOTAL_WoRD_FEATURE_EXTRACTOR_H_
#define MIT_LL_TOTAL_WoRD_FEATURE_EXTRACTOR_H_

#include <map>
#include "word_morphology_feature_extractor.h"
#include <dlib/statistics.h>
#include <dlib/vectorstream.h>
#include <dlib/hash.h>

namespace mitie 
{

    class total_word_feature_extractor
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is a tool for turning a word into a short and dense vector which
                describes what kind of places in text a word can appear.  This is done
                using both word morphology and general distributional word features.  Or in
                other words, this object is basically a combination of the
                word_morphology_feature_extractor along with a general table mapping words
                to descriptive feature vectors (generally created from some CCA based
                distributional feature thing).

            THREAD SAFETY
                Note that this object uses mutable internal scratch space.  Therefore, it is
                unsafe for two threads to touch the same instance of this object at a time
                without mutex locking it first.
        !*/

        inline static std::string convert_numbers (
            const std::string& str_
        )
        {
            std::string str(str_);
            for (unsigned long i = 0; i < str.size(); ++i)
            {
                if ('0' <= str[i] && str[i] <= '9')
                    str[i] = '#';
            }
            return str;
        }

    public:

        total_word_feature_extractor() : fingerprint(0), non_morph_feats(0) {}

        total_word_feature_extractor(
            const std::map<std::string, dlib::matrix<float,0,1> >& word_vectors,
            const word_morphology_feature_extractor& morph_fe_
        ) :
            morph_fe(morph_fe_)
        {
            DLIB_CASSERT(word_vectors.size() != 0, "Invalid arguments");

            std::map<std::string, dlib::matrix<float,0,1> >::const_iterator i;

            i = word_vectors.begin();
            non_morph_feats = i->second.size() + 1; // plus one for the OOV indicator


            // now figure out how to relatively scale the word_vectors and morph features.
            dlib::running_stats<double> rs_word, rs_morph;
            dlib::matrix<float,0,1> feats;
            for (; i != word_vectors.end(); ++i)
            {
                morph_fe.get_feature_vector(i->first, feats);
                rs_morph.add(mean(abs(feats)));
                rs_word.add(mean(abs(i->second)));
            }

            // scale the morphological features so they have an average feature value of 1.
            morph_fe.premultiply_vectors_by(1/rs_morph.mean());

            // We also want to scale the word vectors to have an average feature value of 1.
            const double scale = 1/rs_word.mean();
            // Now go though all the words again and compute the complete feature vectors for
            // each and store that into total_word_vectors.
            for (i = word_vectors.begin(); i != word_vectors.end(); ++i)
            {
                morph_fe.get_feature_vector(i->first, feats);
                total_word_vectors[i->first] = join_cols(join_cols(dlib::zeros_matrix<float>(1,1), scale*i->second), feats);
            }


            // finally, don't forget to set the fingerprint to something
            compute_fingerprint();
        }

        dlib::uint64 get_fingerprint(
        ) const { return fingerprint; }
        /*!
            ensures
                - returns a 64bit ID number that uniquely identifies this object instance.
                  The ID is computed based on the state of this object, so any copy of it
                  that has the same state will have the same fingerprint ID number.  This
                  is useful since down stream models that have been trained to use a
                  specific total_word_feature_extractor instance can record the fingerprint
                  of the total_word_feature_extractor they used and use that fingerprint ID
                  to verify that the same total_word_feature_extractor is being used later
                  on.
        !*/

        void get_feature_vector(
            const std::string& word_,
            dlib::matrix<float,0,1>& feats
        ) const
        /*!
            ensures
                - #feats == a dense vector describing the given word
                - #feats.size() == get_num_dimensions()
        !*/
        {
            const std::string word = convert_numbers(word_);
            std::map<std::string, dlib::matrix<float,0,1> >::const_iterator i;
            i = total_word_vectors.find(word);
            if (i != total_word_vectors.end())
            {
                feats = i->second;
                return;
            }

            if (get_num_dimensions() == 0)
            {
                feats.set_size(0);
                return;
            }

            morph_fe.get_feature_vector(word, temp);
            feats = join_cols(dlib::zeros_matrix<float>(non_morph_feats,1), temp);
            // This is an indicator feature used to model the fact that this word is
            // outside our dictionary.
            feats(0) = 1;
        }

        unsigned long get_num_dimensions(
        ) const
        /*!
            ensures
                - returns the dimensionality of the feature vectors produced by this
                  object.
        !*/
        {
            return non_morph_feats + morph_fe.get_num_dimensions();
        }

        unsigned long get_num_words_in_dictionary (
        ) const
        {
            return total_word_vectors.size();
        }

        std::vector<std::string> get_words_in_dictionary (
        ) const
        {
            std::vector<std::string> temp;
            temp.reserve(total_word_vectors.size());
            std::map<std::string, dlib::matrix<float,0,1> >::const_iterator i;
            for (i = total_word_vectors.begin(); i != total_word_vectors.end(); ++i)
            {
                temp.push_back(i->first);
            }
            return temp;
        }

        friend void serialize(const total_word_feature_extractor& item, std::ostream& out)
        {
            int version = 2;
            dlib::serialize(version, out);
            dlib::serialize(item.fingerprint, out);
            dlib::serialize(item.non_morph_feats, out);
            dlib::serialize(item.total_word_vectors, out);
            serialize(item.morph_fe, out);
        }

        friend void deserialize(total_word_feature_extractor& item, std::istream& in)
        {
            int version = 0;
            dlib::deserialize(version, in);
            if (version != 2)
                throw dlib::serialization_error("Unexpected version found while deserializing total_word_feature_extractor.");
            dlib::deserialize(item.fingerprint, in);
            dlib::deserialize(item.non_morph_feats, in);
            dlib::deserialize(item.total_word_vectors, in);
            deserialize(item.morph_fe, in);
        }

    private:

        void compute_fingerprint()
        {
            std::vector<char> buf;
            dlib::vectorstream sout(buf);
            sout << "fingerprint";
            dlib::serialize(non_morph_feats, sout);
            dlib::serialize(total_word_vectors, sout);
            serialize(morph_fe, sout);

            fingerprint = dlib::murmur_hash3_128bit(&buf[0], buf.size()).first;
        }

        dlib::uint64 fingerprint;
        long non_morph_feats;
        std::map<std::string, dlib::matrix<float,0,1> > total_word_vectors;
        word_morphology_feature_extractor morph_fe;

        // This object does not logically contribute to the state of this object.  It is
        // just here to avoid reallocating it over and over in get_feature_vector().
        mutable dlib::matrix<float,0,1> temp;
    };

}

#endif // MIT_LL_TOTAL_WoRD_FEATURE_EXTRACTOR_H_

