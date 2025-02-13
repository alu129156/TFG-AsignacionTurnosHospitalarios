#include <iostream>
#include <string>
#include <vector>

using namespace std;
class Empleado {
    public:
        string nombre;
        int actualDayCWD;
        int actualDayCFD;
        int prevDayCWD;
        vector<int> cwds;
        Empleado(string nombre)
            : nombre(nombre),
            prevDayCWD(0),
            actualDayCWD(0),
            cwds(){}    
};

void modificarEmpleado(Empleado& e) {
    const Empleado copia = e;
    e.nombre = "Pedro";
    e.actualDayCWD++;
    e.cwds.push_back(43);
    cout << "Copia: {" << copia.nombre << "," << copia.actualDayCWD << "," << copia.cwds.empty() << endl;
    cout << "OUTPUT_DENTRO: {" << e.nombre << "," << e.actualDayCWD << "," << e.cwds.empty() << endl;
    e = copia;
}

int main() {
    Empleado e = Empleado("Carlos");
    //cout << e.nombre << endl;
    modificarEmpleado(e);
    cout << "OUTPUT: {" << e.nombre << "," << e.actualDayCWD << "," << e.cwds.empty() << endl;
}