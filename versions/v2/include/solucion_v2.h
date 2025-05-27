#ifndef SOLUCION_v2_H
#define SOLUCION_v2_H
#include "empleado_v2.h"
#include <vector>

struct SolucionFinal {
    double funcionObjetivo;
    std::vector<std::vector<Empleado>> solucion;
    bool operator==(const SolucionFinal& other) const;
};

#endif