#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <random>
#include <unordered_set>
#include "include/empleado_v2.h"
#include "include/general_utils_v2.h"
#include "include/main_utils_v2.h"
#include "include/solucion_v2.h"
#include "include/turnos_v2.h"

#define TRABAJA true
#define ULTIMO_TURNO 2
#define TURNOS 3
#define TABU_TENURE 10
#define NUM_VECINOS 5

void calcularErroresEmpelados(vector<Empleado>& empleados, const vector<vector<Empleado>>& horario) {
    for(auto& e: empleados) {
        vector<bool> vector_trab_e; // T = True , L = False

        for (const auto& dia : horario) {
            bool trabaja_en_dia = !TRABAJA;
            for (const auto& asignacion : dia) {
                if (asignacion == e) {
                    trabaja_en_dia = TRABAJA;
                    e.noT = false;
                    break;
                }
            }
            vector_trab_e.push_back(trabaja_en_dia);
            if(!trabaja_en_dia) { // L
                e.noL = false;
            }
        }
        if(e.noL) { // Todo T --> 0 L
            e.errorIncumpleMinL += MIN_DIAS_LIBRES_CONSECUTIVOS * PESO_W1;
            e.errorIncumpleMinT += max(0, MIN_DIAS_TRABAJADOS_CONSECUTIVOS - DIAS) * PESO_W1;
            e.errorIncumpleMaxT += max(0, DIAS - MAX_DIAS_TRABAJADOS_CONSECUTIVOS) * PESO_W1;
            continue;
        } else if(e.noT) { // Todo L --> 0 T
            e.errorIncumpleMinT += MIN_DIAS_TRABAJADOS_CONSECUTIVOS * PESO_W1;
            e.errorIncumpleMinL += max(0, MIN_DIAS_LIBRES_CONSECUTIVOS - DIAS) * PESO_W1;
            e.errorIncumpleMaxL += max(0, DIAS - MAX_DIAS_LIBRES_CONSECUTIVOS) * PESO_W1;
            continue;
        }

        int trab_consec = 0;
        int free_consec = 0;

        for (int dia = 0; dia < DIAS; dia++) {
            bool actual = vector_trab_e[dia];
            bool siguienteDiaTrabajado = ((dia + 1) < DIAS) ? vector_trab_e[dia + 1] : false;
            bool ultimoDia = (dia == (DIAS - 1)) ? true : false;

            if (actual) { // T
                trab_consec++;
                if (!siguienteDiaTrabajado) { // T->L or ultimo dia --> Mirar los trabajados
                    e.errorIncumpleMaxT += max(0, trab_consec - MAX_DIAS_TRABAJADOS_CONSECUTIVOS) * PESO_W1;
                    e.errorIncumpleMinT += max(0, MIN_DIAS_TRABAJADOS_CONSECUTIVOS - trab_consec) * PESO_W1;
                    trab_consec = 0;
                }
            } else { // L
                free_consec++;
                if (siguienteDiaTrabajado || ultimoDia) { // L->T or ultimo dia --> Mirar los libres
                    e.errorIncumpleMaxL += max(0, free_consec - MAX_DIAS_LIBRES_CONSECUTIVOS) * PESO_W1;
                    e.errorIncumpleMinL += max(0, MIN_DIAS_LIBRES_CONSECUTIVOS - free_consec) * PESO_W1;
                    free_consec = 0;
                }
            }
        }
    }
}

SolucionFinal generarSolucionAleatoria(const vector<Empleado>& empleados) {
    vector <Empleado> employees = empleados;
    vector<Turno> turnos = {Turnos().early, Turnos().day, Turnos().late};
    vector<int> demandas = {DEM_EARLY, DEM_DAY, DEM_LATE};
    vector<vector<Empleado>> horario(DIAS, vector<Empleado>());
    unordered_map<string, int> empleadoAsignaciones;

    for(const auto& e: empleados) {
        empleadoAsignaciones[e.nombre] = 0;
    }

    for (int dia = 0; dia < DIAS; dia++) {
        vector<Empleado> empleadosDisponibles = empleados;
        shuffle(empleadosDisponibles.begin(), empleadosDisponibles.end(),
                     mt19937(chrono::steady_clock::now().time_since_epoch().count()));

        int demandaAcomulada = 0;
        for (int t = 0; t < TURNOS; t++) {
            for(int i = demandaAcomulada; i < (demandaAcomulada + demandas[t]); i++) {
                horario[dia].push_back(empleadosDisponibles[i]);
                empleadoAsignaciones[empleadosDisponibles[i].nombre]++;
            }
            demandaAcomulada += demandas[t];
        }
    }

    calcularErroresEmpelados(employees, horario);
    calcularErroresPatronesEmpleados(employees, horario, TRABAJA);
    double fo = calcularFuncionObjetivo(employees, horario, empleadoAsignaciones);
    return {fo, horario};
}

vector<SolucionFinal> generarVecinos(
    const SolucionFinal& solucionActual,
    const vector<Empleado>& empleados,
    mt19937& rng
) {
    vector<SolucionFinal> vecinos;
    vector<int> demandas = {DEM_EARLY, DEM_DAY, DEM_LATE};
    int DEMANDA_EN_DIA = (DEM_EARLY + DEM_DAY + DEM_LATE);

    for (int i = 0; i < NUM_VECINOS; i++) {
        SolucionFinal nuevoVecino = solucionActual;

        // 1. Intercambio de patrones de turnos en 2 días consecutivos (d y d+1)
        if (rng() % 2 == 0) {
            int dia1 = rng() % DIAS;
            int dia2 = (dia1 + 1) % DIAS;

            int demandaAcomulada = 0;
            for (int t = 0; t < TURNOS; t++) {
                for(int j = demandaAcomulada; j < (demandaAcomulada + demandas[t]); j++) {
                    swap(nuevoVecino.solucion[dia1][j], nuevoVecino.solucion[dia2][j]);
                }
                demandaAcomulada += demandas[t];
            }
        }

        // 2. Reasignación de turnos --> Reemplazar una enf por otra no asignada en un día d
        if (rng() % 2 == 0) {
            int dia = rng() % DIAS;
            int idx = rng() % (DEMANDA_EN_DIA);
            
            vector<Empleado> solucionEnDia = nuevoVecino.solucion[dia];
            vector<Empleado> restoCandidatos;
            for (const auto& e : empleados) {
                if (find(solucionEnDia.begin(), solucionEnDia.end(), e) == solucionEnDia.end()) {
                    restoCandidatos.push_back(e);
                }
            }

            if (!restoCandidatos.empty()) {
                nuevoVecino.solucion[dia][idx] = restoCandidatos[rng() % restoCandidatos.size()];
            }
        }

        unordered_map<string, int> empleadoAsignaciones;
        for(const auto& e: empleados) {
            empleadoAsignaciones[e.nombre] = 0;
        }
        
        for (const auto& dia : nuevoVecino.solucion) {
            for (const auto& e : dia) {
                empleadoAsignaciones[e.nombre]++;
            }
        }

        vector<Empleado> employees = empleados;
        calcularErroresEmpelados(employees, nuevoVecino.solucion);
        calcularErroresPatronesEmpleados(employees, nuevoVecino.solucion, TRABAJA);
        nuevoVecino.funcionObjetivo = calcularFuncionObjetivo(employees, nuevoVecino.solucion, empleadoAsignaciones);
        vecinos.push_back(nuevoVecino);
    }

    return vecinos;
}

SolucionFinal tabuSearch(const vector<Empleado>& empleados) {
    auto start_time = chrono::high_resolution_clock::now();
    SolucionFinal mejorSolucion = generarSolucionAleatoria(empleados);
    SolucionFinal solucionActual = mejorSolucion;
    vector<SolucionFinal> listaTabu;

    while (chrono::duration<double>(chrono::high_resolution_clock::now() - start_time).count() < LIMITE_TIEMPO) {
        mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
        vector<SolucionFinal> vecinos = generarVecinos(solucionActual, empleados, rng);   
        SolucionFinal mejorVecino = {numeric_limits<double>::max(), {}};
        bool todosLosElementosEnListaTabu = true;

        for (const auto& vecino : vecinos) {
            if (find(listaTabu.begin(), listaTabu.end(), vecino) == listaTabu.end()) {
                if (vecino.funcionObjetivo < mejorVecino.funcionObjetivo) {
                    mejorVecino = vecino;
                    todosLosElementosEnListaTabu = false;
                }
            }
        }

        if (todosLosElementosEnListaTabu) { // Si todos los vecinos están en la lista tabú escoge el mejor tabú
            mejorVecino = *min_element(vecinos.begin(), vecinos.end(), [](const SolucionFinal& a, const SolucionFinal& b) {
                return a.funcionObjetivo < b.funcionObjetivo;
            });
        }

        solucionActual = mejorVecino;

        if (mejorVecino.funcionObjetivo < mejorSolucion.funcionObjetivo) {
            mejorSolucion = mejorVecino;
                //cout << mejorSolucion.funcionObjetivo << endl;
        }

        listaTabu.push_back(solucionActual);
        if (listaTabu.size() > TABU_TENURE) {
            listaTabu.erase(listaTabu.begin());
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

    auto start = chrono::high_resolution_clock::now();
    SolucionFinal mejorSolucion = tabuSearch(empleados);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    imprimirResultado(mejorSolucion, duration.count());

    return 0;
}