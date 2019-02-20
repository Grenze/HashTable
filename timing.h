//
// Created by lingo on 19-1-14.
//

#ifndef CUCKOOHASH_TIMING_H
#define CUCKOOHASH_TIMING_H

#include <cstdint>
#include <chrono>

::std::uint64_t NowNanos() {
    return static_cast<uint64_t>(::std::chrono::duration_cast<::std::chrono::nanoseconds>(
                ::std::chrono::steady_clock::now().time_since_epoch())
                .count());
}


#endif //CUCKOOHASH_TIMING_H
