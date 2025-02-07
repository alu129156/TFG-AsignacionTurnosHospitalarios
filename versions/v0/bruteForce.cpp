#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>
#include <chrono>

int DIAS;
int DEMANDA;
#define ULTIMO_TURNO 2
int LIMITE_INFERIOR;
int LIMITE_SUPERIOR;
#define PESO_W1 10.0
#define PESO_W2 10.0

using namespace std;

class Empleado {
public:
    string nombre;
    Empleado(string nombre)
        : nombre(nombre) {}
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
        for(const auto& empleado: empleadosDia) {
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

int it = 0;

void explorarArbol(int dia, int turno, const vector<Empleado>& employees, vector<Empleado> restEmployees,
                            vector<vector<Empleado>> posibleSolucion, SolucionFinal& mejorSolucion, int actualDemanda) {
    for(int i = 0; i < restEmployees.size(); i++) {
        it++;
        Empleado e = restEmployees[i];
        restEmployees.erase(restEmployees.begin() + i);
        posibleSolucion[dia].push_back(e);

        if(actualDemanda == DEMANDA - 1 && turno == ULTIMO_TURNO && dia == DIAS - 1) { // HOJA: calcula FO
            double fo = calcularFuncionObjetivo(posibleSolucion, LIMITE_INFERIOR, LIMITE_SUPERIOR, PESO_W1, PESO_W2);
            if (fo < mejorSolucion.funcionObjetivo) {
                mejorSolucion.funcionObjetivo = fo;
                mejorSolucion.solucion = posibleSolucion;
            }
        } else if(turno == 2 && actualDemanda == DEMANDA - 1) {
            vector<Empleado> restEmployeesNext = employees;
            explorarArbol(dia + 1, 0, employees, restEmployeesNext, posibleSolucion, mejorSolucion, 0);
        } else { // turno < 2
            if(actualDemanda == DEMANDA - 1) { // Cambio de turno
                explorarArbol(dia, turno + 1, employees, restEmployees, posibleSolucion, mejorSolucion, 0);
            } else { // Mismo turno, actualizando la demanda
                explorarArbol(dia, turno, employees, restEmployees, posibleSolucion, mejorSolucion, actualDemanda + 1);
            }
        }

        // Restaurar estado para backtracking
        posibleSolucion[dia].pop_back();
        restEmployees.insert(restEmployees.begin() + i, e);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        cout << "Uso: programa <num_enfermeras> <num_dias> <demanda> <LS> <US>" << endl;
        return 1;
    }

    int num_enfermeras = stoi(argv[1]);
    DIAS = stoi(argv[2]);
    DEMANDA = stoi(argv[3]);
    LIMITE_INFERIOR = stoi(argv[4]);
    LIMITE_SUPERIOR = stoi(argv[5]);
    if(num_enfermeras < 3 * DEMANDA) {
        cout << "Necesitas " << 3 * DEMANDA - num_enfermeras << " o mas enfermeras para asignar segun la demanda que pides";
        return 1;
    }

    vector<Empleado> empleados;
    for (int i = 0; i < num_enfermeras; i++) {
        empleados.emplace_back("e_" + to_string(i + 1));
    }

    vector<vector<Empleado>> posibleSolucion(DIAS, vector<Empleado>());
    vector<Empleado> restEmployees = empleados;
    SolucionFinal solucion = {INFINITY, {}};

    auto start = chrono::high_resolution_clock::now();
    explorarArbol(0, 0, empleados, restEmployees, posibleSolucion, solucion, 0);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "{" << endl;
    cout << "\tFO Minima: " << solucion.funcionObjetivo << "," << endl;
    cout << "\tIteraciones: " << it << "," << endl;
    cout << "\tTiempo de exec: " << duration.count() << " segundos," << endl;
    cout << "\tHorario optimo:\n\t{" << endl;
    Turnos turnos;
    vector<string> tiposTurnos = {turnos.early.nombre, turnos.day.nombre, turnos.late.nombre};
    for (int dia = 0; dia < solucion.solucion.size(); dia++) {
        cout << "\t\tDia " << dia + 1 << ":\n\t\t{\n";
        for (int turno = 0; turno < 3; turno++) {
            cout << "\t\t\t" << tiposTurnos[turno] << ": {";

            int inicio = turno * DEMANDA;
            int fin = inicio + DEMANDA;

            for (int j = inicio; j < fin && j < solucion.solucion[dia].size(); j++) {
                cout << solucion.solucion[dia][j].nombre;

                if (j < fin - 1) {
                    cout << ", ";
                } else {
                    cout << "},";
                }
            }
            cout << endl;
        }
        cout << "\t\t},\n";
    }
    cout << "\t}\n}\n\n";
    return 0;
}