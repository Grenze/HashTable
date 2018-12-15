//
// Created by lingo on 18-12-15.
//

#ifndef CUCKOOHASH_PRINTUTIL_H
#define CUCKOOHASH_PRINTUTIL_H

#include <string>

namespace CuckooHash {
class PrintUtil {
public:
    static std::string bytes_to_hex(const char *data, size_t len) {
        std::string hexstr = "";
        static const char hexes[] = "0123456789ABCDEF ";

        for (size_t i = 0; i < len; i++) {
            unsigned char c = data[i];
            hexstr.push_back(hexes[c >> 4]);
            hexstr.push_back(hexes[c & 0xf]);
            hexstr.push_back(hexes[16]);
        }
        return hexstr;
    }

    static std::string bytes_to_hex(const std::string &s) {
        return bytes_to_hex((const char *)s.data(), s.size());
    }

};  // class PrintUtil
}   // namespace Cuckoo Hash

#endif //CUCKOOHASH_PRINTUTIL_H
