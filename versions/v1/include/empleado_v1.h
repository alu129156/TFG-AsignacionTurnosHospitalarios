#ifndef EMPLEADO_v1_H
#define EMPLEADO_v1_H

#include <string>
#include <stack>

class Empleado {
    public:
        std::string nombre;
        int errorIncumpleMinT;
        int errorIncumpleMaxT;
        int errorIncumpleMinL;
        int errorIncumpleMaxL;
        std::stack<int> restriccionesMaxT;
        std::stack<int> restriccionesMaxL;
        bool noL;
        bool noT;
    
        Empleado(std::string nombre);
        bool operator==(const Empleado& other) const;         
};

#endif