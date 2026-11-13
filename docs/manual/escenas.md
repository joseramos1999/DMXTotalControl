# Escenas

![La página Escenas](img/escenas.png)

**Escenas** (F4) es la biblioteca de todo lo que has grabado: escenas estáticas, pasos, FX, efectos de color, racks, super escenas y cuelists. A la izquierda está la lista; a la derecha, el editor de la escena elegida, distinto según su tipo.

Casi todo se crea desde el programador: los efectos nuevos toman los fixtures seleccionados en [Programar](programar.md) y los pasos y cues, sus valores.

## La lista de escenas

- **«Buscar…»**: filtra por nombre (o por el nombre de la carpeta).
- **«Carpetas»** / **«Grupos»**: ver las escenas por [carpetas](escenas.md#carpetas) o por los grupos de Directo en los que están («Sin grupo» para el resto).
- **Plegar todo** / **Desplegar todo**: el botón de la derecha.
- **Filtro por tipo**: «Todos», «Estática», «Pasos», «Cuelists», «FX», «Colour FX», «Racks» y «Super escenas».
- **Ctrl+clic** y **Mayús+clic** seleccionan varias escenas para moverlas a una carpeta («A carpeta…»).

Cada escena lleva una franja con el color de su tipo. Haz clic en una para abrirla en el editor.

## Crear una escena

Pulsa **«Nueva»**:

| Opción | Qué crea |
|---|---|
| **Estática (desde el programador)** | Una escena estática con los valores del programador. |
| **Pasos (el programador es el primer paso)** | Una escena de pasos. |
| **Cuelist (el programador es la cue 1)** | Una cuelist. |
| **FX (en el programador)** | Te lleva a Programar › FX: los FX se hacen allí y se graban con «Grabar…». |
| **Colour FX (fixtures seleccionados)** | Un efecto de color sobre los fixtures seleccionados, en su orden. |
| **Mapping en el plano 2D (fixtures seleccionados)** | Un efecto de color colocado donde están los fixtures en el plano 2D. |
| **Rack de efectos** | Un rack vacío. |
| **Super escena (línea de tiempo con audio)** | Una super escena vacía. |
| **Importar desde un archivo…** | Escenas y efectos exportados de otro show (consulta [Importar y exportar](escenas.md#importar-y-exportar)). |
| **Cue lists de QLC+…** | Cuelists con los chasers, las secuencias y las listas de cues de un show de QLC+ (consulta [Cue lists de QLC+](escenas.md#cue-lists-de-qlc)). |

## La cabecera del editor

- **El nombre**: haz clic para cambiarlo.
- **La carpeta** (icono de carpeta): mover la escena a una carpeta.
- **«Probar»** / **«Parar»**: la reproduce ahora, fuera de los grupos. Las cuelists tienen su propio GO.
- **«Añadir al grupo…»**: la añade a un grupo de Directo.
- **Convertir a pasos** (racks, FX, Colour FX, Move FX, Curve FX y chasers): graba un ciclo en una escena de pasos nueva, solo con los canales a los que la escena da un valor: los demás, aunque tengan un valor por defecto (el pan o el zoom en el centro de muchas cabezas móviles), no entran en los pasos. Si un FX tiene retardos, graba el ciclo cuando ya han arrancado todas sus luminarias.
- **Convertir en cuelist** (pasos y chasers): consulta [Cuelists](escenas.md#cuelists).
- **«QLC+»** (cuelists): guarda una copia del show para QLC+; consulta [Cuelists y QLC+](escenas.md#cuelists-y-qlc).
- **Exportar**, **«Duplicar»** y **«Eliminar»** (se quita también de sus grupos y racks).

## Cómo suena una escena

Debajo de la cabecera está el panel de reproducción. Muestra lo que tiene sentido para cada tipo:

- **«Fundido de entrada»** y **«Fundido de salida»**.
- **«Tiempo de ciclo»** (o **«Tiempo por paso»** en un Colour FX) y **«Velocidad»** (de un ciclo por minuto a diez por segundo; doble clic: un ciclo por segundo), con **×½** y **×2**.
- **«Entrada gradual»** (estáticas y pasos): cada fixture empieza su fundido este tiempo después del anterior, en orden de selección.
- **«Modo de conducción»**: **«Duración»** (los tiempos normales), **«BPM»** (los tiempos pasan a beats y siguen el tempo del show, también los fundidos de cada paso; el tiempo de crossfade de un grupo de Directo y los fundidos de los bloques de una super escena siguen en segundos), **«BeatGO»** o **«Pulso»** (avanza con el botón BeatGO o con los picos de la entrada de audio). Con BeatGO o Pulso, **«Divisiones»** (/2, ×2).
- **«Ritmo marcado»** (secuencias, chasers y escenas FX con FX en «BPM del show»): el ritmo marcado pulsando su botón varias veces seguidas ([Directo](directo.md#ritmo-de-una-escena-con-pasos)), en BPM. Mientras lo tiene, sus pasos van a ese ritmo y no por sus tiempos; en una escena FX, sus FX en «BPM del show» dan un ciclo en cada pulso en lugar de seguir el BPM del show. **«Quitar»** vuelve a sus tiempos (o al BPM del show). Con «BPM», un beat es un paso de ese ritmo: un fundido de 0,5 b dura medio paso.
- **«Inicio»**: **«Reproduciendo»** o **«En pausa, pulsar otra vez para reproducir»**.
- **«Se reproduce»**: **«Siempre»** o **«N veces»**, **«y luego»** **«Liberarla»** o **«Mantener el último fotograma»**, **«y reproducir»** «Nada», «La siguiente escena del grupo», «La escena anterior del grupo» o cualquier otra escena.

## Tipos de escena

### Estática

Guarda valores fijos. Muestra cuántos canales y fixtures tiene y las paletas que usa («Usa estas paletas: cuando cambian, la escena cambia también»).

Para cambiarla:

1. Pulsa **«Editar en el programador»**: se carga en Programar.
2. Cambia lo que quieras.
3. Vuelve a Escenas y pulsa **«Guardar el programador en esta escena»**.

Si guardas FX en una escena estática, pasa a ser una escena FX.

### Pasos

Una sucesión de looks, cada uno con su **fundido de entrada**, su **espera** y su **fundido de salida**.

- **«Añadir paso»**: el programador se convierte en un paso nuevo tras el seleccionado.
- **«Guardar en el paso»**: guarda el programador en el paso seleccionado.
- **«Cargar paso»** (o doble clic en un paso): lo lleva al programador para verlo y cambiarlo.
- **Duplicar**, **▲ ▼** para reordenar y **eliminar** el paso.
- El botón **∞** de la espera hace que el paso espere un GO («Hasta GO»).

**Ctrl+clic** y **Mayús+clic** seleccionan varios pasos: al cambiar un tiempo, cambia en todos. Los pasos usan las paletas **por referencia**, como las escenas y las cues: si actualizas una paleta, cambian los pasos grabados con ella (el que está sonando, al momento). El icono de paleta de un paso dice cuáles usa.

### FX

Una escena FX tiene **valores base** y una o varias **capas de FX**. Se crea grabando desde el programador con FX (consulta [FX](programar.md#fx)).

- A la izquierda, la lista de FX: parámetro, fixtures y velocidad (en «BPM del show», el ritmo marcado de la escena si lo tiene). La papelera quita una capa.
- A la derecha, los controles de la capa elegida (los mismos que en Programar) y su vista previa animada. Los cambios se oyen en directo mientras la escena suena.
- **«Enlazar»**: las capas enlazadas comparten velocidad y timing.
- Si algunas luminarias hacen el FX a su manera, se avisa: si cambias aquí la forma, el tamaño, el sentido o la fase, todas harán lo mismo.
- **«Por luminaria»**: el desfase y el retardo de cada luminaria, también en directo. Aquí «Repartir…» y «Sin desfase ni retardo» van sobre todas (consulta [Desfase y retardo por luminaria](programar.md#desfase-y-retardo-por-luminaria)).
- Para añadir capas o cambiar los valores base: **«Editar en el programador»** y luego **«Guardar el programador en esta escena»**.

### Colour FX y mappings

Un **Colour FX** es un efecto de píxel (más de 60: degradados, fuego, meteoros, estrobos, ondas…) que recorre los fixtures en el orden de selección.

- **«Efecto»**: cuál.
- **«Actúa sobre»**: «Color (RGB)», «Blanco», «Ámbar», «UV», «Dimmer (valor)» o «Shutter».
- **«Colores»**: hasta 5, según el efecto. Haz clic en un color para cambiarlo (muestras o tono), o «Sin color».
- **«Texto»**: en los efectos de texto, el texto que recorre los fixtures.
- **«Imagen»** y **«Animación»**: en los efectos de imagen, el archivo y «Fija», «Desplazar a los lados», «Desplazar arriba y abajo» o «Animada (GIF)».
- **«Disposición»**: **«En fila»** (en orden de selección) o **«En el plano 2D»** (donde están los fixtures).
- **«Parámetros»**: los propios de cada efecto.
- **«Usar la selección del programador»**: cambia sus fixtures por los seleccionados.

Arriba se ve el color real de cada cabeza mientras suena (pulsa «Probar» para verlo).

Un **mapping en el plano 2D** es un Colour FX que ya nace con la disposición del plano: el efecto, el texto o la imagen se colocan sobre el escenario tal como están los fixtures.

### Move FX y Curve FX

Son efectos del módulo anterior: los **Move FX** mueven pan/tilt con una forma y los **Curve FX** dan una onda (seno, triángulo, rampas, cuadrada, aleatoria, estrobo…) a canales. Los encontrarás en shows antiguos o en shows de QLC+; ya no se crean desde «Nueva».

Siguen sonando igual y tienen su editor (forma, ancho, alto, rotación, posición, fase, reparto entre fixtures, simétrico, relativo…). Un aviso ofrece **«Convertir a FX»**: pasan al editor de FX, también en el programador, y los grupos, botones del SHOW y mapeos los conservan. La fase de un Move FX pasa a la «Fase» del FX, y el desfase de inicio de cada fixture, a su «Desfase» ([Desfase y retardo por luminaria](programar.md#desfase-y-retardo-por-luminaria)).

### Rack de efectos

Varias escenas y efectos que suenan juntos como uno solo, cada uno en sus fixtures.

- En **«Añadir»** (a la derecha), haz clic en una escena para meterla en el rack.
- En **«En el rack (N)»**, la **×** la quita.

Un rack no puede contener una escena que lo reproduzca a él.

### Chasers

Los chasers de shows de QLC+ suenan con normalidad, pero no tienen editor: conviértelos a pasos (graba un ciclo) o en cuelist con los botones de la cabecera.

## Super escenas

Una **super escena** es una línea de tiempo con pistas de escenas y de audio. Se lanza como cualquier escena (en un grupo, en el SHOW, con un mapeo). Su editor ocupa toda la página; el botón de la lista, arriba a la izquierda, vuelve a la lista de escenas.

### La barra de herramientas

- **«Reproducir»** / **«Parar»** (**Espacio**): desde el cursor. **⏮** vuelve al principio. Al lado se ve el tiempo (y el compás).
- Las flechas **Deshacer** y **Rehacer** (**Ctrl+Z** / **Ctrl+Y**). Consulta [Deshacer](escenas.md#deshacer).
- **«Pegar»** (**Ctrl+V**): los bloques copiados, en el cursor y desde la pista seleccionada. Consulta [Copiar y pegar](escenas.md#copiar-y-pegar).
- **«Tiempo»** / **«Compases»**: la rejilla. En compases, el **BPM**, el compás (**4/4**, **3/4**, **2/4**) y **«Compás 1 aquí»** para que el primer compás empiece en el cursor y cuadre con la música.
- **«Ajustar»**: los bloques y puntos se ajustan a la rejilla.
- **«Fin aquí»**: la super escena acaba en el cursor (la **×** quita el marcador: acaba con su último bloque). **«Bucle»**: vuelve a empezar.
- **«+ Pista»** y **«+ Audio»** (un archivo de audio en la pista seleccionada, en el cursor).
- **«Valores estáticos»**: **«Tomar los valores del programador»** (suenan mientras la super escena está en marcha) o **«Borrar los valores estáticos»**.
- **Zoom** − y +.

### Poner escenas en la línea de tiempo

1. Pulsa **«+ Pista»** si no hay ninguna.
2. Arrastra una escena de la lista de la izquierda a una pista. O selecciona una pista, coloca el cursor y haz **doble clic** en la escena: se pone en el cursor.

Los bloques se pueden solapar.

Una cuelist **«Timecode»** con su reloj propio en un bloque sigue la línea de tiempo: el 00:00 de su reloj es el inicio del bloque. Ponla en otra pista, junto al bloque de audio de la canción y desde el mismo punto: cada cue suena en su momento de la canción. Si reproduces desde otro punto, la cuelist va directa a la cue de esa hora. Estirada, su reloj va más deprisa o más despacio para que la cuelist dure el bloque.

### Trabajar con los bloques

- **Arrastra** un bloque para moverlo, también a otra pista. Arrastra sus **bordes** para cambiar su duración.
- Con un solo bloque seleccionado, los **tiradores** de las esquinas superiores son sus fundidos de entrada y de salida.
- Al alargar un bloque, la escena **se repite en bucle** (se ven marcas en cada vuelta) o se **estira** para que un ciclo dure todo el bloque. Se cambia en el menú del bloque o en la barra de abajo («En bucle» / «Estirada»).
- **Clic derecho**: «Repetir la escena en el bloque» / «Estirar la escena al bloque», «Copiar», «Cortar», «Duplicar», «Bloquear» / «Desbloquear», «Abrir la escena» y «Eliminar». Sobre un bloque de la selección, el menú actúa sobre toda la selección; «Abrir la escena» abre la del bloque pulsado.
- **Doble clic** en un bloque abre su escena. **Supr** (o **Retroceso**) elimina los bloques seleccionados; los de pistas bloqueadas se quedan. **Ctrl+D** los duplica justo detrás: cada copia en su pista.
- La barra de abajo muestra el bloque seleccionado: **Inicio**, **Duración**, **Fundido de entrada**, **Fundido de salida**. Con varios, dice cuántos hay. Con uno o con varios, tiene **Copiar**, **Cortar**, **Duplicar** y **Eliminar**.

### Seleccionar varios bloques

- **Clic** en un bloque: solo ese. Su pista pasa a ser la pista seleccionada.
- **Ctrl+clic** añade un bloque a la selección o lo quita.
- **Mayús+clic** selecciona desde el último bloque elegido hasta este: los bloques de las pistas entre los dos, del inicio más temprano al final más tardío.
- **Arrastrar en una zona vacía** de las pistas dibuja un recuadro que selecciona los bloques que toca. Con **Ctrl** o **Mayús**, los suma a la selección.
- **Ctrl+A** selecciona todos los bloques de las pistas a la vista. Los de una pista que ocultas salen de la selección.
- Un clic en una zona vacía quita la selección (y mueve el cursor). Con Ctrl o Mayús, la selección se queda.

Al **arrastrar** un bloque seleccionado, se mueve toda la selección:

- Todos se mueven lo mismo en el tiempo: el bloque arrastrado se ajusta a la rejilla y los demás guardan sus distancias. El primero no pasa de 0.
- Suben o bajan el mismo número de pistas. Las pistas a las que van se iluminan.
- Los bloques bloqueados y los de pistas bloqueadas se quedan donde están.
- Si alguno caería fuera de las pistas o en una pista bloqueada, solo se mueven en el tiempo.
- Los bordes y los fundidos cambian solo el bloque bajo el ratón.

Pulsar un bloque de la selección y soltar sin moverlo lo deja solo a él seleccionado. Cada acción sobre la selección es un solo paso de [Deshacer](escenas.md#deshacer) («Mover 3 bloques»).

### Copiar y pegar

- **Ctrl+C** copia los bloques seleccionados. **Ctrl+X** los corta: los copia y los quita. Los de pistas bloqueadas se copian, pero se quedan.
- **Ctrl+V** o **«Pegar»** los pega en el cursor, desde la pista seleccionada (sin pista seleccionada, desde la primera).
- **Clic derecho** en una zona vacía de una pista: **«Pegar aquí»** pega en ese punto y en esa pista. **«Seleccionar todo»** hace lo mismo que **Ctrl+A**.
- El primer bloque cae en el cursor y los demás guardan sus distancias en el tiempo. El de la pista más alta va a la pista seleccionada y los demás guardan sus distancias en pistas.
- Si faltan pistas, se añaden al final. Una pista oculta que recibe bloques se muestra.
- Los bloques pegados guardan su duración, sus fundidos, el bucle o estirado y su color. Llegan desbloqueados, como con Duplicar.
- Lo pegado queda seleccionado y el cursor pasa a su final: pulsa **Ctrl+V** otra vez y la siguiente copia va justo detrás.
- Puedes pegar en otra super escena del mismo show: ábrela y pega. Las escenas no se copian: el bloque pegado reproduce la misma escena.
- Un bloque de audio pegado en la misma super escena reproduce el mismo audio. En otra super escena tiene su propio audio, leído del mismo archivo: borrarlo allí nunca se lleva el original. Si cortaste el bloque, su audio se lee otra vez del archivo.
- No se pega nada en una pista bloqueada, ni una escena que reproduce esta super escena (por ejemplo, un rack que la contiene). Un aviso lo dice.
- Lo copiado se guarda mientras el show está abierto. Abrir otro show o empezar uno nuevo lo vacía. Una escena que borras de la lista sale de lo copiado.
- Pegar y cortar son un solo paso de [Deshacer](escenas.md#deshacer). Lo pegado con la super escena en marcha suena desde el siguiente «Reproducir».

### Audio

Los bloques de audio (wav, flac, ogg, aiff y mp3 si el sistema lo admite) muestran su **forma de onda**, que se calcula en segundo plano. Hace falta una pista sin bloquear para añadirlo.

### Pistas y automatización

En la cabecera de cada pista:

- **M**: silenciar (la pista no suena).
- **Candado**: bloquear (sus bloques y puntos no se mueven).
- **D**: la línea de automatización del **dimmer** (en una pista de audio, el volumen).
- **φ**: la línea de automatización de la **fase** de sus efectos.
- **⋯**: «Renombrar» (también con doble clic en el nombre), «Subir», «Bajar», «Borrar la automatización de esta línea» y «Eliminar la pista».
- La flecha de la derecha oculta o muestra la pista.

En una línea de automatización: **clic** añade un punto, **arrastrar** lo mueve, **doble clic** lo borra y **clic derecho** alterna entre recta y curva suave. Sin puntos, el dimmer va a tope y la fase a 0.

### Moverse por la línea de tiempo

- Clic o arrastrar en la **regla** mueve el cursor; **doble clic** en la regla reproduce desde ahí. Clic en una pista vacía también mueve el cursor.
- **Ctrl+rueda**: zoom. **Mayús+rueda**: desplazarse en el tiempo. La rueda sola sube y baja por las pistas.

### Deshacer

- **Ctrl+Z** deshace el último cambio. **Ctrl+Y** (o **Ctrl+Mayús+Z**) lo rehace. Las flechas de la barra de herramientas hacen lo mismo; al pasar el ratón dicen qué cambio deshacen o rehacen.
- Cada super escena tiene su propio historial, de hasta 100 pasos. Dura mientras el show está abierto: guardar no lo borra, abrir otro show o empezar uno nuevo sí.
- Cuentan los cambios de pistas, bloques, automatización, rejilla, BPM, fin, bucle y valores estáticos. Varios cambios del mismo campo de la barra de abajo (o del BPM) en menos de un segundo son un solo paso.
- No cuentan ocultar o mostrar pistas, el zoom, el cursor, «Ajustar» ni la selección.
- Mientras escribes en un campo, Ctrl+Z deshace lo escrito en el campo.
- Funciona con la super escena en marcha. Un bloque que se quita deja de sonar; uno que vuelve suena desde el siguiente «Reproducir».
- Un bloque de audio vuelve con su audio, que se lee otra vez de su archivo.
- Los bloques de una escena borrada de la lista no vuelven.

## Cuelists

Una **cuelist** es una lista de cues que avanzan con GO. Funciona con **tracking**: cada cue guarda **solo lo que cambia** y el resto sigue de las cues anteriores. Un GO solo cambia los canales que cambian: sin bajones y sin retomar lo que no toca.

Con otras cuelists y escenas de la misma prioridad, cada canal que no es de intensidad (posición, gobo, rueda de color…) lo da el último que lo cambió: el GO que lo cambia lo toma, y lo que la cuelist solo mantiene de cues anteriores no lo retoma. Al liberarlo, el canal hace un fundido hacia quien lo tenía antes. Consulta [Prioridades](directo.md#prioridades).

Las cuelists se graban desde el programador, se operan en [Playbacks](playbacks.md) y se editan aquí.

### Opciones de la cuelist

Arriba del editor:

- **«Tipo»**: cómo se reproduce la cuelist. Una línea junto a los botones dice lo que hace el tipo elegido. En Playbacks y en este editor, una cuelist de otro tipo lleva su etiqueta: **SUB**, **CHASE**, **TC** u **OVR**.
  - **«Normal»** (por defecto): cada GO lanza la cue siguiente.
  - **«Chase»**: las cues se siguen solas, en bucle, a una velocidad (ver abajo).
  - **«Timecode»**: cada cue suena a su hora en un reloj (ver abajo).
  - **«Submaster»**: manda el fader de su playback (ver abajo).
  - **«Override»**: su fader hace un fundido de todo lo que controla entre lo que hay debajo y su cue (ver abajo).
- **«Tracking»**: encendido, cada cue guarda solo lo que cambia. Apagado, cada cue va por su cuenta.
- **«Tras la última cue»**: **«Bucle»** (vuelve a la primera), **«Liberar»** o **«Quedarse»**.
- **«Tiempo de liberación»**: el fundido al liberar la cuelist.
- **«Al liberar, volver a la cue 1»**: tras liberarla, el GO empieza otra vez por la primera cue.
- **«Se libera con el GO de otra cuelist»**: una canción que acaba cuando empieza la siguiente.
- **«Fundido de las cues nuevas»**: el fundido de la primera cue grabada; las siguientes toman los tiempos de la anterior.
- **«Atrás»**: los tiempos de BACK: **«Tiempos de la cue»** o **«Tiempo fijo»**.
- **«Prioridad»**.
- **«GO al ritmo»**: «No», «BeatGO» o «Pulso» dan un GO en cada división.
- **«Ritmo marcado»**: el ritmo marcado pulsando su botón varias veces seguidas, en BPM: un GO en cada pulso. Mientras lo tiene, sus Follow y Wait no cuentan. **«Quitar»** vuelve a sus tiempos.
- **«Mover en negro»**: cuando una cue ha terminado, los fixtures a oscuras que la cue siguiente enciende toman ya su posición, color y haz. En el GO solo suben de intensidad. **«Después de»** es la espera tras el final de la cue (0 s por defecto). **«Tiempo»** es lo que tardan esos movimientos (2 s por defecto).
  - No se mueve un fixture que ilumina otro playback o la propia cue.
  - En un fixture sin dimmer, su RGB es su luz: solo se preparan sus otros atributos.
  - Los canales de un FX no se preparan: el FX empieza con su GO.
  - Si eliges otra cue para el próximo GO («Siguiente» en [Playbacks](playbacks.md)), se prepara esa.
  - No vale en una cuelist «Chase» ni «Override».

En una ventana estrecha, parte de las opciones se pliega: **«Más opciones»** las muestra.

Una cuelist **«Submaster»** la maneja el fader de su [playback](playbacks.md#el-fader):

- Al subir el fader desde cero, arranca en su cue siguiente, al nivel del fader y nunca a tope.
- Al bajarlo a cero, se libera con su tiempo de liberación. Se puede cambiar en las [propiedades](playbacks.md#propiedades-de-un-playback) del playback.
- Mientras suena, GO y Atrás cambian de cue al nivel del fader.
- Liberada, GO y Atrás solo eligen la cue con la que arrancará (también Pausa/Atrás e «Ir»). El playback la muestra como siguiente («▸ 3»). Tras la última cue, GO vuelve a la primera; con «Quedarse», se queda en la última.
- Si la liberas con el fader arriba, bájalo a cero y vuelve a subirlo para arrancarla.
- El flash la lleva a tope mientras lo pulsas.
- En un grupo de Directo y en un botón del SHOW suena como una cuelist normal. También si no está en ningún playback o si su fader no la arranca («El fader lo arranca» apagado).

Una cuelist **«Chase»** recorre sus cues sola, en bucle:

- **«Velocidad»**: un paso en cada pulso, en BPM (de 10 a 600; 120 por defecto). Con **«BPM del show»**, sigue el tempo del show. **×¼**, **×½**, **×1**, **×2** y **×4** lo multiplican.
- **«Fundido»**: la parte del paso que dura el cambio, en %. Con 0 (por defecto), es un corte. Con **«Tiempos de la cue»**, cada cue usa su retardo y su fundido.
- **«Sentido»**: **«Adelante»**, **«Al revés»**, **«Ida y vuelta»** (da la vuelta en la primera y en la última cue) o **«Aleatorio»** (nunca repite un paso dos veces seguidas).
- GO pasa al paso siguiente al momento, y los siguientes cuentan desde ahí. Atrás va un paso en el otro sentido. Pausa lo detiene donde está.
- Un **«Ritmo marcado»** manda sobre su velocidad; **«Quitar»** vuelve a ella. Con BeatGO o Pulso, avanza solo en cada división.
- La [velocidad de las cues](playbacks.md#velocidad-de-las-cues) y la velocidad en vivo también lo aceleran o lo frenan.
- El tracking sigue valiendo: un paso puede cambiar solo el color y dejar el resto.
- Siempre vuelve a empezar: no tiene columna **DISPARO** ni **«Tras la última cue»**, y los enlaces de las cues no cuentan. Con un fundido en %, los tiempos de las cues se ven atenuados: no se usan.
- En una super escena, un bloque dura sus pasos por lo que dura un paso. En «Ida y vuelta», la ida y la vuelta.

En una cuelist **«Override»**, el fader de su [playback](playbacks.md#el-fader) la mezcla con lo que hay debajo:

- El fader hace un fundido de todo lo que controla: intensidades, colores, posiciones y gobos. A la mitad, los móviles están a medio camino. A cero, todo queda como si no estuviera.
- A tope, su cue sustituye a lo que hay debajo, también en las intensidades: un azul sobre un rojo da azul, no magenta.
- Las cues, los GO y los fundidos funcionan como en una cuelist normal. El fundido de una cue y el fader se multiplican.
- Al liberarla, todo vuelve con un fundido a lo que hay debajo, en su tiempo de liberación. Con el fader a cero sigue cargada, como cualquier cuelist.
- Al elegir «Override», una cuelist con la prioridad por defecto (5) pasa a 10. El programador sigue por encima.
- Una cuelist o una escena de su misma prioridad lanzada después se mezcla por encima como siempre: en las intensidades gana el valor más alto.
- Un canal con **«Fundidos»** apagado en los [límites del fixture](patch.md#límites) salta a su valor en cuanto el fader sale de cero.
- Sus FX suenan como en una cuelist normal: el fader no les hace este fundido.
- Si la cuelist ya suena, el cambio a «Override» (o desde «Override») vale desde su siguiente arranque.
- Usos típicos: un cegador o la luz de sala, una posición fija para un solo, una luz de emergencia en un fader.

Una cuelist **«Timecode»** lanza cada cue a su hora en un reloj:

- Cada cue tiene su **HORA** (horas:minutos:segundos.centésimas). La columna **HORA** sustituye a **DISPARO**. Escribe la hora («00:01:23.40», «1:23.4» o «83.4») o usa la rueda. Si la borras, la cue no tiene hora y el reloj no la lanza.
- **«Reloj»**: **«Reloj propio»** o **«Timecode MIDI (MTC)»** (ver abajo). Con el reloj propio, GO lanza la cuelist y pone en marcha el reloj en 00:00, o a la hora de la cue en la que está (la elegida con «Ir a la cue» o, con «Al liberar, volver a la cue 1» apagado, la siguiente tras liberarla).
- Suena la última cue de la lista cuya hora ha llegado. Antes de la hora de la primera cue, la cuelist no da nada.
- Al pasar a la cue siguiente, la cue hace su fundido. Un salto (hacia atrás o de varias cues) va directo al aspecto de esa hora, sin fundido.
- GO pasa a la cue siguiente con su fundido, y el reloj salta a su hora. Atrás va a la cue anterior y a su hora, sin fundido (con un tiempo fijo en **«Atrás»**, en ese tiempo). «Ir a la cue» pone el reloj en su hora. Con una cue sin hora, el reloj sigue como iba.
- Pausa para el reloj. La [velocidad de las cues](playbacks.md#velocidad-de-las-cues) y la velocidad en vivo lo aceleran o lo frenan.
- Sus Seguir, Esperar y enlaces no cuentan. «Tras la última cue» vale para el GO en la última cue.
- La cabecera y su playback muestran el reloj: **«TC 00:01:23.4»**.
- **«Poner en el reloj»** (el reloj de la barra de herramientas) pone la cue seleccionada a la hora del reloj mientras la cuelist suena.
- Una cue grabada mientras el reloj corre toma su hora. Con la cuelist liberada o en pausa, toma la hora de la cue anterior más 5 s.
- Un triángulo avisa de una cue cuya hora es anterior a la de la cue de antes. De las dos, solo suena la última.
- Al elegir «Timecode», las cues sin hora reciben una: 00:00 la primera y 5 s más que la anterior las demás.
- En una [super escena](escenas.md#super-escenas), el reloj es la línea de tiempo.

Con **«Timecode MIDI (MTC)»**, la cuelist sigue el timecode que un DAW, un reproductor de vídeo o un generador de timecode mandan a una entrada MIDI:

- Activa la entrada del dispositivo MIDI en [Mapeos › Dispositivos](mapeos.md#dispositivos). No hace falta ningún mapeo.
- **«Desfase»**: el timecode en el que está el 00:00 de la cuelist, en horas:minutos:segundos:fotogramas (por ejemplo «01:00:00:00»). Antes de esa hora, la cuelist no da nada.
- Al lado se ve el timecode que llega, **«MTC 01:00:12:05 · 25 fps»**, o **«Sin timecode MIDI»**.
- Frecuencias de fotograma: 24, 25, 29,97 drop-frame y 30.
- Al lanzarla, la cuelist espera al timecode y va directa a la cue de esa hora. Si es la primera cue, la cue hace su fundido.
- Si el timecode se para o se corta, la cuelist se queda en su cue. Si vuelve desde otra hora (un rebobinado o un salto), va directa a la cue de esa hora, sin fundido.
- GO, Atrás e «Ir a la cue» no hacen nada: manda el timecode. GO en la cuelist liberada la lanza.
- La velocidad de las cues no mueve el reloj; solo cambia los fundidos.
- También en una super escena, el reloj es el timecode.

### La tabla de cues

| Columna | Qué es |
|---|---|
| **CUE** | El número. Admite decimales: la 2.5 va entre la 2 y la 3. Doble clic para cambiarlo. Un lápiz indica que la cue está cargada en el programador. |
| **NOMBRE** | Su nombre. |
| **DISPARO** | **GO** (espera un GO), **Seguir** (cuando acaba la cue anterior, más un tiempo) o **Esperar** (un tiempo después del GO de la cue anterior). |
| **HORA** | Solo en una cuelist «Timecode», en lugar de DISPARO: la hora de la cue en el reloj. Un triángulo avisa si es anterior a la de la cue de antes. |
| **RETARDO** | Cuánto espera la cue antes de empezar su fundido. |
| **FUNDIDO** | El tiempo de fundido. |
| **BAJADA** | El fundido de los dimmers que bajan. «=» es igual que el fundido. |
| **CAMBIOS** | Cuántos canales cambia, y las marcas **FX**, **[R]** (valores liberados), el icono de paleta si usa paletas, un reloj si tiene tiempos propios o una curva, un eslabón si tiene un enlace y un rayo si tiene acciones (al pasar el ratón se ven). |
| **S** | **Saltar** (solo esta cue): lo que cambia no pasa a las siguientes. |
| **B** | **Bloqueo**: Grabar y Actualizar guardan también los valores iguales a los que hereda. |
| **MIB** | Solo con «Mover en negro»: la cue se prepara mientras sus fixtures están a oscuras. Desmarcada, sus movimientos se ven en su GO. |
| **NOTA** | Una nota para ti. |

Para cambiar un tiempo, haz clic en él y escribe los segundos, o usa la **rueda** del ratón sobre el tiempo pulsado: ±0,1 s (con Mayús, 1 s). Con **Ctrl+clic** o **Mayús+clic** seleccionas varias cues y cambias sus tiempos, su disparo o sus marcas a la vez.

La cue que suena lleva una barra verde con el avance del fundido; la siguiente, un borde verde. Arriba se ve el estado («Q 3 · Verso ▸ 4», con «↺ 1/3» si la siguiente es el salto de un enlace, o «Liberada · GO lanza la 1») y los botones **Atrás**, **GO** y **liberar**, los mismos que los de su playback.

Mientras una cue hace su fundido, la cabecera muestra también **«Tiempo en vivo»**: el tiempo de esa cue, que puedes cambiar mientras corre (escribiendo otro, con **«×½»**, **«×2»** o **«Ahora»**), y el botón de guardarlo en la cue. Funciona como en [Playbacks](playbacks.md#tiempo-en-vivo).

### Grabar y editar cues

- **«Grabar la cue N»**: el programador se convierte en una cue nueva tras la seleccionada, solo con lo que cambia respecto a lo que hereda.
- **«Actualizar ▾»**: fusiona el programador con la cue seleccionada: **«Actualizar la cue N (sigue en las siguientes)»** o **«Solo esta cue (las siguientes mantienen su aspecto)»**.
- **«Cargar ▾»** (o doble clic en una cue): la lleva al programador: **«Lo que muestra el escenario en la cue N»** o **«Solo lo que cambia la cue N»**. Después, actualízala desde la barra «Editando la cue…» de Programar.
- **«Sacar ▾»**: con los canales del programador, **«Quitar de la cue N los canales del programador (vuelven a heredar)»** o **«[Liberar] los canales del programador en la cue N»** (desde esa cue vuelven a lo que hay debajo: otras escenas o su valor de reposo).
- **Copiar** al final: **«Copiar lo que cambia la cue N»** o **«Copiar todo el aspecto de la cue N (cue de bloqueo)»**.
- **«Número…»**: otro número para la cue seleccionada. Mover una cue cambia lo que heredan las que van detrás.
- **Eliminar**: con **«Mantener el aspecto de las cues siguientes»** marcada, lo que cambia la cue pasa a la siguiente y las de detrás se ven igual.

En una lista estrecha (o en un idioma con palabras largas), «Actualizar», «Cargar» y «Sacar» se quedan con su icono y su flecha **▾**; al pasar el ratón se ve su nombre.

El clic derecho en una cue tiene **«Ir a la cue N»**, las dos formas de cargar, **«Actualizar con el programador»**, **«Tiempos…»**, **«Enlace y acciones…»**, **«Número…»** y **«Eliminar…»**.

Las cues pueden usar **paletas por referencia** y llevar **capas de FX**, que se heredan de cue en cue.

### La hoja de tracking

**«Detalles de la cue»** tiene tres pestañas: **«Tracking»**, **«Tiempos»** y **«Enlace y acciones»**. Si la ventana es ancha, el panel se ve solo al lado de las cues.

**«Tracking»** muestra lo que el escenario tiene en la cue seleccionada, fixture a fixture («En escena: cue N»).

- Las fichas de colores dicen qué es cada valor: **«Cambios»** (lo cambia esta cue), **«Heredados»** (viene de una cue anterior, que se indica: «Q2»), **«Bloqueo»** y **«Liberados»** (tachados, con [R]).
- Arriba se ven las paletas que usa la cue.
- Haz clic en canales para elegirlos y pulsa **«Liberar»** (los pone en [RELEASE] desde esta cue) o **«Sacar»** (vuelven a heredar de las cues anteriores).

### Tiempos de la cue

Dentro de una cue, cada atributo y cada fixture pueden tener su propio retardo y su propio fundido. Selecciona una cue (o varias) y abre **«Detalles de la cue»** › **«Tiempos»**, o haz clic derecho › **«Tiempos…»**. Con varias cues seleccionadas, los cambios valen para todas.

- **«Tiempos por atributo»**: **Intensidad**, **Color**, **Posición** y **Haz**, cada uno con RETARDO y FUNDIDO. Vacío o «=» es el tiempo de la cue. El strobe va con Intensidad; gobo, prisma, zoom, foco e iris van con Haz.
- **«Tiempos por fixture»**: **«+ Fixtures seleccionados»** añade los fixtures seleccionados en Programar, con los tiempos de la cue. Después cambia sus tiempos. **×** quita uno y **«Quitar»**, todos.
- El tiempo de un fixture gana al de su atributo, y el de un atributo gana al de la cue.
- Los dimmers que bajan usan el fundido de su fixture, si lo tiene. Si no, la BAJADA de la cue; con «=», el fundido de Intensidad.
- Una cue dura lo que su tiempo más largo: Seguir, la barra verde y la duración en una super escena lo cuentan. También cuenta el tiempo de un atributo que la cue no cambia.
- Un solo tiempo para todo, como antes: el fundido de entrada de un grupo o de un bloque de super escena en la primera cue, y un tiempo fijo de **«Atrás»**. Con **«Tiempos de la cue»**, BACK usa todos los tiempos de la cue a la que vuelve.
- Las capas de FX entran con el fundido de la cue.
- Una cue nueva toma los tiempos de atributo de la anterior, pero no los de sus fixtures. Copiar una cue copia todos sus tiempos.

Ejemplo: la cue 2 cambia color y posición con fundido 0, y Posición tiene retardo 1 s y fundido 3 s. Con GO, el color cambia al momento; un segundo después, las cabezas móviles se mueven durante 3 s.

**Curvas.** **«Curva de los fundidos»** dice cómo avanzan los fundidos de la cue:

- **Lineal** (por defecto): a la misma velocidad todo el tiempo.
- **Suave**: empieza y acaba despacio.
- **Arranque lento**: apenas se mueve al principio y acelera al final.
- **Arranque rápido**: avanza casi de golpe y después se asienta.

La curva vale para todo lo que funde la cue: dimmers que suben y que bajan, colores, movimientos y los canales que libera. Cada atributo tiene también su **CURVA**; «=» es la de la cue. El tiempo de liberación de la cuelist, un tiempo fijo de **«Atrás»**, el fundido de entrada de un grupo y las capas de FX son siempre lineales. El reloj de la columna CAMBIOS también sale si la curva de la cue no es lineal.

**Abanico de tiempos.** En **«Tiempos por fixture»**, **«Abanico»** reparte un retardo o un fundido entre varios fixtures:

- Elige **Retardo** o **Fundido**, y los tiempos **de** y **a**.
- Elige la forma: **Uno tras otro**, **Desde el centro** o **Desde los extremos**.
- Elige los fixtures: **«Seleccionados en Programar»**, en el orden en que los seleccionaste, o **«De esta cue, por número»**.
- **«Aplicar»** da a cada fixture su tiempo en las cues seleccionadas. Su otro tiempo no cambia.

Con 5 fixtures y un retardo de 0 a 2 s, **Uno tras otro** da 0; 0,5; 1; 1,5 y 2 s. **Desde el centro** da 2; 1; 0; 1 y 2 s. **Desde los extremos** da 0; 1; 2; 1 y 0 s. Si «de» es mayor que «a», el orden se invierte. El abanico escribe tiempos fijos, no una regla: después puedes cambiar cada fila.

### Enlaces y acciones

Selecciona una cue y abre **«Detalles de la cue»** › **«Enlace y acciones»**, o haz clic derecho › **«Enlace y acciones…»**. Con varias cues seleccionadas, se edita la última que elegiste.

**Enlace.** **«Después de esta cue»** dice qué cue suena tras ella: **«La cue siguiente»** (lo normal) u otra cue de la lista. **«Veces»** dice cuántas veces salta el enlace antes de seguir; 0 es siempre. Un enlace nuevo empieza con 1.

- Con la cue 5 → cue 2, 3 veces, los GO tocan 1 2 3 4 5, 2 3 4 5, 2 3 4 5, 2 3 4 5 y 6: las cues 2 a 5 suenan 4 veces. Debajo, una frase lo explica: «Las cues 2 a 5 suenan 4 veces y después la 6.»
- Con 0 (siempre), las cues se repiten hasta que vas a otra cue.
- Un enlace hacia delante salta cues; un enlace a la propia cue la repite.
- Vale el disparo de la cue a la que salta: si la cue 2 tiene Seguir, el bucle avanza solo.
- Esa cue es la siguiente en todas partes: el borde verde de la tabla, la cabecera («▸ 2 ↺ 1/3»: el primer salto de tres; solo «↺» si es siempre), los playbacks, el SHOW, el móvil y los LED del controlador.
- BACK e «Ir a la cue» no cuentan saltos. Liberar la cuelist, o lanzarla otra vez, empieza la cuenta de nuevo.
- Hechos todos los saltos, la lista sigue tras la cue. La próxima vez que llega a ella, la cuenta empieza de nuevo.
- En la última cue, el enlace va antes que **«Tras la última cue»**.
- En una cuelist «Chase» o «Timecode», los enlaces no cuentan: la pestaña lo dice. El enlace se guarda para cuando la lista vuelva a ser «Normal».
- Si eliminas la cue a la que salta, el enlace desaparece y un aviso lo dice.

**Acciones al empezar.** **«+ Añadir una acción ▾»** añade lo que hace la cue cuando empieza:

- **«GO de una cuelist»**, **«Ir a una cue…»** de otra cuelist y **«Liberar una cuelist»**.
- **«Lanzar una escena»** y **«Parar una escena»**, en su grupo si lo tiene (como los botones del SHOW). No cambian la escena seleccionada.
- **«Liberar un grupo»** (o todos) y **«Liberar todos los playbacks»**.
- **«Poner el BPM»**, de 20 a 300.

Cada fila se cambia ahí mismo; la **×** la quita. Las acciones se hacen en orden cuando la cue empieza con GO, Seguir, Esperar o «Ir a la cue», también la primera cue al lanzar la cuelist. No se hacen con BACK, cuando una edición vuelve a aplicar la cue ni cuando una super escena empieza la cuelist a mitad.

- La propia cuelist no aparece: para eso está el enlace.
- Una acción sobre una escena o cuelist eliminada muestra «(eliminada)» y no hace nada.
- Si dos cuelists se lanzan una a otra, el bucle se para: una cuelist lanzada por una acción no lanza, con sus acciones, una cuelist anterior de esa cadena. Además, la misma cue no repite sus acciones antes de 200 ms.
- Copiar una cue no copia su enlace ni sus acciones.

### Deshacer

Las flechas **Deshacer** y **Rehacer**, tras la papelera, deshacen y rehacen las ediciones de la cuelist, una a una. También **Ctrl+Z** y **Ctrl+Y** (o **Ctrl+Mayús+Z**). Al pasar el ratón dicen qué deshacen: «Deshacer: Grabar la cue 3».

- Se deshace todo lo que cambia las cues o las opciones de la cuelist: grabar (también desde Programar, un playback o la línea de comandos), actualizar, sacar, liberar, copiar, eliminar, el número, los tiempos, el disparo, el nombre, la nota, las marcas S y B, el enlace, las acciones y las opciones de arriba, salvo «GO al ritmo» y «Ritmo marcado».
- Cada cuelist guarda sus 50 últimos pasos.
- Varios giros de rueda (o valores escritos) en el mismo tiempo, con menos de 2 segundos entre ellos, son un solo paso. Si vuelves al valor de partida, el paso desaparece.
- Después de deshacer queda seleccionada la primera cue que cambia. Si la cuelist suena, el escenario cambia al momento.
- En un campo con texto escrito, Ctrl+Z deshace primero ese texto.
- No se deshace crear la cuelist ni convertir pasos en cuelist. Deshacer mantiene los valores actuales de las paletas y no recupera los canales de fixtures borrados.
- Una prioridad o un tiempo de liberación cambiados en otro sitio (Directo, Playbacks) se quedan como están.
- El historial no se guarda con el show. Nuevo y Abrir lo vacían, y también eliminar la cuelist.

### Convertir pasos o un chaser en cuelist

En una escena de pasos, el botón **«Convertir en cuelist»** de la cabecera convierte cada paso en una cue con lo que cambia respecto al anterior; los pasos con tiempo se encadenan con **Esperar**. Sus grupos, botones del SHOW y mapeos pasan a reproducir la cuelist. Los pasos desaparecen. Las cues siguen usando las paletas de los pasos.

En un chaser, cada cue lleva la escena de su paso (los Move FX pasan a FX) y libera lo que el paso anterior tenía y este no; los pasos que no son escenas ni efectos quedan en una cue vacía con una nota. Sus escenas se quedan.

Un ping-pong va y vuelve; un orden aleatorio suena en orden. Los tiempos en beats pasan a segundos con el BPM de ese momento. Los pasos de una copia para QLC+ vuelven con el número, el nombre y la nota de cada cue.

### Cuelists y QLC+

QLC+ no lee las cuelists: si abres este show en QLC+, no aparecen. Para llevarlas a QLC+, pulsa **«QLC+»** en la cabecera de una cuelist. **«Guardar una copia para QLC+»** escribe otro `.qxw` en el que cada cuelist marcada es una lista de pasos de QLC+:

- **Sequence** (por defecto): un solo elemento en QLC+.
- **Chaser**: una escena por cue, en una carpeta con el nombre de la cuelist.

Con **«Una lista de cues para cada una en la Consola Virtual»** marcada, la copia se abre en la Consola Virtual de QLC+, con un marco «Cuelists» y una lista de cues por cuelist: «Siguiente» es el GO, «Anterior» el BACK y «Stop» la libera. Tu show no cambia.

La casilla **«Con sus teclas y botones del controlador»** da a cada lista de cues las teclas y los botones del controlador de su cuelist: GO, Atrás, Pausa/Atrás y Liberar pasan a «Siguiente», «Anterior», «Reproducir» y «Stop». Cuentan los mapeos de la cuelist y los del playback que la tiene en la página actual. Cada botón de QLC+ lleva una tecla y una entrada. No pasan los controles OSC ni los de un dispositivo sin activar; la ventana lo avisa.

Cada paso lleva todo lo que muestra el escenario en esa cue, y su nota dice el número y el nombre («Q2.5 · Verso»). El fundido de la cue pasa a ser el de entrada del paso, y la bajada, el de salida. GO deja el paso esperando a «Siguiente»; Esperar y Seguir pasan a un tiempo. En una cuelist «Chase», los pasos van solos, en bucle y en su sentido: cada uno dura un paso de su velocidad, con su fundido. En una cuelist «Timecode», cada paso dura hasta la hora de la cue siguiente. Los pasos llevan el ID de la cuelist: las super escenas y los racks que la reproducen reproducen los pasos.

Debajo de cada cuelist, la ventana avisa de lo que no pasa:

- «Chase» con «BPM del show» o con un ritmo marcado: los pasos mantienen un BPM fijo, por su multiplicador: el del show en ese momento con «BPM del show», si no el de la cuelist. No siguen el ritmo marcado.
- «Timecode»: los pasos corren desde el primer GO; no siguen el timecode MIDI ni la línea de tiempo de una super escena. El primer paso suena con el GO, no a la hora de su cue. En una cue sin hora, se paran hasta un GO.
- «Submaster»: los pasos empiezan con un GO, no con el fader.
- «Override»: los pasos se mezclan con lo que hay debajo como los demás.
- Los FX: solo sus valores base.
- El retardo, también el de un atributo o un fixture: el paso empieza enseguida.
- Una bajada distinta del fundido: solo aproximada.
- Los tiempos por atributo y por fixture: solo el fundido de la cue.
- Las curvas de los fundidos: solo lineales.
- Un Esperar más corto que el fundido, o en «Chase» y «Timecode» un paso más corto que su fundido: el paso siguiente espera a que acabe el fundido.
- Los enlaces: los pasos suenan en orden. Si la última cue enlaza siempre con la primera, los pasos vuelven a empezar.
- Las acciones de las cues: no se hacen.
- En Sequence, los valores liberados van a su valor de reposo. En Chaser, quedan en manos de QLC+.
- Con «Quedarse», GO en la última cue para los pasos.
- GO al ritmo y ritmo marcado: los pasos no siguen el ritmo.
- «Mover en negro»: los fixtures se mueven mientras suben.
- «Se libera con el GO de otra cuelist»: esas listas van a un marco «Cuelists (una a la vez)», donde lanzar una para las demás.

Una cuelist que ya tiene su lista de cues en la Consola Virtual (un show de QLC+ cuyos pasos convertiste en cuelist) no lleva otra: la de antes la vuelve a lanzar. Prueba la copia en QLC+ antes de usarla en directo: QLC+ hace los fundidos a su manera.

### Cue lists de QLC+

**«Nueva» › «Cue lists de QLC+…»** convierte en cuelists los chasers, las secuencias y las listas de cues de la Consola Virtual de un show de QLC+: del show abierto o de otro («De otro show…»). Marca las que quieres convertir. Salen primero las listas de cues de la Consola Virtual, marcadas; después, los chasers y las secuencias que ninguna lista usa. Las que ya tienen una cuelist con su nombre salen sin marcar.

Cada paso es una cue con lo que cambia; los pasos con tiempo se encadenan con Esperar y los que esperan son GO. Una lista de cues de la Consola Virtual da su nombre a la cuelist.

- **«Sustituir los originales»** (solo en el show abierto): la cuelist ocupa su lugar y su ID. Sus grupos, botones del SHOW y mapeos pasan a la cuelist, pero QLC+ ya no los ve. Sin marcar, los originales se quedan y el show sigue sonando igual en QLC+.
- **«Ponerlas en los playbacks libres de esta página»**. Si no caben todas, el aviso dice cuántas se quedan fuera.
- **«Traer sus teclas y botones del controlador como mapeos»**: «Siguiente», «Anterior», «Reproducir» y «Stop» de cada lista de cues pasan a GO, Atrás, Pausa/Atrás y Liberar de su cuelist. No pasan los controles OSC, las teclas con Ctrl, Alt o Mayús, las entradas de un universo sin controlador en este show ni los controles que ya tienen un mapeo. El aviso dice cuántos mapeos se crean y qué controles se quedan fuera. Un botón de una Akai APC trae sus LEDs, como al aprenderlo.

De otro show solo entran las cuelists, en los fixtures de este show que coinciden con los suyos (primero la misma dirección y el mismo modelo). El aviso dice cuántos fixtures no están.

No pasan: el orden aleatorio (suenan en orden), los beats (pasan a segundos con el BPM actual), el fader lateral de las listas de cues y los pasos que no son escenas ni efectos. Las listas de cues del formato antiguo de QLC+ no se pueden convertir: ábrelas y guárdalas antes en QLC+.

Al abrir un show de QLC+ con listas de cues y sin cuelists, el aviso «Abierto» te recuerda dónde convertirlas.

## Carpetas

Las carpetas ordenan la lista de escenas (también en el SHOW, en las super escenas, en los racks y al añadir escenas a un grupo).

- **Crear**: el botón de carpeta con **+** junto al buscador, o **«Nueva carpeta…»** en el menú de una escena.
- **Meter escenas**: arrástralas sobre la carpeta, o clic derecho › **«Mover a la carpeta»**. Funciona con varias escenas seleccionadas.
- **Sacar**: clic derecho › **«Sacar de su carpeta»**.
- **Renombrar o eliminar**: el botón **⋯** de la carpeta (o clic derecho): **«Renombrar la carpeta»** y **«Eliminar la carpeta (sus escenas se quedan)»**.
- Haz clic en la cabecera de una carpeta para plegarla o desplegarla.

Las carpetas se guardan con el show.

## Importar y exportar

- **Exportar**: el botón de exportar de la cabecera guarda la escena en un archivo `.dtcfx` («Efectos de DMX Total Control») para cargarla en otros shows. Funciona también con racks.
- **Importar**: **«Nueva» › «Importar desde un archivo…»**. Los fixtures se emparejan con los de este show; si alguno del archivo no está, su parte se omite y se avisa.
- **Las escenas de otro show entero**: en [Patch › Importar](patch.md#importar-de-otro-show), «Escenas de otro show…».
- **Para QLC+**: «QLC+» en la cabecera de una cuelist ([Cuelists y QLC+](escenas.md#cuelists-y-qlc)).
- **De QLC+**: «Nueva» › «Cue lists de QLC+…» ([Cue lists de QLC+](escenas.md#cue-lists-de-qlc)).
