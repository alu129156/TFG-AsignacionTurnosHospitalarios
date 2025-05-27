#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <set>
#include <stack>
#include <unordered_map>
#include <chrono>
#include <random>
#include "include/empleado_v1.h"
#include "include/general_utils_v1.h"
#include "include/main_utils_v1.h"
#include "include/solucion_v1.h"
#include "include/incumplimientos.h"
#include "include/solucion_random_generator.h"

using namespace std;

double calcularCotaInf(
    const vector<Empleado>& employees,
    const unordered_map<string, int>& empleadoAsignaciones,
    int dia,
    unordered_map<string, bool>& empleadoTurnoEnDia,
    Incumplimientos& A
) {
    double f = 0.0;
    for (const auto& [nombreEmpleado, asignacionesEmpleado] : empleadoAsignaciones) {
        bool turnoEmpleadoEnDia = empleadoTurnoEnDia[nombreEmpleado];
        double restDays = (turnoEmpleadoEnDia) ? DIAS - dia - 1 : DIAS - dia;

        double ei = max(0, asignacionesEmpleado - MAX_ASIGNACIONES);
        double ei_prime = max(0, MIN_ASIGNACIONES - asignacionesEmpleado);
        double ki = max(0.0, ei_prime - restDays);

        f += PESO_W1 * ei + PESO_W2 * ki;

        // Incumplimientos
        int idx = empleadoIdx(employees, nombreEmpleado);
        if(ei > 0) {
            A.max_asingaciones[idx] = true;
        } if(ki > 0) {
            A.min_asignaciones[idx] = true;
        }


        Empleado e = employees[idx];
        f += (e.errorIncumpleMinL + e.errorIncumpleMinT);
        f += e.errorIncumpleMaxT;
        if(!e.restriccionesMaxT.empty()) {
            f += e.restriccionesMaxT.top();
        }
        f += e.errorIncumpleMaxL;
        if(!e.restriccionesMaxL.empty()) {
            f += e.restriccionesMaxL.top();
        }

        if(e.errorIncumpleMaxL > 0) {
            A.max_dias_libres_consec[idx] = true;
        } if(e.errorIncumpleMinL > 0) {
            A.min_dias_libres_consec[idx] = true;
        } if(e.errorIncumpleMaxT > 0) {
            A.max_dias_trab_consec[idx] = true;
        } if(e.errorIncumpleMinT > 0) {
            A.min_dias_trab_consec[idx] = true;
        }
    }

    return f;
}

double calcularUmbralPoda(double elapsedTime) {
    double ratio = elapsedTime / LIMITE_TIEMPO;
    int phase = static_cast<int>(ratio * 100) % 3;

    if (phase == 0 || phase == 1) {
        return 0.2;
    } else {
        return 0.6;
    }
}

bool podaHeuristica(
    const SolucionFinal_Piston& mejorSolucion,
    const vector<vector<Empleado>>& posibleSolucion,
    const vector<Empleado>& employees,
    const unordered_map<string, int>& empleadoAsignaciones,
    int dia,
    unordered_map<string, bool>& empleadoTurnoEnDia,
    const chrono::time_point<chrono::high_resolution_clock>& startTime
) {
    Incumplimientos A(NUM_ENFERMERAS);
    double cotaInf = calcularCotaInf(employees, empleadoAsignaciones, dia, empleadoTurnoEnDia, A);

    if(cotaInf >= mejorSolucion.funcionObjetivo) { // B&B
        return true;
    }

    double Jaccard = static_cast<double>(contarIncumplimientos(interseccionIncumplimientos(A, mejorSolucion.B))) / contarIncumplimientos(unionIncumplimientos(A, mejorSolucion.B));
    double distanciaJaccard = 1.0 - Jaccard;

    auto now = chrono::high_resolution_clock::now();
    double elapsedTime = chrono::duration<double>(now - startTime).count();
    double umbralPoda = calcularUmbralPoda(elapsedTime);

    if (distanciaJaccard <= umbralPoda) {
        return true;
    }

    return false;
}

int explorarArbol(
    int dia, int turno, vector<Empleado>& employees, const vector<Empleado>& copyEmployees,
    vector<Empleado> restEmployees,
    vector<vector<Empleado>>& posibleSolucion, 
    SolucionFinal_Piston& mejorSolucion,
    int actualDemanda,
    unordered_map<string, int>& empleadoAsignaciones,
    unordered_map<string, bool>& empleadoTurnoEnDia,
    unordered_map<string, int>& consecutivosT, unordered_map<string, int>& consecutivosL,
    const chrono::time_point<chrono::high_resolution_clock>& startTime
) {
    int nodos = 0;

    // Ordenar empleados por flexibilidad
    sort(restEmployees.begin(), restEmployees.end(), [&](const Empleado& a, const Empleado& b) {
        return calcularFlexibilidad(a, empleadoAsignaciones, consecutivosT, consecutivosL) > 
                calcularFlexibilidad(b, empleadoAsignaciones, consecutivosT, consecutivosL);
    });

    for(int i = 0; i < restEmployees.size(); i++) {
        if(isTimeCompleted(startTime)) {
            return nodos;
        }

        nodos++;
        Empleado e = restEmployees[i];
        bool turnoEnDiaPrevio = empleadoTurnoEnDia[e.nombre];
        restEmployees.erase(restEmployees.begin() + i);
        posibleSolucion[dia].push_back(e);
        empleadoAsignaciones[e.nombre]++;
        empleadoTurnoEnDia[e.nombre] = true;

        int idx = empleadoIdx(employees, e.nombre);
        int prevConsecT = consecutivosT[e.nombre];
        int prevConsecL = consecutivosL[e.nombre];
        int prevErrorMaxL = employees[idx].errorIncumpleMaxL;
        int prevErrorMinL = employees[idx].errorIncumpleMinL;
        int prevErrorMaxT = employees[idx].errorIncumpleMaxT;
        int prevErrorMinT = employees[idx].errorIncumpleMinT;
        bool prevNoT = employees[idx].noT;

        if(consecutivosL[e.nombre] > 0) { // *L -> T  
            employees[idx].errorIncumpleMinL += max(0, MIN_DIAS_LIBRES_CONSECUTIVOS - consecutivosL[e.nombre]) * PESO_W6;
            employees[idx].errorIncumpleMaxL += max(0, consecutivosL[e.nombre] - MAX_DIAS_LIBRES_CONSECUTIVOS) * PESO_W5;
            consecutivosL[e.nombre] = 0;
        }
        consecutivosT[e.nombre]++;
        employees[idx].noT = false;
        if(dia == DIAS - 1) {
            //Acaba en T
            employees[idx].errorIncumpleMinT += 
                        max(0, MIN_DIAS_TRABAJADOS_CONSECUTIVOS - consecutivosT[e.nombre]) * PESO_W4;
            employees[idx].errorIncumpleMaxT += 
                        max(0, consecutivosT[e.nombre] - MAX_DIAS_TRABAJADOS_CONSECUTIVOS) * PESO_W3;
            if(employees[idx].noL) { // Caso TTTT --> 0 Dias Libres
                employees[idx].errorIncumpleMinL += MIN_DIAS_LIBRES_CONSECUTIVOS * PESO_W6;
            }
        }
        int restriccionMaxT = max(0, consecutivosT[e.nombre] - MAX_DIAS_TRABAJADOS_CONSECUTIVOS) * PESO_W3;
        employees[idx].restriccionesMaxT.push(restriccionMaxT);

        bool poda = podaHeuristica(mejorSolucion, posibleSolucion, employees, empleadoAsignaciones, dia, empleadoTurnoEnDia, startTime);

        if(actualDemanda == DEMANDA - 1 && turno == ULTIMO_TURNO) {
            unordered_map<string, int> prevConsecT, prevConsecL;
            unordered_map<string, tuple<int, int, int, int, bool>> prevErrors;

            //posibleSolucion[dia] U restEmployees = employees        
            for(auto& rE : restEmployees) {
                int rIdx = empleadoIdx(employees, rE.nombre);
                prevConsecT[rE.nombre] = consecutivosT[rE.nombre];
                prevConsecL[rE.nombre] = consecutivosL[rE.nombre];
                prevErrors[rE.nombre] = make_tuple(
                    employees[rIdx].errorIncumpleMaxT,
                    employees[rIdx].errorIncumpleMinT,
                    employees[rIdx].errorIncumpleMinL,
                    employees[rIdx].errorIncumpleMaxL,
                    employees[rIdx].noL
                );

                if (consecutivosT[rE.nombre] > 0) { // *T -> L
                    employees[rIdx].errorIncumpleMinT += 
                                max(0, MIN_DIAS_TRABAJADOS_CONSECUTIVOS - consecutivosT[rE.nombre]) * PESO_W4;
                    employees[rIdx].errorIncumpleMaxT += 
                                max(0, consecutivosT[rE.nombre] - MAX_DIAS_TRABAJADOS_CONSECUTIVOS) * PESO_W3;
                }
                consecutivosT[rE.nombre] = 0;
                consecutivosL[rE.nombre]++;
                employees[rIdx].noL = false;

                if(dia == DIAS - 1) { // Acaba en L
                    employees[rIdx].errorIncumpleMinL += 
                                max(0, MIN_DIAS_LIBRES_CONSECUTIVOS - consecutivosL[rE.nombre]) * PESO_W6;
                    employees[rIdx].errorIncumpleMaxL += 
                                max(0, consecutivosL[rE.nombre] - MAX_DIAS_LIBRES_CONSECUTIVOS) * PESO_W5;
                    if(employees[rIdx].noT) { // Caso LLLL --> 0 Dias Trabajados
                        employees[rIdx].errorIncumpleMinT += MIN_DIAS_TRABAJADOS_CONSECUTIVOS * PESO_W4;
                    }
                }

                int restriccionMaxL = max(0, consecutivosL[rE.nombre] - MAX_DIAS_LIBRES_CONSECUTIVOS) * PESO_W5;
                employees[rIdx].restriccionesMaxL.push(restriccionMaxL);
            }

            if(dia == DIAS - 1) { // HOJA: calcula FO
                Incumplimientos B(NUM_ENFERMERAS);
                double fo = calcularFuncionObjetivoPiston(employees, posibleSolucion, empleadoAsignaciones, B);
                if (fo < mejorSolucion.funcionObjetivo) {
                    mejorSolucion.funcionObjetivo = fo;
                    mejorSolucion.solucion = posibleSolucion;
                    mejorSolucion.B = B;
                    cout << fo << endl;
                }
            } else { // Siguiente Dia
                bool poda_sigDia = podaHeuristica(mejorSolucion, posibleSolucion, employees, empleadoAsignaciones, dia, empleadoTurnoEnDia, startTime);

                if(!poda_sigDia) {
                    vector<Empleado> restEmployeesNext = copyEmployees;
                    unordered_map<string, bool> empleadosTurnosEnDiaPrevio = empleadoTurnoEnDia;
                    for (const auto& empleado : employees) {
                        empleadoTurnoEnDia[empleado.nombre] = false;
                    }
                    nodos += explorarArbol(dia + 1, 0, employees, copyEmployees, restEmployeesNext, posibleSolucion,
                        mejorSolucion, 0, empleadoAsignaciones, empleadoTurnoEnDia, consecutivosT, consecutivosL, startTime);
                    empleadoTurnoEnDia = empleadosTurnosEnDiaPrevio;
                }
            }

            // Restaurar estado para Backtracking
            for (auto& rE: restEmployees) {
                consecutivosT[rE.nombre] = prevConsecT[rE.nombre];
                consecutivosL[rE.nombre] = prevConsecL[rE.nombre];

                int rIdx = empleadoIdx(employees, rE.nombre);
                tie(
                    employees[rIdx].errorIncumpleMaxT,
                    employees[rIdx].errorIncumpleMinT,
                    employees[rIdx].errorIncumpleMinL,
                    employees[rIdx].errorIncumpleMaxL,
                    employees[rIdx].noL
                ) = prevErrors[rE.nombre];
                employees[rIdx].restriccionesMaxL.pop();
            }
        } else if(poda) {
                // Poda: No continuamos con esta rama porque la cota inferior ya es peor que la mejor FO encontrada.
        } else { // turno < 2
            if(actualDemanda == DEMANDA - 1) { // Cambio de turno
                nodos += explorarArbol(dia, turno + 1, employees, copyEmployees, restEmployees, posibleSolucion,
                    mejorSolucion, 0, empleadoAsignaciones, empleadoTurnoEnDia, consecutivosT, consecutivosL, startTime);
            } else { // Mismo turno, actualizando la demanda
                nodos += explorarArbol(dia, turno, employees,copyEmployees, restEmployees, posibleSolucion, mejorSolucion,
                    actualDemanda + 1, empleadoAsignaciones, empleadoTurnoEnDia, consecutivosT, consecutivosL, startTime);
            }
        }

        // Restaurar estado para backtracking
        posibleSolucion[dia].pop_back();
        restEmployees.insert(restEmployees.begin() + i, e);
        empleadoAsignaciones[e.nombre]--;
        empleadoTurnoEnDia[e.nombre] = turnoEnDiaPrevio;
        consecutivosT[e.nombre] = prevConsecT;
        consecutivosL[e.nombre] = prevConsecL;
        employees[idx].errorIncumpleMaxL = prevErrorMaxL;
        employees[idx].errorIncumpleMinL = prevErrorMinL; 
        employees[idx].errorIncumpleMaxT = prevErrorMaxT;
        employees[idx].errorIncumpleMinT = prevErrorMinT;
        employees[idx].noT = prevNoT;
        employees[idx].restriccionesMaxT.pop();
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
    MIN_DIAS_LIBRES_CONSECUTIVOS = cfg.min_dias_libres_consec;
    MAX_DIAS_LIBRES_CONSECUTIVOS = cfg.max_dias_libres_consec;
    MIN_DIAS_TRABAJADOS_CONSECUTIVOS = cfg.min_dias_trab_consec;
    MAX_DIAS_TRABAJADOS_CONSECUTIVOS = cfg.max_dias_trab_consec;
    cargarPesosDesdeJSON();
    
    vector<Empleado> empleados = generarEmpleados_v1(NUM_ENFERMERAS);

    vector<vector<Empleado>> posibleSolucion(DIAS, vector<Empleado>());
    vector<Empleado> restEmployees = empleados;
    vector<Empleado> copyEmployees = empleados;
    SolucionFinal_Piston mejorSolucion = generarSolucionAleatoriaPiston(empleados);
    unordered_map<string, int> empleadoAsignaciones;
    unordered_map<string, bool> empleadoTurnoEnDia;
    unordered_map<string, int> consecutivosT;
    unordered_map<string, int> consecutivosL;
    for (const auto& empleado : empleados) {
        empleadoAsignaciones[empleado.nombre] = 0;
        empleadoTurnoEnDia[empleado.nombre] = false;
        consecutivosT[empleado.nombre] = 0;
        consecutivosL[empleado.nombre] = 0;
    }

    auto start = chrono::high_resolution_clock::now();
    int nodosExplorados = explorarArbol(0, 0, empleados, copyEmployees, restEmployees, posibleSolucion, mejorSolucion, 0,
        empleadoAsignaciones, empleadoTurnoEnDia, consecutivosT, consecutivosL, start);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    SolucionFinal solucionFinal = {mejorSolucion.funcionObjetivo, mejorSolucion.solucion};
    imprimirResultado(solucionFinal, duration.count(), nodosExplorados);
    return 0;
}