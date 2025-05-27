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

double calcularCotaInf(
    unordered_map<string, int>& empleadoAsignaciones,
    int dia,
    unordered_map<string, bool>& empleadoTurnoEnDia
) {
    double f = 0.0;
    for (const auto& [nombreEmpleado, asignacionesEmpleado] : empleadoAsignaciones) {
        bool turnoEmpleadoEnDia = empleadoTurnoEnDia[nombreEmpleado];
        double restDays = (turnoEmpleadoEnDia) ? DIAS - dia - 1 : DIAS - dia;

        double ei = max(0, asignacionesEmpleado - MAX_ASIGNACIONES);
        double ei_prime = max(0, MIN_ASIGNACIONES - asignacionesEmpleado);
        double ki = max(0.0, ei_prime - restDays);

        f += PESO_W1 * ei + PESO_W2 * ki;
    }
    return f;
}

int explorarArbol(
    int dia, int turno,
    const vector<Empleado>& employees, 
    vector<Empleado> restEmployees,
    vector<vector<Empleado>>& posibleSolucion,
    unordered_map<string, int>& empleadoAsignaciones,
    unordered_map<string, bool>& empleadoTurnoEnDia,
    int actualDemanda, SolucionFinal& mejorSolucion,
    const chrono::time_point<chrono::high_resolution_clock>& startTime
) {
    int nodos = 0;

    for (int i = 0; i < restEmployees.size(); i++) {
        verifyTime(startTime);

        nodos++;
        Empleado e = restEmployees[i];
        bool turnoEnDiaPrevio = empleadoTurnoEnDia[e.nombre];
        restEmployees.erase(restEmployees.begin() + i);
        posibleSolucion[dia].push_back(e);
        empleadoAsignaciones[e.nombre]++;
        empleadoTurnoEnDia[e.nombre] = true;

        double cotaInf = calcularCotaInf(empleadoAsignaciones, dia, empleadoTurnoEnDia);

        if (actualDemanda == DEMANDA - 1 && turno == ULTIMO_TURNO && dia == DIAS - 1) { // HOJA
            double fo = calcularFuncionObjetivo(posibleSolucion, empleadoAsignaciones);
            if (fo < mejorSolucion.funcionObjetivo) {
                mejorSolucion.funcionObjetivo = fo;
                mejorSolucion.solucion = posibleSolucion;
            }
        } else if (cotaInf >= mejorSolucion.funcionObjetivo) { // Poda
            empleadoAsignaciones[e.nombre]--;
            empleadoTurnoEnDia[e.nombre] = turnoEnDiaPrevio;
            posibleSolucion[dia].pop_back();
            restEmployees.insert(restEmployees.begin() + i, e);
            continue;
        } else if (turno == ULTIMO_TURNO && actualDemanda == DEMANDA - 1) {
            vector<Empleado> restEmployeesNext = employees;
            unordered_map<string, bool> empleadosTurnosEnDiaPrevio = empleadoTurnoEnDia;
            for (const auto& empleado : employees) {
                empleadoTurnoEnDia[empleado.nombre] = false;
            }
            nodos += explorarArbol(dia + 1, 0, employees, restEmployeesNext, posibleSolucion,
             empleadoAsignaciones, empleadoTurnoEnDia, 0, mejorSolucion, startTime);

            // Restaurar estado para el backtracking
            empleadoTurnoEnDia = empleadosTurnosEnDiaPrevio;
        } else { // turno < 2
            if (actualDemanda == DEMANDA - 1) { // Cambio de turno
                nodos += explorarArbol(dia, turno + 1, employees, restEmployees, posibleSolucion,
                 empleadoAsignaciones, empleadoTurnoEnDia, 0, mejorSolucion, startTime);
            } else { // Mismo turno, actualizando la demanda
                nodos += explorarArbol(dia, turno, employees, restEmployees, posibleSolucion, empleadoAsignaciones,
                 empleadoTurnoEnDia, actualDemanda + 1, mejorSolucion, startTime);
            }
        }

        // Restaurar estado para backtracking
        empleadoAsignaciones[e.nombre]--;
        empleadoTurnoEnDia[e.nombre] = turnoEnDiaPrevio;
        posibleSolucion[dia].pop_back();
        restEmployees.insert(restEmployees.begin() + i, e);
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
    unordered_map<string, int> empleadoAsignaciones;
    unordered_map<string, bool> empleadoTurnoEnDia;

    for (const auto& empleado : empleados) {
        empleadoAsignaciones[empleado.nombre] = 0;
        empleadoTurnoEnDia[empleado.nombre] = false;
    }

    SolucionFinal mejorSolucion = {INFINITY, {}};
    auto start = chrono::high_resolution_clock::now();
    int nodosExplorados = explorarArbol(0, 0, empleados, restEmployees, posibleSolucion, empleadoAsignaciones,
                     empleadoTurnoEnDia, 0, mejorSolucion, start);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    imprimirResultado(mejorSolucion, chrono::duration<double>(end - start).count(), nodosExplorados);
    return 0;
}
