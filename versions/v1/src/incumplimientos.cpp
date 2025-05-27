#include "incumplimientos.h"

Incumplimientos::Incumplimientos(int num_empleados) {
        max_asingaciones.resize(num_empleados, false);
        min_asignaciones.resize(num_empleados, false);
        min_dias_libres_consec.resize(num_empleados, false);
        max_dias_libres_consec.resize(num_empleados, false);
        min_dias_trab_consec.resize(num_empleados, false);
        max_dias_trab_consec.resize(num_empleados, false);
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