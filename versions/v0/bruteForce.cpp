#include "include/empleado.h"
#include "include/solucion.h"
#include "include/general_utils.h"
#include "include/main_utils.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>
#include <chrono>

using namespace std;

int explorarArbol(int dia, int turno, const vector<Empleado>& employees, vector<Empleado> restEmployees,
                    vector<vector<Empleado>> posibleSolucion, SolucionFinal& mejorSolucion,
                    int actualDemanda, unordered_map<string, int> empleadoAsignaciones,
                    const chrono::time_point<chrono::high_resolution_clock>& startTime) {
    int nodos = 0;
    for(int i = 0; i < restEmployees.size(); i++) {
        verifyTime(startTime);

        nodos++;
        Empleado e = restEmployees[i];
        restEmployees.erase(restEmployees.begin() + i);
        posibleSolucion[dia].push_back(e);
        empleadoAsignaciones[e.nombre]++;

        if(actualDemanda == DEMANDA - 1 && turno == ULTIMO_TURNO && dia == DIAS - 1) { // HOJA: calcula FO
            double fo = calcularFuncionObjetivo(posibleSolucion, empleadoAsignaciones);
            if (fo < mejorSolucion.funcionObjetivo) {
                mejorSolucion.funcionObjetivo = fo;
                mejorSolucion.solucion = posibleSolucion;
            }
        } else if(turno == ULTIMO_TURNO && actualDemanda == DEMANDA - 1) {
            vector<Empleado> restEmployeesNext = employees;
            nodos += explorarArbol(dia + 1, 0, employees, restEmployeesNext, posibleSolucion, mejorSolucion,
                 0, empleadoAsignaciones, startTime);
        } else { // turno < 2
            if(actualDemanda == DEMANDA - 1) { // Cambio de turno
                nodos += explorarArbol(dia, turno + 1, employees, restEmployees, posibleSolucion, mejorSolucion,
                     0, empleadoAsignaciones, startTime);
            } else { // Mismo turno, actualizando la demanda
                nodos += explorarArbol(dia, turno, employees, restEmployees, posibleSolucion, mejorSolucion,
                     actualDemanda + 1, empleadoAsignaciones, startTime);
            }
        }

        // Restaurar estado para backtracking
        posibleSolucion[dia].pop_back();
        restEmployees.insert(restEmployees.begin() + i, e);
        empleadoAsignaciones[e.nombre]--;
    }
    return nodos;
}

int main(int argc, char* argv[]) {
    Config cfg = leerParametros(argc, argv);
    NUM_ENFERMERAS = cfg.enfermeras;
    DIAS = cfg.dias;
    DEMANDA = cfg.demanda;
    MIN_ASIGNACIONES = cfg.min_asig;
    MAX_ASIGNACIONES = cfg.max_asig;
    cargarPesosDesdeJSON();
    
    vector<Empleado> empleados = generarEmpleados(NUM_ENFERMERAS);

    vector<vector<Empleado>> posibleSolucion(DIAS, vector<Empleado>());
    vector<Empleado> restEmployees = empleados;
    SolucionFinal mejorSolucion = {INFINITY, {}};
    unordered_map<string, int> empleadoAsignaciones;
    for (const auto& e : empleados) {
        empleadoAsignaciones[e.nombre] = 0;
    }

    auto start = chrono::high_resolution_clock::now();
    int nodosExplorados = explorarArbol(0, 0, empleados, restEmployees, posibleSolucion, mejorSolucion,
         0, empleadoAsignaciones, start);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    imprimirResultado(mejorSolucion, chrono::duration<double>(end - start).count(), nodosExplorados);
    return 0;
}