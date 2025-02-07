#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>
#include <chrono>

#define DIAS 3
#define DEMANDA 3
#define ULTIMO_TURNO 2
#define LIMITE_INFERIOR 3
#define LIMITE_SUPERIOR 4
#define PESO_W1 10.0
#define PESO_W2 10.0

using namespace std;

class Empleado {
public:
    string nombre;
    bool turnoEnDia;
    Empleado(string nombre)
        : nombre(nombre), turnoEnDia(false) {}
};

class Nurse : public Empleado {
public:
    Nurse(string nombre)
        : Empleado(nombre) {}
};

class Doctor : public Empleado {
public:
    Doctor(string nombre)
        : Empleado(nombre) {}
};

struct Turno {
    string nombre;
};

struct Turnos {
    Turno early = {"Early"};
    Turno day = {"Day"};
    Turno late = {"Late"};
};

struct SolucionFinal {
    double funcionObjetivo;
    vector<vector<Empleado>> solucion;
};

double calcularFuncionObjetivo(const vector<vector<Empleado>>& solucion, int LS, int US, double w1, double w2) {
    double fo = 0;
    unordered_map<string, int> empleadoMap;

    for (const auto& empleadosDia : solucion) {
        for (const auto& empleado : empleadosDia) {
            empleadoMap[empleado.nombre]++;
        }
    }

    for (const auto& empleado : empleadoMap) {
        int totalTurnosEmpleado = empleado.second;
        int e1 = max(0, totalTurnosEmpleado - US);
        double r1 = w1 * e1;

        int e2 = max(0, LS - totalTurnosEmpleado);
        double r2 = w2 * e2;

        fo += (r1 + r2);
    }

    return fo;
}

int podas = 0, it = 0;

void explorarArbol(int dia, int turno, const vector<Empleado>& employees, vector<Empleado> restEmployees,
                   vector<vector<Empleado>> posibleSolucion,
                   unordered_map<string, int>& empleadoMap,
                   int actualDemanda, SolucionFinal& mejorSolucion, double F_actual) {
    for (int i = 0; i < restEmployees.size(); i++) {
        Empleado e = restEmployees[i];
        restEmployees.erase(restEmployees.begin() + i);
        posibleSolucion[dia].push_back(e);
        it++;

        // Actualizar F de forma incremental
        double restDays = (turno == ULTIMO_TURNO) ? DIAS - dia - 1 : DIAS - dia;
        int asignaciones = empleadoMap[e.nombre] + 1;
        double ei = max(0, asignaciones - LIMITE_SUPERIOR);
        double ei_prime = max(0, LIMITE_INFERIOR - asignaciones);
        //double ki = (ei_prime - restDays > 0) ? ei_prime : 0.0;
        double ki = max(0.0, ei_prime - restDays);
        double nuevo_F = F_actual + (PESO_W1 * ei + PESO_W2 * ki);
        empleadoMap[e.nombre]++;

        if (nuevo_F >= mejorSolucion.funcionObjetivo) { // Poda
                //cout << "(nuevo_F, dia, turno, demanda) = (" << nuevo_F <<"," << dia << "," << turno << "," << actualDemanda <<")" << endl;
            podas++;
            empleadoMap[e.nombre]--;
            posibleSolucion[dia].pop_back();
            restEmployees.insert(restEmployees.begin() + i, e);
            continue;
        }

        if (actualDemanda == DEMANDA - 1 && turno == ULTIMO_TURNO && dia == DIAS - 1) { // HOJA: calcula FO
            double fo = calcularFuncionObjetivo(posibleSolucion, LIMITE_INFERIOR, LIMITE_SUPERIOR, PESO_W1, PESO_W2);
            // cout << fo << endl;
            if (fo < mejorSolucion.funcionObjetivo) {
                // cout << "entro fo: " << fo << endl;
                mejorSolucion.funcionObjetivo = fo;
                mejorSolucion.solucion = posibleSolucion;
            }
        } else if (turno == 2 && actualDemanda == DEMANDA - 1) {
            vector<Empleado> restEmployeesNext = employees;
            explorarArbol(dia + 1, 0, employees, restEmployeesNext, posibleSolucion, empleadoMap, 0, mejorSolucion, nuevo_F);
        } else { // turno < 2
            if (actualDemanda == DEMANDA - 1) { // Cambio de turno
                explorarArbol(dia, turno + 1, employees, restEmployees, posibleSolucion, empleadoMap, 0, mejorSolucion, nuevo_F);
            } else { // Mismo turno, actualizando la demanda
                explorarArbol(dia, turno, employees, restEmployees, posibleSolucion, empleadoMap, actualDemanda + 1, mejorSolucion, nuevo_F);
            }
        }

        // Restaurar estado para backtracking
        empleadoMap[e.nombre]--;
        posibleSolucion[dia].pop_back();
        restEmployees.insert(restEmployees.begin() + i, e);
    }
}

int main() {
    vector<Nurse> enfermeras = {
        {"Ana"}, {"Bea"}, {"Clara"}, {"Maria"}, {"Silvia"}, {"Paula"}, {"Andrea"},
        {"Antonia"}, {"Berta"}, {"Ainhoa"}, {"Sandra"}, {"Mia"}
    };

    if (enfermeras.size() < 3 * DEMANDA) {
        cout << "Necesitas " << 3 * DEMANDA - enfermeras.size() << " o mas enfermeras para asignar segun la demanda que pides";
        return 0;
    }

    vector<Empleado> empleados;
    for (auto& e : enfermeras) {
        empleados.push_back(e);
    }

    vector<vector<Empleado>> posibleSolucion(DIAS, vector<Empleado>());
    vector<Empleado> restEmployees = empleados;
    unordered_map<string, int> empleadoMap;
    for (const auto& empleado : empleados) {
        empleadoMap[empleado.nombre] = 0;
    }

    SolucionFinal mejorSolucion = {INFINITY, {}};

    auto start = chrono::high_resolution_clock::now();
    explorarArbol(0, 0, empleados, restEmployees, posibleSolucion, empleadoMap, 0, mejorSolucion, 0.0);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "FO Minima: " << mejorSolucion.funcionObjetivo << endl;
    cout << "Ramas podadas: " << podas << endl;
    cout << "Iteraciones podadas: " << (static_cast<double>(podas) / it) * 100 << "%" << endl;
    cout << "Tiempo de exec: " << duration.count() << " s" << endl;

    cout << "Horario optimo: " << endl;
    Turnos turnos;
    vector<string> tiposTurnos = {turnos.early.nombre, turnos.day.nombre, turnos.late.nombre};
    for (int dia = 0; dia < mejorSolucion.solucion.size(); ++dia) {
        cout << endl << "Dia " << dia + 1 << ":" << endl;
        for (int turno = 0; turno < 3; ++turno) {
            cout << "\t" << tiposTurnos[turno] << ": {";

            int inicio = turno * DEMANDA;
            int fin = inicio + DEMANDA;

            for (int j = inicio; j < fin && j < mejorSolucion.solucion[dia].size(); j++) {
                cout << mejorSolucion.solucion[dia][j].nombre;

                if (j < fin - 1) {
                    cout << ", ";
                } else {
                    cout << "}";
                }
            }
            cout << endl;
        }
    }
    return 0;
}