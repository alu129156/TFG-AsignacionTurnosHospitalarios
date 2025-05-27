#ifndef MAIN_UTILS_v2_H
#define MAIN_UTILS_v2_H

#include <vector>
#include <string>
#include "empleado_v2.h"
#include "solucion_v2.h"

struct Config {
    int enfermeras;
    int dias;
    int demE;
    int demD;
    int demL;
    int min_asig;
    int max_asig;
    int min_dias_libres_consec;
    int max_dias_libres_consec;
    int min_dias_trab_consec;
    int max_dias_trab_consec;
};

Config leerParametros(int argc, char* argv[]);
std::vector<Empleado> generarEmpleados_v2(int n);
void imprimirResultado(const SolucionFinal& solucion, double tiempo, int nodosExplorados = -1, 
                        bool isAStar = false, bool needQuickExploration = false);

#endif