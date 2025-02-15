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

3. Activa el entorno virtual:
    - En Windows:
        ```sh
        venv\Scripts\activate
        ```
    - En macOS/Linux:
        ```sh
        source venv/bin/activate
        ```

4. Instala las dependencias:
    ```sh
    pip install -r requirements.txt
    ```

## 💪🏼Ejecución de una versión

1. Accede a la carpeta scripts y después a la versión que deseas ejecutar:
    ```sh
    cd scripts\v_{idVersion}
    ```
2. Ejecuta este script concreto:
    ```sh
    .\run_all_experiments.bat
    ```
    Se encarga de ejecutar todas las comparaciones de algoritmos de la versión elegida.
> Nota: En versiones posteriores, se agregarán scripts en bash para macOS.