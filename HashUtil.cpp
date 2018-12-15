//
// Created by lingo on 18-12-15.
//

#include "HashUtil.h"

namespace CuckooHash {


/*
  Murmurhash from http://sites.google.com/site/murmurhash/

  All code is released to the public domain. For business purposes, Murmurhash
  is under the MIT license.
*/
uint64_t HashUtil::MurmurHash64A(const void *key, int len, unsigned int seed) {

    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;

    uint64_t h = seed ^(len * m);

    const uint64_t *data = (const uint64_t *) key;
    const uint64_t *end = data + (len / 8);

    while (data != end) {
        uint64_t k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char *data2 = (const unsigned char *) data;

    switch (len & 7) {
        case 7:
            h ^= ((uint64_t) data2[6]) << 48;
            FALLTHROUGH_INTENDED;
        case 6:
            h ^= ((uint64_t) data2[5]) << 40;
            FALLTHROUGH_INTENDED;
        case 5:
            h ^= ((uint64_t) data2[4]) << 32;
            FALLTHROUGH_INTENDED;
        case 4:
            h ^= ((uint64_t) data2[3]) << 24;
            FALLTHROUGH_INTENDED;
        case 3:
            h ^= ((uint64_t) data2[2]) << 16;
            FALLTHROUGH_INTENDED;
        case 2:
            h ^= ((uint64_t) data2[1]) << 8;
            FALLTHROUGH_INTENDED;
        case 1:
            h ^= ((uint64_t) data2[0]);
            h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

}   // namespace CuckooHash