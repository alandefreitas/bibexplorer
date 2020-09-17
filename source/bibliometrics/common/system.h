//
// Created by Alan Freitas on 17/08/20.
//

#ifndef BIBLIOMETRICS_SYSTEM_H
#define BIBLIOMETRICS_SYSTEM_H

namespace bibliometrics {
    constexpr bool is_debug() {
#ifdef NDEBUG
        return false;
#else
        return true;
#endif
    }
}

#endif //BIBLIOMETRICS_SYSTEM_H
