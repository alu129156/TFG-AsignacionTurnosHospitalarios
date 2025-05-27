#include "general_utils_v1.h"
#include "solucion_random_generator.h"
#include "solucion_v1.h"
#include "empleado_v1.h"
#include <vector>
#include <unordered_map>
#include <random>
using namespace std;

SolucionFinal_Piston generarSolucionAleatoriaPiston(const vector<Empleado>& empleados) {
    vector<vector<Empleado>> solucion(DIAS, vector<Empleado>());
    unordered_map<string, int> empleadoAsignaciones;
    vector<Empleado> employees = empleados;

    unordered_map<string, int> consecutivosT;
    unordered_map<string, int> consecutivosL;

    for (const auto& empleado : empleados) {
        consecutivosT[empleado.nombre] = 0;
        consecutivosL[empleado.nombre] = 0;
    }

    for (int dia = 0; dia < DIAS; dia++) {
        vector<Empleado> empleadosDisponibles = employees;
        shuffle(empleadosDisponibles.begin(), empleadosDisponibles.end(),
                 mt19937(chrono::steady_clock::now().time_since_epoch().count()));
        for (int turno = 0; turno < ULTIMO_TURNO + 1; turno++) {
            for (int i = 0; i < DEMANDA; i++) {
                Empleado e = empleadosDisponibles[i + turno * DEMANDA];
                int idx = empleadoIdx(employees, e.nombre);
                solucion[dia].push_back(e);
                empleadoAsignaciones[e.nombre]++;
                employees[idx].noT = false;

                if (consecutivosL[e.nombre] > 0) { // *L -> T
                    employees[idx].errorIncumpleMinL += max(0, MIN_DIAS_LIBRES_CONSECUTIVOS - consecutivosL[e.nombre]) * PESO_W6;
                    employees[idx].errorIncumpleMaxL += max(0, consecutivosL[e.nombre] - MAX_DIAS_LIBRES_CONSECUTIVOS) * PESO_W5;
                    consecutivosL[e.nombre] = 0;
                }
                consecutivosT[e.nombre]++;

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
            }
        }

        // Actualizar libres consecutivos para el rest Employers del dia
        for (const auto& empleado : employees) {
            if (
                find(solucion[dia].begin(), solucion[dia].end(), empleado) == solucion[dia].end()
               ) { // L
                int rIdx = empleadoIdx(employees, empleado.nombre);

                if (consecutivosT[empleado.nombre] > 0) { // *T -> L
                    employees[rIdx].errorIncumpleMinT += 
                                max(0, MIN_DIAS_TRABAJADOS_CONSECUTIVOS - consecutivosT[empleado.nombre]) * PESO_W4;
                    employees[rIdx].errorIncumpleMaxT += 
                                max(0, consecutivosT[empleado.nombre] - MAX_DIAS_TRABAJADOS_CONSECUTIVOS) * PESO_W3;
                }
                consecutivosT[empleado.nombre] = 0;
                consecutivosL[empleado.nombre]++;
                employees[rIdx].noL = false;
            }

            if(dia == DIAS - 1) { // Acaba en L
                int rIdx = empleadoIdx(employees, empleado.nombre);
                employees[rIdx].errorIncumpleMinL += 
                            max(0, MIN_DIAS_LIBRES_CONSECUTIVOS - consecutivosL[empleado.nombre]) * PESO_W6;
                employees[rIdx].errorIncumpleMaxL += 
                            max(0, consecutivosL[empleado.nombre] - MAX_DIAS_LIBRES_CONSECUTIVOS) * PESO_W5;
                if(employees[rIdx].noT) { // Caso LLLL --> 0 Dias Trabajados
                    employees[rIdx].errorIncumpleMinT += MIN_DIAS_TRABAJADOS_CONSECUTIVOS * PESO_W3;
                }
            }
        }
    }

    Incumplimientos B(NUM_ENFERMERAS);
    double fo = calcularFuncionObjetivoPiston(employees, solucion, empleadoAsignaciones, B);
    return {fo, solucion, B};
}