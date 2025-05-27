#ifndef TURNOS_v2_H
#define TURNOS_v2_H
#include <string>

struct Turno {
    std::string nombre;
    bool operator==(const Turno& other) const;
};

struct Turnos {
    Turno early = {"Early"};
    Turno day = {"Day"};
    Turno late = {"Late"};
    Turno free = {"Free"};
};

#endif