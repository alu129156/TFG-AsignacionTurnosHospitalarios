import json
import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np
import sys
import matplotlib.patches as mpatches


def get_input_key(entry):
    key_items = []
    for k, v in entry["input_data"].items():
        if isinstance(v, dict):
            key_items.append((k, tuple(sorted(v.items()))))
        else:
            key_items.append((k, v))
    return tuple(sorted(key_items))

def get_visual_jitter(fo):
    if fo < 20:
        return 2.5
    if fo < 50:
        return 2
    elif fo < 100:
        return 1.5
    elif fo < 300:
        return 1
    elif fo < 1000:
        return 0.5
    else:
        return 0.25 


if len(sys.argv) != 5:
    print("Uso: python heuristic_comparison.py <image_folder> <heuristic1_json> <heuristic2_json> <Days|Demand>")
    sys.exit(1)

output_dir = sys.argv[1]
heuristic1_file = sys.argv[2]
heuristic2_file = sys.argv[3]
categoria = sys.argv[4].lower()


clave_map = {"days": "dias", "demand": "demanda"}
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

# indexar por input
indexed1 = {
    get_input_key(e): e for e in heuristic1_data
    if "mensaje" not in e and isinstance(e.get("FO"), (int, float))
}
indexed2 = {
    get_input_key(e): e for e in heuristic2_data
    if "mensaje" not in e and isinstance(e.get("FO"), (int, float))
}

# intersección de inputs válidos
common_keys = set(indexed1.keys()) & set(indexed2.keys())

fo_pairs = []
for key in common_keys:
    e1 = indexed1[key]
    e2 = indexed2[key]
    categoria_val = e1["input_data"].get(clave)
    if clave == "demanda":
        if isinstance(categoria_val, dict):
            categoria_val = sum(categoria_val.values())
        elif isinstance(categoria_val, int):
            categoria_val = categoria_val * 3
    exploracion_rapida = e1.get("exploracion_rapida", False) or e2.get("exploracion_rapida", False)
    fo_pairs.append((e1["FO"], e2["FO"], categoria_val, exploracion_rapida))

# Determinar límites
if not fo_pairs:
    print("No hay datos válidos para graficar.")
    sys.exit(1)

fo1_vals, fo2_vals = zip(*[(f1, f2) for f1, f2, _, _ in fo_pairs])
min_value = min(min(fo1_vals), min(fo2_vals))
max_value = max(max(fo1_vals), max(fo2_vals))

# Asignar colores por categoría y bordes por exploración rápida
unique_vals = sorted(set(p[2] for p in fo_pairs))
color_map = {val: plt.cm.tab10(i % 10) for i, val in enumerate(unique_vals)}


plt.figure(figsize=(8, 8))
legend_patches = {}
exploracion_patch_added = False

for f1, f2, categoria_val, exploracion_rapida in fo_pairs:
    fill_color = color_map[categoria_val]
    edge_color = 'black' if exploracion_rapida else 'none'

    jittered_f1 = f1 + np.random.normal(0, get_visual_jitter(f1))
    jittered_f2 = f2 + np.random.normal(0, get_visual_jitter(f2))

    if exploracion_rapida and not exploracion_patch_added:
        legend_patches["exploracion"] = mpatches.Patch(edgecolor='black', facecolor='white', label="Exploración Rápida", linewidth=2)
        exploracion_patch_added = True

    plt.scatter(jittered_f1, jittered_f2, facecolors=fill_color, edgecolors=edge_color, s=100, alpha=0.6)

    if categoria_val not in legend_patches:
        legend_patches[categoria_val] = mpatches.Patch(color=fill_color, label=f"{categoria.title()} {categoria_val}")

# Construir leyenda ordenada por claves numéricas
sorted_handles = []
for key in sorted(k for k in legend_patches if isinstance(k, (int, float))):
    sorted_handles.append(legend_patches[key])

# Añadir "exploracion" al final si existe
if "exploracion" in legend_patches:
    sorted_handles.append(legend_patches["exploracion"])

plt.legend(handles=sorted_handles)
plt.plot([min_value, max_value], [min_value, max_value], linestyle="--", color="red", label="y = x")
plt.xlim(min_value, max_value)
plt.ylim(min_value, max_value)
plt.xlabel(f'FO {heuristic1_name}')
plt.ylabel(f'FO {heuristic2_name}')
plt.title(f'Comparación de FO por {categoria.title()}')


output_suffix = f"by_{categoria}"
comparison_plot_path = os.path.join(output_dir, f"{heuristic1_name}_vs_{heuristic2_name}_{output_suffix}.png")
plt.savefig(comparison_plot_path, bbox_inches="tight", dpi=300)
plt.close()

print("Gráfica correctamente generada en:", comparison_plot_path)
