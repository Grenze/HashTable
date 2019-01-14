//
// Created by lingo on 19-1-14.
//

#include "timing.h"

#include <cstdint>
#include <chrono>

::std::uint64_t NowNanos() {
    return ::std::chrono::duration_cast<::std::chrono::nanoseconds>(
            ::std::chrono::steady_clock::now().time_since_epoch())
            .count();
}
