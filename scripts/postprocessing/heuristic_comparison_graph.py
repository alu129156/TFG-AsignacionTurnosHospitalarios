import json
import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
import sys

# Función para extraer datos de FO
def extract_fo_data(data):
    return [entry["FO"] for entry in data if "mensaje" not in entry] # if "mensaje" not in entry

# Verificar los argumentos de entrada
if len(sys.argv) != 4:
    print("Uso: python heuristic_comparison.py <image_folder> <heuristic1_json> <heuristic2_json>")
    sys.exit(1)

# Obtener los argumentos
output_dir = sys.argv[1]
heuristic1_file = sys.argv[2]
heuristic2_file = sys.argv[3]

# Extraer nombres de los archivos sin extensión
heuristic1_name = os.path.splitext(os.path.basename(heuristic1_file))[0]
heuristic2_name = os.path.splitext(os.path.basename(heuristic2_file))[0]

# Verificar que los archivos son heurísticas
if not (heuristic1_name.startswith("heuristic_") and heuristic2_name.startswith("heuristic_")):
    print("Los dos archivos deben de ser heurísticas para realizar la comparación")
    sys.exit(1)

# Cargar los archivos JSON
with open(heuristic1_file, "r") as f:
    heuristic1_data = json.load(f)

with open(heuristic2_file, "r") as f:
    heuristic2_data = json.load(f)

# Extraer valores de FO
fo_heuristic1 = extract_fo_data(heuristic1_data)
fo_heuristic2 = extract_fo_data(heuristic2_data)

fo_pairs = [
    (entry1["FO"], entry2["FO"])
    for entry1, entry2 in zip(heuristic1_data, heuristic2_data)
    if "mensaje" not in entry1 and "mensaje" not in entry2 and isinstance(entry1["FO"], (int, float)) and isinstance(entry2["FO"], (int, float))
]

# Separar los valores en listas para graficar
# if fo_pairs:
#     fo_heuristic1, fo_heuristic2 = zip(*fo_pairs)
# else:
#     fo_heuristic1, fo_heuristic2 = [], []

# if not fo_heuristic1 or not fo_heuristic2:
#     print("No hay datos válidos para graficar.")
#     sys.exit(1)

# Determinar los valores mínimo y máximo de las FO
min_value = min(min(fo_heuristic1), min(fo_heuristic2))
max_value = max(max(fo_heuristic1), max(fo_heuristic2))

# Generar la gráfica
plt.figure(figsize=(8, 8))
plt.scatter(fo_heuristic1, fo_heuristic2, color='blue', label='Puntos (FO_heuristic1, FO_heuristic2)')
plt.plot([min_value, max_value], [min_value, max_value], linestyle="--", color="red", label="y = x")
plt.xlim(min_value, max_value)
plt.ylim(min_value, max_value)
plt.xlabel(f'FO {heuristic1_name}')
plt.ylabel(f'FO {heuristic2_name}')
plt.title('Comparación de FO entre Heurísticas')
plt.legend()

# Guardar la imagen
comparison_plot_path = os.path.join(output_dir, f"{heuristic1_name}_vs_{heuristic2_name}_graph.png")
plt.savefig(comparison_plot_path, bbox_inches="tight", dpi=300)
plt.close()

print("Gráfica correctamente generada en ", comparison_plot_path)
