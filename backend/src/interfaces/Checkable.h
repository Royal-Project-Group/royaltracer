//
// Created by nikla on 27.11.2023.
//

#ifndef ROYAL_TRACER_CHECKABLE_H
#define ROYAL_TRACER_CHECKABLE_H

#include <cassert>

namespace dbc {
    enum Mode {
        DEBUG,
        BETA,
        RELEASE
    };

    class Checkable {
    private:
        Mode _debugMode;

        // class invariants
        bool isValid();
    public:
        void setDebugMode(Mode m);
        // preconditions
        void require(bool condition);
        // postconditions
        void ensure(bool condition);
    };
}


#endif //ROYAL_TRACER_CHECKABLE_H
