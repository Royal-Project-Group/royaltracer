//
// Created by nikla on 27.11.2023.
//

#include "Checkable.h"

void dbc::Checkable::setDebugMode(dbc::Mode m) {
    _debugMode = m;
}

bool dbc::Checkable::isValid() {
    return true;
}

void dbc::Checkable::require(bool condition) {
    if(_debugMode == DEBUG || _debugMode == BETA) {
        // check class invariants
        assert(isValid());
        // check preconditions
        assert(condition);
    }
}

void dbc::Checkable::ensure(bool condition) {
    if(_debugMode == DEBUG) {
        // check class invariants
        assert(isValid());
        // check postconditions
        assert(condition);
    }
}