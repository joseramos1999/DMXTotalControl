# Mapeos

![La página Mapeos](img/mapeos.png)

**Mapeos** (F8) sirve para controlar el show desde fuera: controladores **MIDI**, **OSC** desde otra aplicación, una mesa por **entrada DMX** o el **teclado**. Un mapeo une un control de tu dispositivo (un pad, un fader, una tecla…) con algo del programa (una escena, un master, un GO…). Los mapeos se guardan con el show.

La página tiene tres pestañas: **«Mapeos»**, **«Dispositivos»** y **«Monitor»**, y el botón **«Aprender»**.

## Dispositivos

En **«Dispositivos»** aparecen los dispositivos de entrada que el programa encuentra, con su tipo (MIDI, OSC o DMX), su nombre y el universo que usan.

1. Conecta el controlador y, si no aparece, pulsa **«Buscar de nuevo»**.
2. Activa su **«Entrada»**.
3. Si el dispositivo tiene salida, activa **«Feedback (LEDs)»** para que el programa encienda sus LEDs o mueva sus faders motorizados.

Un mapeo de un dispositivo que no está conectado se conserva: vuelve a funcionar cuando lo conectas.

## Modo Mapear

La forma más rápida de crear mapeos es el **modo Mapear** (aprender):

1. Activa la entrada de tu dispositivo en «Dispositivos».
2. Pulsa la **diana** de la barra superior (o **«Aprender»** en esta página). Los controles que se pueden mapear se marcan en **morado**; un número en la esquina dice cuántos mapeos tienen ya.
3. Haz clic en un control del programa: una escena de Directo, un master, un botón del mezclador, un playback…
4. La barra morada dice «Mueve un control de tu dispositivo o pulsa una tecla para: …». Mueve el fader, pulsa el pad o pulsa una tecla.
5. El programa avisa «Mapeado: …». Sigue con el siguiente control.
6. Pulsa **«Hecho»** (o Esc) para salir.

En el modo Mapear, **Mayús+clic** sigue usando el control en lugar de mapearlo. **Esc** cancela el control pendiente; otro Esc sale del modo. El botón «Mapeos» de la barra morada te trae a esta página.

Un control del programa puede tener varios mapeos (por ejemplo, un pad de un controlador y una tecla).

> **Consejo:** al aprender un botón de una Akai APC, el mapeo ya trae los LEDs en verde mientras suena.

### Qué se puede mapear

- **Barra superior**: GO, Stop, BeatGO, gran master, Blackout y Parar todo.
- **Directo**: cada escena, ◀ ▶, Pausa y Liberar grupos de la cabecera, y en cada grupo su liberar, anterior, siguiente y master.
- **Mezclador**: en cada tira, el dimmer, el tono, la saturación, el estrobo y su velocidad, y los botones Full, Blackout, Flash, Flash excl. y Full excl.
- **Programar**: las paletas.
- **Playbacks**: los nombres de página, ◀ ▶ de página, «Liberar todo», «Atrás», el GO grande, y en cada playback su ×, su nombre (seleccionar), sus tres botones y su fader.
- **SHOW**: los botones y los faders (el fader entero, con todos sus elementos), y ◀ ▶ de las secciones de playbacks.

## La lista de mapeos

En **«Mapeos»**, cada fila muestra el control del dispositivo, lo que controla y su comportamiento. Un punto morado indica un botón y uno azul un fader. Haz clic en un mapeo para editarlo a la derecha, o pulsa **«Añadir»** para crear uno vacío.

## Editar un mapeo

### Control

- **El tipo**: **MIDI**, **OSC**, **«Entrada DMX»** o **«Teclado»**.
- **Dispositivo**: uno concreto o **«Cualquier dispositivo»**.
- **MIDI**: **CC** o **Nota**, su número (0–127) y el **Canal** MIDI (1–16).
- **OSC**: la ruta, por ejemplo `/scene/1`.
- **Entrada DMX**: el **Canal** (1–512).
- **Teclado**: la tecla, por ejemplo `Q` o `F9`.
- **«Aprender el control»**: pulsa y mueve el control del dispositivo para rellenar todo lo anterior.

### Controla

Lo que hace el mapeo. Los **botones**:

| Destino | Qué hace |
|---|---|
| Lanzar una escena | La escena elegida, en su grupo. |
| Escena N de un grupo | La escena que está en ese puesto del grupo (**Número de escena**). |
| Liberar un grupo / Escena siguiente de un grupo / Escena anterior de un grupo | En un grupo o en «Todos los grupos». |
| Blackout, Parar todo, Pausar todo, Tap tempo, BeatGO, GO, Stop | Como sus botones (GO y Stop se encienden y se apagan, como en la barra superior). |
| Paleta | La paleta en sus propias fixtures o en un grupo de fixtures. |
| Mezclador: full, blackout, flash, flash exclusivo, full exclusivo, estrobo | En la tira elegida del mezclador. |
| Playback: GO, atrás, pausa / atrás, flash, liberar, seleccionar | En un playback. |
| Playback: ir a una cue | Salta a la **Cue** indicada (1 es la primera de la lista). |
| GO del playback seleccionado / Atrás del playback seleccionado | Como el GO grande y «Atrás» de Playbacks. |
| Página de playbacks N / siguiente / anterior | Cambia la página de playbacks. |
| Liberar todos los playbacks | Como «Liberar todo». |

Los **faders**:

| Destino | Qué mueve |
|---|---|
| Gran master | El GM. |
| Master de un grupo | El master de ese grupo. |
| Dimmer en vivo de una escena / Velocidad en vivo de una escena | Sus deslizadores en vivo. |
| BPM | El tempo. |
| Mezclador: dimmer, tono, saturación, velocidad del estrobo | En la tira elegida. |
| Fader del SHOW | Un fader del SHOW entero. |
| Playback: fader | El fader de un playback. |

Para los playbacks se elige la **página** («Página actual (sigue)», que sigue a la página que se ve, o una fija) y el número de **Playback**. Con **«O una cuelist o una escena, esté donde esté»** el mapeo actúa sobre esa cuelist o escena en cualquier playback donde esté. En «ir a una cue» se ve a qué cue lleva ahora («Ahora: cue 3 de Canción 1»).

### Comportamiento

En los **botones**:

- **«Alternar: on / off»**: cada pulsación enciende o apaga.
- **«Flash: mientras se pulsa»**.
- **«Solo encender»** / **«Solo apagar»**.

Los botones de playback actúan al pulsar, salvo el flash, y no tienen esta opción.

En los **faders**:

- **«Absoluto: sigue al fader»**.
- **«Incremental: un paso por pulsación»** (para botones que suben o bajan).
- **«Relativo: encoder sin fin»** y **«Relativo: encoder, 1 sube y 127 baja»** (encoders sin fin).
- **«De»** … **«a»**: el rango, de 0 a 100 %. **«Invertir»** le da la vuelta.
- **«Paso %»** y **«En bucle»** (incremental y relativo).
- **«Recoger el fader (sin motor)»** (absoluto): el fader del controlador solo mueve el nivel después de alcanzarlo, sin saltos.

### Feedback

Con MIDI u OSC, **«Feedback»** hace que el programa **encienda el botón del dispositivo** o **mueva su fader motorizado**. Solo se envía lo que cambia.

Para los LEDs de los botones MIDI:

1. En **«LEDs de»**, elige **«Cualquier controlador (números)»** o tu controlador: **Akai APC mini**, **Akai APC mini mk2** o **Akai APC40 mkII**.
2. Para **«Encendido»**, **«Apagado»** y **«En pausa»**, elige el color en las muestras (en una APC) o escribe el valor (0–127) para cualquier otro controlador.
3. En los pads RGB de la APC mini mk2, elige también cómo se encienden: **«Tenue»**, **«Pleno»**, **«Pulsando»** o **«Parpadeo»** (en la APC40 mkII, **«Fijo»**).

Los LEDs del controlador se apagan al cerrar el programa, al borrar un mapeo o moverlo a otro control, al abrir otro show o al desactivar el dispositivo o su feedback.

### Duplicar y eliminar

**«Duplicar»** crea una copia del mapeo (útil para otro dispositivo o canal) y **«Eliminar»** lo borra.

## Teclado

Los mapeos de teclado funcionan en cualquier página y tienen preferencia sobre los atajos del programa que usen la misma tecla. No se disparan mientras escribes en un campo de texto (en Programar, mientras la línea de comandos tiene el teclado) ni con Ctrl o Alt pulsados. Los botones del SHOW tienen además su propia tecla, que funciona solo en la página Show (consulta [Show](show.md#opciones-de-un-botón)).

## Plantillas para Akai APC

**«Mapear todo»** asigna de una vez un controlador entero. Está en cada dispositivo MIDI de la pestaña «Dispositivos»:

1. Pulsa **«Mapear todo»** en tu controlador.
2. Elige la plantilla:
   - **«Akai APC mini → Grupos»** / **«Akai APC mini mk2 → Grupos»**
   - **«Akai APC mini → Playbacks»** / **«Akai APC mini mk2 → Playbacks»**
   - **«Akai APC40 mkII → Playbacks»**
3. Lee qué hace cada control y pulsa **«Mapear»**. Los mapeos que tuviera ese dispositivo se sustituyen; si su entrada estaba apagada, se activa.

**→ Grupos** (APC mini y mk2): cada columna de pads lanza las escenas de un grupo (verde: sonando; amarillo: lista; apagado: no hay escena). Los faders son los masters de los grupos y el último el gran master. Los botones de debajo de los pads liberan los grupos; los de la derecha son blackout, parar todo, pausa, tap, BeatGO, anterior, siguiente y flash.

**→ Playbacks** (APC mini y mk2): cada columna es un playback 1-8 de la página de playbacks actual: su fader (recoge el nivel) y, desde abajo, GO, Pausa/Atrás, Flash y sus 5 primeras cues (la cue 1 arriba). El botón de debajo de los pads lo libera. El último fader es el gran master; los botones de la derecha son página anterior y siguiente, GO y Atrás del playback seleccionado, blackout, tap, BeatGO y parar todo.

**→ Playbacks** (APC40 mkII): cada pista es un playback 1-8 de la página actual: su fader, GO en el botón de pista, Pausa/Atrás en S, Flash en el armado de grabación, seleccionar y liberar, y sus 5 primeras cues en los clips. Los botones de escena van a las páginas 1-5, los de banco cambian la página y el grupo, y Play y Record son GO y Atrás del playback seleccionado. Los mandos de device son los faders de los playbacks 9-16 y los mandos de pista, los masters de los primeros grupos.

> **Consejo:** después de «Mapear todo», cada control aparece como un mapeo normal en la lista: puedes cambiar cualquiera, o borrarlo, sin tocar los demás.

## Monitor

**«Monitor»** muestra los **mensajes de entrada y salida**: la hora, **IN** u **OUT**, el dispositivo (o «Teclado»), el mensaje y su valor. Mueve un control de un dispositivo activado para ver qué envía. **«Limpiar»** vacía la lista.

El piloto verde de la diana, en la barra superior, parpadea con cada mensaje que llega.

## Alt+clic

Fuera del modo Mapear, **Alt+clic** en un control que se puede mapear lo lleva al SHOW: consulta [Show](show.md#llevar-cualquier-control-al-show).
