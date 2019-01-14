//
// Created by lingo on 18-12-15.
//

#ifndef CUCKOOHASH_BASETABLE_H
#define CUCKOOHASH_BASETABLE_H

#include <cstring>
#include <cstdint>
#include <sstream>

#include "BitsUtil.h"

//Generally every bucket contains slots with the number of associativity,
//slot contains key's hash tag and location of key-value pair
namespace CuckooHash{
    template <size_t bits_per_tag, size_t bits_per_slot, size_t associativity>
    class BaseTable {
        static const size_t slotsPerBucket = associativity;
        //plus 7 to make sure there is enough space per Bucket to store slots
        //when bits_per_slot = 1
        static const size_t bytesPerBucket =
                (bits_per_slot * slotsPerBucket + 7) >> 3;
        static const uint32_t tagMask = static_cast<const uint32_t>((1ULL << bits_per_tag) - 1);
        static const size_t moveSlotToTag = (bits_per_slot - bits_per_tag);
        static const size_t paddingBuckets =
                ((((bytesPerBucket + 7) / 8) * 8) - 1) / bytesPerBucket;

        struct Bucket {
            char bits_[bytesPerBucket];
        } __attribute__((__packed__));

        // using a pointer adds one more indirection
        Bucket *buckets_;
        size_t num_buckets_;

    public:
        explicit BaseTable(const size_t num) : num_buckets_(num) {
            //cout<<bytesPerBucket<<" "<<paddingBuckets<<" "<<num_buckets_<<" "<<endl;
            buckets_ = new Bucket[num_buckets_ + paddingBuckets];
            memset(buckets_, 0, bytesPerBucket * (num_buckets_ + paddingBuckets));
        }

        ~BaseTable() {
            delete[] buckets_;
        }

        size_t NumBuckets() const {
            return num_buckets_;
        }

        uint32_t TagMask() const{
            return tagMask;
        }

        size_t SizeInBytes() const {
            return bytesPerBucket * num_buckets_;
        }

        size_t SizeInSlots() const {
            return slotsPerBucket * num_buckets_;
        }

        std::string Info() const {
            std::stringstream ss;
            ss << "BaseTable with tag size and slot size: " << bits_per_tag << " / " << bits_per_slot << " bits \n";
            ss << "\t\tAssociativity: " << slotsPerBucket << "\n";
            ss << "\t\tTotal # of rows: " << num_buckets_ << "\n";
            ss << "\t\tTotal # slots: " << SizeInSlots() << "\n";
            return ss.str();
        }

        // read slot from pos(i,j)
        inline uint64_t ReadSlot(const size_t i, const size_t j) const {
            const char *p = buckets_[i].bits_;
            uint64_t slot = 0;
            /* following code only works for little-endian */
            if (bits_per_slot == 2) {
                slot = *((uint8_t *)p) >> (j * 2);
            } else if (bits_per_slot == 4) {
                p += (j >> 1);
                slot = *((uint8_t *)p) >> ((j & 1) << 2);
            } else if (bits_per_slot == 8) {
                p += j;
                slot = *((uint8_t *)p);
            } else if (bits_per_slot == 12) {
                p += j + (j >> 1);
                slot = *((uint16_t *)p) >> ((j & 1) << 2);
            } else if (bits_per_slot == 16) {
                p += (j << 1);
                slot = *((uint16_t *)p);
            } else if (bits_per_slot == 32) {
                slot = ((uint32_t *)p)[j];
            } else if (bits_per_slot == 64) {
                slot = ((uint64_t *)p)[j];
            }
            return slot;
        }

        inline uint64_t ReadTag(const size_t i, const size_t j) const {
            return ReadSlot(i, j) >> moveSlotToTag;
        }

        // write slot to pos(i,j)
        inline void WriteSlot(const size_t i, const size_t j, const uint64_t s) {
            char *p = buckets_[i].bits_;
            uint64_t slot = s;
            /* following code only works for little-endian */
            if (bits_per_slot == 2) {
                *((uint8_t *)p) |= slot << (2 * j);
            } else if (bits_per_slot == 4) {
                p += (j >> 1);
                if ((j & 1) == 0) {
                    *((uint8_t *)p) &= 0xf0;
                    *((uint8_t *)p) |= slot;
                } else {
                    *((uint8_t *)p) &= 0x0f;
                    *((uint8_t *)p) |= (slot << 4);
                }
            } else if (bits_per_slot == 8) {
                ((uint8_t *)p)[j] = static_cast<uint8_t>(slot);
            } else if (bits_per_slot == 12) {
                p += (j + (j >> 1));
                if ((j & 1) == 0) {
                    ((uint16_t *)p)[0] &= 0xf000;
                    ((uint16_t *)p)[0] |= slot;
                } else {
                    ((uint16_t *)p)[0] &= 0x000f;
                    ((uint16_t *)p)[0] |= (slot << 4);
                }
            } else if (bits_per_slot == 16) {
                ((uint16_t *)p)[j] = static_cast<uint16_t>(slot);
            } else if (bits_per_slot == 32) {
                ((uint32_t *)p)[j] = static_cast<uint32_t>(slot);
            } else if (bits_per_slot == 64){
                ((uint64_t *)p)[j] = slot;
            }
        }

        // find slot with specific tag in buckets
        inline uint64_t FindSlotInBuckets(const size_t i1, const size_t i2,
                                     const uint32_t tag) const {
            const char *p1 = buckets_[i1].bits_;
            const char *p2 = buckets_[i2].bits_;

            uint64_t slot1 = 0;
            for (size_t j = 0; j < slotsPerBucket; j++) {
                slot1 = ReadSlot(i1, j);
                if (slot1 >> moveSlotToTag == tag){
                    return slot1;
                }
                slot1 = ReadSlot(i2, j);
                if(slot1 >> moveSlotToTag == tag) {
                    return slot1;
                }
            }
            return static_cast<uint64_t >(-1);

        }

        inline bool FindTagInBucket(const size_t i, const uint32_t tag) const {
            // caution: unaligned access & assuming little endian
            if (bits_per_slot == 4 && slotsPerBucket == 4) {
                const char *p = buckets_[i].bits_;
                uint64_t v = *(uint64_t *)p;  // uint16_t may suffice
                return hasvalue4(v, tag);
            } else if (bits_per_slot == 8 && slotsPerBucket == 4) {
                const char *p = buckets_[i].bits_;
                uint64_t v = *(uint64_t *)p;  // uint32_t may suffice
                return hasvalue8(v, tag);
            } else if (bits_per_slot == 12 && slotsPerBucket == 4) {
                const char *p = buckets_[i].bits_;
                uint64_t v = *(uint64_t *)p;
                return hasvalue12(v, tag);
            } else if (bits_per_slot == 16 && slotsPerBucket == 4) {
                const char *p = buckets_[i].bits_;
                uint64_t v = *(uint64_t *)p;
                return hasvalue16(v, tag);
            } else {
                for (size_t j = 0; j < slotsPerBucket; j++) {
                    if (ReadTag(i, j) == tag) {
                        return true;
                    }
                }
                return false;
            }
        }


        //delete slot with specific tag from bucket
        inline bool DeleteSlotFromBucket(const size_t i, const uint32_t tag) {
            for (size_t j = 0; j < slotsPerBucket; j++) {
                if (ReadTag(i, j) == tag) {
                    assert(FindTagInBucket(i, tag));
                    WriteSlot(i, j, 0);
                    return true;
                }
            }
            return false;
        }

        inline bool InsertSlotToBucket(const size_t i, const uint64_t slot,
                                      const bool kickout, uint64_t &oldslot) {
            for (size_t j = 0; j < slotsPerBucket; j++) {
                if (ReadSlot(i, j) == 0) {
                    WriteSlot(i, j, slot);
                    return true;
                }
            }
            if (kickout) {
                size_t r = rand() & (slotsPerBucket - 1);
                oldslot = ReadSlot(i, r);
                WriteSlot(i, r, slot);
            }
            return false;
        }

        inline size_t NumSlotsInBucket(const size_t i) const {
            size_t num = 0;
            for (size_t j = 0; j < slotsPerBucket; j++) {
                if (ReadSlot(i, j) != 0) {
                    num++;
                }
            }
            return num;
        }
    };  // class BaseTable
}   // namespace CuckooHash



#endif //CUCKOOHASH_BASETABLE_H
