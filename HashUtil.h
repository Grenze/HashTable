//
// Created by lingo on 18-12-15.
//

#ifndef CUCKOOHASH_HASHUTIL_H
#define CUCKOOHASH_HASHUTIL_H

#include <cstdint>


// The FALLTHROUGH_INTENDED macro can be used to annotate implicit fall-through
// between switch labels. The real definition should be provided externally.
// This one is a fallback version for unsupported compilers.
#ifndef FALLTHROUGH_INTENDED
#define FALLTHROUGH_INTENDED do { } while (0)
#endif

namespace CuckooHash {
class HashUtil {
    public:
        static uint64_t MurmurHash64A(const void *key, int len, unsigned int seed);

    };
} // namespace CuckooHash


#endif //CUCKOOHASH_HASHUTIL_H
