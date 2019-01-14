//
// Created by lingo on 18-12-16.
//

#ifndef CUCKOOHASH_HASHTABLE_H
#define CUCKOOHASH_HASHTABLE_H

#include <cstring>
#include <assert.h>

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

// Hash Table providing methods of Add, Delete, Find.
// It takes three template parameters:
//  PointerType: type of pointer points to the location of data
//  TableType: the storage of table, BaseTable by default
template <typename ItemType, size_t bits_per_tag, size_t bits_per_slot,
          template <size_t, size_t, size_t> class TableType = BaseTable>
class HashTable {
private:
    // maximum number of cuckoo kicks before claiming failure
    static const size_t kMaxCuckooKickCount = 500;
    static const size_t moveSlotToTag = (bits_per_slot - bits_per_tag);
    //assoc = slotsPerBucket
    static const size_t assoc = 4;

    // storage of pointers
    // typically 32 bits tag, 32 bits location
    TableType<bits_per_tag, bits_per_slot, assoc> *table_;

    // number of pointers stored
    size_t num_items_;

    typedef struct {
        size_t index;
        uint64_t slot;
        bool used;
    } VictimItem;

    VictimItem victim_;

    inline size_t IndexHash(uint32_t hv) const {
        // table_->num_buckets is always a power of two,
        // so modulo can be replaced with bitwise-and
        return hv & (table_->NumBuckets() - 1);
    }

    inline uint32_t TagHash(uint32_t hv) const {
        uint32_t tag;
        tag = hv & (table_->TagMask());
        tag += (tag == 0);
        return tag;
    }

    const uint32_t cuckooMurmurSeedMultiplier = 816922183;

    inline void GenerateIndexTagHash(const ItemType &item, size_t *index,
                                     uint32_t *tag) const {
        const uint64_t hash = CuckooHash::HashUtil::MurmurHash64A((void *) &item, sizeof(item), 100 * cuckooMurmurSeedMultiplier);
        *index = IndexHash(static_cast<uint32_t>(hash >> 32));
        *tag = TagHash((uint32_t) hash);
    }

    inline size_t AltIndex(const size_t index, const uint32_t tag) const {
        return IndexHash((uint32_t) (index ^ (tag * 0x5bd1e995)));;
    }

    Status AddImpl(const size_t i, const uint32_t tag, const uint32_t location);

    // number of current inserted items;
    size_t Size() const { return num_items_; }

    // load factor is the fraction of occupancy
    double LoadFactor() const { return 1.0 * Size() / table_->SizeInSlots(); }

    double BitsPerItem() const { return 8.0 * table_->SizeInBytes() / Size(); }

public:
    explicit HashTable(const size_t max_num_keys) : num_items_(0), victim_() {

        //table_->num_buckets is always a power of two greater than max_num_keys
        size_t num_buckets = upperpower2(std::max<uint64_t>(1, max_num_keys / assoc));
        //check if max_num_keys/(num_buckets*assoc) <= 0.96
        //if not double num_buckets
        double frac = (double) max_num_keys / num_buckets / assoc;
        if (frac > 0.96) {
            num_buckets <<= 1;
        }
        victim_.used = false;
        table_ = new TableType<bits_per_tag, bits_per_slot, 4>(num_buckets);
    }

    ~HashTable() { delete table_; }

    // Add an item to the filter.
    Status Add(const ItemType &item, const uint32_t location);

    // Report if the item is inserted, with false positive rate.
    Status Find(const ItemType &key, uint32_t *location) const;

    // Delete an key from the filter
    Status Delete(const ItemType &item);

    /* methods for providing stats  */
    // summary information
    std::string Info() const;

    // size of the filter in bytes.
    size_t SizeInBytes() const { return table_->SizeInBytes(); }
};


template <typename ItemType, size_t bits_per_tag, size_t bits_per_slot,
        template <size_t, size_t, size_t > class TableType>
Status HashTable<ItemType, bits_per_tag, bits_per_slot, TableType>::Add(
        const ItemType &item, const uint32_t location) {
    size_t i;
    uint32_t tag;

    if (victim_.used) {
        return NotEnoughSpace;
    }

    GenerateIndexTagHash(item, &i, &tag);
    return AddImpl(i, tag, location);
}

template <typename ItemType, size_t bits_per_tag, size_t bits_per_slot,
        template <size_t, size_t, size_t > class TableType>
Status HashTable<ItemType, bits_per_tag, bits_per_slot, TableType>::AddImpl(
        const size_t i, const uint32_t tag, const uint32_t location) {
    size_t curindex = i;
    uint64_t curslot = tag;
    curslot = (curslot << moveSlotToTag) + location;
    uint64_t oldslot;

    for (uint32_t count = 0; count < kMaxCuckooKickCount; count++) {
        bool kickout = count > 0;
        oldslot = 0;
        if (table_->InsertSlotToBucket(curindex, curslot, kickout, oldslot)) {
            num_items_++;
            return Ok;
        }
        if (kickout) {
            curslot = oldslot;
        }
        //beign kick out after both index tried
        curindex = AltIndex(curindex, curslot >> moveSlotToTag);
    }

    victim_.index = curindex;
    victim_.slot = curslot;
    victim_.used = true;
    num_items_++;
    return Ok;
}

template <typename ItemType, size_t bits_per_tag, size_t bits_per_slot,
        template <size_t, size_t, size_t > class TableType>
Status HashTable<ItemType, bits_per_tag, bits_per_slot, TableType>::Find(
        const ItemType &key, uint32_t *location) const {//location &l passed in
    bool found = false;
    size_t i1, i2;
    uint32_t tag;

    GenerateIndexTagHash(key, &i1, &tag);
    i2 = AltIndex(i1, tag);

    assert(i1 == AltIndex(i2, tag));

    uint64_t slot = 0;
    slot = victim_.slot;
    found = victim_.used && (tag == slot >> moveSlotToTag) &&
            (i1 == victim_.index || i2 == victim_.index);

    if (found) {
        *location  = static_cast<uint32_t>(slot);
        return Ok;
    }

    slot = table_->FindSlotInBuckets(i1, i2, tag);
    *location = static_cast<uint32_t >(slot);
    if (slot != -1) {
        return Ok;
    } else {
        return NotFound;
    }
}

template <typename ItemType, size_t bits_per_tag, size_t bits_per_slot,
        template <size_t, size_t, size_t > class TableType>
Status HashTable<ItemType, bits_per_tag, bits_per_slot, TableType>::Delete(
        const ItemType &key) {
    size_t i1, i2;
    uint32_t tag;

    GenerateIndexTagHash(key, &i1, &tag);
    i2 = AltIndex(i1, tag);

    if (table_->DeleteSlotFromBucket(i1, tag)) {
        num_items_--;
        goto TryEliminateVictim;
    } else if (table_->DeleteSlotFromBucket(i2, tag)) {
        num_items_--;
        goto TryEliminateVictim;
    } else if (victim_.used && (tag == victim_.slot >> moveSlotToTag) &&
               (i1 == victim_.index || i2 == victim_.index)) {
        num_items_--;
        victim_.used = false;
        return Ok;
    } else {
        return NotFound;
    }
    TryEliminateVictim:
    if (victim_.used) {
        num_items_--;
        victim_.used = false;
        size_t i = victim_.index;
        uint64_t slot = victim_.slot;
        auto tag1 = static_cast<uint32_t>(slot >> moveSlotToTag);
        auto position = static_cast<uint32_t>(slot);
        AddImpl(i, tag1, position);
    }
    return Ok;
}

template <typename ItemType, size_t bits_per_tag, size_t bits_per_slot,
        template <size_t, size_t, size_t > class TableType>
std::string HashTable<ItemType, bits_per_tag, bits_per_slot, TableType>::Info() const {
    std::stringstream ss;
    ss << "CuckooFilter Status:\n"
       << "\t\t" << table_->Info() << "\n"
       << "\t\tKeys stored: " << Size() << "\n"
       << "\t\tLoad factor: " << LoadFactor() << "\n"
       << "\t\tHashtable size: " << (table_->SizeInBytes() >> 10) << " KB\n";
    if (Size() > 0) {
        ss << "\t\tbit/key:   " << BitsPerItem() << "\n";
    } else {
        ss << "\t\tbit/key:   N/A\n";
    }
    return ss.str();
}
};  // namespace CuckooHash



#endif //CUCKOOHASH_HASHTABLE_H
