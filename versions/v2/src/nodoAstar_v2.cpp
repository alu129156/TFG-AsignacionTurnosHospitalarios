#include "nodoAstar_v2.h"

bool NodoAStar::operator>(const NodoAStar& otro) const {
    return f > otro.f;
}

bool NodoAStar::operator<(const NodoAStar& otro) const {
    return f < otro.f;
}