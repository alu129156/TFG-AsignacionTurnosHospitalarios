#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>
#include <queue>
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
#define MAX_COLA_SIZE 1000
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
    bool noL;
    bool noT;

    Empleado(string nombre)
        : nombre(nombre),
        errorIncumpleMinT(0),
        errorIncumpleMaxT(0),
        errorIncumpleMinL(0),
        errorIncumpleMaxL(0),
        noL(true),
        noT(true) {}
};

struct NodoAStar {
    vector<vector<Empleado>> solucion;
    vector<Empleado> empleados;
    vector<Empleado> restEmpleados;
    unordered_map<string, int> empleadoAsignaciones;
    unordered_map<string, bool> empleadoTurnoEnDia;
    unordered_map<string, int> consecutivosT;
    unordered_map<string, int> consecutivosL;
    int dia, turno, demanda;
    double g;
    double h;
    double f;
    bool operator>(const NodoAStar& otro) const {
        return f > otro.f;
    }
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

// Calcular h(n) --> Saco un caso desfavorable, que puede subir h(n)
double calcularHeuristica(NodoAStar actual, bool hoja) {
    double h = 0.0;
    if (!hoja) {
        for (const auto& [nombreEmpleado, asignaciones_e] : actual.empleadoAsignaciones) {
            bool eAsignado = actual.empleadoTurnoEnDia[nombreEmpleado];
            int rest_days_e = (eAsignado) ? DIAS - actual.dia - 1 : DIAS - actual.dia;
            int consecT = actual.consecutivosT[nombreEmpleado];
            int consecL = actual.consecutivosL[nombreEmpleado];

            int flexibilidad = max(0, MAX_ASIGNACIONES - asignaciones_e) +
                               max(0, MAX_DIAS_TRABAJADOS_CONSECUTIVOS - consecT) +
                               max(0, MAX_DIAS_LIBRES_CONSECUTIVOS - consecL);

            int er_1 = max(0, MIN_ASIGNACIONES - (rest_days_e + asignaciones_e));
            int er_2 = max(0, (rest_days_e + asignaciones_e) - MAX_ASIGNACIONES);

            int er_3 = max(0, MIN_DIAS_LIBRES_CONSECUTIVOS - (consecL + rest_days_e));
            int er_4 = max(0, (consecL + rest_days_e) - MAX_DIAS_LIBRES_CONSECUTIVOS);

            int er_5 = max(0, MIN_DIAS_TRABAJADOS_CONSECUTIVOS - (consecT + rest_days_e));
            int er_6 = max(0, (consecT + rest_days_e) - MAX_DIAS_TRABAJADOS_CONSECUTIVOS);

            // Ponderación en flexibilidad (penaliza menos a empleados con más margen)
            double penalizacion = (er_1 + er_2 + er_3 + er_4 + er_5 + er_6);
            h += penalizacion / (1.0 + flexibilidad);
        }
    }
    return h;
}

//g(n)
double calcularFuncionObjetivo(
    NodoAStar actual
) {
    double fo = 0;
    unordered_map<string, int> empleadoMap;

    for (const auto& empleado : actual.empleadoAsignaciones) {
        int asignacionesEmpleado = empleado.second;

        int e1 = max(0, asignacionesEmpleado - MAX_ASIGNACIONES);
        double r1 = PESO_W1 * e1;

        int e2 = max(0, MIN_ASIGNACIONES - asignacionesEmpleado);
        double r2 = PESO_W2 * e2;

        fo += (r1 + r2);
    }

    for(const auto& e: actual.empleados) {
        fo += (e.errorIncumpleMaxL + e.errorIncumpleMaxT + e.errorIncumpleMinL + e.errorIncumpleMinT);
    }

    return fo;
}

int empleadoIdx(const vector<Empleado>& empleados, string empleadoNombre) {
    for (int i = 0; i < empleados.size(); i++) {
        if (empleados[i].nombre == empleadoNombre) {
            return i;
        }
    }
    return -1;
}

NodoAStar explorarArbolAStar(
    const vector<Empleado>& empleados,
    const unordered_map<string, int>& empleadoAsignaciones,
    const unordered_map<string, bool>& empleadoTurnoEnDia,
    const unordered_map<string, int>& consecL,
    const unordered_map<string, int>& consecT,
    const chrono::time_point<chrono::high_resolution_clock>& startTime
) {
    priority_queue<NodoAStar, vector<NodoAStar>, greater<NodoAStar>> frontera;
    vector<vector<Empleado>> solucionInicial(DIAS, vector<Empleado>());

    frontera.push({solucionInicial, empleados, empleados,
         empleadoAsignaciones, empleadoTurnoEnDia, consecT, consecL, 0, 0, 0, 0.0, 0.0, 0.0});

    NodoAStar mejorSolucion;
    mejorSolucion.f = numeric_limits<double>::max();
    bool primera_hoja_encontrada = false;
    double umbral_factor = 1.75; // 175% ==> Se aceptan soluciones hasta un 75% peores que la mejorFO encontrada

    while (!frontera.empty()) {
        NodoAStar actual = frontera.top();
        frontera.pop();

        vector<Empleado> empleadosDisponibles = actual.restEmpleados;

        if (primera_hoja_encontrada && actual.f > mejorSolucion.f * umbral_factor) {
            //cout << "Se detiene la exploracion, f(n)=" << actual.f << " supera umbral de " << mejorSolucion.f * umbral_factor << endl;
            break;
        }

        // Explorar hijos
        for (int i = 0; i < empleadosDisponibles.size(); i++) {
            if(isTimeCompleted(startTime)) {
                return mejorSolucion;
            }

            NodoAStar hijo = actual;
            bool isHoja = (actual.dia == DIAS - 1 && actual.turno == ULTIMO_TURNO && actual.demanda == DEMANDA);
            if(!isHoja) {
                string nombreE = empleadosDisponibles[i].nombre;
                int idx = empleadoIdx(hijo.empleados, nombreE);
    
                hijo.solucion[actual.dia].push_back(empleadosDisponibles[i]);
                hijo.demanda++;
    
                hijo.empleadoAsignaciones[nombreE]++;
                hijo.restEmpleados.erase(hijo.restEmpleados.begin() + i);
                hijo.empleadoTurnoEnDia[nombreE] = true;
    
                if (hijo.consecutivosL[nombreE] > 0) { // *L -> T
                    hijo.empleados[idx].errorIncumpleMinL += max(0, MIN_DIAS_LIBRES_CONSECUTIVOS - hijo.consecutivosL[nombreE]) * PESO_W6;
                    hijo.empleados[idx].errorIncumpleMaxL += max(0, hijo.consecutivosL[nombreE] - MAX_DIAS_LIBRES_CONSECUTIVOS) * PESO_W5;
                    hijo.consecutivosL[nombreE] = 0;
                }
                hijo.consecutivosT[nombreE]++;
                hijo.empleados[idx].noT = false;
                if(hijo.dia == DIAS - 1) {
                    // Acaba en T
                    hijo.empleados[idx].errorIncumpleMinT += 
                                max(0, MIN_DIAS_TRABAJADOS_CONSECUTIVOS - hijo.consecutivosT[nombreE]) * PESO_W4;
                    hijo.empleados[idx].errorIncumpleMaxT += 
                                max(0, hijo.consecutivosT[nombreE] - MAX_DIAS_TRABAJADOS_CONSECUTIVOS) * PESO_W3;
                    if(hijo.empleados[idx].noL) { // Caso TTTT --> 0 Dias Libres
                        hijo.empleados[idx].errorIncumpleMinL += MIN_DIAS_LIBRES_CONSECUTIVOS * PESO_W6;
                    }
                }
    
                if (hijo.demanda == DEMANDA) {
                    if (hijo.turno == ULTIMO_TURNO) {
                        for (Empleado& rE : hijo.restEmpleados) {
                            int rIdx = empleadoIdx(hijo.empleados, rE.nombre);
                            if (hijo.consecutivosT[rE.nombre] > 0) { // *T -> L
                                hijo.empleados[rIdx].errorIncumpleMinT += 
                                            max(0, MIN_DIAS_TRABAJADOS_CONSECUTIVOS - hijo.consecutivosT[rE.nombre]) * PESO_W4;
                                hijo.empleados[rIdx].errorIncumpleMaxT += 
                                            max(0, hijo.consecutivosT[rE.nombre] - MAX_DIAS_TRABAJADOS_CONSECUTIVOS) * PESO_W3;
                            }
                            hijo.consecutivosL[rE.nombre]++;
                            hijo.consecutivosT[rE.nombre] = 0;
                            hijo.empleados[rIdx].noL = false;
                
                            if (hijo.dia == DIAS - 1) { // Último día, valida restricciones finales
                                hijo.empleados[rIdx].errorIncumpleMinL += 
                                            max(0, MIN_DIAS_LIBRES_CONSECUTIVOS - hijo.consecutivosL[rE.nombre]) * PESO_W6;
                                hijo.empleados[rIdx].errorIncumpleMaxL += 
                                            max(0, hijo.consecutivosL[rE.nombre] - MAX_DIAS_LIBRES_CONSECUTIVOS) * PESO_W5;
                                if (hijo.empleados[rIdx].noT) { // Caso LLLL --> 0 Dias Trabajados
                                    hijo.empleados[rIdx].errorIncumpleMinT += MIN_DIAS_TRABAJADOS_CONSECUTIVOS * PESO_W4;
                                }
                            }
                        }
    
                        if (hijo.dia < DIAS - 1) {
                            hijo.dia++;
                            hijo.restEmpleados = empleados;
                            hijo.empleadoTurnoEnDia = empleadoTurnoEnDia;
                            hijo.turno = 0;
                            hijo.demanda = 0;
                        } else {
                            hijo.g = calcularFuncionObjetivo(hijo);
                            hijo.h = calcularHeuristica(hijo, true);
                            hijo.f = hijo.g + hijo.h;
                            if (hijo.f < mejorSolucion.f) {
                                primera_hoja_encontrada = true;
                                mejorSolucion = hijo;
                                //cout << "F: " << mejorSolucion.f << endl;
                            }
                            continue;
                        }
                    } else {
                        hijo.turno++;
                        hijo.demanda = 0;
                    }
                }
                
                hijo.g = calcularFuncionObjetivo(hijo);
                hijo.h = calcularHeuristica(hijo, false);
                hijo.f = hijo.g + hijo.h;

                if (frontera.size() < MAX_COLA_SIZE) {
                    frontera.push(hijo);
                } else if (hijo.f < frontera.top().f) {
                    frontera.pop();
                    frontera.push(hijo);
                }
                
            }
        }
    }
    return mejorSolucion;
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

    vector<Empleado> empleados = vector<Empleado>();
    unordered_map<string, bool> empleadoTurnoEnDia;
    unordered_map<string, int> empleadoAsignaciones;
    unordered_map<string, int> consecL;
    unordered_map<string, int> consecT;
    for (int i = 0; i < NUM_ENFERMERAS; i++) {
        Empleado e = Empleado("e_" + to_string(i + 1));
        empleados.push_back(e);
        empleadoTurnoEnDia[e.nombre] = false;
        empleadoAsignaciones[e.nombre] = 0;
        consecT[e.nombre] = 0;
        consecL[e.nombre] = 0;
    }

    auto start = chrono::high_resolution_clock::now();
    NodoAStar solucionEncontrada = explorarArbolAStar(empleados, empleadoAsignaciones, empleadoTurnoEnDia, consecL, consecT, start);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "\t{" << endl;
    printInput();
    cout << "\t\t\"FO\": " << solucionEncontrada.f << "," << endl;
    cout << "\t\t\"tiempo_de_exec_segundos\": " << duration.count() << "," << endl;

    cout << "\t\t\"horario_optimo\":\n\t\t{" << endl;

    vector<string> tiposTurnos = {"Early", "Day", "Night"};
    for (int dia = 0; dia < solucionEncontrada.solucion.size(); dia++) {
        cout << "\t\t\t\"dia_" << dia + 1 << "\":\n\t\t\t{\n";
        for (int turno = 0; turno < 3; turno++) {
            cout << "\t\t\t\t\"" << tiposTurnos[turno] << "\": [";

            int inicio = turno * DEMANDA;
            int fin = inicio + DEMANDA;

            for (int j = inicio; j < fin && j < solucionEncontrada.solucion[dia].size(); j++) {
                cout << "\"" << solucionEncontrada.solucion[dia][j].nombre << "\"";

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
        if(!(dia == solucionEncontrada.solucion.size() -1)) {
            posibleComa += ",";
        }
        cout << "\t\t\t}" + posibleComa + "\n";
    }
    cout << "\t\t}\n\t}";
    return solucionEncontrada.solucion.empty() ? 1 : 0;
}