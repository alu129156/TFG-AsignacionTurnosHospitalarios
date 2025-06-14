#include "nodoAstar_v1.h"

bool NodoAStar::operator>(const NodoAStar& otro) const {
    return f > otro.f;
}

bool NodoAStar::operator<(const NodoAStar& otro) const {
    return f < otro.f;
}