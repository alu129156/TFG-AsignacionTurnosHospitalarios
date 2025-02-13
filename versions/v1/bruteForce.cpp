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
int MIN_TURNOS_SANITARIO; //LIMITE_INFERIOR
int MAX_TURNOS_SANITARIO; //LIMITE_SUPERIOR
int MAX_CONSECUTIVE_DAYS_WORKING;
int MIN_CONSECUTIVE_DAYS_WORKING;
int MAX_CONSECUTIVE_DAYS_FREE;
int MIN_CONSECUTIVE_DAYS_FREE;
#define LIMITE_TIEMPO 420
#define ULTIMO_TURNO 2
#define PESO_W1 10.0
#define PESO_W2 10.0
#define PESO_W3 10.0
#define PESO_W4 10.0
#define PESO_W5 10.0
#define PESO_W6 10.0

using namespace std;

class Empleado {
public:
    string nombre;
    int actualDayCWD;
    int actualDayCFD;
    int prevDayCWD;
    vector<int> cwds;
    vector<int> cfds;
    Empleado(string nombre)
        : nombre(nombre),
        prevDayCWD(0),
        actualDayCWD(0),
        cwds(), cfds(){}
    bool operator==(const Empleado& other) const {
        return nombre == other.nombre;
    }
    
};

namespace std {
    template <>
    struct hash<Empleado> {
        size_t operator()(const Empleado& e) const {
            return hash<string>()(e.nombre);
        }
    };
}

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


double calcularFuncionObjetivo(const vector<vector<Empleado>>& solucion) {
    double fo = 0;
    unordered_map<Empleado, int> empleadoMap;

    for (const auto& empleadosDia : solucion) {
        for(const auto& empleado: empleadosDia) {
            empleadoMap[empleado]++;
        }
    }

    for (const auto& [empleado, totalTurnos] : empleadoMap) {
        int e1 = max(0, totalTurnos - MAX_TURNOS_SANITARIO);
        double r1 = PESO_W1 * e1;

        int e2 = max(0, MIN_TURNOS_SANITARIO - totalTurnos);
        double r2 = PESO_W2 * e2;

        double r3 = 0.0, r4 = 0.0, r5 = 0.0, r6 = 0.0;
        for (int cwd : empleado.cwds) {
            r3 += PESO_W3 * max(0, cwd - MAX_CONSECUTIVE_DAYS_WORKING);
            r4 += PESO_W4 * max(0, MIN_CONSECUTIVE_DAYS_WORKING - cwd);
        }
        for (int cfd : empleado.cfds) {
            r5 += PESO_W5 * max(0, cfd - MAX_CONSECUTIVE_DAYS_FREE);
            r6 += PESO_W6 * max(0, MIN_CONSECUTIVE_DAYS_FREE - cfd);
        }

        fo += (r1 + r2 + r3 + r4 + r5 + r6);
    }

    return fo;
}

void actualizarDiasTrabajoYLibres(vector<vector<Empleado>>& posibleSolucion) {
    for (vector<Empleado>& empleadosDia : posibleSolucion) {
        for (Empleado& e : empleadosDia) {
            if (e.prevDayCWD == e.actualDayCWD) {  // Día libre
                e.cwds.push_back(e.actualDayCWD);
                e.actualDayCFD++;
                e.actualDayCWD = 0;
            } else if (e.actualDayCFD > 0) {  // Inicio de trabajo después de descanso
                e.cfds.push_back(e.actualDayCFD);
                e.actualDayCFD = 0;
            }
            e.prevDayCWD = e.actualDayCWD;
        }
    }
}

int it = 0;

void explorarArbol(
    int dia, int turno, const vector<Empleado>& employees, vector<Empleado> restEmployees,
    vector<vector<Empleado>>& posibleSolucion, SolucionFinal& mejorSolucion,
    int actualDemanda,
    const chrono::time_point<chrono::high_resolution_clock>& startTime
    ) {
    for(int i = 0; i < restEmployees.size(); i++) {
        verifyTime(startTime);

        it++;
        Empleado e = restEmployees[i];
        restEmployees.erase(restEmployees.begin() + i);
        e.actualDayCWD++;
        posibleSolucion[dia].push_back(e);

        if(turno == ULTIMO_TURNO && actualDemanda == DEMANDA - 1) {
            vector<Empleado> restEmployeesNext = employees;
            const vector<vector<Empleado>> copiaSolucion = posibleSolucion;
            actualizarDiasTrabajoYLibres(posibleSolucion);

            if(dia == DIAS - 1) {
                double fo = calcularFuncionObjetivo(posibleSolucion);
                if (fo < mejorSolucion.funcionObjetivo) {
                    mejorSolucion.funcionObjetivo = fo;
                    mejorSolucion.solucion = posibleSolucion;
                }
            } else {
                explorarArbol(dia + 1, 0, employees, restEmployeesNext,
                    posibleSolucion, mejorSolucion, 0, startTime);
            }

            // Restaurar estado para backtracking
            posibleSolucion = copiaSolucion;
            for(vector<Empleado>& empleadosDia : posibleSolucion) {
                for(Empleado& e : empleadosDia) {
                    
                }
            }
        } else { // turno < 2
            if(actualDemanda == DEMANDA - 1) { // Cambio de turno
                explorarArbol(dia, turno + 1, employees, restEmployees,
                     posibleSolucion, mejorSolucion, 0, startTime);
            } else { // Mismo turno, actualizando la demanda
                explorarArbol(dia, turno, employees, restEmployees,
                     posibleSolucion, mejorSolucion, actualDemanda + 1, startTime);
            }
        }

        // Restaurar estado para backtracking
        posibleSolucion[dia].pop_back();
        e.actualDayCWD--;
        restEmployees.insert(restEmployees.begin() + i, e);
    }
}