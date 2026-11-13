# Playbacks

![La página Playbacks](img/playbacks.png)

**Playbacks** (F10) es una mesa de faders. Cada **playback** reproduce una cuelist o cualquier escena, y tiene un **GO**, un botón **Pausa/Atrás**, un botón **Flash** y un **fader** que es su intensidad. Los playbacks se organizan en **páginas** de 8 a 20 (10 por defecto).

Es la forma más cómoda de llevar un show con cuelists: grabas las cues desde el programador directamente en un playback y las lanzas con GO o con la barra espaciadora.

## La tira de un playback

De arriba abajo:

- **El número** sobre el color del playback. Se ilumina mientras suena. La **×** de la derecha lo libera. Una cuelist de otro tipo lleva su etiqueta: **SUB** (submaster), **CHASE**, **TC** (timecode) u **OVR** (override).
- **El nombre**: haz clic en él para **seleccionar** el playback (borde naranja). Un playback sin nada muestra «Vacío».
- **La cue que suena y la siguiente** («3 · Verso», «▸ 4 · Puente») y una barra con el avance del fundido. Si la siguiente es el salto de un enlace de la cue, lleva **↺** y el número del salto: «▸ 2 ↺ 1/3» es el primero de tres (ver [Cuelists](escenas.md#cuelists)). Un chase muestra antes su velocidad: «♩ 120 ▸ 4». Una cuelist timecode muestra antes su reloj: «TC 00:01:23.4 · 3 · Verso». Una cuelist parada muestra «Liberada»; una escena muestra su tipo y si está «Encendido», «Apagado» o «En pausa».
- **Botón 1 (GO)**: la cue siguiente; si el playback está apagado, lo arranca.
- **Botón 2 (Pausa / Atrás)**: pausa un fundido en curso; si no hay fundido, vuelve a la cue anterior.
- **El fader** y su porcentaje. Un fader A/B muestra «A/B» y lo que lleva su fundido.
- **Botón 3 (Flash)**: a tope mientras lo pulsas (ver [El flash](playbacks.md#el-flash)).

Los botones se iluminan según el estado: **GO** en verde mientras suena (parpadea deprisa durante un fundido o una espera y despacio en pausa), **Pausa/Atrás** en naranja en pausa y **Flash** en naranja mientras está pulsado y hasta que acaba su caída.

### El flash

Mientras pulsas **Flash**, el playback va a tope. Al soltarlo, vuelve al nivel de su fader. Si estaba apagado, el flash lo arranca y al soltarlo lo libera.

Con un **ataque** y una **caída** en sus [propiedades](playbacks.md#propiedades-de-un-playback), el cambio no es de golpe:

- Al pulsar, sube a tope en el tiempo del ataque, desde donde está. Un playback apagado sube desde cero.
- Al soltar, vuelve al nivel del fader en el tiempo de la caída. Un playback que arrancó el flash baja a cero y se libera al acabar la caída.
- Si pulsas otra vez durante la caída, sube desde donde está, sin bajar antes.
- Si mueves el fader durante la caída, la caída acaba en el nuevo nivel.
- Un GO durante la caída lo deja encendido: la caída acaba en el nivel del fader.

Los canales que no son de intensidad (posición, rueda de color) entran al momento, como siempre.

### El fader

El fader es la intensidad de lo que reproduce el playback:

- Al **subirlo desde cero**, el playback arranca.
- Al **bajarlo a cero**, una escena se libera; una cuelist se queda en su cue (se puede cambiar en las [propiedades](playbacks.md#propiedades-de-un-playback)).
- Un **GO con el fader abajo** reproduce a tope.

En una cuelist **submaster**, el fader manda: subirlo desde cero la arranca al nivel del fader, nunca a tope, y bajarlo a cero la libera. Liberada, GO y Atrás solo eligen la cue con la que arrancará (ver [Opciones de la cuelist](escenas.md#opciones-de-la-cuelist)).

En una cuelist **override**, el fader hace un fundido de todo lo que controla, también colores, posiciones y gobos, entre lo que hay debajo y su cue. A tope, su cue sustituye a lo que hay debajo (ver [Opciones de la cuelist](escenas.md#opciones-de-la-cuelist)).

En una cuelist normal, el fader puede ser un **fundido cruzado (A/B)** («Fader» en las [propiedades](playbacks.md#propiedades-de-un-playback)). Entonces no es la intensidad: el playback suena al nivel que tenía, y cada recorrido del fader de un extremo al otro es un GO cuyo fundido haces a mano.

- Al salir de su extremo, empieza la cue siguiente y su fundido sigue al fader: el 40 % del recorrido es el 40 % del fundido. Los retardos, las bajadas y los tiempos de atributos y fixtures van en proporción.
- Al llegar al otro extremo, la cue se completa. El recorrido siguiente, en sentido contrario, lanza la cue que viene después.
- Si vuelves al extremo del que saliste, se cancela y vuelve la cue anterior. Si ese recorrido arrancó la cuelist, se libera.
- Una cuelist parada arranca con la cue de un GO, que entra con el fader. Una cue sin tiempo hace el fundido de forma lineal a lo largo del recorrido.
- Los FX de la cue entran con su propio tiempo.
- **GO** (el botón, Main GO o un mapeo) lanza la cue siguiente con su tiempo. A mitad de un recorrido, termina ese fundido con su tiempo y no lanza otra cue.
- Seguir y Esperar de la cue siguiente esperan a que el recorrido termine. La velocidad de las cues y el tiempo en vivo no cambian el fundido que llevas a mano.

Doble clic en el fader lo lleva a tope o a cero. En un fader A/B, es un recorrido entero: la cue se completa al momento.

## Poner algo en un playback

Hay varias formas:

- **Grabar desde el programador**: consulta [Grabar en un playback](playbacks.md#grabar-en-un-playback).
- **«Añadir»** en un playback vacío: **«Nueva cuelist desde el programador»** o **«Poner aquí una escena…»**.
- **Asignar**: pulsa **«Asignar»** en la cabecera. A la derecha aparece la lista de escenas: arrástralas a los playbacks (mientras la lista está abierta, una escena soltada en un playback ocupado lo sustituye) o haz doble clic en una para ponerla en el primer playback libre.
- **Clic derecho › «Asignar una escena…»**: elige la cuelist o la escena en la lista y pulsa **«Ponerla en el playback N»** (o doble clic).

Desde Programar también puedes grabar una cuelist nueva y ponerla en el primer playback libre (consulta [Grabar](programar.md#grabar)).

Las cue lists de QLC+ también pueden llegar a los playbacks: ver [Escenas › Cue lists de QLC+](escenas.md#cue-lists-de-qlc).

## GO, Atrás y el playback seleccionado

A la derecha de la cabecera tienes los controles del **playback seleccionado**:

- **GO** (el botón grande verde, o **Espacio**): la cue siguiente del playback seleccionado. Muestra su nombre y qué cue viene («Canción 1 ▸ 3 · Puente»).
- **«Atrás»** (**Mayús+Espacio**): la cue anterior.
- **«Liberar todo»**: libera todos los playbacks y todas las cuelists que suenan.
- **«Velocidad de las cues»**: la velocidad de todas las cuelists (ver [Velocidad de las cues](playbacks.md#velocidad-de-las-cues)).

Espacio y Mayús+Espacio funcionan en esta página siempre que no estés escribiendo en un campo de texto. Cada pulsación da un solo GO: mantener la tecla no recorre las cues.

### El panel del playback seleccionado

Abajo se ve el playback seleccionado («Playback N · nombre»), su estado y sus botones: atrás, **GO**, pausa / continuar, liberar y **«Editar en Escenas»**.

Si es una cuelist, aparece la lista de sus cues (CUE, NOMBRE, DISPARO, RETARDO, FUNDIDO). La cue que suena lleva una barra verde y la siguiente un borde verde. En cada fila:

- **«Siguiente»**: esa cue será la del próximo GO.
- **«Ir»**: salta a esa cue ahora.

Un clic en una fila solo la marca: no lanza nada por error.

## Velocidad de las cues y tiempo en vivo

### Velocidad de las cues

**«Velocidad de las cues»**, en la cabecera, cambia la velocidad de todas las cuelists a la vez: sus fundidos, retardos, Seguir y Esperar, su ritmo marcado y los pasos de un chase. Va de ×0,1 a ×10, con ×1 en el centro.

- Actúa al momento, también en los fundidos en curso: aceleran o frenan desde donde están, sin saltos.
- El valor se pone naranja cuando no es ×1. Doble clic lo vuelve a ×1.
- No cambia los FX ni las demás escenas.
- No se guarda: cada show se abre a ×1.

También está en los faders del [SHOW](show.md#faders) y en los [mapeos](mapeos.md#controla).

### Tiempo en vivo

Mientras una cue hace su fundido, el panel del playback seleccionado muestra **«Tiempo en vivo»**: el retardo más el fundido de esa cue (por ejemplo, «4,0 s»). Puedes cambiarlo mientras corre:

- Escribe otro tiempo, o pulsa **«×½»** o **«×2»**: el fundido sigue desde donde está y acaba en el tiempo nuevo.
- **«Ahora»** termina el fundido al momento.
- Si la cue siguiente tiene **Seguir**, espera al nuevo final.

El cambio vale solo para esta vez: la cue siguiente usa sus propios tiempos. «Tiempo en vivo» se pone naranja cuando el tiempo no es el de la cue. **«Guardar en la cue»** guarda en la cue los tiempos cambiados (también los de sus atributos y fixtures). No se puede cuando la cue va con un tiempo de BACK o con el fundido de entrada de un grupo o de un bloque, que no son sus tiempos. Se deshace como cualquier otra edición de la cuelist.

La misma barra está en la cabecera de la cuelist en [Escenas](escenas.md#la-tabla-de-cues). Desde un controlador, usa los mapeos «Tiempo en vivo de un playback» o «Tiempo en vivo del playback seleccionado» (ver [Mapeos](mapeos.md#controla)).

## Grabar en un playback

1. Monta un look en [Programar](programar.md).
2. En Playbacks, pulsa **«Grabar»**: parpadea en rojo y cada playback muestra qué pasará al hacer clic.
3. Haz clic en un playback:
   - **Vacío** («Nueva cuelist»): se crea una cuelist nueva con el programador como cue 1.
   - **Con una cuelist** («Grabar la cue N»): se graba su cue siguiente, solo con lo que cambia respecto a lo que hereda.
   - **Con una escena** («Tiene una escena»): no se puede grabar en él.

**Esc** cancela la grabación armada, y también se desarma al salir de la página.

### Actualizar una cue

Cambia los valores en el programador, selecciona el playback y pulsa **«Actualizar ▾»**:

- **«Actualizar la cue N (sigue en las siguientes)»**: el cambio pasa a las cues siguientes que heredan esos canales.
- **«Solo esta cue (las siguientes mantienen su aspecto)»**.

La cue que se actualiza es la que está cargada en el programador o, si no hay ninguna, la que está sonando. El menú del clic derecho de cada playback tiene también **«Grabar la cue N»**, **«Actualizar la cue actual (sigue en las siguientes)»** y **«Actualizar solo la cue actual»**.

> **Consejo:** al terminar de grabar, pulsa «Restablecer todo» en Programar (o marca «Limpiar el programador después de grabar»): mientras el programador tiene valores, manda sobre lo que lanzas.

## Páginas

- Haz clic en el nombre de una página para verla. **◀ ▶** (o **RePág** / **AvPág**) pasan a la anterior o la siguiente.
- **«+ Página»** añade una página.
- **«Faders por página»**: 8, 10, 12, 16 o 20 playbacks por página.
- Doble clic en una página la renombra. Clic derecho: **«Renombrar…»**, **«Mover a la izquierda»**, **«Mover a la derecha»** y **«Eliminar la página…»** (con más de una página). Al eliminarla, sus playbacks se vacían; lo que reproducen sigue sonando y las cuelists y escenas se quedan en Escenas.

## Propiedades de un playback

Clic derecho › **«Propiedades…»**. Los cambios se aplican al momento.

- **Etiqueta**: el nombre que muestra (vacía: el de la escena).
- **Color**: una muestra, o la **A** para el color de su tipo de escena.
- **Botón 1**, **Botón 2** y **Botón 3**: qué hace cada uno: «Nada», «GO», «Atrás», «Pausa», «Pausa / Atrás», «Liberar», «Flash», «Flash + GO» (a tope mientras se pulsa, y la cue siguiente), «Encender / apagar» o «Seleccionar» (para que Main GO, Grabar y Actualizar actúen sobre él).
- **Ataque del flash** y **Caída del flash**: lo que tarda el flash en subir a tope y en volver al soltarlo, de 0 a 10 s (0 = al momento). Valen para su botón Flash o Flash + GO, los mapeos, el SHOW y el remoto (ver [El flash](playbacks.md#el-flash)).
- **Fader**: «Intensidad» (por defecto) o «Fundido cruzado (A/B)», solo en una cuelist normal (ver [El fader](playbacks.md#el-fader)). En A/B no aparecen las dos opciones siguientes: el fader siempre la arranca y nunca la libera.
- **El fader lo arranca**: si al subir el fader arranca el playback.
- **El fader a cero lo libera**: «Por defecto» (una escena y una cuelist submaster sí; las demás cuelists no: mantienen su cue), «Sí» o «No».

El mismo menú tiene **«Editar en Escenas»**, **«Liberar»** y **«Vaciar el playback»** (lo quita del playback; la cuelist o la escena sigue en Escenas).

## Liberar

- La **×** del número de un playback que suena, o **«Liberar»** en su menú.
- **«Liberar todo»** en la cabecera.

Liberar un playback también libera esa escena si está sonando en un grupo de Directo.

## Controladores Akai APC

En [Mapeos](mapeos.md#plantillas-para-akai-apc) › «Dispositivos», **«Mapear todo»** asigna de una vez un controlador entero a los playbacks:

- **Akai APC mini** y **APC mini mk2** → Playbacks: cada columna es un playback 1-8 de la página actual: su fader y, desde abajo, GO, Pausa/Atrás, Flash y sus 5 primeras cues. El botón bajo los pads lo libera, el último fader es el gran master y los botones de la derecha son página anterior y siguiente, GO y Atrás del seleccionado, blackout, tap, BeatGO y parar todo.
- **Akai APC40 mkII** → Playbacks: cada pista es un playback 1-8, con sus 5 primeras cues en los clips (la siguiente pulsa en verde); los mandos de device son los faders de los playbacks 9-16.

Los LEDs muestran el estado de cada playback.

## Playbacks en el SHOW y en el remoto

- El [SHOW](show.md#playbacks-en-el-show) puede tener una sección de playbacks que sigue la página actual o muestra una fija.
- En el [control remoto](remoto.md), cuando algún playback tiene algo, aparece una página «Playbacks» con sus tiras.

## Con la programación bloqueada

Si bloqueas la **«Programación»** con el candado de la barra superior, los playbacks siguen funcionando (GO, faders, flash), pero no se puede grabar, actualizar, asignar ni cambiar las páginas o las propiedades.
