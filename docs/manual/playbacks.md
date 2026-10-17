# Playbacks

![La página Playbacks](img/playbacks.png)

**Playbacks** (F10) es una mesa de faders. Cada **playback** reproduce una cuelist o cualquier escena, y tiene un **GO**, un botón **Pausa/Atrás**, un botón **Flash** y un **fader** que es su intensidad. Los playbacks se organizan en **páginas** de 8 a 20 (10 por defecto).

Es la forma más cómoda de llevar un show con cuelists: grabas las cues desde el programador directamente en un playback y las lanzas con GO o con la barra espaciadora.

## La tira de un playback

De arriba abajo:

- **El número** sobre el color del playback. Se ilumina mientras suena. La **×** de la derecha lo libera.
- **El nombre**: haz clic en él para **seleccionar** el playback (borde naranja). Un playback sin nada muestra «Vacío».
- **La cue que suena y la siguiente** («3 · Verso», «▸ 4 · Puente») y una barra con el avance del fundido. Una cuelist parada muestra «Liberada»; una escena muestra su tipo y si está «Encendido», «Apagado» o «En pausa».
- **Botón 1 (GO)**: la cue siguiente; si el playback está apagado, lo arranca.
- **Botón 2 (Pausa / Atrás)**: pausa un fundido en curso; si no hay fundido, vuelve a la cue anterior.
- **El fader** y su porcentaje.
- **Botón 3 (Flash)**: a tope mientras lo pulsas.

Los botones se iluminan según el estado: **GO** en verde mientras suena (parpadea deprisa durante un fundido o una espera y despacio en pausa), **Pausa/Atrás** en naranja en pausa y **Flash** en naranja mientras está pulsado.

### El fader

El fader es la intensidad de lo que reproduce el playback:

- Al **subirlo desde cero**, el playback arranca.
- Al **bajarlo a cero**, una escena se libera; una cuelist se queda en su cue (se puede cambiar en las [propiedades](playbacks.md#propiedades-de-un-playback)).
- Un **GO con el fader abajo** reproduce a tope.

Doble clic en el fader lo lleva a tope o a cero.

## Poner algo en un playback

Hay varias formas:

- **Grabar desde el programador**: consulta [Grabar en un playback](playbacks.md#grabar-en-un-playback).
- **«Añadir»** en un playback vacío: **«Nueva cuelist desde el programador»** o **«Poner aquí una escena…»**.
- **Asignar**: pulsa **«Asignar»** en la cabecera. A la derecha aparece la lista de escenas: arrástralas a los playbacks (mientras la lista está abierta, una escena soltada en un playback ocupado lo sustituye) o haz doble clic en una para ponerla en el primer playback libre.
- **Clic derecho › «Asignar una escena…»**: elige la cuelist o la escena en la lista y pulsa **«Ponerla en el playback N»** (o doble clic).

Desde Programar también puedes grabar una cuelist nueva y ponerla en el primer playback libre (consulta [Grabar](programar.md#grabar)).

## GO, Atrás y el playback seleccionado

A la derecha de la cabecera tienes los controles del **playback seleccionado**:

- **GO** (el botón grande verde, o **Espacio**): la cue siguiente del playback seleccionado. Muestra su nombre y qué cue viene («Canción 1 ▸ 3 · Puente»).
- **«Atrás»** (**Mayús+Espacio**): la cue anterior.
- **«Liberar todo»**: libera todos los playbacks y todas las cuelists que suenan.

Espacio y Mayús+Espacio funcionan en esta página siempre que no estés escribiendo en un campo de texto. Cada pulsación da un solo GO: mantener la tecla no recorre las cues.

### El panel del playback seleccionado

Abajo se ve el playback seleccionado («Playback N · nombre»), su estado y sus botones: atrás, **GO**, pausa / continuar, liberar y **«Editar en Escenas»**.

Si es una cuelist, aparece la lista de sus cues (CUE, NOMBRE, DISPARO, RETARDO, FUNDIDO). La cue que suena lleva una barra verde y la siguiente un borde verde. En cada fila:

- **«Siguiente»**: esa cue será la del próximo GO.
- **«Ir»**: salta a esa cue ahora.

Un clic en una fila solo la marca: no lanza nada por error.

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
- **El fader lo arranca**: si al subir el fader arranca el playback.
- **El fader a cero lo libera**: «Por defecto» (una escena sí, una cuelist no: mantiene su cue), «Sí» o «No».

El mismo menú tiene **«Editar en Escenas»**, **«Liberar»** y **«Vaciar el playback»** (lo quita del playback; la cuelist o la escena sigue en Escenas).

## Liberar

- La **×** del número de un playback que suena, o **«Liberar»** en su menú.
- **«Liberar todo»** en la cabecera.

Liberar un playback también libera esa escena si está sonando en un grupo de Directo.

## Controladores Akai APC

En [Mapeos](mapeos.md#plantillas-para-akai-apc) › «Dispositivos», **«Mapear todo»** asigna de una vez un controlador entero a los playbacks:

- **Akai APC mini** y **APC mini mk2** → Playbacks: cada columna es un playback 1-8 de la página actual: su fader y, desde abajo, GO, Pausa/Atrás, Flash y sus 5 primeras cues. El botón bajo los pads lo libera, el último fader es el gran master y los botones de la derecha son página anterior y siguiente, GO y Atrás del seleccionado, blackout, tap, BeatGO y parar todo.
- **Akai APC40 mkII** → Playbacks: cada pista es un playback 1-8, con sus 5 primeras cues en los clips; los mandos de device son los faders de los playbacks 9-16.

Los LEDs muestran el estado de cada playback.

## Playbacks en el SHOW y en el remoto

- El [SHOW](show.md#playbacks-en-el-show) puede tener una sección de playbacks que sigue la página actual o muestra una fija.
- En el [control remoto](remoto.md), cuando algún playback tiene algo, aparece una página «Playbacks» con sus tiras.

## Con la programación bloqueada

Si bloqueas la **«Programación»** con el candado de la barra superior, los playbacks siguen funcionando (GO, faders, flash), pero no se puede grabar, actualizar, asignar ni cambiar las páginas o las propiedades.
