# Contribuir

¡Gracias por querer mejorar DMX Total Control!

- Para un cambio grande, abre antes una [idea](https://github.com/joseramos1999/DMXTotalControl/issues/new/choose) y lo hablamos.
- El código nuevo va en `dtc/`. Los cambios en `engine/` y `plugins/` se limitan a lo imprescindible, para que los merges con el proyecto original del motor sigan siendo sencillos.
- Compila sin avisos (`./build-windows.sh`, ver [Compilar en Windows](README.md#compilar-en-windows)) y pasa las pruebas (`./.github/run-tests.sh`). Cada cambio lleva su prueba en `dtc/test`.
- Los textos nuevos de la interfaz van en inglés con `qsTr()` y con su traducción al español en `dtc/i18n/dtc_es.ts`.
- Si cambia cómo se usa algo, pon al día el manual (`docs/manual`, en sus seis idiomas).
- El pull request sigue su plantilla: qué cambia, por qué y cómo se ha probado.

Al contribuir, aceptas que tu código se publique con la licencia del proyecto, Apache 2.0 ([COPYING](COPYING)).
