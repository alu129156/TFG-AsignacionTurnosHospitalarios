#include "general_utils_v2.h"
#include "main_utils_v2.h"
#include "turnos_v2.h"
#include <iostream>
#include <cstdlib>
#include <chrono>

using namespace std;

Config leerParametros(int argc, char* argv[]) {
    if (argc != 12) {
        cout << "Uso: programa <num_enfermeras> <num_dias> <demE> <demD> <demL> <LS> <US> <LCF> <UCF> <LCW> <UCW>" << endl;
        exit(1);
    }

    Config cfg;
    cfg.enfermeras = stoi(argv[1]);
    cfg.dias = stoi(argv[2]);
    cfg.demE = stoi(argv[3]);
    cfg.demD = stoi(argv[4]);
    cfg.demL = stoi(argv[5]);
    cfg.min_asig = stoi(argv[6]);
    cfg.max_asig = stoi(argv[7]);
    cfg.min_dias_libres_consec = stoi(argv[8]);
    cfg.max_dias_libres_consec = stoi(argv[9]);
    cfg.min_dias_trab_consec = stoi(argv[10]);
    cfg.max_dias_trab_consec = stoi(argv[11]);

    int demanda = cfg.demE + cfg.demD + cfg.demL;
    if (cfg.enfermeras < demanda) {
        cout << "Necesitas " << demanda - cfg.enfermeras
             << " o mas enfermeras para asignar segun la demanda que pides";
        exit(1);
    }

    return cfg;
}

vector<Empleado> generarEmpleados_v2(int n) {
    vector<Empleado> empleados;
    for (int i = 0; i < n; i++) {
        empleados.emplace_back("e_" + to_string(i + 1));
    }
    return empleados;
}

void imprimirResultado(const SolucionFinal& solucion, double tiempo, int nodosExplorados,
    bool isAStar, bool needQuickExploration) {
    cout << "\t{" << endl;
    printInput();
    cout << "\t\t\"FO\": " << solucion.funcionObjetivo << "," << endl;

    if (nodosExplorados >= 0) {
        cout << "\t\t\"nodos_explorados\": " << nodosExplorados << ","  << endl;
    }

    cout << "\t\t\"tiempo_de_exec_segundos\": " << tiempo << "," << endl;
    if(isAStar) {
        cout << "\t\t\"exploracion_rapida\": " << needQuickExploration << "," << endl;
    }

    cout << "\t\t\"horario_optimo\":\n\t\t{" << endl;
    Turnos turnos;
    vector<string> tiposTurnos = {turnos.early.nombre, turnos.day.nombre, turnos.late.nombre};
    vector<int> demandas = {DEM_EARLY, DEM_DAY, DEM_LATE};

    for (int dia = 0; dia < solucion.solucion.size(); dia++) {
        cout << "\t\t\t\"dia_" << dia + 1 << "\":\n\t\t\t{\n";
        int dem = 0;
        for (int turno = 0; turno < 3; turno++) {
            cout << "\t\t\t\t\"" << tiposTurnos[turno] << "\": [";
            int inicio = dem;
            int fin = inicio + demandas[turno];

            for (int j = inicio; j < fin && j < solucion.solucion[dia].size(); j++) {
                cout << "\"" << solucion.solucion[dia][j].nombre << "\"";
                if (j < fin - 1) cout << ", ";
            }

            cout << "]";
            if (turno != ULTIMO_TURNO) { cout << ","; }

            dem += demandas[turno];
            cout << endl;
        }
        cout << "\t\t\t}" << (dia != solucion.solucion.size() - 1 ? "," : "") << "\n";
    }
    cout << "\t\t}\n\t}";
}
