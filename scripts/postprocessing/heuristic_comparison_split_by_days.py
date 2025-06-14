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

def graficar_por_grupo(fo_pairs, nombre1, nombre2, output_dir, grupo_id, min_dia, max_dia, categoria_nombre):
    grupo = [p for p in fo_pairs if min_dia <= p[2] <= max_dia]
    if not grupo:
        return

    colores = plt.cm.tab10
    categorias = sorted(set(p[3] for p in grupo))
    color_map = {c: colores(i % 10) for i, c in enumerate(categorias)}

    plt.figure(figsize=(8, 8))
    leyenda = {}
    min_fo = min(min(p[0], p[1]) for p in grupo)
    max_fo = max(max(p[0], p[1]) for p in grupo)

    for f1, f2, dias, categoria, exploracion in grupo:
        jitter = get_visual_jitter((f1 + f2) / 2)
        x = f1 + np.random.normal(0, jitter)
        y = f2 + np.random.normal(0, jitter)
        color = color_map[categoria]
        borde = 'black' if exploracion else 'none'
        plt.scatter(x, y, s=100, alpha=0.6, facecolors=color, edgecolors=borde)

        if categoria not in leyenda:
            leyenda[categoria] = mpatches.Patch(color=color, label=f"{categoria_nombre.title()} {categoria}")
        if exploracion and "exploracion" not in leyenda:
            leyenda["exploracion"] = mpatches.Patch(facecolor='white', edgecolor='black', label='Exploración Rápida', linewidth=1)

    plt.plot([min_fo, max_fo], [min_fo, max_fo], '--', color='red')
    plt.xlim(min_fo, max_fo)
    plt.ylim(min_fo, max_fo)
    plt.xlabel(f'FO {nombre1}')
    plt.ylabel(f'FO {nombre2}')
    plt.title(f'Grupo {grupo_id}: Días {min_dia}–{max_dia}')

    # Ordenar leyenda
    legend_sorted = [leyenda[k] for k in sorted(k for k in leyenda if isinstance(k, (int, float)))]
    if "exploracion" in leyenda:
        legend_sorted.append(leyenda["exploracion"])
    plt.legend(handles=legend_sorted)

    os.makedirs(output_dir, exist_ok=True)
    filename = f"group{grupo_id}_{nombre2}_vs_{nombre1}_by_{categoria_nombre}.png"
    plt.savefig(os.path.join(output_dir, filename), dpi=300, bbox_inches="tight")
    plt.close()

def main():
    if len(sys.argv) != 5:
        print("Uso: python heuristic_comparison_split_by_days.py <image_folder> <heuristic1_json> <heuristic2_json> <days|demand>")
        sys.exit(1)

    output_dir = sys.argv[1]
    f1_path = sys.argv[2]
    f2_path = sys.argv[3]
    categoria = sys.argv[4].lower()
    clave_color = "dias" if categoria == "days" else "demanda" if categoria == "demand" else None
    if not clave_color:
        print("Categoría inválida. Usa 'days' o 'demand'.")
        sys.exit(1)

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
        if clave_color == "dias":
            categoria_val = dias
        elif clave_color == "demanda":
            demanda = input_data.get("demanda")
            if isinstance(demanda, dict):
                categoria_val = sum(demanda.get(k, 0) for k in ["early", "day", "late"])
            elif isinstance(demanda, int):
                categoria_val = demanda * 3
            else:
                categoria_val = -1  # fallback
        exploracion = e1.get("exploracion_rapida", False) or e2.get("exploracion_rapida", False)
        fo_pairs.append((e1["FO"], e2["FO"], dias, categoria_val, exploracion))

    # Grupos por días
    graficar_por_grupo(fo_pairs, "Astar_3", "tabuSearch", output_dir, 1, 2, 8, categoria)
    graficar_por_grupo(fo_pairs, "Astar_3", "tabuSearch", output_dir, 2, 16, 16, categoria)
    graficar_por_grupo(fo_pairs, "Astar_3", "tabuSearch", output_dir, 3, 28, 32, categoria)

if __name__ == "__main__":
    main()