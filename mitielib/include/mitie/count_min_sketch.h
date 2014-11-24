// Copyright (C) 2012 Massachusetts Institute of Technology, Lincoln Laboratory
// License: Boost Software License   See LICENSE.txt for the full license.
// Authors: Davis E. King (davis@dlib.net)
#ifndef MIT_LL_CoUNT_MIN_SKETCH_H_
#define MIT_LL_CoUNT_MIN_SKETCH_H_

#include "dlib/array2d.h"
#include "dlib/uintn.h"
#include "dlib/hash.h"
#include "dlib/byte_orderer.h"
#include <queue>
#include "dlib/image_transforms.h"

namespace mitie
{
    class count_min_sketch
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This is an implementation of the count-min sketch data structure described
                in the paper:
                    An Improved Data Stream Summary: The Count-Min Sketch and its
                    Applications by Graham Cormode and S. Muthukrishnan

                This is a data structure used for counting how many times you see various
                objects.  It uses a fixed amount of RAM but provides only approximate
                counts.
        !*/
    public:

        count_min_sketch (
        )
        /*!
            ensures
                - #get_hash_table_size() == 1000000 
                - #get_num_hashes() == 8
                - #get_total_count() == 0
                - for all valid X:
                    - #get_count(X) == 0
        !*/
        {
            counts.set_size(8, 1000000);
            set_counts_to_zero();
        }

        count_min_sketch (
            const count_min_sketch& item
        )
        {
            total_count = item.total_count;
            assign_image(counts, item.counts);
        }

        explicit count_min_sketch (
            unsigned int hash_table_size
        )
        /*!
            requires
                - hash_table_size > 0
            ensures
                - #get_hash_table_size() == hash_table_size
                - #get_num_hashes() == 8
                - #get_total_count() == 0
                - for all valid X:
                    - #get_count(X) == 0
        !*/
        {
            DLIB_CASSERT(hash_table_size > 0, "Invalid inputs were given to this function");

            counts.set_size(8, hash_table_size);
            set_counts_to_zero();
        }

        count_min_sketch (
            unsigned int hash_table_size,
            unsigned int num_hashes
        )
        /*!
            requires
                - hash_table_size > 0
                - num_hashes > 0
            ensures
                - #get_hash_table_size() == hash_table_size
                - #get_num_hashes() == num_hashes 
                - #get_total_count() == 0
                - for all valid X:
                    - #get_count(X) == 0
        !*/
        {
            DLIB_CASSERT(hash_table_size > 0 && num_hashes > 0, "Invalid inputs were given to this function");

            counts.set_size(num_hashes, hash_table_size);
            set_counts_to_zero();
        }

        void set_counts_to_zero (
        )
        /*!
            ensures
                - #get_total_count() == 0
                - for all valid X:
                    - #get_count(X) == 0
        !*/
        {
            for (long r = 0; r < counts.nr(); ++r)
            {
                for (long c = 0; c < counts.nc(); ++c)
                {
                    counts[r][c] = 0;
                }
            }
            total_count = 0;
        }

        dlib::uint64 get_total_count(
        ) const { return total_count; }
        /*!
            ensures
                - returns the total of all values added into this object via the
                  increment() member function.
        !*/

        unsigned int get_num_hashes (
        ) const { return counts.nr(); }
        /*!
            ensures
                - returns the number of hash tables used in this count min sketch object.
                  Note that this is the 'd' parameter from the count min sketch paper.
        !*/

        unsigned int get_hash_table_size (
        ) const { return counts.nc(); }
        /*!
            ensures
                - returns the number of elements in each of the get_num_hashes() hash
                  tables.  Note that this is the 'w' parameter from the count min sketch
                  paper.
        !*/

        template <typename T>
        void increment (
            const T& item,
            unsigned long amount = 1
        )
        /*!
            ensures
                - increments the count for item.  Note that the count min sketch data
                  structure maintains only approximate counts.  But the idea is that it
                  attempts to perform the following:
                    - #get_count(item) == get_count(item) + amount 
                - #total_count() == total_count() + amount
        !*/
        {
            using namespace dlib;
            for (long r = 0; r < counts.nr(); ++r)
            {
                const dlib::uint32 h = dlib::hash(item,(dlib::uint32)r)%counts.nc();
                counts[r][h] += amount;
            }
            total_count += amount;
        }

        template <typename T>
        dlib::uint64 get_count (
            const T& item
        ) const
        /*!
            ensures
                - returns the current count for the given item.  Note that this count is
                  approximate, however, it is always at least as large as the actual count
                  for this item.
        !*/
        {
            using namespace dlib;
            uint64 val = std::numeric_limits<uint64>::max();

            for (long r = 0; r < counts.nr(); ++r)
            {
                const dlib::uint32 h = dlib::hash(item,(dlib::uint32)r)%counts.nc();
                if ( counts[r][h] < val)
                    val = counts[r][h];
            }

            return val;
        }

        dlib::uint64 get_count_at_top_n (
            unsigned long n
        ) const 
        /*!
            requires
                - n > 0
            ensures
                - returns an estimate of the count of the n-th most frequently occurring
                  item.
        !*/
        {
            DLIB_CASSERT(n > 0, "Invalid inputs were given to this function");

            dlib::uint64 thresh = std::numeric_limits<dlib::uint64>::max();

            // figure out the n-th threshold for each hash table
            for (long r = 0; r < counts.nr(); ++r)
            {
                std::priority_queue<dlib::int64> top_counts;
                for (long c = 0; c < counts.nc(); ++c)
                {
                    top_counts.push(-static_cast<dlib::int64>(counts[r][c]));
                    if (top_counts.size() > n)
                        top_counts.pop();
                }

                thresh = std::min(thresh, static_cast<dlib::uint64>(-top_counts.top()));
            }

            return thresh;
        }

        friend void serialize (const count_min_sketch& item, std::ostream& out)
        /*!
            ensures
                - serializes item to the given output stream
        !*/
        {
            int version = 1;
            dlib::serialize(version, out);
            dlib::serialize(item.counts.nr(), out);
            dlib::serialize(item.counts.nc(), out);
            if (item.counts.size() != 0)
            {
                dlib::byte_orderer bo;
                if (bo.host_is_big_endian())
                {
                    dlib::uint64* temp = const_cast<dlib::uint64*>(&item.counts[0][0]);
                    for (unsigned long i = 0; i < item.counts.size(); ++i)
                        bo.host_to_little(temp[i]);
                }

                out.write((char*)&item.counts[0][0], item.counts.size()*sizeof(item.counts[0][0]));

                // swap back to normal if necessary 
                if (bo.host_is_big_endian())
                {
                    dlib::uint64* temp = const_cast<dlib::uint64*>(&item.counts[0][0]);
                    for (unsigned long i = 0; i < item.counts.size(); ++i)
                        bo.little_to_host(temp[i]);
                }
            }
            dlib::serialize(item.total_count, out);
        }

        friend void deserialize (count_min_sketch& item, std::istream& in)
        /*!
            ensures
                - deserializes item from the given input stream
        !*/
        {
            int version = 0;
            dlib::deserialize(version, in);
            if (version != 1)
                throw dlib::serialization_error("Wrong version found while deserializing a mitie::count_min_sketch object.");

            long nr, nc;
            dlib::deserialize(nr, in);
            dlib::deserialize(nc, in);
            item.counts.set_size(nr,nc);
            if (item.counts.size() != 0)
            {
                in.read((char*)&item.counts[0][0], item.counts.size()*sizeof(item.counts[0][0]));

                dlib::byte_orderer bo;
                if (bo.host_is_big_endian())
                {
                    dlib::uint64* temp = const_cast<dlib::uint64*>(&item.counts[0][0]);
                    for (unsigned long i = 0; i < item.counts.size(); ++i)
                        bo.little_to_host(temp[i]);
                }
            }
            dlib::deserialize(item.total_count, in);
        }

        count_min_sketch& operator= (
            const count_min_sketch& item
        )
        {
            count_min_sketch(item).swap(*this);
            return *this;
        }

        void swap (count_min_sketch& item)
        /*!
            ensures
                - swaps *this with item
        !*/
        {
            counts.swap(item.counts);
            std::swap(total_count, item.total_count);
        }

        void absorb (
            const count_min_sketch& item
        )
        /*!
            requires
                - get_hash_table_size() == item.get_hash_table_size() 
                - get_num_hashes() == item.get_num_hashes()
            ensures
                - Absorbs all the count data from item into *this.  That is, it will be as
                  if all the calls to increment() which have been applied to item were also
                  applied to this*.  In particular, this means that the following will be
                  true:
                    - #total_count() == total_count() + item.total_count()
                    - for all items:
                        - #get_count(obj) == get_count(obj) + item.get_count(obj)
        !*/
        {
            // make sure requires clause is not broken
            DLIB_CASSERT(get_hash_table_size() == item.get_hash_table_size() &&
                         get_num_hashes() == item.get_num_hashes(), "Invalid inputs were given to this function"
                         << "\n\t get_hash_table_size():      " << get_hash_table_size() 
                         << "\n\t item.get_hash_table_size(): " << item.get_hash_table_size() 
                         << "\n\t get_num_hashes():           " << get_num_hashes() 
                         << "\n\t item.get_num_hashes():      " << item.get_num_hashes()  );

            total_count += item.total_count;
            for (long r = 0; r < counts.nr(); ++r)
            {
                for (long c = 0; c < counts.nc(); ++c)
                {
                    counts[r][c] += item.counts[r][c];
                }
            }
        }

    private:

        dlib::array2d<dlib::uint64> counts;
        dlib::uint64 total_count;
    };

    inline void swap (
        count_min_sketch& a, 
        count_min_sketch& b
    )
    /*!
        ensures
            - swaps the state of a and b
    !*/
    {
        a.swap(b);
    }
}

#endif // MIT_LL_CoUNT_MIN_SKETCH_H_

