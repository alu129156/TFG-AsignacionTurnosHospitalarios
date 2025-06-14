#include "empleado_v1.h"

Empleado::Empleado(std::string nombre)
    : nombre(nombre),
    errorIncumpleMinT(0),
    errorIncumpleMaxT(0),
    errorIncumpleMinL(0),
    errorIncumpleMaxL(0),
    restriccionesMaxT(),
    restriccionesMaxL(),
    noL(true),
    noT(true) {}

bool Empleado::operator==(const Empleado& other) const {
    return nombre == other.nombre;
}