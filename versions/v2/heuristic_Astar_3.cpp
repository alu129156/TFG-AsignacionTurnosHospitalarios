#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>
#include <queue>
#include <chrono>
#include "include/empleado_v2.h"
#include "include/general_utils_v2.h"
#include "include/main_utils_v2.h"
#include "include/solucion_v2.h"
#include "include/nodoAstar_v2.h"

#define MAX_COLA_SIZE 1000
#define LIMITE_EXPLORACION_PORCENTAJE 0.1

// Calcular h(n) --> Saco un caso desfavorable, que puede subir h(n)
double calcularHeuristica(NodoAStar actual, bool hoja) {
    double h = 0.0;
    if (!hoja) {
        for (const auto& [nombreEmpleado, asignaciones_e] : actual.empleadoAsignaciones) {
            bool eAsignado = actual.empleadoTurnoEnDia[nombreEmpleado];
            int rest_days_e = (eAsignado) ? DIAS - actual.dia - 1 : DIAS - actual.dia;
            int consecT = actual.consecutivosT[nombreEmpleado];
            int consecL = actual.consecutivosL[nombreEmpleado];

            int flexibilidad = max(0, MAX_ASIGNACIONES - asignaciones_e) * PESO_W1 +
                               max(0, MAX_DIAS_TRABAJADOS_CONSECUTIVOS - consecT) * PESO_W3 +
                               max(0, MAX_DIAS_LIBRES_CONSECUTIVOS - consecL) * PESO_W5;

            int er_2 = max(0, MIN_ASIGNACIONES - (rest_days_e + asignaciones_e));
            int er_1 = max(0, (rest_days_e + asignaciones_e) - MAX_ASIGNACIONES);
    
            int er_4 = max(0, MIN_DIAS_TRABAJADOS_CONSECUTIVOS - (consecT + rest_days_e));
            int er_3 = max(0, (consecT + rest_days_e) - MAX_DIAS_TRABAJADOS_CONSECUTIVOS);  

            int er_6 = max(0, MIN_DIAS_LIBRES_CONSECUTIVOS - (consecL + rest_days_e));
            int er_5 = max(0, (consecL + rest_days_e) - MAX_DIAS_LIBRES_CONSECUTIVOS);

            int uwPat = actual.empleados[empleadoIdx(actual.empleados, nombreEmpleado)].erroresPorPenalizacionesUnwanted;

            // Ponderación en flexibilidad (penaliza menos a empleados con más margen)
            double penalizacion =  
                  er_1 * PESO_W1
                + er_2 * PESO_W2
                + er_3 * PESO_W3
                + er_4 * PESO_W4
                + er_5 * PESO_W5
                + er_6 * PESO_W6
                + uwPat;

            h += penalizacion / (1.0 + flexibilidad);
        }
    }
    return h;
}

NodoAStar explorarArbolAStar(
    const vector<Empleado>& empleados,
    const unordered_map<string, int>& empleadoAsignaciones,
    const unordered_map<string, bool>& empleadoTurnoEnDia,
    const unordered_map<string, int>& consecL,
    const unordered_map<string, int>& consecT,
    const chrono::time_point<chrono::high_resolution_clock>& startTime
) {
    multiset<NodoAStar, less<NodoAStar>> frontera;
    vector<vector<Empleado>> solucionInicial(DIAS, vector<Empleado>());
    vector<Turno> turnosTrabajados = {Turnos().early, Turnos().day, Turnos().late};
    vector<int> DEMANDAS = {DEM_EARLY, DEM_DAY, DEM_LATE};

    frontera.insert({false, solucionInicial, empleados, empleados,
         empleadoAsignaciones, empleadoTurnoEnDia, consecT, consecL, 0, 0, 0, 0.0, 0.0, 0.0});

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
            actual.needQuickExploration = true;
            frontera.clear();
        }

        vector<Empleado> empleadosDisponibles = actual.restEmpleados;

        if (primera_hoja_encontrada && actual.f > mejorSolucion.f * umbral_factor) {
            break;
        }

        // Explorar hijos
        for (int i = 0; i < empleadosDisponibles.size(); i++) {
            if(isTimeCompleted(startTime)) {
                if(mejorSolucion.f == numeric_limits<double>::max()) {
                    verifyTime(startTime); // Solución no encontrada, exit y muestra que se ha excedido
                }
                return mejorSolucion;
            }

            NodoAStar hijo = actual;
            bool isHoja = (actual.dia == DIAS - 1 && actual.turno == ULTIMO_TURNO && actual.demanda == DEM_LATE);
            if(!isHoja) {
                string nombreE = empleadosDisponibles[i].nombre;
                int idx = empleadoIdx(hijo.empleados, nombreE);
    
                hijo.solucion[actual.dia].push_back(empleadosDisponibles[i]);
                hijo.demanda++;
    
                hijo.empleadoAsignaciones[nombreE]++;
                hijo.restEmpleados.erase(hijo.restEmpleados.begin() + i);
                hijo.empleadoTurnoEnDia[nombreE] = true;

                // Actualizamos el turno del empleado al turno actual
                hijo.empleados[idx].turnoAnteriorDia = hijo.empleados[idx].turnoActualDia;
                hijo.empleados[idx].turnoActualDia = turnosTrabajados[hijo.turno];

                // Unwanted pattern: L-L-L
                if(hijo.empleados[idx].turnoActualDia == Turnos().late) {
                    hijo.empleados[idx].actual_noches_consec ++;
                } else {
                    hijo.empleados[idx].actual_noches_consec = 0;
                }

                // Buscamos patrones no queridos y se suma su penalización
                hijo.empleados[idx].erroresPorPenalizacionesUnwanted += 
                                        patronDeTurnosUnwanted(hijo.empleados[idx]);
    
                if (hijo.consecutivosL[nombreE] > 0) { // *L -> T
                    hijo.empleados[idx].errorIncumpleMinL += max(0, MIN_DIAS_LIBRES_CONSECUTIVOS - hijo.consecutivosL[nombreE]) * PESO_W6;
                    hijo.empleados[idx].errorIncumpleMaxL += max(0, hijo.consecutivosL[nombreE] - MAX_DIAS_LIBRES_CONSECUTIVOS) * PESO_W5;
                    hijo.consecutivosL[nombreE] = 0;
                }
                hijo.consecutivosT[nombreE]++;
                hijo.empleados[idx].noT = false;
                if(hijo.dia == DIAS - 1) {
                    // Acaba en T
                    hijo.empleados[idx].errorIncumpleMinT += 
                                max(0, MIN_DIAS_TRABAJADOS_CONSECUTIVOS - hijo.consecutivosT[nombreE]) * PESO_W4;
                    hijo.empleados[idx].errorIncumpleMaxT += 
                                max(0, hijo.consecutivosT[nombreE] - MAX_DIAS_TRABAJADOS_CONSECUTIVOS) * PESO_W3;
                    if(hijo.empleados[idx].noL) { // Caso TTTT --> 0 Dias Libres
                        hijo.empleados[idx].errorIncumpleMinL += MIN_DIAS_LIBRES_CONSECUTIVOS * PESO_W6;
                    }
                }
    
                if (hijo.demanda == DEMANDAS[hijo.turno]) {
                    if (hijo.turno == ULTIMO_TURNO) {
                        for (Empleado& rE : hijo.restEmpleados) {
                            int rIdx = empleadoIdx(hijo.empleados, rE.nombre);

                            // Al acabar el día actualizamos el turno del empleado a libre
                            hijo.empleados[rIdx].turnoAnteriorDia = hijo.empleados[rIdx].turnoActualDia;
                            hijo.empleados[rIdx].turnoActualDia = Turnos().free;

                            // Not unwanted pattern: L-L-L
                            if(hijo.empleados[rIdx].turnoAnteriorDia == Turnos().late) { // L-F
                                hijo.empleados[rIdx].actual_noches_consec = 0;
                            }

                            // Buscamos patrones no queridos y se suma su penalización
                            hijo.empleados[rIdx].erroresPorPenalizacionesUnwanted += 
                                                    patronDeTurnosUnwanted(hijo.empleados[rIdx]);

                            if (hijo.consecutivosT[rE.nombre] > 0) { // *T -> L
                                hijo.empleados[rIdx].errorIncumpleMinT += 
                                            max(0, MIN_DIAS_TRABAJADOS_CONSECUTIVOS - hijo.consecutivosT[rE.nombre]) * PESO_W4;
                                hijo.empleados[rIdx].errorIncumpleMaxT += 
                                            max(0, hijo.consecutivosT[rE.nombre] - MAX_DIAS_TRABAJADOS_CONSECUTIVOS) * PESO_W3;
                            }
                            hijo.consecutivosL[rE.nombre]++;
                            hijo.consecutivosT[rE.nombre] = 0;
                            hijo.empleados[rIdx].noL = false;
                
                            if (hijo.dia == DIAS - 1) {
                                // Acaba en L
                                hijo.empleados[rIdx].errorIncumpleMinL += 
                                            max(0, MIN_DIAS_LIBRES_CONSECUTIVOS - hijo.consecutivosL[rE.nombre]) * PESO_W6;
                                hijo.empleados[rIdx].errorIncumpleMaxL += 
                                            max(0, hijo.consecutivosL[rE.nombre] - MAX_DIAS_LIBRES_CONSECUTIVOS) * PESO_W5;
                                if (hijo.empleados[rIdx].noT) { // Caso LLLL --> 0 Dias Trabajados
                                    hijo.empleados[rIdx].errorIncumpleMinT += MIN_DIAS_TRABAJADOS_CONSECUTIVOS * PESO_W4;
                                }
                            }
                        }
    
                        if (hijo.dia < DIAS - 1) {
                            hijo.dia++;
                            hijo.restEmpleados = empleados;
                            hijo.empleadoTurnoEnDia = empleadoTurnoEnDia;
                            hijo.turno = 0;
                            hijo.demanda = 0;
                        } else {
                            hijo.g = calcularFuncionObjetivoAstar(hijo);
                            hijo.h = calcularHeuristica(hijo, true);
                            hijo.f = hijo.g + hijo.h;
                            if (hijo.f < mejorSolucion.f) {
                                    //cout << "FO: " << hijo.f << endl;
                                primera_hoja_encontrada = true;
                                mejorSolucion = hijo;
                            }
                            continue;
                        }
                    } else {
                        hijo.turno++;
                        hijo.demanda = 0;
                    }
                }
                
                hijo.g = calcularFuncionObjetivoAstar(hijo);
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
    DEM_EARLY = cfg.demE;
    DEM_DAY = cfg.demD;
    DEM_LATE = cfg.demL;
    MIN_ASIGNACIONES = cfg.min_asig;
    MAX_ASIGNACIONES = cfg.max_asig;
    MIN_DIAS_LIBRES_CONSECUTIVOS = cfg.min_dias_libres_consec;
    MAX_DIAS_LIBRES_CONSECUTIVOS = cfg.max_dias_libres_consec;
    MIN_DIAS_TRABAJADOS_CONSECUTIVOS = cfg.min_dias_trab_consec;
    MAX_DIAS_TRABAJADOS_CONSECUTIVOS = cfg.max_dias_trab_consec;
    cargarPesosDesdeJSON();
    
    vector<Empleado> empleados = generarEmpleados_v2(NUM_ENFERMERAS);
    unordered_map<string, bool> empleadoTurnoEnDia;
    unordered_map<string, int> empleadoAsignaciones;
    unordered_map<string, int> consecL;
    unordered_map<string, int> consecT;
    for (const auto& e: empleados) {
        empleadoTurnoEnDia[e.nombre] = false;
        empleadoAsignaciones[e.nombre] = 0;
        consecT[e.nombre] = 0;
        consecL[e.nombre] = 0;
    }

    auto start = chrono::high_resolution_clock::now();
    NodoAStar solucionEncontrada = explorarArbolAStar(empleados, empleadoAsignaciones, empleadoTurnoEnDia, consecL, consecT, start);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    SolucionFinal solucion = {solucionEncontrada.f, solucionEncontrada.solucion};
    imprimirResultado(solucion, duration.count(), -1, true, solucionEncontrada.needQuickExploration);
    
    return solucionEncontrada.solucion.empty() ? 1 : 0;
}