#include "include/empleado.h"
#include "include/solucion.h"
#include "include/general_utils.h"
#include "include/main_utils.h"
#include "include/nodoAstar.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <limits>

#define MAX_COLA_SIZE 1000
#define LIMITE_EXPLORACION_PORCENTAJE 0.1

using namespace std;

// Calcular h(n) --> Saco un caso desfavorable, que puede subir h(n)
double calcularHeuristica(NodoAStar actual, bool hoja) {
    double h = 0.0;
    if (!hoja) {
        for (const auto& [nombreEmpleado, asignaciones_e] : actual.empleadoAsignaciones) {
            bool eAsignado = actual.empleadoTurnoEnDia[nombreEmpleado];
            int rest_days_e = (eAsignado) ? DIAS - actual.dia - 1 : DIAS - actual.dia;

            int flexibilidad = max(0, MAX_ASIGNACIONES - asignaciones_e);

            int er_1 = max(0, MIN_ASIGNACIONES - (rest_days_e + asignaciones_e));
            int er_2 = max(0, (rest_days_e + asignaciones_e) - MAX_ASIGNACIONES);

            // Ponderación en flexibilidad (penaliza menos a empleados con más margen)
            double penalizacion = (er_1 + er_2);
            h += penalizacion / (1.0 + flexibilidad);
        }
    }
    return h;
}

NodoAStar explorarArbolAStar(
    const vector<Empleado>& empleados,
    const unordered_map<string, int>& empleadoAsignaciones,
    const unordered_map<string, bool>& empleadoTurnoEnDia,
    const chrono::time_point<chrono::high_resolution_clock>& startTime
) {
    multiset<NodoAStar, less<NodoAStar>> frontera;
    vector<vector<Empleado>> solucionInicial(DIAS, vector<Empleado>());

    frontera.insert({false, solucionInicial, empleados, empleados,
         empleadoAsignaciones, empleadoTurnoEnDia, 0, 0, 0, 0.0, 0.0, 0.0});

    NodoAStar mejorSolucion;
    mejorSolucion.f = numeric_limits<double>::max();
    bool primera_hoja_encontrada = false;
    double umbral_factor = 1.75; // 175% ==> Se aceptan soluciones hasta un 75% peores que la mejorFO encontrada

    while (!frontera.empty()) {
        auto now = chrono::high_resolution_clock::now();
        double restTime = 1.0 - chrono::duration<double>(now - startTime).count()/LIMITE_TIEMPO;
        auto it = frontera.begin();
        NodoAStar actual = *it;
        if(!actual.needQuickExploration) {
            frontera.erase(it);
        }

        if (restTime <= LIMITE_EXPLORACION_PORCENTAJE && mejorSolucion.f == numeric_limits<double>::max()) {
            //cout << "Modo exploracion activao" << endl;
            actual.needQuickExploration = true;
            frontera.clear();
        }

        vector<Empleado> empleadosDisponibles = actual.restEmpleados;

        if (primera_hoja_encontrada && actual.f > mejorSolucion.f * umbral_factor) {
            break;
        }

        // Explorar hijos
        for (int i = 0; i < empleadosDisponibles.size(); i++) {
            NodoAStar hijo = actual;
            auto now = chrono::high_resolution_clock::now();
            double elapsed = chrono::duration<double>(now - startTime).count();
            if(isTimeCompleted(elapsed)) {
                return mejorSolucion;
            }

            bool isHoja = (actual.dia == DIAS - 1 && actual.turno == ULTIMO_TURNO && actual.demanda == DEMANDA);
            if(!isHoja) {
                string nombreE = empleadosDisponibles[i].nombre;
    
                hijo.solucion[actual.dia].push_back(empleadosDisponibles[i]);
                hijo.demanda++;
    
                hijo.empleadoAsignaciones[nombreE]++;
                hijo.restEmpleados.erase(hijo.restEmpleados.begin() + i);
                hijo.empleadoTurnoEnDia[nombreE] = true;
    
                if (hijo.demanda == DEMANDA) {
                    if (hijo.turno == ULTIMO_TURNO) {
                        if (hijo.dia < DIAS - 1) {
                            hijo.dia++;
                            hijo.restEmpleados = empleados;
                            hijo.empleadoTurnoEnDia = empleadoTurnoEnDia;
                            hijo.turno = 0;
                            hijo.demanda = 0;
                        } else {
                            hijo.g = calcularFuncionObjetivoAStar(hijo);
                            hijo.h = calcularHeuristica(hijo, true);
                            hijo.f = hijo.g + hijo.h;
                            if (hijo.f < mejorSolucion.f) {
                                primera_hoja_encontrada = true;
                                mejorSolucion = hijo;
                                //cout << "F: " << mejorSolucion.f << endl;
                            }
                            continue;
                        }
                    } else {
                        hijo.turno++;
                        hijo.demanda = 0;
                    }
                }
                
                hijo.g = calcularFuncionObjetivoAStar(hijo);
                hijo.h = calcularHeuristica(hijo, false);
                hijo.f = hijo.g + hijo.h;

                if (frontera.size() >= MAX_COLA_SIZE) {
                    auto peorNodo = prev(frontera.end());
                    if (hijo.f < peorNodo->f) {
                        frontera.erase(peorNodo);
                        frontera.insert(hijo);
                    }
                } else {
                    frontera.insert(hijo);
                }
            }
        }
    }
    return mejorSolucion;
}

int main(int argc, char* argv[]) {
    Config cfg = leerParametros(argc, argv);
    NUM_ENFERMERAS = cfg.enfermeras;
    DIAS = cfg.dias;
    DEMANDA = cfg.demanda;
    MIN_ASIGNACIONES = cfg.min_asig;
    MAX_ASIGNACIONES = cfg.max_asig;

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
    NodoAStar solucionEncontrada = explorarArbolAStar(empleados, empleadoAsignaciones, empleadoTurnoEnDia, start);
    mejorSolucion.funcionObjetivo = solucionEncontrada.f;
    mejorSolucion.solucion = solucionEncontrada.solucion;
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    //cout << solucionEncontrada.needQuickExploration << endl;
    imprimirResultado(mejorSolucion, chrono::duration<double>(end - start).count(),
                         -1, true, solucionEncontrada.needQuickExploration);
    return 0;
}