#ifndef MAIN_UTILS_v1_H
#define MAIN_UTILS_v1_H

#include <vector>
#include <string>
#include "empleado_v1.h"
#include "solucion_v1.h"

struct Config {
    int enfermeras;
    int dias;
    int demanda;
    int min_asig;
    int max_asig;
    int min_dias_libres_consec;
    int max_dias_libres_consec;
    int min_dias_trab_consec;
    int max_dias_trab_consec;
};

Config leerParametros(int argc, char* argv[]);
std::vector<Empleado> generarEmpleados_v1(int n);
void imprimirResultado(const SolucionFinal& solucion, double tiempo, int nodosExplorados = -1, 
                        bool isAStar = false, bool needQuickExploration = false);

#endif