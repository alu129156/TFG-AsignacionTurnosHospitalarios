#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>
#include <chrono>

// #define DIAS 4
// #define DEMANDA 3
# define LIMITE_TIEMPO 420
#define ULTIMO_TURNO 2
// #define LIMITE_INFERIOR 5
// #define LIMITE_SUPERIOR 5
#define PESO_W1 10.0
#define PESO_W2 10.0
int DIAS;
int DEMANDA;
int LIMITE_INFERIOR;
int LIMITE_SUPERIOR;

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

void verifyTime(const chrono::time_point<chrono::high_resolution_clock>& startTime) {
    auto now = chrono::high_resolution_clock::now();
    double elapsed = chrono::duration<double>(now - startTime).count();
    if (elapsed > LIMITE_TIEMPO) {
        cout << "{\n\terror: Tiempo excedido\n}\n\n";
        exit(1);
    }
}

double calcularFuncionObjetivo(const vector<vector<Empleado>>& solucion, int LS, int US, double w1, double w2) {
    double fo = 0.0;
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

double calcularF(
    unordered_map<string, int>& empleadoAsignaciones,
    int dia,
    unordered_map<string, bool>& empleadoTurnoEnDia
) {
    double f = 0.0;
    for (const auto& [nombreEmpleado, asignacionesEmpleado] : empleadoAsignaciones) {
        if (asignacionesEmpleado > 0) {
            bool turnoEmpleadoEnDia = empleadoTurnoEnDia[nombreEmpleado];
            double restDays = (turnoEmpleadoEnDia) ? DIAS - dia - 1 : DIAS - dia;

            double ei = max(0, asignacionesEmpleado - LIMITE_SUPERIOR);
            double ei_prime = max(0, LIMITE_INFERIOR - asignacionesEmpleado);
            double ki = max(0.0, ei_prime - restDays);

            f += PESO_W1 * ei + PESO_W2 * ki;
        }
    }
    return f;
}

int it = 0;

void explorarArbol(
    int dia, int turno,
    const vector<Empleado>& employees, 
    vector<Empleado> restEmployees,
    vector<vector<Empleado>>& posibleSolucion,
    unordered_map<string, int>& empleadoAsignaciones,
    unordered_map<string, bool> empleadoTurnoEnDia,
    int actualDemanda, SolucionFinal& mejorSolucion,
    const chrono::time_point<chrono::high_resolution_clock>& startTime
) {

    for (int i = 0; i < restEmployees.size(); i++) {
        verifyTime(startTime);

        it++;
        Empleado e = restEmployees[i];
        bool turnoEnDiaPrevio = empleadoTurnoEnDia[e.nombre];
        restEmployees.erase(restEmployees.begin() + i);
        posibleSolucion[dia].push_back(e);
        empleadoAsignaciones[e.nombre]++;
        empleadoTurnoEnDia[e.nombre] = true;

        double F = calcularF(empleadoAsignaciones, dia, empleadoTurnoEnDia);

        if (actualDemanda == DEMANDA - 1 && turno == ULTIMO_TURNO && dia == DIAS - 1) { // HOJA
            double fo = calcularFuncionObjetivo(posibleSolucion, LIMITE_INFERIOR, LIMITE_SUPERIOR, PESO_W1, PESO_W2);
            //cout << "Entro"<<endl;
            if (fo < mejorSolucion.funcionObjetivo) {
                    //cout <<"fo:"<<fo<<endl;
                mejorSolucion.funcionObjetivo = fo;
                mejorSolucion.solucion = posibleSolucion;
            }
        } else if (F >= mejorSolucion.funcionObjetivo) { // Poda
                    //cout << "(nuevo_F, dia, turno, demanda) = (" << F <<"," << dia << "," << turno << "," << actualDemanda <<")" << endl;
            empleadoAsignaciones[e.nombre]--;
            empleadoTurnoEnDia[e.nombre] = turnoEnDiaPrevio;
            posibleSolucion[dia].pop_back();
            restEmployees.insert(restEmployees.begin() + i, e);
            continue;
        } else if (turno == 2 && actualDemanda == DEMANDA - 1) {
            vector<Empleado> restEmployeesNext = employees;
            unordered_map<string, bool> empleadosTurnosEnDiaPrevio = empleadoTurnoEnDia;
            for (const auto& empleado : employees) {
                empleadoTurnoEnDia[empleado.nombre] = false;
            }
            explorarArbol(dia + 1, 0, employees, restEmployeesNext, posibleSolucion,
             empleadoAsignaciones, empleadoTurnoEnDia, 0, mejorSolucion, startTime);

            // Restaurar estado para el backtracking
            empleadoTurnoEnDia = empleadosTurnosEnDiaPrevio;
        } else { // turno < 2
            if (actualDemanda == DEMANDA - 1) { // Cambio de turno
                explorarArbol(dia, turno + 1, employees, restEmployees, posibleSolucion,
                 empleadoAsignaciones, empleadoTurnoEnDia, 0, mejorSolucion, startTime);
            } else { // Mismo turno, actualizando la demanda
                explorarArbol(dia, turno, employees, restEmployees, posibleSolucion, empleadoAsignaciones,
                 empleadoTurnoEnDia, actualDemanda + 1, mejorSolucion, startTime);
            }
        }

        // Restaurar estado para backtracking
        empleadoAsignaciones[e.nombre]--;
        empleadoTurnoEnDia[e.nombre] = turnoEnDiaPrevio;
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
    unordered_map<string, int> empleadoAsignaciones;
    unordered_map<string, bool> empleadoTurnoEnDia;

    for (const auto& empleado : empleados) {
        empleadoAsignaciones[empleado.nombre] = 0;
        empleadoTurnoEnDia[empleado.nombre] = false;
    }

    SolucionFinal mejorSolucion = {INFINITY, {}};
    auto start = chrono::high_resolution_clock::now();
    explorarArbol(0, 0, empleados, restEmployees, posibleSolucion, empleadoAsignaciones,
                     empleadoTurnoEnDia, 0, mejorSolucion, start);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "{" << endl;
    cout << "\tFO Minima: " << mejorSolucion.funcionObjetivo << "," << endl;
    cout << "\tIteraciones: " << it << ","  << endl;
    cout << "\tTiempo de exec: " << duration.count() << " segundos," << endl;

    cout << "\tHorario optimo:\n\t{" << endl;
    Turnos turnos;
    vector<string> tiposTurnos = {turnos.early.nombre, turnos.day.nombre, turnos.late.nombre};
    for (int dia = 0; dia < mejorSolucion.solucion.size(); dia++) {
        cout << "\t\tDia " << dia + 1 << ":\n\t\t{\n";
        for (int turno = 0; turno < 3; turno++) {
            cout << "\t\t\t" << tiposTurnos[turno] << ": {";

            int inicio = turno * DEMANDA;
            int fin = inicio + DEMANDA;

            for (int j = inicio; j < fin && j < mejorSolucion.solucion[dia].size(); j++) {
                cout << mejorSolucion.solucion[dia][j].nombre;

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