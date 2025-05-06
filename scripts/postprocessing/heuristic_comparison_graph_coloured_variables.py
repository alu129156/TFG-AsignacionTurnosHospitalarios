import json
import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
import sys
import matplotlib.patches as mpatches

# Función para extraer datos enriquecidos según la clave especificada
def extract_fo_metadata(data, key):
    enriched = []
    for entry in data:
        if "mensaje" not in entry and isinstance(entry.get("FO"), (int, float)):
            enriched.append({
                "FO": entry["FO"],
                "Categoria": entry["input_data"].get(key),
                "ExploracionRapida": entry.get("exploracion_rapida", False)
            })
    return enriched

# Verificar los argumentos de entrada
if len(sys.argv) != 5:
    print("Uso: python heuristic_comparison.py <image_folder> <heuristic1_json> <heuristic2_json> <Days|Demand|Nurses>")
    sys.exit(1)

# Obtener los argumentos
output_dir = sys.argv[1]
heuristic1_file = sys.argv[2]
heuristic2_file = sys.argv[3]
categoria = sys.argv[4].lower()

# Determinar clave
clave_map = {"days": "dias", "demand": "demanda", "nurses": "numero_enfermeras"}
if categoria not in clave_map:
    print("La categoría debe ser una de: Days, Demand, Nurses")
    sys.exit(1)
clave = clave_map[categoria]

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
meta1 = extract_fo_metadata(heuristic1_data, clave)
meta2 = extract_fo_metadata(heuristic2_data, clave)

# Agrupar pares válidos
fo_pairs = []
for e1, e2 in zip(meta1, meta2):
    if isinstance(e1["FO"], (int, float)) and isinstance(e2["FO"], (int, float)):
        exploracion_rapida = e1.get("ExploracionRapida", False) or e2.get("ExploracionRapida", False)
        fo_pairs.append((e1["FO"], e2["FO"], e1["Categoria"], exploracion_rapida))

# Determinar límites
if not fo_pairs:
    print("No hay datos válidos para graficar.")
    sys.exit(1)

fo1_vals, fo2_vals = zip(*[(f1, f2) for f1, f2, _, _ in fo_pairs])
min_value = min(min(fo1_vals), min(fo2_vals))
max_value = max(max(fo1_vals), max(fo2_vals))

# Asignar colores por categoría y bordes por exploración rápida
unique_vals = sorted(set(p[2] for p in fo_pairs))
color_map = {val: plt.cm.tab20(i % 20) for i, val in enumerate(unique_vals)}

# Dibujar la gráfica
plt.figure(figsize=(8, 8))
legend_patches = {}
exploracion_patch_added = False

for f1, f2, categoria_val, exploracion_rapida in fo_pairs:
    fill_color = color_map[categoria_val]
    edge_color = 'black' if exploracion_rapida else 'none'

    if exploracion_rapida and not exploracion_patch_added:
        legend_patches["exploracion"] = mpatches.Patch(edgecolor='black', facecolor='white', label="Exploración Rápida", linewidth=2)
        exploracion_patch_added = True

    plt.scatter(f1, f2, facecolors=fill_color, edgecolors=edge_color, s=100)

    if categoria_val not in legend_patches:
        legend_patches[categoria_val] = mpatches.Patch(color=fill_color, label=f"{categoria.title()} {categoria_val}")

plt.legend(handles=list(legend_patches.values()))
plt.plot([min_value, max_value], [min_value, max_value], linestyle="--", color="red", label="y = x")
plt.xlim(min_value, max_value)
plt.ylim(min_value, max_value)
plt.xlabel(f'FO {heuristic1_name}')
plt.ylabel(f'FO {heuristic2_name}')
plt.title(f'Comparación de FO por {categoria.title()}')

# Guardar la imagen
output_suffix = f"by_{categoria}"
comparison_plot_path = os.path.join(output_dir, f"{heuristic1_name}_vs_{heuristic2_name}_graph_{output_suffix}.png")
plt.savefig(comparison_plot_path, bbox_inches="tight", dpi=300)
plt.close()

print("Gráfica correctamente generada en:", comparison_plot_path)
