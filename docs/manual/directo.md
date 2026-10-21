# Directo

![La página Directo](img/directo.png)

**Directo** (F1) es la página para lanzar escenas durante el show. Las escenas se ordenan en **grupos**, que son columnas que funcionan en paralelo: cada grupo tiene sus escenas, su modo (exclusiva o mezcla), su tiempo de crossfade y su master.

En la cabecera de la página tienes:

- **◀ ▶**: escena anterior o siguiente en cada grupo que suena (también con Ctrl+← y Ctrl+→).
- **«Pausa»** / **«Reanudar»**: congela todo lo que suena y lo continúa.
- **«Liberar grupos»**: para las escenas de todos los grupos.
- **«Mezclador»**: cambia a la vista del [mezclador](directo.md#mezclador). Un punto naranja avisa de que el mezclador está cambiando la salida.
- **«Nuevo grupo»**: añade una columna.

## Grupos

### Crear y organizar grupos

1. Pulsa **«Nuevo grupo»** (o el recuadro «Añadir grupo» al final de las columnas).
2. Haz clic en el nombre de la columna para cambiarlo (por ejemplo `Wash`, `Spots`, `Strobos`, `Efectos`) y pulsa Intro.

El botón del engranaje de cada columna abre su menú: **«Cambiar color»** (pasa al siguiente color), **«Mover a la izquierda»**, **«Mover a la derecha»** y **«Eliminar grupo»**. También puedes arrastrar el grupo por el asa que hay a la izquierda de su nombre y soltarlo sobre otro grupo para ocupar su lugar.

La flecha **‹** junto al nombre **pliega** el grupo: queda una tira estrecha con su nombre y cuántas escenas suenan, para dejar sitio a los demás. Sus escenas siguen sonando. La flecha **›** de la tira lo despliega. Se guarda con el show.

### Añadir y ordenar escenas

1. Pulsa **«Añadir escenas»** al final de la columna.
2. En la lista, haz clic en las escenas que quieras (se marcan con ✓). Puedes buscar por nombre o por carpeta y verlas por carpetas o por grupos.
3. Pulsa **«Añadir N»**. Con doble clic en una escena se añade directamente.

Para reordenar, pasa el ratón por una ficha y arrástrala por su asa (a la derecha de la ficha): suéltala sobre otra escena para ponerla delante, o en el hueco de la columna para ponerla al final. Puedes soltarla en **otro grupo** y se mueve con sus opciones. En el menú de la ficha (clic derecho) también tienes **«Subir»**, **«Bajar»** y **«Quitar del grupo»**.

> **Consejo:** al grabar desde Programar puedes meter la escena directamente en un grupo con «Añadir al grupo». Consulta [Grabar](programar.md#grabar).

## Lanzar escenas

Haz clic en una ficha para lanzar la escena; vuelve a hacer clic para liberarla. La ficha que suena se colorea con el color del grupo y tiene un punto que late.

En cada ficha se ve:

- El **nombre** y, debajo, el **tipo** de escena. En una cuelist que suena, la cue en la que está («Q 3 · Verso»). Detrás, el [ritmo marcado](directo.md#ritmo-de-una-escena-con-pasos) si lo tiene («120 BPM»).
- **P7**, **P2**…: la prioridad, solo cuando no es la normal (5). En naranja si es mayor que 5.
- Un **rayo**: es una escena flash.
- Una **barra partida en dos** abajo: una escena con pasos o una cuelist que suena; alterna en cada paso, a su ritmo.

### Exclusiva o mezcla

El botón de cada columna alterna entre:

- **«Exclusiva»**: solo suena una escena a la vez. Al lanzar otra, hace crossfade a la nueva.
- **«Mezcla»**: las escenas se suman; puedes tener varias sonando a la vez.

### Tiempo de crossfade

El desplegable junto a «Exclusiva»/«Mezcla» es el tiempo de fundido del grupo:

- **«Por escena»**: cada escena usa sus propios fundidos de entrada y salida.
- **«Corte»**: sin fundido.
- **0,5 s a 10 s**: ese tiempo para todas las escenas del grupo, en lugar de los suyos.

### Escena anterior y siguiente

Los botones **◀ ▶** de cada columna lanzan la escena anterior o siguiente del grupo, en el orden de la columna. Los de la cabecera de la página hacen lo mismo en todos los grupos que están sonando.

### Escenas flash

Una escena con **flash** suena solo mientras mantienes pulsada su ficha. Se activa en sus [propiedades](directo.md#propiedades-de-la-escena) con «Flash (suena mientras se pulsa)».

### Cuelists en un grupo

Una cuelist en un grupo es una ficha más: el clic la enciende y la apaga. Su menú (clic derecho) añade **«GO (cue siguiente)»**, **«Atrás (cue anterior)»** y **«Liberar la cuelist»**. Para operarla con GO cómodamente, ponla en un [playback](playbacks.md).

### Ritmo de una escena con pasos

Las escenas con pasos (secuencias y chasers) y las cuelists siguen el ritmo al que pulsas su ficha:

- Un clic la lanza o la libera, como siempre.
- Con la escena sonando, pulsa su ficha **varias veces seguidas** al ritmo de la música, con menos de 2 segundos entre pulsaciones. Cada pulsación pasa al paso siguiente (en una cuelist, da un GO) y la escena sigue sola a ese ritmo: un paso en cada pulso, en lugar de sus propios tiempos. El ritmo es la media de las últimas pulsaciones.
- Solo cuentan las pulsaciones con la escena sonando: la que la lanza empieza la cuenta. Así, cambiar de escena en un grupo exclusivo y volver no le pone ritmo a ninguna.
- Tras una pausa de más de 2 segundos, un clic la libera. Para liberarla justo después de marcar el ritmo, espera 2 segundos.
- Se reproduce el ritmo que marcaste, también con una **velocidad en vivo** puesta; si luego mueves la velocidad, el ritmo se acelera o se frena con ella. En pausa, las pulsaciones no dan pasos.
- El ritmo se guarda con la escena y es suyo: también lo sigue cuando suena en una super escena o en un playback. En sus [propiedades](directo.md#propiedades-de-la-escena) (y en [Escenas](escenas.md#cómo-suena-una-escena)) puedes escribirlo o pulsar **«Quitar»** para que vuelva a sus tiempos.

Funciona igual con sus botones del [SHOW](show.md), con sus mapeos de controladores y teclas en modo «Alternar» y con el [control remoto](remoto.md).

#### GO y Stop

Los botones **GO** y **Stop** de la barra superior (y de la sección «Ritmo» del SHOW) cambian lo que hace una pulsación de cualquier escena, para todo el que la pulse:

- **GO** encendido: pulsar una escena la **lanza**; en una que ya suena, cada pulsación da un paso (en una cuelist, un GO) y varias seguidas marcan su ritmo. Nunca la para, aunque pases más de 2 segundos sin pulsar: así marcas el tiempo tranquilo.
- **Stop** encendido: pulsar una escena la **para**. Una que no suena se queda como está.
- Los dos apagados (vuelve a pulsar el que está encendido): como siempre, un clic la lanza o la libera.

Las escenas flash siguen funcionando mientras las mantienes pulsadas, en cualquier modo.

Mientras suena, cada ficha de una escena con pasos o de una cuelist lleva abajo una **barra del ritmo** partida en dos: se enciende un lado y luego el otro en cada paso (en una cuelist, en cada cue). Así ves a qué ritmo va cada una. La llevan también sus botones del SHOW, sus tiras de playback y sus botones en el móvil.

## Masters

- **Master del grupo**: el deslizador «Master» al pie de cada columna escala la intensidad de todas sus escenas.
- **Gran master**: el «GM» de la barra superior escala toda la salida. En Opciones decides si actúa solo sobre los canales de intensidad o sobre todos.
- **Dimmer en vivo**: cada escena tiene además su propio dimmer, en sus propiedades, por encima del master del grupo.

Los masters y el gran master no afectan a los canales que tienen desactivada la opción «Dimmer» en los [límites del fixture](patch.md#límites).

## Propiedades de la escena

Haz clic derecho en una ficha y elige **«Propiedades…»**.

| Campo | Qué hace |
|---|---|
| **Nombre** | El nombre de la escena. |
| **Fundido de entrada** / **Fundido de salida** | Sus tiempos, en segundos. Si el grupo tiene un tiempo de crossfade puesto, se usa ese. |
| **Prioridad (0-10)** | Qué escena gana cuando dos tocan el mismo canal (consulta [Prioridades](directo.md#prioridades)). |
| **Al empezar, libera** | Qué para al arrancar: [modos de liberación](directo.md#modos-de-liberación-y-protección). |
| **Protegida de liberaciones de** | Qué liberaciones ignora. |
| **Flash (suena mientras se pulsa)** | La convierte en escena flash. |
| **Dimmer en vivo** | Su intensidad, encima del master del grupo. |
| **Ritmo marcado** | Pasos y cuelists: el [ritmo marcado](directo.md#ritmo-de-una-escena-con-pasos) con su ficha, en BPM. Escríbelo o pulsa «Quitar» para volver a sus tiempos. Actúa al momento. |
| **Velocidad en vivo** | De ×0,1 a ×10 (escenas que se mueven: pasos, FX, cuelists…). |
| **Fase en vivo** / **Tamaño en vivo** | La fase (0–359°) y el tamaño (0–200 %) de los FX. |
| **Velocidad de fundido** | Acelera o frena sus fundidos, de ×0,1 a ×10. |

Los deslizadores en vivo actúan al momento, mientras los mueves, y cambian cómo suena la escena en ese grupo, no la escena grabada. El resto de campos se aplican al pulsar **«Aplicar»**.

El mismo menú de la ficha tiene **«Abrir en Escenas»**, **«Editar en el programador»** (escenas estáticas) y el submenú **«Prioridad: N»** para cambiarla sin abrir las propiedades.

## Prioridades

Cada escena tiene una prioridad de 0 a 10; las nuevas toman la que indiques en [Opciones](opciones.md#reproducción) (5 por defecto).

- La escena de **mayor prioridad manda en todos los canales que toca**, ceros incluidos: una escena roja de prioridad 8 sobre una azul de prioridad 5 da rojo, no magenta.
- Con la **misma prioridad**, en los canales de intensidad (dimmer, RGB…) se mezclan y gana el valor más alto; en el resto de canales gana la última escena lanzada.
- Si cambias la prioridad de una escena que suena, se aplica al momento.
- Al liberar una escena de mayor prioridad con fundido, hace un fundido hacia lo que hay debajo.
- **El programador manda sobre todo**, intensidades incluidas. En Opciones puedes bajar la prioridad de la edición en vivo.

## Modos de liberación y protección

**Al empezar, libera** decide qué escenas se paran cuando arranca esta:

- **«La del grupo»**: en un grupo exclusivo, las demás de su grupo; en uno de mezcla, ninguna.
- **«Ninguno»**: no para nada.
- **«Su grupo»**: las demás escenas de su grupo.
- **«Todas las escenas»**: todas las demás.
- **«Los demás grupos»**: las escenas de los otros grupos.

**Protegida de liberaciones de** hace que la escena ignore esas liberaciones:

- **«Ninguno»**: se libera como siempre.
- **«Todo (solo se para a mano)»**: solo la paras tú.
- **«Su propio grupo»**: ignora las de su grupo.
- **«Los demás grupos»**: ignora las que vienen de otros grupos.

> **Consejo:** pon una escena de ambiente con protección «Todo (solo se para a mano)» y lanza encima escenas con «Todas las escenas»: el ambiente se queda.

## Pausa, liberar y parar

- **«Pausa»** congela todo lo que suena. **«Reanudar»** lo continúa.
- **«Liberar grupos»** para las escenas de todos los grupos; el cuadrado de cada columna, solo las de ese grupo.
- **«Parar todo»** (Esc), en la barra superior, para todo lo que suena, también fuera de los grupos. No vacía el programador.
- **«Blackout»** (Ctrl+B) saca la salida a negro sin parar nada.

## Mezclador

El mezclador actúa sobre lo que sale, por encima de cada escena y de la edición en vivo. Pulsa **«Mezclador»** para verlo: hay una tira para **«Todos los fixtures»** y una por cada [grupo de fixtures](patch.md#grupos-de-fixtures).

Cada tira tiene:

- Un **fader de dimmer** con su porcentaje.
- **«Full»**: el grupo a tope. **«Blackout»**: el grupo a negro.
- **«Tono»** y **«Saturación»**: tiñen el color. **«Restablecer color»** quita el tinte.
- **«Estrobo»** y su velocidad, de 0,5 a 20 Hz.
- **«Flash»**: a tope mientras se pulsa. **«Flash excl.»**: a tope ese grupo y el resto a negro mientras se pulsa.
- **«Full excl.»**: ese grupo a tope y el resto a negro.

Ctrl+clic en un botón de flash lo deja encendido hasta el siguiente clic. **«Restablecer el mezclador»** lo devuelve todo a su estado normal. El borde de una tira se pone naranja con un flash o un full, y rojo con blackout.

> **Consejo:** el mezclador también puede ser una sección del [SHOW](show.md), y sus faders y botones se pueden [mapear](mapeos.md) a un controlador MIDI.

## Controles en el SHOW y en controladores

Las fichas de escena, los masters de los grupos, los botones ◀ ▶ y de liberar de cada grupo, los de la cabecera (◀ ▶, «Pausa» y «Liberar grupos») y los controles del mezclador:

- Con **Alt+clic** se copian a la página del SHOW que esté abierta (consulta [Show](show.md#llevar-cualquier-control-al-show)).
- En el modo Mapear se pueden asignar a un controlador (consulta [Mapeos](mapeos.md#modo-mapear)).
