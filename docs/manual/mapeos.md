# Mapeos

![La página Mapeos](img/mapeos.png)

**Mapeos** (F8) sirve para controlar el show desde fuera: controladores **MIDI**, **OSC** desde otra aplicación, una mesa por **entrada DMX** o el **teclado**. Un mapeo une un control de tu dispositivo (un pad, un fader, una tecla…) con algo del programa (una escena, un master, un GO…). Los mapeos se guardan con el show.

La página tiene tres pestañas: **«Mapeos»**, **«Dispositivos»** y **«Monitor»**, y el botón **«Aprender»**.

## Dispositivos

En **«Dispositivos»** aparecen los dispositivos de entrada que el programa encuentra, con su tipo (MIDI, OSC o DMX), su nombre y el universo que usan. Primero salen los controladores (MIDI y OSC) y los dispositivos que ya usas, en **«Controladores y dispositivos en uso»**; después, en **«Entradas DMX: interfaces y red»**, las demás entradas DMX, con una línea por cada tarjeta de red.

1. Conecta el controlador y, si no aparece, pulsa **«Buscar de nuevo»**.
2. Activa su **«Entrada»**.
3. Si el dispositivo tiene salida, activa **«Feedback (LEDs)»** para que el programa encienda sus LEDs o mueva sus faders motorizados.

En una Akai APC (APC mini, APC mini mk2 o APC40 mkII) con la entrada y el feedback activados aparece **«Preparar»**. Pone el controlador en el modo que usan sus mapeos y le vuelve a enviar todos sus LEDs. Úsalo después de apagar y encender el controlador o de cambiarle el modo.

Un mapeo de un dispositivo que no está conectado se conserva: vuelve a funcionar cuando lo conectas.

La entrada de un dispositivo MIDI también trae su timecode MIDI (MTC) a las [cuelists «Timecode»](escenas.md#opciones-de-la-cuelist) que lo siguen. Basta con activar su **«Entrada»**; no hace falta ningún mapeo.

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

Las teclas y los botones del controlador de las listas de cues de QLC+ se pueden traer como mapeos, y los de tus cuelists viajan a la copia para QLC+ (consulta [Escenas](escenas.md#cuelists-y-qlc)).

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
| Lanzar una escena | La escena elegida (o «La escena seleccionada»), en su grupo. |
| Escena N de un grupo | La escena que está en ese puesto del grupo (**Número de escena**). |
| Liberar un grupo / Escena siguiente de un grupo / Escena anterior de un grupo | En un grupo o en «Todos los grupos». |
| Blackout, Parar todo, Pausar todo, Tap tempo, BeatGO, GO, Stop | Como sus botones (GO y Stop eligen el modo, como en la barra superior: siempre hay uno encendido). |
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
| Dimmer en vivo de una escena / Velocidad en vivo de una escena | Sus deslizadores en vivo, de la escena elegida (o «La escena seleccionada»). |
| BPM | El tempo. |
| Mezclador: dimmer, tono, saturación, velocidad del estrobo | En la tira elegida. |
| Fader del SHOW | Un fader del SHOW entero. |
| Playback: fader | El fader de un playback. |
| Velocidad de las cues (todas las cuelists) | La [velocidad de todas las cuelists](playbacks.md#velocidad-de-las-cues): ×0,1 abajo, ×1 en medio, ×10 arriba. |
| Tiempo en vivo de un playback / Tiempo en vivo del playback seleccionado | El [tiempo de la cue](playbacks.md#tiempo-en-vivo) que está haciendo su fundido. Como fader: en medio, el tiempo con el que empezó; abajo ×0,1 y arriba ×10. Como encoder o con pulsaciones: un 10 % más o menos por paso. |

Para los playbacks se elige la **página** («Página actual (sigue)», que sigue a la página que se ve, o una fija) y el número de **Playback**. Con **«O una cuelist o una escena, esté donde esté»** el mapeo actúa sobre esa cuelist o escena en cualquier playback donde esté. En «ir a una cue» se ve a qué cue lleva ahora («Ahora: cue 3 de Canción 1»).

### La escena seleccionada

En «Lanzar una escena», «Dimmer en vivo de una escena» y «Velocidad en vivo de una escena», la lista de escenas empieza por **«La escena seleccionada»**. Ese mapeo actúa sobre la última escena que pulsaste.

- **Seleccionan una escena**: pulsarla en Directo, en un botón del SHOW, en un controlador o en el móvil, y lanzarla con «Escena siguiente» o «Escena anterior» de su grupo.
- **No la cambian**: los faders del SHOW, los playbacks, las acciones de las cues ni «Probar» en Escenas.
- Sigue seleccionada cuando se para. No se guarda: al abrir un show o empezar uno nuevo no hay ninguna.
- Debajo de la lista se ve cuál es ahora («Ahora: Verso»). Sin ninguna, el mapeo no hace nada y su LED queda apagado.
- Un botón lanza, para o hace flash de la escena seleccionada en el momento de pulsarlo, en su grupo y según GO / Stop. Su LED muestra el estado de esa escena.
- En un fader absoluto se activa **«Recoger el fader (sin motor)»**: al cambiar de escena, el nivel no salta. El fader manda cuando alcanza el nivel de la nueva escena. Puedes desactivarlo.
- En Directo, la escena seleccionada lleva una barra fina en su borde inferior, mientras algún mapeo use «La escena seleccionada» o en el modo Mapear.

Ejemplo: los pads de la APC lanzan escenas, y así las seleccionan. Un mando va a «Velocidad en vivo de una escena» › «La escena seleccionada». El mando cambia la velocidad de la última escena que pulsaste.

### Comportamiento

En los **botones**:

- **«Alternar: on / off»**: cada pulsación enciende o apaga. En una escena se llama **«Pulsar: según GO / Stop»**: la lanza o la para según GO y Stop de la barra superior.
- **«Flash: mientras se pulsa»**.
- **«Solo encender»** / **«Solo apagar»**.

Los botones de playback actúan al pulsar, salvo el flash, y no tienen esta opción.

El flash de un playback usa el ataque y la caída de ese playback (ver [Propiedades de un playback](playbacks.md#propiedades-de-un-playback)). Con «O una cuelist o una escena, esté donde esté», usa los del primer playback que la tiene; si no está en ningún playback, es al momento. Con «Alternar», una pulsación durante la caída lo vuelve a subir.

En los **faders**:

- **«Absoluto: sigue al fader»**.
- **«Incremental: un paso por pulsación»** (para botones que suben o bajan).
- **«Relativo: encoder sin fin»** y **«Relativo: encoder, 1 sube y 127 baja»** (encoders sin fin).
- **«De»** … **«a»**: el rango, de 0 a 100 %. **«Invertir»** le da la vuelta.
- **«Paso %»** y **«En bucle»** (incremental y relativo).
- **«Recoger el fader (sin motor)»** (absoluto): el fader del controlador solo mueve el nivel después de alcanzarlo, sin saltos. Con un fader motorizado no hace falta. En el fader A/B de un playback, el primer valor que llega solo dice dónde está; después, el fader del controlador tiene que alcanzar al de la pantalla.

### Feedback

Con MIDI u OSC, **«Feedback»** hace que el programa **encienda el botón del dispositivo** o **mueva su fader motorizado**. Solo se envía lo que cambia.

Un fader motorizado (absoluto) no se mueve mientras lo mueves tú. El programa espera a que lo dejes quieto 0,4 s y solo lo corrige si su destino quedó en otro valor, por ejemplo porque otra cosa lo cambió. Mientras no lo tocas, la pantalla, otra página de playbacks u otro control lo mueven enseguida. Los encoders y los botones no esperan.

Para los LEDs de los botones MIDI:

1. En **«LEDs de»**, elige **«Cualquier controlador (números)»** o tu controlador: **Akai APC mini**, **Akai APC mini mk2** o **Akai APC40 mkII**.
2. Para **«Encendido»**, **«Apagado»**, **«En pausa»** y, en las escenas, **«En fundido»**, elige el color en las muestras (en una APC) o escribe el valor (0–127) para cualquier otro controlador.
3. En los pads RGB, elige también cómo se encienden: **«Tenue»**, **«Pleno»**, **«Pulsando»** o **«Parpadeo»** en la APC mini mk2, y **«Fijo»**, **«Pulsando»** o **«Parpadeo»** en la APC40 mkII. Pulsando o en parpadeo, el pad alterna entre el color y apagado. Sin elegir nada, «Encendido» luce en **«Pleno»** (**«Fijo»** en la APC40 mkII), «Apagado» en **«Tenue»** (**«Fijo»** en la APC40 mkII) y «En fundido» en **«Pulsando»**.

**«En fundido»** (en «Lanzar una escena» y «Escena N de un grupo»): activa **«Color propio durante el fundido»** y el botón luce ese color mientras la escena entra con su fundido y mientras sale con el suyo. En una cuelist, luce mientras su cue hace el fundido. Sin él, al entrar luce como «Encendido» y al salir como «Apagado». Una escena en pausa o esperando luce como «En pausa».

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

**→ Playbacks** (APC mini y mk2): cada columna es un playback 1-8 de la página de playbacks actual: su fader (recoge el nivel) y, desde abajo, GO, Pausa/Atrás, Flash y sus 5 primeras cues (la cue 1 arriba). El botón de debajo de los pads lo libera. El último fader es el gran master; los botones de la derecha son página anterior y siguiente, GO y Atrás del playback seleccionado, blackout, tap, BeatGO y parar todo. Si el fader de un playback está en A/B (ver [El fader](playbacks.md#el-fader)), cada recorrido del fader de la APC es un GO cuyo fundido haces a mano.

**→ Playbacks** (APC40 mkII): cada pista es un playback 1-8 de la página actual: su fader, GO en el botón de pista, Pausa/Atrás en S, Flash en el armado de grabación, seleccionar y liberar, y sus 5 primeras cues en los clips (verde: sonando; verde pulsando: la siguiente; azul tenue: las demás). Los botones de escena van a las páginas 1-5, los de banco cambian la página y el grupo, y Play y Record son GO y Atrás del playback seleccionado. Los mandos de device son los faders de los playbacks 9-16 y los mandos de pista, los masters de los primeros grupos.

La APC mini mk2 se pone sola en modo Session, y la APC40 mkII en modo Ableton: al arrancar el programa, al abrir un show, al activar su feedback y con «Mapear todo». En los modos Nota o Batería, los pads de la mk2 no llegan a los mapeos.

> **Consejo:** después de «Mapear todo», cada control aparece como un mapeo normal en la lista: puedes cambiar cualquiera, o borrarlo, sin tocar los demás.

## Monitor

**«Monitor»** muestra los **mensajes de entrada y salida**: la hora, **IN** u **OUT**, el dispositivo (o «Teclado»), el mensaje y su valor. Mueve un control de un dispositivo activado para ver qué envía. **«Limpiar»** vacía la lista.

El Monitor muestra como **OUT** los cambios de modo que envía el programa: «SysEx · modo Session» en la APC mini mk2 y «SysEx · modo Ableton» en la APC40 mkII.

El piloto verde de la diana, en la barra superior, parpadea con cada mensaje que llega.

## Alt+clic

Fuera del modo Mapear, **Alt+clic** en un control que se puede mapear lo lleva al SHOW: consulta [Show](show.md#llevar-cualquier-control-al-show).
