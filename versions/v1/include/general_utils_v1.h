#ifndef GENERAL_UTILS_v1_H
#define GENERAL_UTILS_v1_H

#include "empleado_v1.h"
#include "solucion_v1.h"
#include "nodoAstar_v1.h"
#include <iostream>
#include <unordered_map>
#include <chrono>
#include <algorithm>

extern int NUM_ENFERMERAS, DIAS, DEMANDA, MIN_ASIGNACIONES, MAX_ASIGNACIONES, MAX_DIAS_TRABAJADOS_CONSECUTIVOS,
            MIN_DIAS_TRABAJADOS_CONSECUTIVOS, MAX_DIAS_LIBRES_CONSECUTIVOS, MIN_DIAS_LIBRES_CONSECUTIVOS;
extern int LIMITE_TIEMPO;
extern double PESO_W1, PESO_W2, PESO_W3, PESO_W4, PESO_W5, PESO_W6, PENALIZACION_UNWANTED_SHIFT_PATTERN;

constexpr int ULTIMO_TURNO = 2;

void cargarPesosDesdeJSON();
void printInput();
void verifyTime(const std::chrono::time_point<std::chrono::high_resolution_clock>& startTime);
bool isTimeCompleted(const chrono::time_point<chrono::high_resolution_clock>& startTime);
double calcularFuncionObjetivo(const vector<Empleado>& employees, const vector<vector<Empleado>>& solucion,
                                unordered_map<string, int>& empleadoAsignaciones);
double calcularFuncionObjetivoAstar(NodoAStar actual);
double calcularFuncionObjetivoPiston(const vector<Empleado>& employees, const vector<vector<Empleado>>& solucion,
    unordered_map<string, int>& empleadoAsignaciones, Incumplimientos& B);
int empleadoIdx(const vector<Empleado>& empleados, string empleadoNombre);
int calcularFlexibilidad(const Empleado& e, const unordered_map<string, int>& empleadoAsignaciones, 
    const unordered_map<string, int>& consecutivosT, const unordered_map<string, int>& consecutivosL);

#endif