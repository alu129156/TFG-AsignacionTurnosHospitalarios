#ifndef INCUMPLIMIENTOS_H
#define INCUMPLIMIENTOS_H
#include <vector>

struct Incumplimientos {
    std::vector<bool> max_asingaciones;
    std::vector<bool> min_asignaciones;
    std::vector<bool> min_dias_libres_consec;
    std::vector<bool> max_dias_libres_consec;
    std::vector<bool> min_dias_trab_consec;
    std::vector<bool> max_dias_trab_consec;

    Incumplimientos(int num_empleados);
};

int contarIncumplimientos(const Incumplimientos& incumplimientos);

Incumplimientos unionIncumplimientos(const Incumplimientos& A, const Incumplimientos& B);

Incumplimientos interseccionIncumplimientos(const Incumplimientos& A, const Incumplimientos& B);

#endif