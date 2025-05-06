#ifndef GENERAL_UTILS_H
#define GENERAL_UTILS_H

#include "empleado.h"
#include "solucion.h"
#include "nodoAstar.h"
#include <iostream>
#include <unordered_map>
#include <chrono>
#include <algorithm>

extern int NUM_ENFERMERAS, DIAS, DEMANDA, MIN_ASIGNACIONES, MAX_ASIGNACIONES;
constexpr double PESO_W1 = 10.0;
constexpr double PESO_W2 = 10.0;
constexpr int LIMITE_TIEMPO = 420;
constexpr int ULTIMO_TURNO = 2;

void printInput();
void verifyTime(const std::chrono::time_point<std::chrono::high_resolution_clock>& startTime);
bool isTimeCompleted(const double& elapsed);
double calcularFuncionObjetivo(const std::vector<std::vector<Empleado>>& solucion,
                               const std::unordered_map<std::string, int>& empleadoAsignaciones);
double calcularFuncionObjetivoAStar(NodoAStar actual);

#endif
