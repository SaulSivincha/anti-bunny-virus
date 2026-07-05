from pathlib import Path
import time


def main() -> None:
    directorio = Path("/tmp/anti_bunny_demo")
    directorio.mkdir(parents=True, exist_ok=True)
    destino = directorio / "archivo_crecimiento_demo.bin"
    bloque = b"0" * (2 * 1024 * 1024)

    with destino.open("wb") as archivo:
        for indice in range(8):
            archivo.write(bloque)
            archivo.flush()
            print(f"Escritura simulada: {(indice + 1) * 2} MB")
            time.sleep(0.2)

    print(f"Archivo de prueba generado en {destino}")


if __name__ == "__main__":
    main()
