# 🏥TFG-AsignacionTurnosHospitalarios

## ⚙️Configuración inicial

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

## 💪🏼Ejecución del benchmark de una versión

1. Accede a la carpeta scripts y después a la versión que deseas ejecutar:
    ```sh
    cd scripts\v_{idVersion}
    ```
2. Ejecuta este script para hacer el benchmark al completo:
    ```sh
    .\run_all_experiments.bat
    ```
    Se encarga de ejecutar todas las comparaciones de algoritmos de la versión elegida.
> Nota: Este benchmark está completamente automatizado pero puede llegar a durar muchas horas para la ejecución completa

## 🧪Ejecución unitaria de un algoritmo

1. Accede a la carpeta del algoritmo concreto a ejecutar en la versión concreta:
    ```sh
    cd scripts\v_{idVersion}\{algorithm}.cpp
    ```
2. Compila y ejecuta el algoritmo, añadiendo los includes necesarios. Se muestra un ejemplo de ejecución:
    
    ```sh
    g++ -Iinclude .\src\main_utils.cpp .\src\empleado.cpp .\src\solucion.cpp .\src\general_utils.cpp .\src\nodoAstar.cpp 
    .\{algorithm}.cpp -o alg_name_1 -std=c++17 
    ```
    ```
    .\alg_name_1.exe {params...}
    ```
    Al ejecutar el algoritmo, saldrán las instrucciones para la correcta ejecución de este y dependerá de la versión y de los inputs que se le pongan.
> Nota: Opción más manual, pero muy útil para pruebas unitarias con inputs concretos

## 📈 Resultados
1. Accede a la carpeta output de una versión concreta:
    ```sh
    cd output\v_{idVersion}
    ```
    * Aquí se encuentran todas las carpetas con todas las comparaciones hechas en el run_all_experiments.bat

    * Dentro de cada una de las carpetas de comparaciones entre todos los algoritmos de dicha versión, se podrán observar
    las imágenes de las comparaciones de algoritmos usados en dicha versión.
