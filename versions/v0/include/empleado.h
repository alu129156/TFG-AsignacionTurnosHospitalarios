#ifndef EMPLEADO_V1_H
#define EMPLEADO_V1_H

#include <string>

class Empleado {
    public:
        std::string nombre;
        Empleado(std::string nombre);
        bool operator==(const Empleado& other) const;
};

#endif