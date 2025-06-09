import json
import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
import sys

def extract_fo_data(data):
    return [entry["FO"] for entry in data if "mensaje" not in entry] # if "mensaje" not in entry


if len(sys.argv) != 4:
    print("Uso: python heuristic_comparison.py <image_folder> <heuristic1_json> <heuristic2_json>")
    sys.exit(1)

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

heuristic1_name = os.path.splitext(os.path.basename(heuristic1_file))[0].replace("heuristic_", "", 1)
heuristic2_name = os.path.splitext(os.path.basename(heuristic2_file))[0].replace("heuristic_", "", 1)

# Quitar sufijos si existen
for suf in ["_general_benchmark", "_limit_cases_benchmark"]:
    if heuristic1_name.endswith(suf):
        heuristic1_name = heuristic1_name[: -len(suf)]
    if heuristic2_name.endswith(suf):
        heuristic2_name = heuristic2_name[: -len(suf)]

with open(heuristic1_file, "r") as f:
    heuristic1_data = json.load(f)

with open(heuristic2_file, "r") as f:
    heuristic2_data = json.load(f)


fo_heuristic1 = extract_fo_data(heuristic1_data)
fo_heuristic2 = extract_fo_data(heuristic2_data)

fo_pairs = [
    (entry1["FO"], entry2["FO"])
    for entry1, entry2 in zip(heuristic1_data, heuristic2_data)
    if "mensaje" not in entry1 and "mensaje" not in entry2 and isinstance(entry1["FO"], (int, float)) and isinstance(entry2["FO"], (int, float))
]

fo1_vals, fo2_vals = zip(*[(f1, f2) for f1, f2 in fo_pairs])
min_value = min(min(fo1_vals), min(fo2_vals))
max_value = max(max(fo1_vals), max(fo2_vals))


plt.figure(figsize=(8, 8))
plt.scatter(fo1_vals, fo2_vals, color='blue', label='Puntos (FO_heuristic1, FO_heuristic2)')
plt.plot([min_value, max_value], [min_value, max_value], linestyle="--", color="red", label="y = x")
plt.xlim(min_value, max_value)
plt.ylim(min_value, max_value)
plt.xlabel(f'FO {heuristic1_name}')
plt.ylabel(f'FO {heuristic2_name}')
plt.title('Comparación de FO entre Heurísticas')

output_dir = os.path.abspath(output_dir)
os.makedirs(output_dir, exist_ok=True)
comparison_plot_path = os.path.join(output_dir, f"{heuristic1_name}_vs_{heuristic2_name}_graph.png")
plt.savefig(comparison_plot_path, bbox_inches="tight", dpi=300)
plt.close()

print("Gráfica correctamente generada en ", comparison_plot_path)
