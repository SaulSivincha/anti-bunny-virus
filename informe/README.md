# Informe en LaTeX

Esta carpeta contiene un informe técnico sencillo del proyecto. Se evita un
marco teórico independiente y se priorizan cinco preguntas: cuál es el
problema, qué se construyó, cómo se probó, qué ocurrió y qué se concluye. Los
recuadros grises indican qué contenido y evidencia deben incorporarse; deben
eliminarse cuando la redacción definitiva esté completa.

El capítulo del sistema es el núcleo del documento y debería ocupar cerca de
la mitad del cuerpo principal. La metodología y los resultados verifican el
funcionamiento, pero no reemplazan su explicación.

## Compilación

Se requiere una distribución LaTeX con `latexmk` y BibTeX:

```bash
make -C informe
```

Para eliminar artefactos auxiliares:

```bash
make -C informe clean
```

## Criterios de redacción

- No inventar datos ni completar tablas con estimaciones.
- Mantener el procedimiento separado de los resultados, aunque los resultados
  y su interpretación aparezcan en el mismo capítulo.
- Citar únicamente fuentes verificadas.
- Identificar el commit, la configuración y el entorno de cada experimento.
- Conservar también corridas fallidas y resultados negativos.
- Evitar definiciones generales y detalles de código que no ayuden a entender
  la solución o interpretar las pruebas.
- Explicar los algoritmos con pseudocódigo. Incluir como máximo dos o tres
  fragmentos C breves cuando evidencien una decisión propia de la
  implementación; el código completo permanece en el repositorio.
- Mantener cada cita junto a la afirmación que respalda y evitar comandos
  `\\cite{...}` con varias fuentes agrupadas.
- Sustituir cada recuadro de orientación por texto académico antes de entregar.
