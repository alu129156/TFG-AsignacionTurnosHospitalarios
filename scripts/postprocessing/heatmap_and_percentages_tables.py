import json
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import sys
import numpy as np
from data_extraction import extract_data_corrected

def nombre_acortado(nombre):
    if nombre == "bruteForce":
        return "bF"
    if nombre == "branchAndBound_1":
        return "B&B1"
    if nombre == "branchAndBound_2":
        return "B&B2"
    return nombre

            ### 1. TABLA COMPARATIVA DE ALGORITMOS ###

if len(sys.argv) != 4:
    print("Uso: python postprocessing_script.py <image_folder> <alg1_json> <alg2_json>")
    sys.exit(1)

output_dir = sys.argv[1]
alg1_file = sys.argv[2]
alg2_file = sys.argv[3]

with open(alg1_file, "r") as f:
    alg1_data = json.load(f)

with open(alg2_file, "r") as f:
    alg2_data = json.load(f)

alg1_name = nombre_acortado(os.path.splitext(os.path.basename(alg1_file))[0])
alg2_name = nombre_acortado(os.path.splitext(os.path.basename(alg2_file))[0])
alg1_df = pd.DataFrame(extract_data_corrected(alg1_data, alg1_name))
alg2_df = pd.DataFrame(extract_data_corrected(alg2_data, alg2_name))

comparison_df = pd.merge(
    alg1_df, alg2_df,
    on=["Dias", "Enfermeras", "Demanda", "LS", "US"],
    suffixes=(f"-{alg1_name}", f"-{alg2_name}")
)
comparison_df = comparison_df.drop(columns=[col for col in comparison_df.columns if "Metodo" in col or "Nodos Explorados" in col], errors='ignore')

# Filtrar filas donde ambos FOs son "X"
total_rows = len(comparison_df)
fo1 = comparison_df[f"FO-{alg1_name}"]
fo2 = comparison_df[f"FO-{alg2_name}"]
mask_both_X = (fo1 == "X") & (fo2 == "X")
skipped_rows = mask_both_X.sum()
comparison_df = comparison_df[~mask_both_X].copy()

# Calcular min/max de tiempos
valid_times = comparison_df[[f"Tiempo-{alg1_name}", f"Tiempo-{alg2_name}"]].replace("X", np.nan).astype(float).dropna().values.flatten()
min_time, max_time = np.min(valid_times), np.max(valid_times) if len(valid_times) else (0, 1)
colors = sns.color_palette("Blues", as_cmap=True)

def color_mapper(value):
    if value != "X":
        norm_value = (float(value) - min_time) / (max_time - min_time) if max_time > min_time else 0
        return colors(norm_value), norm_value
    return "#CCCCCC", 0

# Crear tabla
fig, ax = plt.subplots(figsize=(14, len(comparison_df) * 0.4 + 2))
ax.axis("off")
table_data = [comparison_df.columns.to_list()] + comparison_df.fillna("X").values.tolist()
table = ax.table(cellText=table_data, cellLoc='center', loc='center', colLabels=None)
table.scale(1, 1.5)
table.auto_set_column_width([i for i in range(len(comparison_df.columns))])

# Cabecera en negrita
for j, key in enumerate(table_data[0]):
    table[(0, j)].set_fontsize(12)
    table[(0, j)].set_text_props(weight='bold')

# Colorear y estilizar celdas
for i, row in enumerate(table_data[1:], start=1):
    for j, col_name in enumerate(table_data[0]):
        value = row[j]
        if "Tiempo" in col_name:
            color, norm_val = color_mapper(value)
            table[(i, j)].set_facecolor(color)
            if value != "X":
                table[(i, j)].get_text().set_text(f"{value} seg")
                if norm_val > 0.66:
                    table[(i, j)].get_text().set_color('white')
        elif col_name.startswith("FO") and value != "X":
            table[(i, j)].get_text().set_color('red')

fig.subplots_adjust(bottom=0.25)

# Barra de colores
cax = fig.add_axes([0.2, 0.05, 0.6, 0.03])
cbar = plt.colorbar(plt.cm.ScalarMappable(cmap="Blues"), cax=cax, orientation="horizontal")
cbar.set_label("Tiempo (segundos)")
cbar.set_ticks([0, 0.5, 1])
cbar.set_ticklabels([
    f"{round(min_time, 2)}", 
    f"{round((min_time + max_time) / 2, 2)}", 
    f"{round(max_time, 2)}"
])

# Añadir % no resueltos debajo
pct_skip = round(100 * skipped_rows / total_rows, 2)
ax.text(0.5, -0.2, f"El {pct_skip}% de instancias no han sido resueltas por ninguno de los dos algoritmos", 
        ha='center', va='center', transform=ax.transAxes, fontsize=10)

# Guardar
os.makedirs(output_dir, exist_ok=True)
comparison_table_fixed_path = os.path.join(output_dir, "custom_heatmap_comparison.png")
plt.savefig(comparison_table_fixed_path, bbox_inches="tight", dpi=300)
plt.close()

print("Proceso completado. Archivos generados en:", output_dir)


            ### 2. TABLA % NODOS EXPLORADOS ###


# Filtrar filas sin NaN en nodos explorados para el cálculo de porcentajes
filtered_df = comparison_df.dropna(subset=[f"Tiempo-{alg1_name}", f"Tiempo-{alg2_name}"])

# Tabla de porcentaje de nodos explorados
if not filtered_df.empty:
    percentage_nodes_df = filtered_df[["Dias", "Enfermeras", "Demanda", "LS", "US"]].copy().astype(int)
    valid_indices = alg2_df["Nodos Explorados"].notna() & alg1_df["Nodos Explorados"].notna()
    alg1_nodos = alg1_df.loc[valid_indices, "Nodos Explorados"].astype(int)
    alg2_nodos = alg2_df.loc[valid_indices, "Nodos Explorados"].astype(int)

    if (alg1_nodos < alg2_nodos).sum() > (alg2_nodos < alg1_nodos).sum():
        numerador_alg = alg1_name
        denominador_alg = alg2_name
        numerador_nodos = alg1_nodos
        denominador_nodos = alg2_nodos
    else:
        numerador_alg = alg2_name
        denominador_alg = alg1_name
        numerador_nodos = alg2_nodos
        denominador_nodos = alg1_nodos

    columna_porcentaje = f"% Nodos Explorados: {numerador_alg}/{denominador_alg}"
    percentage_nodes_df[columna_porcentaje] = (
        (numerador_nodos / denominador_nodos) * 100
    )
    percentage_nodes_df[columna_porcentaje] = percentage_nodes_df[columna_porcentaje].round(3)

    for col in ["Dias", "Enfermeras", "Demanda", "LS", "US"]:
        percentage_nodes_df[col] = percentage_nodes_df[col].round().astype(str)


    fig, ax = plt.subplots(figsize=(10, 6))
    ax.axis("off")
    table_data = [percentage_nodes_df.columns.to_list()] + percentage_nodes_df.values.tolist()
    table = ax.table(cellText=table_data, cellLoc='center', loc='center', colLabels=None)

    # Aplicar negrita a la primera fila
    for j, key in enumerate(table_data[0]):
        table[(0, j)].set_fontsize(12)
        table[(0, j)].set_text_props(weight='bold')
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.auto_set_column_width([i for i in range(len(percentage_nodes_df.columns))])
    percentage_table_fixed_path = os.path.join(output_dir, "percentage_explored_nodes.png")
    plt.savefig(percentage_table_fixed_path, bbox_inches="tight", dpi=300)
    plt.close()

print("Proceso completado. Archivos generados en:", output_dir)
