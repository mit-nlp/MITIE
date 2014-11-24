// Copyright (C) 2013 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)

#include "basic_morph.h"
#include <mitie/count_min_sketch.h>
#include <string>
#include <vector>
#include <map>
#include <dlib/set.h>
#include <mitie/approximate_substring_set.h>

using namespace std;
using namespace dlib;
using namespace mitie;

// ----------------------------------------------------------------------------------------

inline void populate_substr (
    const std::string& str,
    std::string& substr,
    long idx,
    unsigned long len
)
/*!
    ensures
        - extracts the substring of str which starts at str[idx] and is of length
          len and stores it into substr.  If the substring is a prefix or suffix
          then the * character is prepended or appended to the substring so that
          prefix and suffix substrings are identifiable.
!*/
{
    substr.clear();

    if (idx == 0)
        substr += '*';

    for (unsigned long i = idx; i < idx+len && i < str.size(); ++i)
        substr += str[i];

    if (idx+len == str.size())
        substr += '*';
}

// ----------------------------------------------------------------------------------------

void get_most_common_substrings (
    const std::map<std::string, unsigned long>& words,
    std::vector<std::pair<unsigned long, std::string> >& substrs,
    const unsigned long max_num_parts 
)
/*!
    ensures
        - #substrs.size() <= max_num_parts
        - for all valid i:
            - #substrs[i].first == the number of times the substring #substrs[i].second was observed.
!*/
{
    substrs.clear();
    mitie::count_min_sketch counts(10000000);

    // Note that we use * to denote the start or end of a string.

    std::string temp;
    for (std::map<std::string, unsigned long>::const_iterator i = words.begin(); i != words.end(); ++i)
    {
        for (unsigned long k = 0; k < i->first.size(); ++k)
        {
            for (unsigned long l = 1; l <= 5; ++l)
            {
                if (k+l > i->first.size())
                    break;

                populate_substr(i->first, temp, k, l);
                if (temp.size() <= 1)
                    continue;

                counts.increment(temp);
            }
        }
    }

    dlib::set<std::string>::kernel_1a cur_words;

    std::priority_queue<std::pair<long,std::string> > best_parts;
    for (std::map<std::string, unsigned long>::const_iterator i = words.begin(); i != words.end(); ++i)
    {
        for (unsigned long k = 0; k < i->first.size(); ++k)
        {
            for (unsigned long l = 1; l <= 5; ++l)
            {
                if (k+l > i->first.size())
                    break;

                populate_substr(i->first, temp, k, l);
                if (temp.size() <= 1)
                    continue;

                if (cur_words.is_member(temp))
                    continue;

                const long hits = counts.get_count(temp);
                if (best_parts.size() < max_num_parts || hits > -best_parts.top().first)
                {
                    if (best_parts.size() >= max_num_parts)
                    {
                        cur_words.destroy(best_parts.top().second);
                        best_parts.pop();
                    }

                    best_parts.push(std::make_pair(-hits, temp));
                    cur_words.add(temp);
                }
            }
        }
    }

    while (best_parts.size() != 0)
    {
        substrs.push_back(make_pair(-best_parts.top().first, best_parts.top().second));
        best_parts.pop();
    }

}

// ----------------------------------------------------------------------------------------

void measure_substring_set_approximation_error(
    const std::vector<std::pair<unsigned long,std::string> >& substrs,
    const std::vector<dlib::uint16>& substr_ids,
    const approximate_substring_set& substrs_set
)
{
    int num_hits = 0;
    int num_misses = 0;
    std::string temp;
    std::vector<uint16> hits;
    for (unsigned long i = 0; i < substrs.size(); ++i)
    {
        temp.clear();
        if (substrs[i].second[0] != '*')
            temp += "_";

        for (unsigned long j = 0; j < substrs[i].second.size(); ++j)
        {
            if (substrs[i].second[j] != '*')
                temp += substrs[i].second[j];
        }

        if (substrs[i].second[substrs[i].second.size()-1] != '*')
            temp += "_";


        substrs_set.find_substrings(temp, hits);
        if (std::find(hits.begin(), hits.end(), substr_ids[i]) != hits.end())
        {
            ++num_hits;
            cout << "hit:  " << temp << endl;
        }
        else
        {
            ++num_misses;
            cout << "miss: " << temp << endl;
        }
    }

    cout << "num_hits: "<< num_hits << endl;
    cout << "num_misses: "<< num_misses << endl;
}

void basic_morph(const command_line_parser& parser)
{
    std::map<std::string, unsigned long> words;
    ifstream fin("top_word_counts.dat", ios::binary);
    deserialize(words, fin);
    cout << "num words: "<< words.size() << endl;

    std::vector<std::pair<unsigned long,std::string> > substrs;
    get_most_common_substrings(words, substrs, 20000);
    // sort the substrings so the least frequently used ones come first.  This way, when we
    // use the approximate substring set object it will most accurately represent the more
    // frequent substrings.
    std::sort(substrs.begin(), substrs.end());

    // We normally don't look for one character patterns, but as a special case we will
    // always look to see if a string has a single number anywhere in it.
    substrs.push_back(make_pair(1, "#"));

    ofstream fout_log("substrings.txt");
    approximate_substring_set substrs_set;
    std::vector<dlib::uint16> substr_ids;
    for (unsigned long i = 0; i < substrs.size(); ++i)
    {
        substr_ids.push_back(substrs_set.add_substring(substrs[i].second));
        fout_log << substrs[i].first << " \t" << substrs[i].second << "\n";
    }


    //measure_substring_set_approximation_error(substrs, substr_ids, substrs_set);

    ofstream fout("substring_set.dat", ios::binary);
    serialize(substrs_set, fout);
}

// ----------------------------------------------------------------------------------------

