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

int DIAS;
int DEMANDA;
int MIN_ASIGNACIONES;
int MAX_ASIGNACIONES;
int NUM_ENFERMERAS;
int MAX_DIAS_TRABAJADOS_CONSECUTIVOS;
int MIN_DIAS_TRABAJADOS_CONSECUTIVOS;
int MAX_DIAS_LIBRES_CONSECUTIVOS;
int MIN_DIAS_LIBRES_CONSECUTIVOS;
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
    int errorIncumpleMinT;
    int errorIncumpleMaxT;
    int errorIncumpleMinL;
    int errorIncumpleMaxL;
    stack<int> restriccionesMaxT;
    stack<int> restriccionesMaxL;
    bool noL;
    bool noT;
    Empleado(string nombre)
        : nombre(nombre),
        errorIncumpleMinT(0),
        errorIncumpleMaxT(0),
        errorIncumpleMinL(0),
        errorIncumpleMaxL(0),
        restriccionesMaxT(),
        restriccionesMaxL(),
        noL(true),
        noT(true) {}
    bool operator==(const Empleado& other) const {
        return nombre == other.nombre;
    }
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

struct Incumplimientos {
    vector<bool> max_asingaciones;
    vector<bool> min_asignaciones;
    vector<bool> min_dias_libres_consec;
    vector<bool> max_dias_libres_consec;
    vector<bool> min_dias_trab_consec;
    vector<bool> max_dias_trab_consec;

    Incumplimientos(int num_empleados) {
        max_asingaciones.resize(num_empleados, false);
        min_asignaciones.resize(num_empleados, false);
        min_dias_libres_consec.resize(num_empleados, false);
        max_dias_libres_consec.resize(num_empleados, false);
        min_dias_trab_consec.resize(num_empleados, false);
        max_dias_trab_consec.resize(num_empleados, false);
    }
};

struct SolucionFinal {
    double funcionObjetivo;
    vector<vector<Empleado>> solucion;
    Incumplimientos B;

    SolucionFinal(double fo, vector<vector<Empleado>> sol, Incumplimientos incumplimientos) 
        : funcionObjetivo(fo), solucion(sol), B(incumplimientos) {}
};

void printInput() {
    cout << "\t\t\"input_data\":\n\t\t{\n\t\t\t\"numero_enfermeras\": " << NUM_ENFERMERAS << 
    ",\n\t\t\t\"dias\": " << DIAS << ",\n\t\t\t\"demanda\": " << DEMANDA << 
    ",\n\t\t\t\"limite_inferior_asignaciones\": " << MIN_ASIGNACIONES << 
    ",\n\t\t\t\"limite_superior_asignaciones\": " << MAX_ASIGNACIONES <<
    ",\n\t\t\t\"limite_inferior_dias_libres_consecutivos\": " << MIN_DIAS_LIBRES_CONSECUTIVOS <<
    ",\n\t\t\t\"limite_superior_dias_libres_consecutivos\": " << MAX_DIAS_LIBRES_CONSECUTIVOS <<
    ",\n\t\t\t\"limite_inferior_dias_trabajados_consecutivos\": " << MIN_DIAS_TRABAJADOS_CONSECUTIVOS <<
    ",\n\t\t\t\"limite_superior_dias_trabajados_consecutivos\": " << MAX_DIAS_TRABAJADOS_CONSECUTIVOS << "\n\t\t},\n";
}

bool isTimeCompleted(const chrono::time_point<chrono::high_resolution_clock>& startTime) {
    auto now = chrono::high_resolution_clock::now();
    double elapsed = chrono::duration<double>(now - startTime).count();
    if (elapsed > LIMITE_TIEMPO) {
        return true;
    }
    return false;
}

int calcularFlexibilidad(const Empleado& e, const unordered_map<string, int>& empleadoAsignaciones, 
    const unordered_map<string, int>& consecutivosT, const unordered_map<string, int>& consecutivosL) {
    return max(0, MAX_ASIGNACIONES - empleadoAsignaciones.at(e.nombre)) +
    max(0, MAX_DIAS_TRABAJADOS_CONSECUTIVOS - consecutivosT.at(e.nombre)) +
    max(0, MAX_DIAS_LIBRES_CONSECUTIVOS - consecutivosL.at(e.nombre));
}

int empleadoIdx(const vector<Empleado>& empleados, string empleadoNombre) {
    for (int i = 0; i < empleados.size(); i++) {
        if (empleados[i].nombre == empleadoNombre) {
            return i;
        }
    }
    return -1;
}

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

double calcularFuncionObjetivo(
    const vector<Empleado>& employees,
    const vector<vector<Empleado>>& solucion,
    unordered_map<string, int>& empleadoAsignaciones,
    Incumplimientos& B
) {
    double fo = 0;
    unordered_map<string, int> empleadoMap;

    for (const auto& empleado : empleadoAsignaciones) {
        int asignacionesEmpleado = empleado.second;
        int e1 = max(0, asignacionesEmpleado - MAX_ASIGNACIONES);
        double r1 = PESO_W1 * e1;

        int e2 = max(0, MIN_ASIGNACIONES - asignacionesEmpleado);
        double r2 = PESO_W2 * e2;

        fo += (r1 + r2);

        // Incumplimientos
        int idx = empleadoIdx(employees, empleado.first);
        if(e1 > 0) {
            B.max_asingaciones[idx] = true;
        } if(e2 > 0) {
            B.min_asignaciones[idx] = true;
        }


        Empleado e = employees[idx];
        fo += (e.errorIncumpleMaxL + e.errorIncumpleMaxT + e.errorIncumpleMinL + e.errorIncumpleMinT);

        // Incumplimientos
        if(e.errorIncumpleMaxL > 0) {
            B.max_dias_libres_consec[idx] = true;
        } if(e.errorIncumpleMinL > 0) {
            B.min_dias_libres_consec[idx] = true;
        } if(e.errorIncumpleMaxT > 0) {
            B.max_dias_trab_consec[idx] = true;
        } if(e.errorIncumpleMinT > 0) {
            B.min_dias_trab_consec[idx] = true;
        }
    }

    return fo;
}

// Función para generar una solución inicial aleatoria
SolucionFinal generarSolucionAleatoria(const vector<Empleado>& empleados) {
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
    double fo = calcularFuncionObjetivo(employees, solucion, empleadoAsignaciones, B);
    //cout <<  "FO_I: " << fo << endl;
    return {fo, solucion, B};
}

int contarIncumplimientos(const Incumplimientos& incumplimientos) {
    int count = 0;
    for (bool inc : incumplimientos.max_asingaciones) if (inc) count++;
    for (bool inc : incumplimientos.min_asignaciones) if (inc) count++;
    for (bool inc : incumplimientos.min_dias_libres_consec) if (inc) count++;
    for (bool inc : incumplimientos.max_dias_libres_consec) if (inc) count++;
    for (bool inc : incumplimientos.min_dias_trab_consec) if (inc) count++;
    for (bool inc : incumplimientos.max_dias_trab_consec) if (inc) count++;
    return count;
}

Incumplimientos unionIncumplimientos(const Incumplimientos& A, const Incumplimientos& B) {
    int num_empleados = A.max_asingaciones.size();
    Incumplimientos result(num_empleados);
    for (int i = 0; i < num_empleados; i++) {
        result.max_asingaciones[i] = A.max_asingaciones[i] || B.max_asingaciones[i];
        result.min_asignaciones[i] = A.min_asignaciones[i] || B.min_asignaciones[i];
        result.min_dias_libres_consec[i] = A.min_dias_libres_consec[i] || B.min_dias_libres_consec[i];
        result.max_dias_libres_consec[i] = A.max_dias_libres_consec[i] || B.max_dias_libres_consec[i];
        result.min_dias_trab_consec[i] = A.min_dias_trab_consec[i] || B.min_dias_trab_consec[i];
        result.max_dias_trab_consec[i] = A.max_dias_trab_consec[i] || B.max_dias_trab_consec[i];
    }
    return result;
}

Incumplimientos interseccionIncumplimientos(const Incumplimientos& A, const Incumplimientos& B) {
    int num_empleados = A.max_asingaciones.size();
    Incumplimientos result(num_empleados);
    for (int i = 0; i < num_empleados; i++) {
        result.max_asingaciones[i] = A.max_asingaciones[i] && B.max_asingaciones[i];
        result.min_asignaciones[i] = A.min_asignaciones[i] && B.min_asignaciones[i];
        result.min_dias_libres_consec[i] = A.min_dias_libres_consec[i] && B.min_dias_libres_consec[i];
        result.max_dias_libres_consec[i] = A.max_dias_libres_consec[i] && B.max_dias_libres_consec[i];
        result.min_dias_trab_consec[i] = A.min_dias_trab_consec[i] && B.min_dias_trab_consec[i];
        result.max_dias_trab_consec[i] = A.max_dias_trab_consec[i] && B.max_dias_trab_consec[i];
    }
    return result;
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
    const SolucionFinal& mejorSolucion,
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
    SolucionFinal& mejorSolucion,
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
                double fo = calcularFuncionObjetivo(employees, posibleSolucion, empleadoAsignaciones, B);
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
                        //cout << "(dia, turno, demanda) = (" << dia << "," << turno << "," << actualDemanda <<")" << endl;
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
    if (argc != 10) {
        cout << "Uso: programa <num_enfermeras> <num_dias> <demanda> <LS> <US> <LCF> <UCF> <LCW> <UCW>" << endl;
        return 1;
    }

    NUM_ENFERMERAS = stoi(argv[1]);
    DIAS = stoi(argv[2]);
    DEMANDA = stoi(argv[3]);
    MIN_ASIGNACIONES = stoi(argv[4]);
    MAX_ASIGNACIONES = stoi(argv[5]);
    MIN_DIAS_LIBRES_CONSECUTIVOS = stoi(argv[6]);
    MAX_DIAS_LIBRES_CONSECUTIVOS = stoi(argv[7]);
    MIN_DIAS_TRABAJADOS_CONSECUTIVOS = stoi(argv[8]);
    MAX_DIAS_TRABAJADOS_CONSECUTIVOS = stoi(argv[9]);

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
    vector<Empleado> copyEmployees = empleados;
    SolucionFinal mejorSolucion = generarSolucionAleatoria(empleados);
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