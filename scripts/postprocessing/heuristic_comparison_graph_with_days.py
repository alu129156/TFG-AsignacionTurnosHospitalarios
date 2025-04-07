import json
import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
import sys
import matplotlib.patches as mpatches

# Función para extraer datos enriquecidos
def extract_fo_metadata(data):
    enriched = []
    for entry in data:
        if "mensaje" not in entry and isinstance(entry.get("FO"), (int, float)):
            enriched.append({
                "FO": entry["FO"],
                "Dias": entry["input_data"].get("dias"),
                "ExploracionRapida": entry.get("exploracion_rapida", False)
            })
    return enriched

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

# Extraer FO y metadata
meta1 = extract_fo_metadata(heuristic1_data)
meta2 = extract_fo_metadata(heuristic2_data)

# Agrupar pares válidos
fo_pairs = []
for e1, e2 in zip(meta1, meta2):
    if isinstance(e1["FO"], (int, float)) and isinstance(e2["FO"], (int, float)):
        exploracion_rapida = e1.get("ExploracionRapida", False) or e2.get("ExploracionRapida", False)
        fo_pairs.append((e1["FO"], e2["FO"], e1["Dias"], exploracion_rapida))

# Determinar límites
if not fo_pairs:
    print("No hay datos válidos para graficar.")
    sys.exit(1)

fo1_vals, fo2_vals = zip(*[(f1, f2) for f1, f2, _, _ in fo_pairs])
min_value = min(min(fo1_vals), min(fo2_vals))
max_value = max(max(fo1_vals), max(fo2_vals))

# Asignar colores por día y bordes por exploración rápida
unique_dias = sorted(set(p[2] for p in fo_pairs))
color_map = {dia: plt.cm.tab20(i % 20) for i, dia in enumerate(unique_dias)}

# Dibujar la gráfica
plt.figure(figsize=(8, 8))
legend_patches = {}
exploracion_patch_added = False

for f1, f2, dia, exploracion_rapida in fo_pairs:
    fill_color = color_map[dia]

    edge_color = 'black' if exploracion_rapida else 'none'

    if exploracion_rapida and not exploracion_patch_added:
        legend_patches["exploracion"] = mpatches.Patch(edgecolor='black', facecolor='white', label="Exploración Rápida", linewidth=2)
        exploracion_patch_added = True

    plt.scatter(f1, f2, facecolors=fill_color, edgecolors=edge_color, s=100)

    if dia not in legend_patches:
        legend_patches[dia] = mpatches.Patch(color=fill_color, label=f"Dia {dia}")

plt.legend(handles=list(legend_patches.values()))
plt.plot([min_value, max_value], [min_value, max_value], linestyle="--", color="red", label="y = x")
plt.xlim(min_value, max_value)
plt.ylim(min_value, max_value)
plt.xlabel(f'FO {heuristic1_name}')
plt.ylabel(f'FO {heuristic2_name}')
plt.title('Comparación de FO entre Heurísticas')

# Guardar la imagen
comparison_plot_path = os.path.join(output_dir, f"{heuristic1_name}_vs_{heuristic2_name}_graph_colored.png")
plt.savefig(comparison_plot_path, bbox_inches="tight", dpi=300)
plt.close()

print("Gráfica correctamente generada en:", comparison_plot_path)
