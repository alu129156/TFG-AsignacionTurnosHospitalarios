#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>
#include <chrono>

# define LIMITE_TIEMPO 420
#define ULTIMO_TURNO 2
#define PESO_W1 10.0
#define PESO_W2 10.0
int NUM_ENFERMERAS;
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

void printInput() {
    cout << "\t\t\"input_data\":\n\t\t{\n\t\t\t\"numero_enfermeras\": " << NUM_ENFERMERAS << 
    ",\n\t\t\t\"dias\": " << DIAS << ",\n\t\t\t\"demanda\": " << DEMANDA << 
    ",\n\t\t\t\"limite_inferior_asignaciones\": " << LIMITE_INFERIOR << 
    ",\n\t\t\t\"limite_superior_asignaciones\": " << LIMITE_SUPERIOR << "\n\t\t},\n";
}

void verifyTime(const chrono::time_point<chrono::high_resolution_clock>& startTime) {
    auto now = chrono::high_resolution_clock::now();
    double elapsed = chrono::duration<double>(now - startTime).count();
    if (elapsed > LIMITE_TIEMPO) {
        cout << "\t{\n";
        printInput();
        cout << "\t\t\"mensaje\": \"se ha excedido del tiempo maximo permitido\"\n\t}";
        exit(1);
    }
}

double calcularFuncionObjetivo(const vector<vector<Empleado>>& solucion, unordered_map<string, int>& empleadoAsignaciones) {
    double fo = 0.0;

    for (const auto& empleado : empleadoAsignaciones) {
        int asignacionesEmpleado = empleado.second;
        if(asignacionesEmpleado > 0) {
            int e1 = max(0, asignacionesEmpleado - LIMITE_SUPERIOR);
            double r1 = PESO_W1 * e1;
    
            int e2 = max(0, LIMITE_INFERIOR - asignacionesEmpleado);
            double r2 = PESO_W2 * e2;
    
            fo += (r1 + r2);
        }
    }
    return fo;
}

double calcularCotaInf(
    unordered_map<string, int>& empleadoAsignaciones,
    int dia
) {
    double f = 0.0;
    for (const auto& [nombreEmpleado, asignacionesEmpleado] : empleadoAsignaciones) {
        if (asignacionesEmpleado > 0) {
            double restDays = DIAS - dia;

            double ei = max(0, asignacionesEmpleado - LIMITE_SUPERIOR);
            double ei_prime = max(0, LIMITE_INFERIOR - asignacionesEmpleado);
            double ki = max(0.0, ei_prime - restDays);

            f += PESO_W1 * ei + PESO_W2 * ki;
        }
    }
    return f;
}

int explorarArbol(
    int dia, int turno,
    const vector<Empleado>& employees, 
    vector<Empleado> restEmployees,
    vector<vector<Empleado>>& posibleSolucion,
    unordered_map<string, int>& empleadoAsignaciones,
    int actualDemanda, SolucionFinal& mejorSolucion,
    const chrono::time_point<chrono::high_resolution_clock>& startTime
) {
    int nodos = 0;

    for (int i = 0; i < restEmployees.size(); i++) {
        verifyTime(startTime);

        nodos++;
        Empleado e = restEmployees[i];
        restEmployees.erase(restEmployees.begin() + i);
        posibleSolucion[dia].push_back(e);
        empleadoAsignaciones[e.nombre]++;

        double cotaInf = calcularCotaInf(empleadoAsignaciones, dia);

        if (actualDemanda == DEMANDA - 1 && turno == ULTIMO_TURNO && dia == DIAS - 1) { // HOJA
            double fo = calcularFuncionObjetivo(posibleSolucion, empleadoAsignaciones);
            if (fo < mejorSolucion.funcionObjetivo) {
                mejorSolucion.funcionObjetivo = fo;
                mejorSolucion.solucion = posibleSolucion;
            }
        } else if (cotaInf >= mejorSolucion.funcionObjetivo) { // Poda
                    //cout << "(nuevo_F, dia, turno, demanda) = (" << cotaInf <<"," << dia << "," << turno << "," << actualDemanda <<")" << endl;
            empleadoAsignaciones[e.nombre]--;
            posibleSolucion[dia].pop_back();
            restEmployees.insert(restEmployees.begin() + i, e);
            continue;
        } else if (turno == ULTIMO_TURNO && actualDemanda == DEMANDA - 1) {
            vector<Empleado> restEmployeesNext = employees;
            nodos += explorarArbol(dia + 1, 0, employees, restEmployeesNext, posibleSolucion,
             empleadoAsignaciones, 0, mejorSolucion, startTime);
        } else { // turno < 2
            if (actualDemanda == DEMANDA - 1) { // Cambio de turno
                nodos += explorarArbol(dia, turno + 1, employees, restEmployees, posibleSolucion,
                 empleadoAsignaciones, 0, mejorSolucion, startTime);
            } else { // Mismo turno, actualizando la demanda
                nodos += explorarArbol(dia, turno, employees, restEmployees, posibleSolucion, empleadoAsignaciones,
                     actualDemanda + 1, mejorSolucion, startTime);
            }
        }

        // Restaurar estado para backtracking
        empleadoAsignaciones[e.nombre]--;
        posibleSolucion[dia].pop_back();
        restEmployees.insert(restEmployees.begin() + i, e);
    }
    return nodos;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        cout << "Uso: programa <num_enfermeras> <num_dias> <demanda> <LS> <US>" << endl;
        return 1;
    }

    NUM_ENFERMERAS = stoi(argv[1]);
    DIAS = stoi(argv[2]);
    DEMANDA = stoi(argv[3]);
    LIMITE_INFERIOR = stoi(argv[4]);
    LIMITE_SUPERIOR = stoi(argv[5]);
    if(NUM_ENFERMERAS < 3 * DEMANDA) {
        cout << "Necesitas " << 3 * DEMANDA - NUM_ENFERMERAS << " o mas enfermeras para asignar segun la demanda que pides";
        return 1;
    }

    vector<Empleado> empleados;
    for (int i = 0; i < NUM_ENFERMERAS; i++) {
        empleados.emplace_back("e_" + to_string(i + 1));
    }
    vector<vector<Empleado>> posibleSolucion(DIAS, vector<Empleado>());
    vector<Empleado> restEmployees = empleados;
    unordered_map<string, int> empleadoAsignaciones;

    for (const auto& empleado : empleados) {
        empleadoAsignaciones[empleado.nombre] = 0;
    }

    SolucionFinal mejorSolucion = {INFINITY, {}};
    auto start = chrono::high_resolution_clock::now();
    int nodosExplorados = explorarArbol(0, 0, empleados, restEmployees, posibleSolucion, empleadoAsignaciones,
                                        0, mejorSolucion, start);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    
    cout << "\t{" << endl;
    printInput();
    cout << "\t\t\"FO\": " << mejorSolucion.funcionObjetivo << "," << endl;
    cout << "\t\t\"nodos_explorados\": " << nodosExplorados << ","  << endl;
    cout << "\t\t\"tiempo_de_exec_segundos\": " << duration.count() << "," << endl;

    cout << "\t\t\"horario_optimo\":\n\t\t{" << endl;
    Turnos turnos;
    vector<string> tiposTurnos = {turnos.early.nombre, turnos.day.nombre, turnos.late.nombre};
    for (int dia = 0; dia < mejorSolucion.solucion.size(); dia++) {
        cout << "\t\t\t\"dia_" << dia + 1 << "\":\n\t\t\t{\n";
        for (int turno = 0; turno < 3; turno++) {
            cout << "\t\t\t\t\"" << tiposTurnos[turno] << "\": [";

            int inicio = turno * DEMANDA;
            int fin = inicio + DEMANDA;

            for (int j = inicio; j < fin && j < mejorSolucion.solucion[dia].size(); j++) {
                cout << "\"" << mejorSolucion.solucion[dia][j].nombre << "\"";

                if (j < fin - 1) {
                    cout << ", ";
                } else {
                    cout << "]";
                    if(!(turno == ULTIMO_TURNO)) {
                        cout << ",";
                    }
                }
            }
            cout << endl;
        }
        string posibleComa = "";
        if(!(dia == mejorSolucion.solucion.size() -1)) {
            posibleComa += ",";
        }
        cout << "\t\t\t}" + posibleComa + "\n";
    }
    cout << "\t\t}\n\t}";
    return 0;
}