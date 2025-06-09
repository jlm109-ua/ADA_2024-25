import os
import re
import subprocess
import time

# Configuración
EJECUTABLE = "./maze_bb"
DIRECTORIO_LABERINTOS = "maze_BB_testfiles"
TIMEOUT = 15  # segundos
SALIDA = "resultados.txt"

# Laberintos a omitir (sin extensión)
OMITIR_LABERINTOS = [
    "13-bb",
    "100-bb",
    "200-bb",
    "300-bb",
    "500-bb",
    "700-bb",
    "900-bb",
    "k01-bb",
    "k02-bb",
    "k03-bb",
    "k05-bb",
    "k10-bb",
]

# Función para leer la solución esperada desde .sol_bb
def leer_solucion_esperada(nombre_base):
    print(f"Evaluando laberinto: {nombre_base}.maze")
    ruta_sol = os.path.join(DIRECTORIO_LABERINTOS, f"{nombre_base}.maze.sol_bb")
    if not os.path.exists(ruta_sol):
        print(f"[AVISO] Archivo .sol_bb no encontrado: {nombre_base}.maze.sol_bb")
        return None, None

    with open(ruta_sol, 'r') as f:
        # Filtra líneas vacías o de solo espacios
        lineas = [linea.strip() for linea in f if linea.strip() != ""]

    if len(lineas) < 3:
        print(f"[AVISO] Archivo .sol_bb mal formado: {nombre_base}.maze.sol_bb")
        return None, None

     # Buscar el primer número entero como coste
    coste = None
    for linea in lineas:
        if re.fullmatch(r"\d+", linea):
            coste = int(linea)
            break

    # Buscar el último <...> como camino
    camino = None
    for linea in reversed(lineas):
        if linea.startswith("<") and linea.endswith(">"):
            camino = linea[1:-1].strip()
            break


    if coste is None or camino is None:
        print(f"[AVISO] No se pudo extraer coste o camino de {nombre_base}.maze.sol_bb")

    return coste, camino


# Ejecuta el programa con timeout
def ejecutar_laberinto(filepath):
    try:
        start = time.time()
        result = subprocess.run(
            [EJECUTABLE, "-p", "-f", filepath, "--debug"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=TIMEOUT
        )
        end = time.time()
        return result.stdout, end - start
    except subprocess.TimeoutExpired:
        return None, None

# Recopilar resultados
resultados = []

for archivo in sorted(os.listdir(DIRECTORIO_LABERINTOS)):
    if not archivo.endswith(".maze"):
        continue

    nombre_base = archivo[:-5]  # sin .maze

    if nombre_base in OMITIR_LABERINTOS:
        print(f"[Omitido] {archivo}")
        continue

    ruta_laberinto = os.path.join(DIRECTORIO_LABERINTOS, archivo)

    esperado_coste, esperado_camino = leer_solucion_esperada(nombre_base)
    salida, duracion = ejecutar_laberinto(ruta_laberinto)

    if salida is None:
        resultados.append((archivo, "?", "?", "?"))
        continue

    # Parsear salida del programa (misma estructura que .sol_bb)
    lineas = salida.strip().splitlines()

    if len(lineas) < 3:
        resultados.append((archivo, "?", "?", f"{duracion:.3f}s"))
        continue

    try:
        coste_obtenido = int(lineas[0].strip())
    except ValueError:
        coste_obtenido = None

    camino_obtenido = None
    for linea in reversed(lineas):
        if linea.strip().startswith("<") and linea.strip().endswith(">"):
            camino_obtenido = linea.strip()[1:-1].strip()
            break

    # Comparación segura
    correcto_coste = (coste_obtenido == esperado_coste) if esperado_coste is not None else False
    print(f"Coste esperado: {esperado_coste}, Coste obtenido: {coste_obtenido}")
    correcto_camino = (camino_obtenido == esperado_camino) if esperado_camino is not None else False
    print(f"Camino esperado: {esperado_camino}, Camino obtenido: {camino_obtenido}\n")

    resultados.append((
        archivo,
        "✅" if correcto_coste else "❌",
        "✅" if correcto_camino else "❌",
        f"{duracion:.3f}s"
    ))

# Guardar resultados en TXT
with open(SALIDA, "w") as f:
    f.write(f"{'Laberinto':<30} {'Coste':<8} {'Camino':<8} {'Tiempo'}\n")
    f.write("=" * 60 + "\n")
    for nombre, coste_ok, camino_ok, tiempo in resultados:
        f.write(f"{nombre:<30} {coste_ok:<8} {camino_ok:<8} {tiempo}\n")

print(f"Evaluación completada. Resultados guardados en '{SALIDA}'.")

# Crear tabla en LaTeX con nombre del laberinto y tiempo
TEX_SALIDA = "resultados_tabla.tex"

with open(TEX_SALIDA, "w") as tex:
    tex.write("\\begin{tabular}{|l|r|}\n")
    tex.write("\\hline\n")
    tex.write("Laberinto & Tiempo (s) \\\\\n")
    tex.write("\\hline\n")

    for nombre, _, _, tiempo in resultados:
        tiempo_str = tiempo if tiempo != "?" else "--"
        nombre_clean = nombre.replace("_", "\\_")  # escapar _
        tex.write(f"{nombre_clean} & {tiempo_str.replace('s','')} \\\\\n")

    tex.write("\\hline\n")
    tex.write("\\end{tabular}\n")

print(f"Tabla LaTeX generada en '{TEX_SALIDA}'.")
