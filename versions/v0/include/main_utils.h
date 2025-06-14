#ifndef MAIN_UTILS_H
#define MAIN_UTILS_H

#include <vector>
#include <string>
#include "empleado.h"
#include "solucion.h"

struct Config {
    int enfermeras;
    int dias;
    int demanda;
    int min_asig;
    int max_asig;
};

Config leerParametros(int argc, char* argv[]);
std::vector<Empleado> generarEmpleados(int n);
void imprimirResultado(const SolucionFinal& solucion, double tiempo, int nodosExplorados = -1, 
                        bool isAStar = false, bool needQuickExploration = false);

#endif
