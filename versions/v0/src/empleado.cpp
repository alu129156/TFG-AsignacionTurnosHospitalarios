#include "empleado.h"

Empleado::Empleado(std::string nombre) : nombre(nombre) {}

bool Empleado::operator==(const Empleado& other) const {
    return nombre == other.nombre;
}