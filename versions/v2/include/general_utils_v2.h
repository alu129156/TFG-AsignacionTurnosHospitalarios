#ifndef GENERAL_UTILS_v2_H
#define GENERAL_UTILS_v2_H

#include "empleado_v2.h"
#include "solucion_v2.h"
#include "nodoAstar_v2.h"
#include "turnos_v2.h"
#include <iostream>
#include <unordered_map>
#include <chrono>
#include <algorithm>

extern int NUM_ENFERMERAS, DIAS, DEM_EARLY, DEM_DAY, DEM_LATE, MIN_ASIGNACIONES, MAX_ASIGNACIONES,
           MAX_DIAS_TRABAJADOS_CONSECUTIVOS, MIN_DIAS_TRABAJADOS_CONSECUTIVOS, MAX_DIAS_LIBRES_CONSECUTIVOS,
           MIN_DIAS_LIBRES_CONSECUTIVOS;
extern int LIMITE_TIEMPO;
extern double PESO_W1, PESO_W2, PESO_W3, PESO_W4, PESO_W5, PESO_W6, PENALIZACION_UNWANTED_SHIFT_PATTERN;

const std::pair<Turno, Turno> uwsPAT1 = {Turnos().day, Turnos().early};
const std::pair<Turno, Turno> uwsPAT2 = {Turnos().late, Turnos().early};
const std::pair<Turno, Turno> uwsPAT3 = {Turnos().late, Turnos().day};
constexpr int TRES_NOCHES_CONSEC = 3;
constexpr int ULTIMO_TURNO = 2;

void cargarPesosDesdeJSON();
void printInput();
void verifyTime(const std::chrono::time_point<std::chrono::high_resolution_clock>& startTime);
bool isTimeCompleted(const chrono::time_point<chrono::high_resolution_clock>& startTime);
double calcularFuncionObjetivo(const vector<Empleado>& employees, const vector<vector<Empleado>>& solucion,
                                unordered_map<string, int>& empleadoAsignaciones);
double calcularFuncionObjetivoAstar(NodoAStar actual);
int empleadoIdx(const vector<Empleado>& empleados, string empleadoNombre);
int patronDeTurnosUnwanted(Empleado& e);
void calcularErroresPatronesEmpleados(vector<Empleado>& empleados,const vector<vector<Empleado>>& horario,bool TRABAJA);
#endif