# Opciones

![La página Opciones](img/opciones.png)

**Opciones** (F9) reúne los ajustes del programa. A la izquierda están los ajustes generales, el arranque, el bloqueo, el autoguardado, la reproducción y el audio; a la derecha, el control remoto, la biblioteca de fixtures, las actualizaciones, la ayuda y «Acerca de».

Los ajustes de esta página son del ordenador, no del show: valen para todos los shows que abras.

## General

- **Idioma**: «Idioma del sistema», Español, English, Português, Français, Italiano o Deutsch. Se aplica la próxima vez que arranque el programa.
- **Tema**: **Oscuro**, **Claro**, **Alto contraste** o **Noche (atenuado)**. Cambia al momento. Claro, para salas con luz; Alto contraste, para leer de lejos; Noche, muy oscuro y en rojo, para una cabina a oscuras sin deslumbrarte. El [control remoto](remoto.md) sigue el mismo tema.
- **Tamaño de la interfaz**: de 80 % a 150 %, más pequeño para portátiles pequeños y más grande para pantallas táctiles. Se aplica la próxima vez que arranque el programa.
- **Abrir el último show al arrancar**.

> **Consejo:** el tema Claro va mejor en ensayos de día o al aire libre; Alto contraste, cuando hay mucha luz alrededor de la pantalla.

## SHOW y bloqueo

- **Abrir el SHOW a pantalla completa al arrancar**: para instalaciones y quioscos. El programa arranca en la página Show a pantalla completa.
- **…y bloqueado**: además arranca bloqueado: solo el SHOW, sin navegación. Para salir, mantén pulsado el botón del candado (con la contraseña, si la hay). Consulta [Show](show.md#pantalla-completa-y-bloqueo).
- **Bloquear pantallas**: **«Bloquear…»** / **«Desbloquear…»**. Es lo mismo que el candado de la barra superior.

### Bloquear pantallas

El bloqueo evita que algo cambie por error durante el show. Directo y el SHOW siguen funcionando, y los playbacks siguen sonando.

1. Pulsa el **candado** de la barra superior (o «Bloquear…» aquí).
2. Elige qué bloquear:
   - **Setup**: Patch, Fixtures, Salidas, Mapeos y Opciones. Tampoco se puede abrir ni crear otro show.
   - **Programación**: Programar y Escenas. En Playbacks no se puede grabar, actualizar ni asignar.
   - **Edición del SHOW**: las páginas del SHOW se usan, no se cambian.
3. Si quieres, escribe una **contraseña** (es opcional).
4. Pulsa **«Bloquear»**.

Las páginas bloqueadas llevan un candado en la barra lateral. Al abrir una, se pide la contraseña (si la hay) y se desbloquea todo. Para desbloquear, pulsa el candado de la barra superior. **«Quitar la contraseña»** está en la misma ventana, al bloquear.

## Autoguardado

- **Guardar automáticamente**, **Cada (minutos)**: de 1 a 60.

Los shows con nombre se sobrescriben y la versión anterior se guarda como `.bak`. Los shows sin título se guardan aparte y se ofrecen al reabrir el programa tras un cierre inesperado («¿Recuperar el show sin guardar?»). En la barra superior se ve la hora del último autoguardado; si falla (un disco lleno, un archivo de solo lectura), aparece un aviso en rojo.

## Reproducción

- **Prioridad por defecto de las escenas nuevas**: de 0 a 10. Si dos escenas controlan el mismo canal, gana la de mayor prioridad (consulta [Prioridades](directo.md#prioridades)).
- **Prioridad de la edición en vivo**: lo que pongas en Programar gana a las escenas de esta prioridad o menor. Con 10, a todas las escenas.
- **Al lanzar una escena, las ediciones en vivo**:
  - **«Le devuelven sus canales a la escena»**: los canales que toca la escena dejan de estar en el programador.
  - **«Se mantienen siempre»**.
  - **«Se restablecen todas»**: el programador se vacía.
- **El grand master actúa sobre**: **«Canales de intensidad»** o **«Todos los canales»**.

## Audio

- **Entrada de audio**: el micrófono o la entrada de línea que escuchan **Pulso** y las matrices de audio. **«Predeterminada del sistema»** usa la que tenga el sistema operativo. Pulso cambia de entrada al momento; una matriz de audio que ya esté sonando, la próxima vez que arranque. Una entrada elegida que ahora no está conectada sale como «(no conectada)», y mientras tanto se usa la predeterminada.

## Shows recientes

La lista de los últimos shows abiertos (también en la flecha **▾** de la barra superior). **«Vaciar lista»** la borra.

## Control remoto

Para usar las páginas del SHOW desde un móvil o una tablet en la misma red wifi, sin instalar nada:

- **Control remoto activado**.
- **Puerto**: cámbialo si otro programa lo usa (por defecto, 8800). Si está ocupado, se avisa en rojo.
- **PIN**: se pide una vez en cada dispositivo. Vacío, cualquiera en la red puede usarlo.
- Las **direcciones** que hay que abrir en el navegador del móvil, con un botón para copiarlas.
- Cuántos dispositivos están conectados.

Consulta [Control remoto](remoto.md).

## Biblioteca de fixtures

Indica cuántas definiciones de fixtures hay disponibles (la biblioteca incluida, los genéricos y los tuyos) y permite ampliarla:

- **Open Fixture Library**: **«Descargar»** (necesita internet). Ya descargada, se ve cuántos fixtures tiene y cuándo se descargó, y el botón pasa a ser **«Actualizar»**: trae los fixtures nuevos y los mejorados, y conserva tus fixtures y los que la biblioteca ya no tenga. Si hay novedades, se dice y el botón se resalta (consulta [Fixtures](fixtures.md#open-fixture-library)).
- **Importar archivos de fixtures**: **«Importar…»** definiciones `.qxf` (el formato de QLC+), archivos `.gdtf` o un `.zip` con ellos.
- **Fixtures GDTF**: **«GDTF-Share»** abre su web; **«Importar GDTF…»** importa archivos, carpetas o el `.zip` de su descarga múltiple. Indica cuántos fixtures GDTF hay en la biblioteca.
- **Importar mis fixtures de QLC+**: **«Importar»** copia los fixtures que creaste en QLC+.
- **Carpeta de fixtures de usuario**: **«Abrir carpeta»**.

Todo esto se explica en [Fixtures](fixtures.md).

## Actualizaciones

- **Buscar actualizaciones al arrancar**: una vez al día, en GitHub: el programa y, si descargaste la Open Fixture Library, si tiene fixtures nuevos (un aviso lo dice). No se descarga ni se instala nada sin preguntarte. Sin internet, no molesta.
- **Incluir versiones de prueba**: las novedades antes, menos probadas.

Debajo se ve el estado («Tienes la última versión», «Ya está disponible la versión…») y los botones:

- **«Buscar actualizaciones»**: comprueba ahora (también la Open Fixture Library).
- **«Novedades»**: lo nuevo de esa versión.
- **«Descargar e instalar»**: descarga el instalador y comprueba su huella SHA-256 antes de guardarlo. Si una versión no publica su huella, solo se puede descargar desde su página («Ver en GitHub»).
- **«Instalar ahora»**: el programa se cierra (antes pregunta si guardar el show), se instala la versión nueva y se vuelve a abrir.
- **«Al cerrar»** / **«Al cerrar no»**: instalarla cuando cierres el programa.
- **«Ver en GitHub»**: la página de la versión.

Cuando hay una versión nueva, aparece también un aviso encima de las páginas con los mismos botones. **«Más tarde»** quita el aviso para esa versión; sigue disponible aquí.

## Ayuda

- **Manual**: **«Abrir el manual»** abre este manual en su índice. F12 o el botón **«?»** de la barra superior lo abren en la página en la que estás. Sale en el idioma del programa (español, inglés, portugués, francés, italiano o alemán); las capturas son las de la versión en español.
- **Guía de primeros pasos**: **«Empezar la guía»** vuelve a abrir la guía (consulta [Primeros pasos](primeros-pasos.md#la-guía)).
- **Shows de ejemplo**: están en la flecha **▾** junto a «Abrir» de la barra superior, en «Ejemplos» (consulta [el índice](index.md#la-guía-y-los-shows-de-ejemplo)).

## Acerca de

La versión del programa, su licencia (software libre, Apache 2.0) y los créditos.
