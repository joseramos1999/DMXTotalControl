# Manual de DMX Total Control

DMX Total Control es un programa libre para controlar iluminación DMX desde un ordenador con Windows. Está pensado para el directo: lanzas escenas por grupos, programas rápido con el programador, operas cuelists con GO en los playbacks y, si quieres, montas tu propia pantalla táctil (el SHOW) y la usas también desde el móvil.

Con DMX Total Control puedes:

- Patchear los fixtures de tu montaje con una biblioteca de miles de definiciones, más la Open Fixture Library, la importación de GDTF y tus propios fixtures.
- Dar dimmer, color, posición, gobos y cualquier otro canal a los fixtures seleccionados, con paletas, FX y una línea de comandos.
- Grabar escenas estáticas, pasos, FX, super escenas con línea de tiempo y audio, y cuelists con tracking.
- Lanzarlas en Directo por grupos, en los Playbacks con GO o desde tu SHOW.
- Sacar DMX por interfaces DMX USB, Art-Net o sACN, y controlarlo todo desde MIDI, OSC, entrada DMX o el teclado.

Los shows se guardan en archivos `.qxw`, el mismo formato que usa QLC+, así que puedes abrir shows hechos con QLC+.

## Cómo abrir este manual

Pulsa **F12** o el botón **«?»** de la barra superior. El manual se abre en el lado derecho de la ventana, en la página que explica la página en la que estás: si estás en Programar, se abre en [Programar](programar.md). Los enlaces llevan a otras páginas del manual; desde el panel puedes volver a la página anterior o a este «Índice». También se abre desde Opciones › Ayuda › **«Abrir el manual»**.

## La guía y los shows de ejemplo

- **La guía de primeros pasos**: una tarjeta abajo a la derecha que te lleva por las páginas en cinco pasos (los fixtures, un look, una escena, Directo y los playbacks). Aparece la primera vez que abres el programa; después, en Opciones › Ayuda › **«Empezar la guía»**. Las páginas se pueden usar mientras está abierta. Consulta [Primeros pasos](primeros-pasos.md).
- **Los shows de ejemplo**: shows ya hechos para abrirlos y probar. Están en la flecha **▾** junto a «Abrir», en **«Ejemplos»**:
  - **Primeros pasos**: el show de prácticas de la guía, con 4 PAR, 2 cabezas móviles y el grupo «Mis escenas» vacío.
  - **Bolo pequeño**: PAR delante y detrás, estrobos y máquina de humo. Colores, efectos y generales en tres grupos, y una página de SHOW hecha con ellos.
  - **Concierto con cabezas móviles**: paletas de color y de posición, un círculo en espejo y una canción como cuelist en los playbacks.
  - **Sala con playbacks**: una cuelist por canción en la primera página de playbacks y un SHOW con esos playbacks.

Los ejemplos se abren como cualquier show: si guardas cambios, usa «Guardar como…» para dejar el ejemplo como estaba.

## La ventana del programa

La ventana tiene tres partes: la barra superior, la barra lateral con las páginas y la página que estás usando.

### Las páginas

| Página | Tecla | Para qué sirve |
|---|---|---|
| Directo | F1 | Lanzar las escenas por grupos y usar el mezclador |
| Show | F2 | Tu pantalla de show: botones, faders, diales, grupos, reloj… |
| Playbacks | F10 | Faders con GO, pausa y flash para cuelists y escenas |
| Programar | F3 | Seleccionar fixtures, darles valores, FX y paletas, y grabar |
| Escenas | F4 | Crear y editar escenas, efectos, super escenas y cuelists |
| Patch | F5 | Los fixtures del montaje, sus direcciones, grupos y plano 2D |
| Fixtures | F6 | Crear tus propias definiciones de fixtures |
| Salidas | F7 | Universos e interfaces DMX |
| Mapeos | F8 | Controladores MIDI, OSC, entrada DMX y teclado |
| Opciones | F9 | Ajustes del programa, biblioteca de fixtures y actualizaciones |

**F11** pone la ventana a pantalla completa o la devuelve a su tamaño. Si una página está bloqueada, en la barra lateral lleva un candado y al abrirla se pide la contraseña, si la hay (consulta [Opciones](opciones.md#show-y-bloqueo)).

### La barra superior

De izquierda a derecha:

- **El show**: su nombre. Un punto **●** delante avisa de que hay cambios sin guardar; si el autoguardado está activo, se ve también la hora del último («autoguardado…»).
- **Archivo**: «Nuevo show (Ctrl+N)», «Abrir (Ctrl+O)», la flecha **▾** con los shows recientes y los «Ejemplos», «Guardar (Ctrl+S)» y «Guardar como…» (Ctrl+Mayús+S).
- **Tempo**: un piloto que parpadea con cada pulso, el **BPM** (haz clic para escribirlo, de 20 a 300), **«Tap»** (pulsa al ritmo, también con la tecla T), **«BeatGO»** (siguiente división de las escenas que van por BeatGO, también con la tecla G) y **«Pulso»**, que escucha la entrada de audio del ordenador: cada pico que pasa la línea del medidor mueve las escenas que van por Pulso. Arrastra la línea del medidor para cambiar el umbral.
- **El candado**: bloquea pantallas para que nada cambie por error (consulta [Opciones](opciones.md#show-y-bloqueo)).
- **La diana**: el modo Mapear, para asignar controles MIDI, OSC o teclas (consulta [Mapeos](mapeos.md#modo-mapear)). Su piloto verde parpadea con cada mensaje que llega.
- **«N en marcha»**: cuántas escenas y efectos están sonando.
- **GM**: el gran master. Por debajo del 100 % se pone naranja.
- **«Blackout»** (Ctrl+B): saca todo a negro mientras está encendido.
- **«Parar todo»** (Esc): para todo lo que suena. El programador no se limpia.
- **«?»**: abre este manual (F12).

En una ventana estrecha o una pantalla pequeña, la barra se compacta: el nombre del show se recorta, «Guardar como…» queda como un icono, «N en marcha» se queda en el número y, en las más estrechas, «Blackout» y «Parar todo» quedan solo con su icono. Pasa el ratón por encima de un icono para ver qué es. Si todo se ve pequeño o grande, cambia el **Tamaño de la interfaz** en [Opciones](opciones.md#general).

Al cerrar el programa, o al abrir o crear otro show, si hay cambios sin guardar se pregunta «¿Guardar los cambios?» con «Cancelar», «No guardar» y «Guardar».

> **Consejo:** si el programa se cerró de forma inesperada y tenías un show sin guardar, al volver a abrirlo te ofrece «¿Recuperar el show sin guardar?». Pulsa «Recuperar» y guárdalo con un nombre.

## Cómo está organizado este manual

Empieza por **Primeros pasos** si es la primera vez. El resto de páginas explican cada página del programa y se pueden leer en cualquier orden.

1. [Primeros pasos](primeros-pasos.md): una primera sesión completa, del patch a guardar el show.
2. [Directo](directo.md): grupos, exclusiva o mezcla, crossfade, masters, liberaciones, flash y mezclador.
3. [Show](show.md): la pantalla táctil, sus secciones, botones, faders, diales, rueda de color, rejilla XY, edición y bloqueo.
4. [Playbacks](playbacks.md): páginas de playbacks, GO, Pausa/Atrás, Flash, niveles, grabar en un playback y controladores APC.
5. [Programar](programar.md): selección, grupos, plano 2D, dimmer, color, posición, features, faders, paletas, FX, abanico, ciego, resaltar y localizar, línea de comandos, grabar y actualizar.
6. [Escenas](escenas.md): todos los tipos de escena, super escenas, cuelists con tracking, carpetas, importar y exportar.
7. [Patch](patch.md): añadir fixtures, direcciones, universos, números, grupos de fixtures, plano 2D, límites, matrices LED e importar de otro show.
8. [Fixtures](fixtures.md): la biblioteca, el creador de fixtures, la Open Fixture Library, GDTF y tus fixtures.
9. [Salidas](salidas.md): universos, interfaces DMX USB, Art-Net y sACN, y el monitor de niveles DMX.
10. [Mapeos](mapeos.md): MIDI, OSC, entrada DMX y teclado, el modo Mapear y las plantillas para Akai APC.
11. [Opciones](opciones.md): idioma, tema, tamaño, arranque, bloqueo, autoguardado, prioridades, control remoto, biblioteca, actualizaciones.
12. [Control remoto](remoto.md): el SHOW en el móvil o la tablet.
13. [Atajos de teclado](atajos.md): todas las teclas y combinaciones.

## Palabras que verás a menudo

- **Fixture**: cada aparato de luz patcheado (un PAR, una cabeza móvil, una tira LED…).
- **Universo**: 512 canales DMX. Cada universo sale por una interfaz.
- **Programador**: donde das valores a los fixtures seleccionados. Lo que hay en el programador manda sobre todo hasta que lo limpias.
- **Escena**: lo que grabas para reproducirlo después. Hay varios tipos: estática, pasos, FX, super escena, cuelist… (consulta [Escenas](escenas.md)).
- **Grupo**: una columna de escenas en Directo. En un grupo exclusivo solo suena una escena a la vez.
- **Grupo de fixtures**: un conjunto de fixtures para seleccionarlos de golpe (se crean en [Patch](patch.md#grupos-de-fixtures)).
- **Cuelist**: una lista de cues que avanzan con GO. Cada cue guarda solo lo que cambia.
- **Playback**: un fader con sus botones (GO, Pausa/Atrás, Flash) que reproduce una cuelist o una escena.
- **Paleta**: un valor guardado (un color, una posición…) que las escenas siguen por referencia: si cambias la paleta, cambian las escenas que la usan.
- **Prioridad**: de 0 a 10. Si dos escenas controlan el mismo canal, gana la de mayor prioridad.
- **SHOW**: la pantalla de operación que montas tú.
