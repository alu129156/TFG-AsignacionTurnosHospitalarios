#ifndef SOLUCION_H
#define SOLUCION_H

#include "empleado.h"
#include <vector>

struct Turno {
    std::string nombre;
};

struct Turnos {
    Turno early = {"Early"};
    Turno day = {"Day"};
    Turno late = {"Late"};
};

struct SolucionFinal {
    double funcionObjetivo;
    std::vector<std::vector<Empleado>> solucion;
    bool operator==(const SolucionFinal& other) const;
};

#endif
