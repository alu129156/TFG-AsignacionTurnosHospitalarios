import json
import os
import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

def cargar_json(path):
    with open(path, "r") as f:
        return json.load(f)

def get_input_key(entry):
    key_items = []
    for k, v in entry["input_data"].items():
        if isinstance(v, dict):
            key_items.append((k, tuple(sorted(v.items()))))
        else:
            key_items.append((k, v))
    return tuple(sorted(key_items))

def get_visual_jitter(fo):
    if fo < 20: return 2.5
    if fo < 50: return 2
    elif fo < 100: return 1.5
    elif fo < 300: return 1
    elif fo < 1000: return 0.5
    else: return 0.25

def plot_grupo(ax, grupo, color_map, nombre1, nombre2, dias_titulo):
    leyenda = {}
    min_fo = min(min(p[0], p[1]) for p in grupo)
    max_fo = max(max(p[0], p[1]) for p in grupo)

    for f1, f2, dias, categoria, exploracion in grupo:
        jitter = get_visual_jitter((f1 + f2) / 2)
        x = f1 + np.random.normal(0, jitter)
        y = f2 + np.random.normal(0, jitter)
        color = color_map.get(categoria, 'gray')
        borde = 'black' if exploracion else 'none'
        ax.scatter(x, y, s=100, alpha=0.6, facecolors=color, edgecolors=borde)

        if categoria not in leyenda:
            leyenda[categoria] = mpatches.Patch(color=color, label=f"Días {categoria}")
        if exploracion and "exploracion" not in leyenda:
            leyenda["exploracion"] = mpatches.Patch(facecolor='white', edgecolor='black', label='Exploración Rápida', linewidth=1)

    ax.plot([min_fo, max_fo], [min_fo, max_fo], '--', color='red')
    ax.set_xlim(min_fo, max_fo)
    ax.set_ylim(min_fo, max_fo)
    ax.set_xlabel(f'FO {nombre1}')
    ax.set_ylabel(f'FO {nombre2}')
    ax.set_title(f'Días: {dias_titulo}')

    legend_sorted = [leyenda[k] for k in sorted(k for k in leyenda if isinstance(k, int))]
    if "exploracion" in leyenda:
        legend_sorted.append(leyenda["exploracion"])
    ax.legend(handles=legend_sorted, fontsize=8)

def main():
    if len(sys.argv) != 4:
        print("Uso: python script.py <image_folder> <heuristic1_json> <heuristic2_json>")
        sys.exit(1)

    output_dir = sys.argv[1]
    f1_path = sys.argv[2]
    f2_path = sys.argv[3]

    d1_raw = cargar_json(f1_path)
    d2_raw = cargar_json(f2_path)

    indexed1 = {
        get_input_key(e): e for e in d1_raw
        if "mensaje" not in e and isinstance(e.get("FO"), (int, float))
    }
    indexed2 = {
        get_input_key(e): e for e in d2_raw
        if "mensaje" not in e and isinstance(e.get("FO"), (int, float))
    }

    common_keys = set(indexed1.keys()) & set(indexed2.keys())

    fo_pairs = []
    for key in common_keys:
        e1 = indexed1[key]
        e2 = indexed2[key]
        input_data = e1["input_data"]
        dias = input_data.get("dias", 0)
        categoria_val = dias
        exploracion = e1.get("exploracion_rapida", False) or e2.get("exploracion_rapida", False)
        fo_pairs.append((e1["FO"], e2["FO"], dias, categoria_val, exploracion))

    # Subgrupos
    subgrupos = {
        "Todos": fo_pairs,
        "2, 4, 8": [p for p in fo_pairs if p[2] in [2, 4, 8]],
        "16": [p for p in fo_pairs if p[2] == 16],
        "28": [p for p in fo_pairs if p[2] >= 28]
    }

    # Colores por días
    colores = plt.cm.tab10
    dias_unicos = sorted(set(p[2] for p in fo_pairs))
    color_map = {d: colores(i % 10) for i, d in enumerate(dias_unicos)}

    fig, axs = plt.subplots(2, 2, figsize=(12, 10))
    axs = axs.flatten()

    for ax, (nombre, grupo) in zip(axs, subgrupos.items()):
        if grupo:
            plot_grupo(ax, grupo, color_map, "Astar_3", "tabuSearch", nombre)
        else:
            ax.set_visible(False)

    plt.tight_layout()
    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, "Astar_3_vs_tabuSearch_by_days_subplots.png")
    plt.savefig(output_path, dpi=300)
    plt.close()
    print("Imagen guardada en:", output_path)

if __name__ == "__main__":
    main()
