#include "turnos_v2.h"

bool Turno::operator==(const Turno &other) const{
    return nombre == other.nombre;
}