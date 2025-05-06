#include "nodoAstar.h"

bool NodoAStar::operator<(const NodoAStar& otro) const {
    return f < otro.f;
}