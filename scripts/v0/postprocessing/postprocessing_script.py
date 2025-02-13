
import json
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import sys

# Verificar los argumentos de entrada
if len(sys.argv) != 3:
    print("Uso: python postprocessing_script.py <brute_force_json> <branch_and_bound_json>")
    sys.exit(1)

# Obtener los argumentos
brute_force_file = sys.argv[1]
branch_and_bound_file = sys.argv[2]

script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, "../../.."))
output_dir = os.path.join(project_root, "output/v0/images")

# Crear la carpeta si no existe
os.makedirs(output_dir, exist_ok=True)

# Cargar los archivos JSON
with open(brute_force_file, "r") as f:
    brute_force_data = json.load(f)

with open(branch_and_bound_file, "r") as f:
    branch_and_bound_data = json.load(f)

# Función para extraer datos
def extract_data_corrected(data, method_name):
    extracted_data = []
    seen_keys = set()

    for entry in data:
        input_data = entry["input_data"]
        dias = input_data["dias"]
        numero_enfermeras = input_data["numero_enfermeras"]
        demanda = input_data["demanda"]
        ls = input_data["limite_inferior_asignaciones"]
        us = input_data["limite_superior_asignaciones"]

        key = f"Dias={dias}, Enfermeras={numero_enfermeras}, Demanda={demanda}, LS={ls}, US={us}"

        if key not in seen_keys:
            seen_keys.add(key)
            if "mensaje" in entry:
                extracted_data.append({"Key": key, "FO": "X", "Tiempo": None, "Nodos Explorados": None, "Metodo": method_name})
            else:
                extracted_data.append({
                    "Key": key,
                    "FO": entry["FO"],
                    "Tiempo": entry["tiempo_de_exec_segundos"],
                    "Nodos Explorados": entry["nodos_explorados"],
                    "Metodo": method_name
                })

    return extracted_data

# Extraer datos corregidos
brute_force_df = pd.DataFrame(extract_data_corrected(brute_force_data, "Backtracking"))
branch_and_bound_df = pd.DataFrame(extract_data_corrected(branch_and_bound_data, "Branch and Bound"))

# Unir ambos resultados en una sola tabla comparativa
comparison_df = pd.merge(
    brute_force_df, branch_and_bound_df, on="Key", suffixes=("_Backtracking", "_BnB")
)[["Key", "FO_Backtracking", "FO_BnB", "Tiempo_Backtracking", "Tiempo_BnB"]]

# Generar la tabla comparativa corregida como imagen
fig, ax = plt.subplots(figsize=(14, 10))
ax.axis("off")

# Crear tabla visualmente amigable
table_data = [comparison_df.columns.to_list()] + comparison_df.fillna("X").values.tolist()
table = ax.table(cellText=table_data, colLabels=None, cellLoc='center', loc='center')

# Ajustar la escala de la tabla
table.auto_set_font_size(False)
table.set_fontsize(10)
table.auto_set_column_width([i for i in range(len(comparison_df.columns))])

# Guardar la tabla corregida como imagen
comparison_table_fixed_path = os.path.join(output_dir, "comparison_table_corrected.png")
plt.savefig(comparison_table_fixed_path, bbox_inches='tight', dpi=300)
plt.close()

# Obtener todos los valores únicos de DIAS en los datos
unique_dias = sorted(set(brute_force_df["Key"].str.extract(r"Dias=(\d+)")[0].dropna().astype(int)))
print("DÍAS DETECTADOS:", unique_dias)  # Para depuración

time_comparison_images = []
for dias in unique_dias:
    print(f"\nProcesando gráfico para DIAS={dias}...")  # Mensaje en consola

    # Filtrar por el número de días actual
    bf_subset = brute_force_df[brute_force_df["Key"].str.contains(f"Dias={dias}", regex=False)]
    bnb_subset = branch_and_bound_df[branch_and_bound_df["Key"].str.contains(f"Dias={dias}", regex=False)]

    print(f"  -> Datos en Backtracking: {len(bf_subset)} filas")
    print(f"  -> Datos en Branch and Bound: {len(bnb_subset)} filas")

    # Eliminar valores nulos en tiempo de ejecución
    bf_subset = bf_subset.dropna(subset=["Tiempo"])
    bnb_subset = bnb_subset.dropna(subset=["Tiempo"])

    # Verificar si al menos uno tiene datos antes de graficar
    if bf_subset.empty and bnb_subset.empty:
        print(f"  ⚠️  No hay datos válidos para DIAS={dias}, saltando...")
        continue  # Si ambos están vacíos, no se genera la gráfica

    # Crear el gráfico
    plt.figure(figsize=(10, 6))

    # Graficar solo los métodos que tienen datos
    if not bf_subset.empty:
        plt.plot(bf_subset["Key"], bf_subset["Tiempo"], label="Backtracking", marker="o", linestyle="-")
    if not bnb_subset.empty:
        plt.plot(bnb_subset["Key"], bnb_subset["Tiempo"], label="Branch and Bound", marker="s", linestyle="--")

    plt.xlabel("Configuraciones de Input")
    plt.ylabel("Tiempo de Ejecución (s)")
    plt.title(f"Comparación de Tiempo de Ejecución para {dias} Días")
    plt.legend()
    plt.xticks(rotation=90)
    plt.grid()

    # Guardar la imagen
    img_path = os.path.join(output_dir, f"time_comparison_dias_{dias}.png")
    plt.savefig(img_path, bbox_inches="tight", dpi=300)
    plt.close()

    time_comparison_images.append(img_path)


# Crear tabla de porcentaje de nodos explorados BnB/Backtracking corregida
percentage_nodes_df = comparison_df[["Key"]].copy()

# Filtrar valores donde ambos métodos tengan valores válidos en nodos explorados
valid_indices = branch_and_bound_df["Nodos Explorados"].notna() & brute_force_df["Nodos Explorados"].notna()
percentage_nodes_df["% Nodos Explorados en B&B"] = (branch_and_bound_df.loc[valid_indices, "Nodos Explorados"] /
                                                      brute_force_df.loc[valid_indices, "Nodos Explorados"]) * 100

# Eliminar valores nulos
percentage_nodes_df = percentage_nodes_df.dropna()

# Guardar tabla de porcentaje como imagen
fig, ax = plt.subplots(figsize=(10, 6))
ax.axis("off")

# Crear tabla visualmente amigable
table_data = [percentage_nodes_df.columns.to_list()] + percentage_nodes_df.values.tolist()
table = ax.table(cellText=table_data, colLabels=None, cellLoc='center', loc='center')

# Ajustar la escala de la tabla
table.auto_set_font_size(False)
table.set_fontsize(10)
table.auto_set_column_width([i for i in range(len(percentage_nodes_df.columns))])

# Guardar la tabla como imagen
percentage_table_fixed_path = os.path.join(output_dir, "percentage_nodes_table_corrected.png")
plt.savefig(percentage_table_fixed_path, bbox_inches='tight', dpi=300)
plt.close()
