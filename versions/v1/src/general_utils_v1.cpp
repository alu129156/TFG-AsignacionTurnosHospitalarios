#include "general_utils_v1.h"
#include "empleado_v1.h"

int NUM_ENFERMERAS;
int DIAS;
int DEMANDA;
int MIN_ASIGNACIONES;
int MAX_ASIGNACIONES;
int MAX_DIAS_TRABAJADOS_CONSECUTIVOS;
int MIN_DIAS_TRABAJADOS_CONSECUTIVOS;
int MAX_DIAS_LIBRES_CONSECUTIVOS;
int MIN_DIAS_LIBRES_CONSECUTIVOS;

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
