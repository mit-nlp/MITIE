// Copyright (C) 2013 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_APPROXIMATE_SUBSTRiNG_SET_H_
#define MIT_LL_APPROXIMATE_SUBSTRiNG_SET_H_

#include <iostream>
#include <dlib/uintn.h>
#include <dlib/serialize.h>
#include <vector>

using namespace std;

// ----------------------------------------------------------------------------------------

namespace mitie
{
    class approximate_substring_set
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object is a tool for finding the substrings of a query string.  In
                particular, it stores a set of substrings and then you can ask it if a new
                string contains any of your substrings.  It will also tell you which
                substrings it finds.

                Importantly, this object gives only approximate answers because it uses a
                simple, but very fast hash table implementation which allows hash
                collisions.   In general, it will only accurately be able to store about
                8000 substrings in its substring search set.  Adding more than this by
                calling add_substring() will begin overwriting previous substrings.
        !*/

    public:

        approximate_substring_set(
        ) :
            mask(0x1FFF),
            mask_bits(13), // how many bits are set to 1 in mask
            init_hash(0xFFFFFFFF),
            max_substr_len(0)
        {
            fill_crc_table();
            hash_table.resize(mask+1);
        }

        dlib::uint16 max_substring_id (
        ) const { return hash_table.size()-1; }
        /*!
            ensures
                - Returns the max possible value that a substring ID can take.  These are
                  the values returned by add_substring() and placed into the hits vectors
                  by find_substrings().
        !*/

        dlib::uint16 add_substring (
            const std::string& str
        )
        /*!
            ensures
                - Inserts the given string into this object.  This means that subsequent
                  calls to find_substrings() will search the given string to see if str is
                  one of its substrings.
                - The substring ID value given to str is returned.  This is the value which
                  will appear in the hits vector output by find_substrings() when str is
                  found in a string.  It is also always <= max_substring_id().
                - This object stores the substring database in an approximate hash table
                  structure.  This means that calls to add_substring() have a small chance
                  of removing substrings previously added by add_substring().  So when
                  populating this object with substrings you should give the most important
                  substrings you wish to find last. 
                - You can use the '*' character to denote the beginning or end of a string.
                  So for example, if you want your substring to match only words ending in
                  "ed" then you can set str to "ed*"
        !*/
        {
            dlib::uint32 h = init_hash;
            for (unsigned long i = 0; i < str.size(); ++i)
                add_to_hash(h, str[i]);

            if (str.size() > max_substr_len)
                max_substr_len = str.size();

            const dlib::uint16 str_id = static_cast<dlib::uint16>(h>>mask_bits);
            const dlib::uint16 bucket_id = static_cast<dlib::uint16>(h&mask);
            hash_table[bucket_id] = str_id;
            return bucket_id;
        }

        void find_substrings (
            const char* begin,
            const char* end,
            std::vector<dlib::uint16>& hits
        ) const
        /*!
            requires
                - begin <= end
            ensures
                - #hits.size() == The number of substrings of the string contained in the
                  half open range [begin, end) which match one of the strings given to this
                  object's add_substring() method.
                - #hits == The set of substring ID values which were found in the input string.
                - All elements of #hits are <= max_substring_id()
        !*/
        {
            const int max_len = 50;
            dlib::uint32 hashes[max_len];
            // We are only going to look at the first max_len-1 characters in the string.
            // If it's longer than that then too bad, it won't be part of the hash.
            end = std::min(end, begin + max_len-1);
            hits.clear();

            if (begin == end)
                return;

            // initialize hashes
            dlib::uint32* ptr = hashes;
            *ptr++ = init_hash;
            for (const char* i = begin; i < end; ++i)
                *ptr++ = init_hash;
            // The first hash bucket is at the front of the string so indicate that it
            // starts with the special '*' string ending marker.
            add_to_hash(hashes[0], '*');


            for (unsigned int iter = 0; iter < max_substr_len && begin<end; ++iter)
            {
                ptr = hashes;
                add_to_hash(*ptr, *begin);
                add_hash_if_in_table(*ptr++, hits);
                for (const char* i = begin; i < end; ++i)
                {
                    add_to_hash(*ptr, *i);
                    add_hash_if_in_table(*ptr++, hits);
                }
                ++begin;

                dlib::uint32 end_hash = *(ptr-1);
                add_to_hash(end_hash, '*');
                add_hash_if_in_table(end_hash, hits);
            }
        }

        void find_substrings (
            const std::string& str,
            std::vector<dlib::uint16>& hits
        ) const
        /*!
            ensures
                - This function is identical to the above find_substrings() routine except
                  that it takes its input string as a std::string instead of an iterator
                  range.
        !*/
        {
            if (str.size() != 0)
            {
                find_substrings(&str[0], &str[0] + str.size(), hits);
            }
            else
            {
                hits.clear();
            }
        }

        friend void serialize(const approximate_substring_set& item, std::ostream& out)
        {
            int version = 1;
            dlib::serialize(version, out); 
            dlib::serialize(item.mask, out); 
            dlib::serialize(item.mask_bits, out); 
            dlib::serialize(item.init_hash, out); 
            dlib::serialize(item.max_substr_len, out); 
            dlib::serialize(item.hash_table, out); 
            dlib::serialize(item.crc_table, out); 
        }

        friend void deserialize(approximate_substring_set& item, std::istream& in)
        {
            int version = 0;
            dlib::deserialize(version, in); 
            if (version != 1)
                throw dlib::serialization_error("Unexpected version found while deserializing mitie::approximate_substring_set");
            dlib::deserialize(const_cast<dlib::uint32&>(item.mask), in); 
            dlib::deserialize(const_cast<dlib::uint32&>(item.mask_bits), in); 
            dlib::deserialize(const_cast<dlib::uint32&>(item.init_hash), in); 
            dlib::deserialize(item.max_substr_len, in); 
            dlib::deserialize(item.hash_table, in); 
            dlib::deserialize(item.crc_table, in); 
        }

    // ------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------
    //                                  IMPLEMENTATION DETAILS
    // ------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------

    private:

        /*!
            We are going to use a crc32 hash for hashing substrings.  The crc_table object stores
            some numbers needed as part of crc32 hash calculation and the hash_table vector is what
            records what substrings are in our object at any given moment.
        !*/

        const dlib::uint32 mask;
        const dlib::uint32 mask_bits;
        const dlib::uint32 init_hash;
        unsigned int max_substr_len;
        std::vector<dlib::uint16> hash_table;
        std::vector<dlib::uint32> crc_table;

        inline void add_hash_if_in_table (
            const dlib::uint32 hash,
            std::vector<dlib::uint16>& hits
        ) const
        {
            const dlib::uint16 str_id = static_cast<dlib::uint16>(hash>>mask_bits);
            const dlib::uint16 bucket_id = static_cast<dlib::uint16>(hash&mask);
            if (hash_table[bucket_id] == str_id)
                hits.push_back(bucket_id);
        }

        inline void add_to_hash (
            dlib::uint32& hash,
            unsigned char item
        ) const
        /*!
            ensures
                - incorporates item into the hash value in hash.
        !*/
        {
            hash = (hash>>8) ^ crc_table[(hash^item) & 0xFF];
        }

        void fill_crc_table (
        )
        {
            dlib::uint32 temp;

            crc_table.resize(256);
            // fill out crc_table
            for (dlib::uint32 i = 0; i < 256; ++i)
            {
                temp = i;
                for (dlib::uint32 j = 0; j < 8; ++j)
                {
                    if (temp&1)
                        temp = (temp>>1)^0xedb88320;
                    else
                        temp >>= 1;
                }
                crc_table[i] = temp;
            }
        }
    };
}

// ----------------------------------------------------------------------------------------

#endif // MIT_LL_APPROXIMATE_SUBSTRiNG_SET_H_

