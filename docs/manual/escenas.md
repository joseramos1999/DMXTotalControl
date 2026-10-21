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

## La cabecera del editor

- **El nombre**: haz clic para cambiarlo.
- **La carpeta** (icono de carpeta): mover la escena a una carpeta.
- **«Probar»** / **«Parar»**: la reproduce ahora, fuera de los grupos. Las cuelists tienen su propio GO.
- **«Añadir al grupo…»**: la añade a un grupo de Directo.
- **Convertir a pasos** (racks, FX, Colour FX, Move FX, Curve FX y chasers): graba un ciclo en una escena de pasos nueva, solo con los canales a los que la escena da un valor: los demás, aunque tengan un valor por defecto (el pan o el zoom en el centro de muchas cabezas móviles), no entran en los pasos.
- **Convertir en cuelist** (pasos): consulta [Cuelists](escenas.md#cuelists).
- **Exportar**, **«Duplicar»** y **«Eliminar»** (se quita también de sus grupos y racks).

## Cómo suena una escena

Debajo de la cabecera está el panel de reproducción. Muestra lo que tiene sentido para cada tipo:

- **«Fundido de entrada»** y **«Fundido de salida»**.
- **«Tiempo de ciclo»** (o **«Tiempo por paso»** en un Colour FX) y **«Velocidad»** (de un ciclo por minuto a diez por segundo; doble clic: un ciclo por segundo), con **×½** y **×2**.
- **«Entrada gradual»** (estáticas y pasos): cada fixture empieza su fundido este tiempo después del anterior, en orden de selección.
- **«Modo de conducción»**: **«Duración»** (los tiempos normales), **«BPM»** (los tiempos pasan a beats y siguen el tempo del show), **«BeatGO»** o **«Pulso»** (avanza con el botón BeatGO o con los picos de la entrada de audio). Con BeatGO o Pulso, **«Divisiones»** (/2, ×2).
- **«Ritmo marcado»** (secuencias y chasers): el ritmo marcado pulsando su botón varias veces seguidas ([Directo](directo.md#ritmo-de-una-escena-con-pasos)), en BPM. Mientras lo tiene, sus pasos van a ese ritmo y no por sus tiempos. **«Quitar»** vuelve a sus tiempos.
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

**Ctrl+clic** y **Mayús+clic** seleccionan varios pasos: al cambiar un tiempo, cambia en todos. Los pasos aplican las paletas por valor, no por referencia.

### FX

Una escena FX tiene **valores base** y una o varias **capas de FX**. Se crea grabando desde el programador con FX (consulta [FX](programar.md#fx)).

- A la izquierda, la lista de FX: parámetro, fixtures y velocidad. La papelera quita una capa.
- A la derecha, los controles de la capa elegida (los mismos que en Programar) y su vista previa animada. Los cambios se oyen en directo mientras la escena suena.
- **«Enlazar»**: las capas enlazadas comparten velocidad y timing.
- Si algunas luminarias hacen el FX a su manera, se avisa: si cambias aquí la forma, el tamaño, el sentido o la fase, todas harán lo mismo.
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

Siguen sonando igual y tienen su editor (forma, ancho, alto, rotación, posición, fase, reparto entre fixtures, simétrico, relativo…). Un aviso ofrece **«Convertir a FX»**: pasan al editor de FX, también en el programador, y los grupos, botones del SHOW y mapeos los conservan.

### Rack de efectos

Varias escenas y efectos que suenan juntos como uno solo, cada uno en sus fixtures.

- En **«Añadir»** (a la derecha), haz clic en una escena para meterla en el rack.
- En **«En el rack (N)»**, la **×** la quita.

Un rack no puede contener una escena que lo reproduzca a él.

### Chasers

Los chasers de shows de QLC+ suenan con normalidad, pero no tienen editor. Puedes convertirlos a pasos con el botón de convertir de la cabecera.

## Super escenas

Una **super escena** es una línea de tiempo con pistas de escenas y de audio. Se lanza como cualquier escena (en un grupo, en el SHOW, con un mapeo). Su editor ocupa toda la página; el botón de la lista, arriba a la izquierda, vuelve a la lista de escenas.

### La barra de herramientas

- **«Reproducir»** / **«Parar»** (**Espacio**): desde el cursor. **⏮** vuelve al principio. Al lado se ve el tiempo (y el compás).
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

### Trabajar con los bloques

- **Arrastra** un bloque para moverlo, también a otra pista. Arrastra sus **bordes** para cambiar su duración.
- Con el bloque seleccionado, los **tiradores** de las esquinas superiores son sus fundidos de entrada y de salida.
- Al alargar un bloque, la escena **se repite en bucle** (se ven marcas en cada vuelta) o se **estira** para que un ciclo dure todo el bloque. Se cambia en el menú del bloque o en la barra de abajo («En bucle» / «Estirada»).
- **Clic derecho**: «Repetir la escena en el bloque» / «Estirar la escena al bloque», «Duplicar», «Bloquear» / «Desbloquear», «Abrir la escena» y «Eliminar».
- **Doble clic** en un bloque abre su escena. **Supr** elimina el bloque seleccionado.
- La barra de abajo muestra el bloque seleccionado: **Inicio**, **Duración**, **Fundido de entrada**, **Fundido de salida**.

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

## Cuelists

Una **cuelist** es una lista de cues que avanzan con GO. Funciona con **tracking**: cada cue guarda **solo lo que cambia** y el resto sigue de las cues anteriores. Un GO solo cambia los canales que cambian: sin bajones y sin retomar lo que no toca.

Las cuelists se graban desde el programador, se operan en [Playbacks](playbacks.md) y se editan aquí.

### Opciones de la cuelist

Arriba del editor:

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

En una ventana estrecha, parte de las opciones se pliega: **«Más opciones»** las muestra.

### La tabla de cues

| Columna | Qué es |
|---|---|
| **CUE** | El número. Admite decimales: la 2.5 va entre la 2 y la 3. Doble clic para cambiarlo. Un lápiz indica que la cue está cargada en el programador. |
| **NOMBRE** | Su nombre. |
| **DISPARO** | **GO** (espera un GO), **Seguir** (cuando acaba la cue anterior, más un tiempo) o **Esperar** (un tiempo después del GO de la cue anterior). |
| **RETARDO** | Cuánto espera la cue antes de empezar su fundido. |
| **FUNDIDO** | El tiempo de fundido. |
| **BAJADA** | El fundido de los dimmers que bajan. «=» es igual que el fundido. |
| **CAMBIOS** | Cuántos canales cambia, y las marcas **FX**, **[R]** (valores liberados) y el icono de paleta si usa paletas. |
| **S** | **Saltar** (solo esta cue): lo que cambia no pasa a las siguientes. |
| **B** | **Bloqueo**: Grabar y Actualizar guardan también los valores iguales a los que hereda. |
| **NOTA** | Una nota para ti. |

Para cambiar un tiempo, haz clic en él y escribe los segundos, o usa la **rueda** del ratón sobre el tiempo pulsado: ±0,1 s (con Mayús, 1 s). Con **Ctrl+clic** o **Mayús+clic** seleccionas varias cues y cambias sus tiempos, su disparo o sus marcas a la vez.

La cue que suena lleva una barra verde con el avance del fundido; la siguiente, un borde verde. Arriba se ve el estado («Q 3 · Verso ▸ 4» o «Liberada · GO lanza la 1») y los botones **Atrás**, **GO** y **liberar**, los mismos que los de su playback.

### Grabar y editar cues

- **«Grabar la cue N»**: el programador se convierte en una cue nueva tras la seleccionada, solo con lo que cambia respecto a lo que hereda.
- **«Actualizar ▾»**: fusiona el programador con la cue seleccionada: **«Actualizar la cue N (sigue en las siguientes)»** o **«Solo esta cue (las siguientes mantienen su aspecto)»**.
- **«Cargar ▾»** (o doble clic en una cue): la lleva al programador: **«Lo que muestra el escenario en la cue N»** o **«Solo lo que cambia la cue N»**. Después, actualízala desde la barra «Editando la cue…» de Programar.
- **«Sacar ▾»**: con los canales del programador, **«Quitar de la cue N los canales del programador (vuelven a heredar)»** o **«[Liberar] los canales del programador en la cue N»** (desde esa cue vuelven a lo que hay debajo: otras escenas o su valor de reposo).
- **Copiar** al final: **«Copiar lo que cambia la cue N»** o **«Copiar todo el aspecto de la cue N (cue de bloqueo)»**.
- **«Número…»**: otro número para la cue seleccionada. Mover una cue cambia lo que heredan las que van detrás.
- **Eliminar**: con **«Mantener el aspecto de las cues siguientes»** marcada, lo que cambia la cue pasa a la siguiente y las de detrás se ven igual.

En una lista estrecha (o en un idioma con palabras largas), «Actualizar», «Cargar» y «Sacar» se quedan con su icono y su flecha **▾**; al pasar el ratón se ve su nombre.

El clic derecho en una cue tiene **«Ir a la cue N»**, las dos formas de cargar, **«Actualizar con el programador»**, **«Número…»** y **«Eliminar…»**.

Las cues pueden usar **paletas por referencia** y llevar **capas de FX**, que se heredan de cue en cue.

### La hoja de tracking

**«Hoja de tracking»** muestra lo que el escenario tiene en la cue seleccionada, fixture a fixture («En escena: cue N»). Si la ventana es ancha, se ve sola al lado de las cues.

- Las fichas de colores dicen qué es cada valor: **«Cambios»** (lo cambia esta cue), **«Heredados»** (viene de una cue anterior, que se indica: «Q2»), **«Bloqueo»** y **«Liberados»** (tachados, con [R]).
- Arriba se ven las paletas que usa la cue.
- Haz clic en canales para elegirlos y pulsa **«Liberar»** (los pone en [RELEASE] desde esta cue) o **«Sacar»** (vuelven a heredar de las cues anteriores).

### Convertir pasos en cuelist

En una escena de pasos, el botón **«Convertir en cuelist»** de la cabecera convierte cada paso en una cue con lo que cambia respecto al anterior; los pasos con tiempo se encadenan con **Esperar**. Sus grupos, botones del SHOW y mapeos pasan a reproducir la cuelist. Los pasos desaparecen.

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
