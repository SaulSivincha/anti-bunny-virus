import subprocess
import sys
import time


def main() -> None:
    procesos = []
    limite = 25
    duracion = 8

    for _ in range(limite):
        procesos.append(
            subprocess.Popen([sys.executable, "-c", "import time; time.sleep(8)"])
        )
        time.sleep(0.05)

    print(f"Simulador activo: {limite} procesos hijos durante {duracion}s")
    time.sleep(duracion)

    for proceso in procesos:
        proceso.terminate()


if __name__ == "__main__":
    main()
