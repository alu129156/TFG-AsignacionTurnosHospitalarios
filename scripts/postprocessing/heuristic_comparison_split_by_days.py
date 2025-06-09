import json
import os
import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

def cargar_json(path):
    with open(path, "r") as f:
        return json.load(f)

def extraer_info(data, clave_color):
    enriched = []
    for entry in data:
        if "mensaje" not in entry and isinstance(entry.get("FO"), (int, float)):
            input_data = entry["input_data"]
            dias = input_data.get("dias", 0)
            if clave_color == "dias":
                categoria = dias
            elif clave_color == "demanda":
                demanda = input_data.get("demanda", {})
                categoria = sum(demanda.get(k, 0) for k in ["early", "day", "late"])
            else:
                raise ValueError("Categoría no reconocida")
            enriched.append({
                "FO": entry["FO"],
                "categoria": categoria,
                "dias": dias,
                "exploracion": entry.get("exploracion_rapida", False)
            })
    return enriched

def graficar_por_grupo(fo_pairs, nombre1, nombre2, output_dir, grupo_id, min_dia, max_dia, categoria_nombre):
    grupo = [p for p in fo_pairs if min_dia <= p[2] <= max_dia]
    if not grupo:
        return

    # Preparar colores
    colores = plt.cm.tab10
    categorias = sorted(set(p[3] for p in grupo))
    color_map = {c: colores(i % 10) for i, c in enumerate(categorias)}

    plt.figure(figsize=(8, 8))
    leyenda = {}
    min_fo = min(min(p[0], p[1]) for p in grupo)
    max_fo = max(max(p[0], p[1]) for p in grupo)
    jitter = 0.005 * (max_fo - min_fo) if max_fo > min_fo else 1

    for f1, f2, dias, categoria, exploracion in grupo:
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
    plt.legend(handles=list(leyenda.values()))

    os.makedirs(output_dir, exist_ok=True)
    filename = f"group{grupo_id}_{nombre2}_vs_{nombre1}_by_{categoria_nombre}.png"
    plt.savefig(os.path.join(output_dir, filename), dpi=300, bbox_inches="tight")
    plt.close()

def main():
    if len(sys.argv) != 5:
        print("Uso: python heuristic_comparison_split_by_group.py <image_folder> <heuristic1_json> <heuristic2_json> <days|demand>")
        sys.exit(1)

    output_dir = sys.argv[1]
    f_astar = sys.argv[2]
    f_tabu = sys.argv[3]
    categoria = sys.argv[4].lower()

    clave_color = "dias" if categoria == "days" else "demanda" if categoria == "demand" else None
    if not clave_color:
        print("La categoría debe ser 'days' o 'demand'")
        sys.exit(1)

    d1 = extraer_info(cargar_json(f_astar), clave_color)
    d2 = extraer_info(cargar_json(f_tabu), clave_color)

    # Combinar entradas (FO1, FO2, días, categoría, exploración)
    fo_pairs = []
    for e1, e2 in zip(d1, d2):
        if isinstance(e1["FO"], (int, float)) and isinstance(e2["FO"], (int, float)):
            fo_pairs.append((e1["FO"], e2["FO"], e1["dias"], e1["categoria"], e1["exploracion"] or e2["exploracion"]))

    # Grupos por días
    graficar_por_grupo(fo_pairs, "Astar_3", "tabuSearch", output_dir, 1, 2, 8, categoria)
    graficar_por_grupo(fo_pairs, "Astar_3", "tabuSearch", output_dir, 2, 16, 16, categoria)
    graficar_por_grupo(fo_pairs, "Astar_3", "tabuSearch", output_dir, 3, 28, 32, categoria)

if __name__ == "__main__":
    main()
