#include "general_utils.h"
#include "nodoAstar.h"
#include "../../common-params/include/json_parser.h"

int NUM_ENFERMERAS;
int DIAS;
int DEMANDA;
int MIN_ASIGNACIONES;
int MAX_ASIGNACIONES;
double PESO_W1, PESO_W2;
double LIMITE_TIEMPO;

void cargarPesosDesdeJSON() {
    auto root = JsonParser::ParseJson();
    LIMITE_TIEMPO = (*root.json)["tiempo_de_ejecucion_limite_algoritmos_segundos"].d;
    auto* pesos = (*root.json)["weights"].json;
    PESO_W1 = (*pesos)["weight_max_asignaciones"].d;
    PESO_W2 = (*pesos)["weight_min_asignaciones"].d;
    //cout << "Peso w2: " << PESO_W2 << "\n";
}

void printInput() {
    std::cout << "\t\t\"input_data\":\n\t\t{\n\t\t\t\"numero_enfermeras\": " << NUM_ENFERMERAS <<
        ",\n\t\t\t\"dias\": " << DIAS << ",\n\t\t\t\"demanda\": " << DEMANDA <<
        ",\n\t\t\t\"limite_inferior_asignaciones\": " << MIN_ASIGNACIONES <<
        ",\n\t\t\t\"limite_superior_asignaciones\": " << MAX_ASIGNACIONES << "\n\t\t},\n";
}

void verifyTime(const std::chrono::time_point<std::chrono::high_resolution_clock>& startTime) {
    auto now = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(now - startTime).count();
    if (elapsed > LIMITE_TIEMPO) {
        std::cout << "\t{\n";
        printInput();
        std::cout << "\t\t\"mensaje\": \"se ha excedido del tiempo maximo permitido\"\n\t}";
        exit(1);
    }
}

bool isTimeCompleted(
    const double& elapsed
) {
    if (elapsed > LIMITE_TIEMPO) {
        return true;
    }
    return false;
}

double calcularFuncionObjetivo(const std::vector<std::vector<Empleado>>& solucion,
                                      const std::unordered_map<std::string, int>& empleadoAsignaciones
) {
    double fo = 0.0;
    for (const auto& empleado : empleadoAsignaciones) {
        int total = empleado.second;
        fo += PESO_W1 * std::max(0, total - MAX_ASIGNACIONES);
        fo += PESO_W2 * std::max(0, MIN_ASIGNACIONES - total);
    }
    return fo;
}

double calcularFuncionObjetivoAStar(
    NodoAStar actual
) {
    double fo = 0.0;

    for (const auto& empleado : actual.empleadoAsignaciones) {
        int asignacionesEmpleado = empleado.second;
        int e1 = max(0, asignacionesEmpleado - MAX_ASIGNACIONES);
        double r1 = PESO_W1 * e1;

        int e2 = max(0, MIN_ASIGNACIONES - asignacionesEmpleado);
        double r2 = PESO_W2 * e2;

        fo += (r1 + r2);
    }
    return fo;
}
