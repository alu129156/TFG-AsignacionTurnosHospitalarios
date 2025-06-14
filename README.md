# 🏥TFG-AsignacionTurnosHospitalarios

## ⚙️Configuración inicial 🔧

1. Clona el repositorio:
    ```sh
    git clone https://github.com/alu129156/TFG-AsignacionTurnosHospitalarios.git
    cd TFG-AsignacionTurnosHospitalarios.git
    ```
2. Crea un entorno virtual:
    ```sh
    python -m venv venv
    ```
3. Instala las dependencias:
    ```sh
    pip install -r requirements.txt
    ```
    
## 🔧Requisitos mínimos para la ejecución del proyecto

1. Contar con el Sistema Operativo Windows, preferiblemente Windows 10 u 11
   
2. Usar el IDE Visual Studio Code para las ejecuciones individuales de los algoritmos

## 💪🏼Ejecución del benchmark de una versión

1. Accede a la carpeta scripts y después a la versión que deseas ejecutar:
    ```sh
    cd scripts\v{idVersion}
    ```
2. Ejecuta este script para hacer el benchmark al completo:
    ```sh
    .\run_all_experiments.bat
    ```
    Se encarga de ejecutar todas las comparaciones de algoritmos de la versión elegida.
> Nota: Este benchmark está completamente automatizado pero puede llegar a durar hasta varios días para la ejecución completa

## 🧪Ejecución unitaria de un algoritmo

1. Selecciona con doble click el algoritmo "{algorithm}.cpp" que desea ejecutar de la versión v{idVersion}:
    ```sh
    cd versions\v{idVersion}\{algorithm}.cpp
    ```
2. Utilizando el IDE de Visual Studio Code, acceda al desplegable Terminal arriba a la izquierda y siga estos pasos:
    ```sh
    Terminal --> Run Build Task --> Build (v{idVersion})
    ```
   * Con esto se habrá compilado el algoritmo seleccionado
> Impotante: La Build a seleccionar debe de corresponder con la versión del algoritmo que está ejecutando
3. Acceda a la carpeta donde se encuentra el ejecutable y ejecute con el input deseado:
    ```sh
    cd versions\v{idVersion}\bin
    .\{algorithm}.exe <params...>
    ```
> Nota: Los parametros son números y van separados de un espacio. Saltará el usage de los parámetros si ejecuta: .\{algorithm}.exe

## 📈 Resultados 
1. Accede a la carpeta output de una versión concreta:
    ```sh
    cd output\v{idVersion}
    ```
    * Aquí se encuentran todas las carpetas con todas las comparaciones hechas en los benchmarks:
            * Ficheros de outputs del benchmark de cada algoritmo en formato JSON
            * En las carpetas que hay, se encuentran las tablas y gráficas comparativas entre pares de algoritmos.
