import json
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import sys

# Verificar los argumentos de entrada
if len(sys.argv) != 4:
    print("Uso: python postprocessing_script.py <image_folder> <alg1_json> <alg2_json>")
    sys.exit(1)

# Obtener los argumentos
output_dir = sys.argv[1]
alg1_file = sys.argv[2]
alg2_file = sys.argv[3]

# Cargar los archivos JSON
with open(alg1_file, "r") as f:
    alg1_data = json.load(f)

with open(alg2_file, "r") as f:
    alg2_data = json.load(f)

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
                    "Dias": dias,
                    "FO": entry["FO"] if "FO" in entry else "X",
                    "Tiempo": entry["tiempo_de_exec_segundos"] if "tiempo_de_exec_segundos" in entry else None,
                    "Nodos Explorados": entry["nodos_explorados"] if "nodos_explorados" in entry else None,
                    "Metodo": method_name
                })

    return extracted_data

# Extraer datos corregidos
alg1_name = os.path.splitext(os.path.basename(alg1_file))[0]
alg2_name = os.path.splitext(os.path.basename(alg2_file))[0]
alg1_df = pd.DataFrame(extract_data_corrected(alg1_data, alg1_name))
alg2_df = pd.DataFrame(extract_data_corrected(alg2_data, alg2_name))

# 1. TABLA GENERAL COMPARATIVA DE LOS ALGORITMOS
comparison_df = pd.merge(
    alg1_df, alg2_df, on="Key", suffixes=(f"_{alg1_name}", f"_{alg2_name}")
)[["Key", f"FO_{alg1_name}", f"FO_{alg2_name}", f"Tiempo_{alg1_name}", f"Tiempo_{alg2_name}"]]

comparison_df.rename(columns={
    f"Tiempo_{alg1_name}": f"Tiempo_{alg1_name} (s)",
    f"Tiempo_{alg2_name}": f"Tiempo_{alg2_name} (s)"
}, inplace=True)

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
plt.savefig(comparison_table_fixed_path, bbox_inches="tight", dpi=300)
plt.close()

# 2. GRÁFICOS UNO POR CADA DÍA
alg1_df["Dias"] = alg1_df["Dias"].fillna(-1).astype(int)
alg2_df["Dias"] = alg2_df["Dias"].fillna(-1).astype(int)

# Obtener todos los valores únicos de DIAS en los datos
unique_dias = sorted(set(alg1_df["Dias"]).union(set(alg2_df["Dias"])))

time_comparison_images = []
for dias in unique_dias:
    print(f"\nProcesando gráfico para DIAS={dias}...")

    # Filtrar por el número de días actual
    alg1_subset = alg1_df[alg1_df["Dias"] == dias][["Key", "Tiempo"]].set_index("Key")
    alg2_subset = alg2_df[alg2_df["Dias"] == dias][["Key", "Tiempo"]].set_index("Key")

    # Obtener todas las configuraciones posibles
    all_keys = sorted(set(alg1_subset.index).union(set(alg2_subset.index)))

    # Asegurar que todas las configuraciones aparecen en el gráfico
    combined_df = pd.DataFrame(index=all_keys)
    combined_df[f"Tiempo_{alg1_name}"] = alg1_subset.reindex(all_keys)
    combined_df[f"Tiempo_{alg2_name}"] = alg2_subset.reindex(all_keys)

    if not (combined_df[f"Tiempo_{alg1_name}"].notna().any() or combined_df[f"Tiempo_{alg2_name}"].notna().any()):
        print(f"⚠️ Ambos algoritmos carecen de datos en DIAS={dias}, saltando...")
        continue

    plt.figure(figsize=(12, 6))
    keys_positions = range(len(all_keys))

    if combined_df[f"Tiempo_{alg1_name}"].notna().any():
        plt.plot(
            keys_positions,  
            combined_df[f"Tiempo_{alg1_name}"] + 0.12,  # Aumentar un poco para diferenciarlo
            label=alg1_name, marker="o", linestyle="-"
        )

    if combined_df[f"Tiempo_{alg2_name}"].notna().any():
        plt.plot(
            keys_positions,  
            combined_df[f"Tiempo_{alg2_name}"] - 0.12,  # Reducir un poco para diferenciarlo
            label=alg2_name, marker="s", linestyle="--"
        )

    plt.xticks(ticks=keys_positions, labels=all_keys, rotation=90)
    plt.xlabel("Configuraciones de Input")
    plt.ylabel("Tiempo de Ejecución (s)")
    plt.title(f"Comparación de Tiempo de Ejecución para {dias} Días")
    plt.legend()
    plt.grid()

    img_path = os.path.join(output_dir, f"time_comparison_dias_{dias}.png")
    plt.savefig(img_path, bbox_inches="tight", dpi=300)
    plt.close()

    time_comparison_images.append(img_path)



# 3. TABLA PORCENTAJE NODOS EXPLORADOS
percentage_nodes_df = comparison_df[["Key"]].copy()

# Filtrar valores donde ambos métodos tengan valores válidos en nodos explorados
valid_indices = alg2_df["Nodos Explorados"].notna() & alg1_df["Nodos Explorados"].notna()
alg1_nodos = alg1_df.loc[valid_indices, "Nodos Explorados"]
alg2_nodos = alg2_df.loc[valid_indices, "Nodos Explorados"]

# Determinar qué algoritmo tiene menos nodos en la mayoría de los casos
alg1_better = (alg1_nodos < alg2_nodos).sum()
alg2_better = (alg2_nodos < alg1_nodos).sum()

if alg1_better > alg2_better:
    numerador_alg = alg1_name
    denominador_alg = alg2_name
    numerador_nodos = alg1_nodos
    denominador_nodos = alg2_nodos
else:
    numerador_alg = alg2_name
    denominador_alg = alg1_name
    numerador_nodos = alg2_nodos
    denominador_nodos = alg1_nodos

# Calcular el porcentaje
percentage_nodes_df[f"% Nodos Explorados ({numerador_alg}/{denominador_alg})"] = (
    (numerador_nodos / denominador_nodos) * 100
)

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
plt.savefig(percentage_table_fixed_path, bbox_inches="tight", dpi=300)
plt.close()

