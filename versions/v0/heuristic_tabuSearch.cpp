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
#include <queue>
#include <chrono>
#include <random>
#include <unordered_set>

#define MAX_COLA_SIZE 1000
#define TURNOS 3
#define TABU_TENURE 10
#define NUM_VECINOS 5

using namespace std;

SolucionFinal generarSolucionAleatoria(const vector<Empleado>& empleados) {
    vector<vector<Empleado>> horario(DIAS, vector<Empleado>());
    unordered_map<string, int> empleadoAsignaciones;

    for(const auto& e: empleados) {
        empleadoAsignaciones[e.nombre] = 0;
    }
    
    for (int dia = 0; dia < DIAS; dia++) {
        vector<Empleado> empleadosDisponibles = empleados;
        shuffle(empleadosDisponibles.begin(), empleadosDisponibles.end(), mt19937(chrono::steady_clock::now().time_since_epoch().count()));

        for (int i = 0; i < TURNOS * DEMANDA; i++) {
            horario[dia].push_back(empleadosDisponibles[i]);
            empleadoAsignaciones[empleadosDisponibles[i].nombre]++;
        }
    }

    double fo = calcularFuncionObjetivo(horario, empleadoAsignaciones);
    return {fo, horario};
}

vector<SolucionFinal> generarVecinos(const SolucionFinal& solucionActual, const vector<Empleado>& empleados, mt19937& rng) {
    vector<SolucionFinal> vecinos;

    for (int i = 0; i < NUM_VECINOS; i++) {
        SolucionFinal nuevoVecino = solucionActual;

        // 1. Intercambio de patrones de turnos en 2 días consecutivos (d y d+1)
        if (rng() % 2 == 0) {
            int dia1 = rng() % DIAS;
            int dia2 = (dia1 + 1) % DIAS;
            for (int j = 0; j < TURNOS * DEMANDA; j++) {
                swap(nuevoVecino.solucion[dia1][j], nuevoVecino.solucion[dia2][j]);
            }
        }

        // 2. Reasignación de turnos --> Reemplazar una enf por otra no asignada en un día d
        if (rng() % 2 == 0) {
            int dia = rng() % DIAS;
            int idx = rng() % (TURNOS * DEMANDA);
            
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

        nuevoVecino.funcionObjetivo = calcularFuncionObjetivo(nuevoVecino.solucion, empleadoAsignaciones);
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
    DEMANDA = cfg.demanda;
    MIN_ASIGNACIONES = cfg.min_asig;
    MAX_ASIGNACIONES = cfg.max_asig;

    vector<Empleado> empleados = generarEmpleados(NUM_ENFERMERAS);

    auto start = chrono::high_resolution_clock::now();
    SolucionFinal mejorSolucion = tabuSearch(empleados);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    imprimirResultado(mejorSolucion, chrono::duration<double>(end - start).count());
    return 0;
}