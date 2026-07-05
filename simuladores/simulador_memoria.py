import time


def main() -> None:
    bloques = []
    bloque_mb = 32
    repeticiones = 10

    for indice in range(repeticiones):
        bloques.append(bytearray(bloque_mb * 1024 * 1024))
        print(f"Memoria simulada: {(indice + 1) * bloque_mb} MB")
        time.sleep(1)

    time.sleep(5)


if __name__ == "__main__":
    main()
