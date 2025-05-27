#ifndef EMPLEADO_v2_H
#define EMPLEADO_v2_H

#include "turnos_v2.h"
#include <string>
#include <stack>


class Empleado {
    public:
        std::string nombre;
        int errorIncumpleMinT;
        int errorIncumpleMaxT;
        int errorIncumpleMinL;
        int errorIncumpleMaxL;
        std::stack<int> restriccionesMaxT;
        std::stack<int> restriccionesMaxL;
        bool noL;
        bool noT;
        Turno turnoAnteriorDia;
        Turno turnoActualDia;
        int actual_noches_consec;
        int erroresPorPenalizacionesUnwanted;
        Empleado(std::string nombre);
        bool operator==(const Empleado& other) const;         
};

#endif