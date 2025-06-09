import json
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import sys
import numpy as np
from data_extraction import extract_data_corrected


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

alg1_name = os.path.splitext(os.path.basename(alg1_file))[0]
alg2_name = os.path.splitext(os.path.basename(alg2_file))[0]
alg1_df = pd.DataFrame(extract_data_corrected(alg1_data, alg1_name))
alg2_df = pd.DataFrame(extract_data_corrected(alg2_data, alg2_name))

# Unir datos en una tabla comparativa
comparison_df = pd.merge(
    alg1_df, alg2_df,
    on=["Dias", "Enfermeras", "Demanda", "LS", "US"],
    suffixes=(f"_{alg1_name}", f"_{alg2_name}")
)

# Eliminar columnas de 'Metodo' y 'Nodos Explorados' si existen
comparison_df = comparison_df.drop(columns=[col for col in comparison_df.columns if "Metodo" in col or "Nodos Explorados" in col], errors='ignore')

fig, ax = plt.subplots(figsize=(14, 10))
ax.axis("off")

colors = sns.color_palette("Blues", as_cmap=True)

# Normalizar tiempos para el color (de azul a rojo, gris si hay "X")
def color_mapper(value):
    if value != "X":
        value = float(value)
        norm_value = (value - min_time) / (max_time - min_time) if max_time > min_time else 0
        return colors(norm_value)
    else:
        return '#CCCCCC'

# Obtener min y max de tiempos
valid_times = comparison_df[[f"Tiempo_{alg1_name}", f"Tiempo_{alg2_name}"]].replace("X", np.nan).astype(float).dropna().values.flatten()
if len(valid_times) > 0:
    min_time, max_time = np.min(valid_times), np.max(valid_times)
else:
    min_time, max_time = 0, 1

# Construcción de la tabla de colores
table_data = [comparison_df.columns.to_list()] + comparison_df.fillna("X").values.tolist()
table = ax.table(cellText=table_data, cellLoc='center', loc='center', colLabels=None)

# Aplicar negrita a la primera fila
for j, key in enumerate(table_data[0]):
    table[(0, j)].set_fontsize(12)
    table[(0, j)].set_text_props(weight='bold')

# Aplicar colores a las celdas
for i, row in enumerate(table_data[1:], start=1):
    for j, col_name in enumerate(table_data[0]):
        if "Tiempo" in col_name:
            cell_value = row[j]
            cell_color = color_mapper(cell_value)
            if cell_color:
                table[(i, j)].set_facecolor(cell_color)
                if cell_value != "X":
                    table[(i, j)].get_text().set_text(f"{cell_value} seg")

fig.subplots_adjust(bottom=0.2)

# Agregar una barra de colores (leyenda) debajo de la tabla
cax = fig.add_axes([0.2, 0.05, 0.6, 0.03])  # Posición: [left, bottom, width, height]
cbar = plt.colorbar(plt.cm.ScalarMappable(cmap="Blues"), cax=cax, orientation="horizontal")
cbar.set_label("Tiempo en Segundos (Escala de Azul)")

# Definir valores de referencia en la barra de colores
cbar.set_ticks([0, 0.5, 1])
cbar.set_ticklabels([
    f"{round(min_time, 2)} (Azul Claro)", 
    f"{round((min_time + max_time) / 2, 2)} (Azul Medio)", 
    f"{round(max_time, 2)} (Azul Oscuro)"
])

comparison_table_fixed_path = os.path.join(output_dir, "custom_heatmap_comparison.png")
plt.savefig(comparison_table_fixed_path, bbox_inches="tight", dpi=300)
plt.close()

print("Proceso completado. Archivos generados en:", output_dir)


            ### 2. TABLA % NODOS EXPLORADOS ###


# Filtrar filas sin NaN en nodos explorados para el cálculo de porcentajes
filtered_df = comparison_df.dropna(subset=[f"Tiempo_{alg1_name}", f"Tiempo_{alg2_name}"])

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

    percentage_nodes_df[f"% Nodos Explorados ({numerador_alg}/{denominador_alg})"] = (
        (numerador_nodos / denominador_nodos) * 100
    )

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
