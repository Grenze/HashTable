//
// Created by lingo on 18-12-16.
//

#ifndef CUCKOOHASH_HASHTABLE_H
#define CUCKOOHASH_HASHTABLE_H

#include <cstring>
#include "BaseTable.h"
#include "HashUtil.h"

namespace CuckooHash{

//status returned by cuckoo hash operation
enum Status {
    Ok = 0,
    NotFound = 1,
    NotEnoughSpace = 2,
    NotSupported = 3,
};

//Hash Table providing methods of Add, Delete, Contain.
//It takes three template parameters:
//  PointerType: type of pointer points to the location of data
//  TableType: the storage of table, BaseTable by default
template <typename PointerType,
          template <size_t, size_t > class TableType = BaseTable,
          typename HashFamily = HashUtil>
class HashTable {
private:
    //maximum number of cuckoo kicks before claiming failure
    const size_t kMaxCuckooKickCount = 500;
    //storage of pointers
    TableType<sizeof(PointerType), 4> *table_;

    //number of pointers stored
    size_t num_pointers_;

    typedef struct {
        size_t index;
        PointerType tag;//tag = pointer
        bool used;
    } VictimPointer;

    VictimPointer victim_;

    HashFamily hasher_;

    inline size_t IndexHash(uint32_t hv) const {
        // table_->num_buckets is always a power of two,
        // so modulo can be replaced with bitwise-and
        return hv & (table_->NumBuckets() - 1);
    }

    const uint32_t kCuckooMurmurSeedMultiplier = 816922183;

    inline void GenerateIndexTagHash(const PointerType& pointer, size_t* index,
                                     PointerType* tag) const {
        const uint64_t hash = CuckooHash::HashUtil::MurmurHash64A(pointer->key, pointer->keyLength, 0);
        *index = IndexHash(hash>>32);
        *tag = pointer;
    }

    inline size_t AltIndex(const PointerType tag) const {
        const uint64_t hash = CuckooHash::HashUtil::MurmurHash64A(tag->key, tag->keyLength, kCuckooMurmurSeedMultiplier);
        return IndexHash(hash>>32);
    }













};
}   // namespace CuckooHash



#endif //CUCKOOHASH_HASHTABLE_H
