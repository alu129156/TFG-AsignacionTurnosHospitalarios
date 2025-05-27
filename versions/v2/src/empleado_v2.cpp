#include "empleado_v2.h"
#include "turnos_v2.h"

Empleado::Empleado(std::string nombre)
    : nombre(nombre),
    errorIncumpleMinT(0),
    errorIncumpleMaxT(0),
    errorIncumpleMinL(0),
    errorIncumpleMaxL(0),
    restriccionesMaxT(),
    restriccionesMaxL(),
    noL(true),
    noT(true),
    turnoAnteriorDia(Turnos().free),
    turnoActualDia(Turnos().free),
    actual_noches_consec(0),
    erroresPorPenalizacionesUnwanted(0){}

bool Empleado::operator==(const Empleado& other) const {
    return nombre == other.nombre;
}