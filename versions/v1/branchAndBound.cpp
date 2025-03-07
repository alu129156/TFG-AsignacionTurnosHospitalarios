#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <set>
#include <stack>
#include <unordered_map>
#include <chrono>

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
    ",\n\t\t\t\"limite_inferior_asignaciones\": " << MIN_ASIGNACIONES << 
    ",\n\t\t\t\"limite_superior_asignaciones\": " << MAX_ASIGNACIONES <<
    ",\n\t\t\t\"limite_inferior_dias_libres_consecutivos\": " << MIN_DIAS_LIBRES_CONSECUTIVOS <<
    ",\n\t\t\t\"limite_superior_dias_libres_consecutivos\": " << MAX_DIAS_LIBRES_CONSECUTIVOS <<
    ",\n\t\t\t\"limite_inferior_dias_trabajados_consecutivos\": " << MIN_DIAS_TRABAJADOS_CONSECUTIVOS <<
    ",\n\t\t\t\"limite_superior_dias_trabajados_consecutivos\": " << MAX_DIAS_TRABAJADOS_CONSECUTIVOS << "\n\t\t},\n";
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

int empleadoIdx(const vector<Empleado>& empleados, string empleadoNombre) {
    for (int i = 0; i < empleados.size(); ++i) {
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
    unordered_map<string, bool>& empleadoTurnoEnDia
) {
    double f = 0.0;
    for (const auto& [nombreEmpleado, asignacionesEmpleado] : empleadoAsignaciones) {
            bool turnoEmpleadoEnDia = empleadoTurnoEnDia[nombreEmpleado];
            double restDays = (turnoEmpleadoEnDia) ? DIAS - dia - 1 : DIAS - dia;

            double ei = max(0, asignacionesEmpleado - MAX_ASIGNACIONES);
            double ei_prime = max(0, MIN_ASIGNACIONES - asignacionesEmpleado);
            double ki = max(0.0, ei_prime - restDays);

            f += PESO_W1 * ei + PESO_W2 * ki;
    }

    for(const auto& e: employees) {
        f += (e.errorIncumpleMinL + e.errorIncumpleMinT);
        f += e.errorIncumpleMaxT;
        if(!e.restriccionesMaxT.empty()) {
            f += e.restriccionesMaxT.top();
        }
        f += e.errorIncumpleMaxL;
        if(!e.restriccionesMaxL.empty()) {
            f += e.restriccionesMaxL.top();
        }
    }
    return f;
}

double calcularFuncionObjetivo(
    const vector<Empleado>& employees,
    const vector<vector<Empleado>>& solucion,
    unordered_map<string, int>& empleadoAsignaciones
) {
    double fo = 0;

    for (const auto& empleado : empleadoAsignaciones) {
        int asignacionesEmpleado = empleado.second;

        int e1 = max(0, asignacionesEmpleado - MAX_ASIGNACIONES);
        double r1 = PESO_W1 * e1;

        int e2 = max(0, MIN_ASIGNACIONES - asignacionesEmpleado);
        double r2 = PESO_W2 * e2;

        fo += (r1 + r2);
    }

    for(const auto& e: employees) {
        fo += (e.errorIncumpleMaxL + e.errorIncumpleMaxT + e.errorIncumpleMinL + e.errorIncumpleMinT);
    }

    return fo;
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
    for(int i = 0; i < restEmployees.size(); i++) {
        verifyTime(startTime);

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

        double cotaInf_noFinalDelDia = calcularCotaInf(employees, empleadoAsignaciones, dia, empleadoTurnoEnDia);

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
                double fo = calcularFuncionObjetivo(employees, posibleSolucion, empleadoAsignaciones);
                if (fo < mejorSolucion.funcionObjetivo) {
                    mejorSolucion.funcionObjetivo = fo;
                    mejorSolucion.solucion = posibleSolucion;
                    //cout << fo << endl;
                }
            } else { // Siguiente Dia
                bool poda = false;
                double cotaInf_finalDelDia = calcularCotaInf(employees, empleadoAsignaciones, dia, empleadoTurnoEnDia);
                if (cotaInf_finalDelDia >= mejorSolucion.funcionObjetivo) { // Poda
                    poda = true;
                }

                if(!poda) {
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
        } else if(cotaInf_noFinalDelDia >= mejorSolucion.funcionObjetivo) {
                // Poda: No continuamos con esta rama porque la cota inferior ya es peor que la mejor FO encontrada.
                        //cout << "(nuevo_F, dia, turno, demanda) = (" << cotaInf_noFinalDelDia <<"," << dia << "," << turno << "," << actualDemanda <<")" << endl;
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
    SolucionFinal mejorSolucion = {INFINITY, {}};

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