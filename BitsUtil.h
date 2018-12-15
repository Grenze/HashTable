//
// Created by lingo on 18-12-15.
//

#ifndef CUCKOOHASH_BITSUTIL_H
#define CUCKOOHASH_BITSUTIL_H

#include <stdint-gcc.h>

namespace CuckooHash {

    class BitsUtil {
        static inline uint64_t upperpower2(uint64_t x) {
            x--;
            x |= x >> 1;
            x |= x >> 2;
            x |= x >> 4;
            x |= x >> 8;
            x |= x >> 16;
            x |= x >> 32;
            x++;
            return x;
        }
    };  // class BitsUtil

}   // namespace CuckooHash


#endif //CUCKOOHASH_BITSUTIL_H
