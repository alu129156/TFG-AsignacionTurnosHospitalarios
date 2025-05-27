#include "general_utils_v2.h"
#include "empleado_v2.h"
#include "turnos_v2.h"
#include "../../common-params/include/json_parser.h"

int NUM_ENFERMERAS;
int DIAS;
int DEM_EARLY;
int DEM_DAY;
int DEM_LATE;
int MIN_ASIGNACIONES;
int MAX_ASIGNACIONES;
int MAX_DIAS_TRABAJADOS_CONSECUTIVOS;
int MIN_DIAS_TRABAJADOS_CONSECUTIVOS;
int MAX_DIAS_LIBRES_CONSECUTIVOS;
int MIN_DIAS_LIBRES_CONSECUTIVOS;
int LIMITE_TIEMPO;
double PESO_W1, PESO_W2, PESO_W3, PESO_W4, 
        PESO_W5, PESO_W6, PENALIZACION_UNWANTED_SHIFT_PATTERNS;

void cargarPesosDesdeJSON() {
    auto root = JsonParser::ParseJson();
    LIMITE_TIEMPO = (*root.json)["tiempo_de_ejecucion_limite_algoritmos_segundos"].i;
    auto* pesos = (*root.json)["weights"].json;
    PESO_W1 = (*pesos)["weight_max_asignaciones"].d;
    PESO_W2 = (*pesos)["weight_min_asignaciones"].d;
    PESO_W3 = (*pesos)["weight_max_dias_trabajados_consecutivos"].d;
    PESO_W4 = (*pesos)["weight_min_dias_trabajados_consecutivos"].d;
    PESO_W5 = (*pesos)["weight_max_dias_libres_consecutivos"].d;
    PESO_W6 = (*pesos)["weight_min_dias_libres_consecutivos"].d;
    PENALIZACION_UNWANTED_SHIFT_PATTERNS = (*pesos)["weight_unwanted_shift_patterns"].d;
    //cout << "Peso w6: " << PESO_W6 << ", uwsP:" << PENALIZACION_UNWANTED_SHIFT_PATTERNS << "\n";
}

void printInput() {
    cout << "\t\t\"input_data\":\n\t\t{\n\t\t\t\"numero_enfermeras\": " << NUM_ENFERMERAS << 
    ",\n\t\t\t\"dias\": " << DIAS << 
    ",\n\t\t\t\"demanda\": {\n" <<
    "\t\t\t\t\"early\": " << DEM_EARLY <<
    ",\n\t\t\t\t\"day\": " << DEM_DAY <<
    ",\n\t\t\t\t\"late\": " << DEM_LATE << "\n" << "\t\t\t}" <<
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

bool isTimeCompleted(const chrono::time_point<chrono::high_resolution_clock>& startTime) {
    auto now = chrono::high_resolution_clock::now();
    double elapsed = chrono::duration<double>(now - startTime).count();
    if (elapsed > LIMITE_TIEMPO) {
        return true;
    }
    return false;
}

double calcularFuncionObjetivo(
    const vector<Empleado>& employees,
    const vector<vector<Empleado>>& solucion,
    unordered_map<string, int>& empleadoAsignaciones
) {
    double fo = 0.0;

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
        fo += e.erroresPorPenalizacionesUnwanted;
    }

    return fo;
}

double calcularFuncionObjetivoAstar(
    NodoAStar actual
) {
    double fo = 0.0;
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
        fo += e.erroresPorPenalizacionesUnwanted;
    }

    return fo;
}

int empleadoIdx(const vector<Empleado>& empleados, string empleadoNombre) {
    for (int i = 0; i < empleados.size(); ++i) {
        if (empleados[i].nombre == empleadoNombre) {
            return i;
        }
    }
    return -1;
}

int patronDeTurnosUnwanted(Empleado& e) {
    std::pair<Turno, Turno> actualPAT = {e.turnoAnteriorDia, e.turnoActualDia};
    if(e.actual_noches_consec == TRES_NOCHES_CONSEC) {
        e.actual_noches_consec = 0;
        return PENALIZACION_UNWANTED_SHIFT_PATTERNS;
    }

    if(actualPAT == uwsPAT1 || actualPAT == uwsPAT2 || actualPAT == uwsPAT3) {
        return PENALIZACION_UNWANTED_SHIFT_PATTERNS;
    }
    return 0;
}

void calcularErroresPatronesEmpleados(
    vector<Empleado>& empleados,
    const vector<vector<Empleado>>& horario,
    bool TRABAJA
) {
    for(auto& e: empleados) {
        vector<Turno> empleadoTurnos;

        // Vector de turnos para un empleado
        for (int i = 0; i < horario.size(); i++) {
            bool trabaja = !TRABAJA;
            for (int j = 0; j < horario[i].size(); j++) {
                if(e == horario[i][j]) {
                    if(j < DEM_EARLY) {
                        empleadoTurnos.push_back(Turnos().early);
                    } else if(j < (DEM_EARLY + DEM_DAY)) {
                        empleadoTurnos.push_back(Turnos().day);
                    } else {
                        empleadoTurnos.push_back(Turnos().late);
                    }
                    trabaja = TRABAJA;
                    break;
                }
            }
            if(!trabaja) {
                empleadoTurnos.push_back(Turnos().free);
            }
        }
        
        // Buscar los patrones unwanted
        for(int dia = 0; dia < DIAS - 1; dia++) {
            Turno actualDiaTurno = empleadoTurnos[dia]; 
            Turno siguienteDiaTurno = empleadoTurnos[dia + 1];
            std::pair<Turno, Turno> actualPAT = {actualDiaTurno, siguienteDiaTurno};
            if(actualDiaTurno == Turnos().late && siguienteDiaTurno == Turnos().late) {
                if((dia + 2) < DIAS) {
                    if(empleadoTurnos[dia + 2] == Turnos().late) { // L-L-L
                        e.erroresPorPenalizacionesUnwanted += PENALIZACION_UNWANTED_SHIFT_PATTERNS;
                    }
                }
            } else if(actualPAT == uwsPAT1 || actualPAT == uwsPAT2 || actualPAT == uwsPAT3) {
                e.erroresPorPenalizacionesUnwanted += PENALIZACION_UNWANTED_SHIFT_PATTERNS;
            }
        }
    } 
}
