#ifndef NODO_ASTAR_H
#define NODO_ASTAR_H

#include "empleado.h"
#include <vector>
#include <unordered_map>
using namespace std;

struct NodoAStar {
    bool needQuickExploration;
    vector<vector<Empleado>> solucion;
    vector<Empleado> empleados;
    vector<Empleado> restEmpleados;
    unordered_map<string, int> empleadoAsignaciones;
    unordered_map<string, bool> empleadoTurnoEnDia;
    int dia, turno, demanda;
    double g;
    double h;
    double f;
    bool operator<(const NodoAStar& otro) const;
};

#endif