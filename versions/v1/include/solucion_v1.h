#ifndef SOLUCION_v1_H
#define SOLUCION_v1_H

#include "empleado_v1.h"
#include "incumplimientos.h"
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

struct SolucionFinal_Piston {
    double funcionObjetivo;
    std::vector<std::vector<Empleado>> solucion;
    Incumplimientos B;
};

#endif