import json
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import os
import numpy as np
import sys

# Verificación de argumentos
if len(sys.argv) != 3:
    print("Uso: python astar_solution_scatter.py <output_dir> <heuristic_json>")
    sys.exit(1)

output_dir = sys.argv[1]
heuristic_file = sys.argv[2]

# Leer y validar contenido
with open(heuristic_file, "r") as f:
    data = json.load(f)

if not isinstance(data, list) or not all("input_data" in d for d in data if isinstance(d, dict)):
    print("El archivo no contiene datos de heurísticas válidos.")
    sys.exit(1)

dias_con_fo = []
dias_sin_fo = []

for entry in data:
    if not isinstance(entry, dict) or "input_data" not in entry:
        continue
    dias = entry["input_data"]["dias"]
    jitter_x = np.random.normal(0, 0.5)
    jitter_y = np.random.normal(0, 0.01)
    if "FO" in entry:
        dias_con_fo.append((dias + jitter_x, 5.0 + jitter_y))
    elif "mensaje" in entry:
        dias_sin_fo.append((dias + jitter_x, 6.0 + jitter_y))

# Separar coordenadas
x_con_fo, y_con_fo = zip(*dias_con_fo) if dias_con_fo else ([], [])
x_sin_fo, y_sin_fo = zip(*dias_sin_fo) if dias_sin_fo else ([], [])

# Eje X ajustado al rango real de días
all_dias = [entry["input_data"]["dias"] for entry in data if "input_data" in entry]
min_dias = min(all_dias)
max_dias = max(all_dias)

# Crear figura
plt.figure(figsize=(10, 3))
plt.axhline(y=5.0, color='black', linestyle='--', linewidth=1.5)
plt.axhline(y=6.0, color='black', linestyle='--', linewidth=1.5)

plt.scatter(x_con_fo, y_con_fo, color='blue', alpha=0.6, s=100, marker='o')
plt.scatter(x_sin_fo, y_sin_fo, color='red', alpha=0.6, s=100, marker='o')

plt.yticks([5.0, 6.0], ['SI', 'NO'])
plt.gca().xaxis.set_major_locator(ticker.MaxNLocator(nbins=15, integer=True))
plt.xlim(min_dias - 1, max_dias + 1)
plt.xlabel('DIAS')
plt.title('¿Tiene solución A*?')
plt.grid(True, axis='x', linestyle='--', alpha=0.5)
plt.legend([], [], frameon=False)

# Texto inferior
plt.subplots_adjust(bottom=0.35)
plt.text(min_dias, 4.6, f'Soluciones encontradas en A* = {len(dias_con_fo)}', fontsize=10, fontweight='bold')
plt.text(min_dias, 4.3, f'Soluciones no encontradas en A* = {len(dias_sin_fo)}', fontsize=10, fontweight='bold')

# Guardar imagen
basename = os.path.splitext(os.path.basename(heuristic_file))[0]
if basename.startswith("heuristic_"):
    basename = basename[len("heuristic_"):]
if basename.endswith("_limit_cases_benchmark"):
    basename = basename[:-len("_limit_cases_benchmark")]

os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, f"{basename}_sol-nosol.png")
plt.savefig(output_path, bbox_inches="tight", dpi=300)
plt.close()

print("Gráfica guardada en:", output_path)