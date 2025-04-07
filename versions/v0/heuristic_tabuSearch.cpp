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

int DIAS;
int DEMANDA;
int MIN_ASIGNACIONES;
int MAX_ASIGNACIONES;
int NUM_ENFERMERAS;
int MAX_DIAS_TRABAJADOS_CONSECUTIVOS;
int MIN_DIAS_TRABAJADOS_CONSECUTIVOS;
int MAX_DIAS_LIBRES_CONSECUTIVOS;
int MIN_DIAS_LIBRES_CONSECUTIVOS;
#define LIMITE_TIEMPO 10
#define MAX_COLA_SIZE 1000
#define ULTIMO_TURNO 2
#define TURNOS 3
#define TABU_TENURE 10
#define NUM_VECINOS 5
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
    bool operator==(const Empleado& other) const {
        return nombre == other.nombre;
    }
         
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
    bool operator==(const SolucionFinal& other) const {
        return solucion == other.solucion;
    }    
};

void printInput() {
    cout << "\t\t\"input_data\":\n\t\t{\n\t\t\t\"numero_enfermeras\": " << NUM_ENFERMERAS << 
    ",\n\t\t\t\"dias\": " << DIAS << ",\n\t\t\t\"demanda\": " << DEMANDA << 
    ",\n\t\t\t\"limite_inferior_asignaciones\": " << MIN_ASIGNACIONES << 
    ",\n\t\t\t\"limite_superior_asignaciones\": " << MAX_ASIGNACIONES << "\n\t\t},\n";
}

bool isTimeCompleted(
    const double& elapsed
) {
    if (elapsed > LIMITE_TIEMPO) {
        return true;
    }
    return false;
}

double calcularFuncionObjetivo(const vector<vector<Empleado>>& horario, const unordered_map<string, int>& empleadoAsignaciones) {
    double fo = 0;
    for (const auto& asignaciones : empleadoAsignaciones) {
        int count = asignaciones.second;
        fo += max(0, count - MAX_ASIGNACIONES) * PESO_W1;
        fo += max(0, MIN_ASIGNACIONES - count) * PESO_W2;
    }
    return fo;
}

SolucionFinal generarSolucionAleatoria(const vector<Empleado>& empleados) {
    vector<vector<Empleado>> horario(DIAS, vector<Empleado>());
    unordered_map<string, int> empleadoAsignaciones;

    for (int dia = 0; dia < DIAS; dia++) {
        vector<Empleado> empleadosDisponibles = empleados;
        shuffle(empleadosDisponibles.begin(), empleadosDisponibles.end(), mt19937(chrono::steady_clock::now().time_since_epoch().count()));

        for (int i = 0; i < TURNOS * DEMANDA; i++) {
            horario[dia].push_back(empleadosDisponibles[i]);
            empleadoAsignaciones[empleadosDisponibles[i].nombre]++;
        }
    }

    double fo = calcularFuncionObjetivo(horario, empleadoAsignaciones);
    //cout << fo << endl;
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
            //cout << "Cambiaaa" << endl;
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
    if (argc != 6) {
        cout << "Uso: programa <num_enfermeras> <num_dias> <demanda> <LS> <US>" << endl;
        return 1;
    }

    NUM_ENFERMERAS = stoi(argv[1]);
    DIAS = stoi(argv[2]);
    DEMANDA = stoi(argv[3]);
    MIN_ASIGNACIONES = stoi(argv[4]);
    MAX_ASIGNACIONES = stoi(argv[5]);
    if(NUM_ENFERMERAS < 3 * DEMANDA) {
        cout << "Necesitas " << 3 * DEMANDA - NUM_ENFERMERAS << " o mas enfermeras para asignar segun la demanda que pides";
        return 1;
    }

    vector<Empleado> empleados;
    for (int i = 0; i < NUM_ENFERMERAS; i++) {
        empleados.emplace_back("e_" + to_string(i + 1));
    }

    auto start = chrono::high_resolution_clock::now();
    SolucionFinal mejorSolucion = tabuSearch(empleados);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "\t{" << endl;
    printInput();
    cout << "\t\t\"FO\": " << mejorSolucion.funcionObjetivo << "," << endl;
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