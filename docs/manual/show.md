# Show

![La página Show](img/show.png)

La página **Show** (F2) es tu pantalla de operación: la montas tú, a tu medida, con botones, faders, diales, grupos, playbacks, reloj, notas… Está pensada para pantallas táctiles, para dejarla a pantalla completa durante el show y para usarla también desde el móvil con el [control remoto](remoto.md). Se guarda con el show.

Un SHOW tiene **páginas**. Cada página está hecha de **secciones** que llenan la pantalla. Cada sección se puede dividir en dos mitades, lado a lado o una encima de otra, tantas veces como quieras, y cada sección muestra una cosa: una tabla de botones, una tabla de faders, un grupo, un reloj…

## Empezar

Con un show nuevo la página está vacía y ofrece dos botones:

- **«Página nueva»**: crea una página con una tabla de botones y entra en el modo edición.
- **«Partir de mis grupos»**: crea la página «Grupos» con todos tus grupos de Directo arriba y, abajo, una tabla de faders «Masters» (el gran master y los masters de los primeros grupos), una sección de BPM y tap y un reloj. Solo aparece si ya tienes grupos.

## El modo edición

Pulsa **«Editar»** para montar la pantalla y **«Hecho»** para volver a usarla. En el modo edición:

- Haz clic en el título de una sección (o en cualquier parte de ella) para **seleccionarla**. Sus propiedades aparecen en el panel **«Propiedades»** de la derecha.
- En la barra de título de cada sección hay tres botones: **dividir en dos mitades lado a lado**, **dividir una encima de otra** y **quitar esta sección**. Los mismos están en las propiedades: «Lado a lado», «Una encima» y «Quitar sección».
- Arrastra el **separador** entre dos mitades para cambiar su tamaño. Se ajusta solo a cuartos, tercios y mitades.
- Una sección **«Vacía»** muestra botones con todos los tipos: haz clic en uno para elegir qué muestra.

### Tipos de sección

| Tipo | Qué muestra |
|---|---|
| **Botones** | Una tabla de botones (columnas × filas). |
| **Faders** | Una fila de faders. |
| **Un grupo** | Un grupo de Directo con botones grandes. |
| **Todos los grupos** | Todos los grupos de Directo, uno al lado de otro. |
| **Playbacks** | Los playbacks de la página Playbacks, con su GO, sus botones y su fader. |
| **Mezclador en vivo** | El [mezclador](directo.md#mezclador) de Directo. |
| **Reloj** | La hora, con segundos y fecha opcionales. |
| **Temporizador** | Un cronómetro o una cuenta atrás. |
| **BPM y tap** | El BPM con un piloto al ritmo y los botones TAP y BeatGO. |
| **Notas** | Un texto tuyo, del tamaño que quieras. |
| **Sonando ahora** | Lo que está sonando, con un botón para pararlo. |
| **Diales y ruedas** | Una tabla de diales o ruedas sin fin. |
| **Rueda de color** | Una rueda para dar color a un grupo de fixtures. |
| **Rejilla XY (pan y tilt)** | Un área para mover cabezas móviles. |
| **Imagen** | Una imagen (un logo, un plano…). |
| **Niveles DMX** | Los 512 canales de un universo, en vivo. |
| **Vacía** | Nada (en el modo edición, la lista de tipos). |

### Propiedades de la sección

Según el tipo, el panel de propiedades muestra:

- **Muestra**: el tipo de sección.
- **Título**: el texto de su barra de título. Sin título, fuera del modo edición la barra no se ve.
- **Columnas** y **Filas**: el tamaño de la tabla (hasta 32). Los botones, faders y diales llenan sus celdas.
- **Llenar con las escenas de un grupo** (botones): pone de una vez un botón por cada escena de ese grupo.
- **Grupo** (un grupo): cuál muestra.
- **Fixtures** (rueda de color y rejilla XY): sobre qué grupo de fixtures actúan, o «Todas las fixtures con color» / «Todas las cabezas móviles».
- **Imagen** (imagen): el archivo, y «Imagen entera» o «Llenar la sección».
- **Segundos** y **Fecha** (reloj).
- **Cuenta** (temporizador): «Hacia arriba (cronómetro)» o «Hacia abajo (cuenta atrás)», con sus **Minutos**.
- **Texto** y **Tamaño** (notas).
- **Universo** (niveles DMX).
- **Página de playbacks**, **Playbacks** y **Primer playback** (playbacks): consulta [Playbacks en el SHOW](show.md#playbacks-en-el-show).

## Botones

1. Selecciona una sección de botones y haz clic en una celda vacía (la del **+**).
2. Pulsa **«Añadir un botón aquí»**.
3. Elige su **Acción** y lo que necesite (la escena, el grupo, la paleta…).

La forma más rápida de llenar una tabla es **arrastrar escenas** desde la lista que hay bajo las propiedades («Arrastra una escena a una celda:») hasta las celdas: cada escena se convierte en un botón que la lanza.

### Acciones de un botón

| Acción | Qué hace |
|---|---|
| **Reproducir una escena** | Lanza la escena en su grupo (o sola, si no está en ningún grupo). |
| **Siguiente escena de un grupo** / **Escena anterior de un grupo** | En un grupo, o en «Todos los grupos». |
| **Liberar un grupo** | Un grupo, o todos. |
| **Blackout** | Activa o quita el blackout. |
| **Parar todo** | Como el botón de la barra superior. |
| **Tap tempo** | Marca el BPM. |
| **BeatGO (siguiente división)** | Como el botón BeatGO de la barra superior. |
| **Una paleta** | Pone la paleta en los fixtures con los que se grabó («Sus propias fixtures») o en un grupo de fixtures, como una edición en vivo del programador: va por encima de las escenas hasta que vuelves a pulsar. |
| **GO de cuelist** / **Atrás de cuelist** / **Liberar una cuelist** | Sobre la cuelist que elijas en **Cuelist**. |
| **Ir a una cue…** | Salta a la **Cue** que elijas de esa cuelist. |

Un botón traído de otra página con Alt+clic muestra «Un control de otra página» y debajo lo que hace («Hace: …»).

### Opciones de un botón

- **Etiqueta**: su texto. Vacía («Automática»), muestra el nombre de la escena, de la paleta o de la acción.
- **Comportamiento** (escena, blackout y paleta): **«Alternar: encender / apagar»** o **«Flash: mientras se pulsa»**. Los botones flash llevan un rayo en la esquina.
- **Tecla**: una tecla del teclado que lo pulsa mientras estás en la página Show. La letra se ve en la esquina del botón.
- **Color**, **Icono** e **Imagen** de fondo (la imagen se ve atenuada mientras el botón está apagado).

Un botón encendido se rellena con su color.

### Mover y quitar

En el modo edición, arrastra un botón, fader o dial a otra celda de la misma sección para moverlo. Para quitarlo, selecciónalo y pulsa **Supr** (o «Quitar el botón» / «Quitar el fader» en las propiedades). **Esc** quita la selección.

## Faders

Un fader del SHOW puede controlar **varias cosas a la vez**. En sus propiedades, bajo **Controla**, cada elemento es una de estas:

| Elemento | Qué hace el fader |
|---|---|
| **Lanzar una escena (su intensidad)** | Al subir desde 0, la escena arranca en su grupo (respetando si es exclusivo); al bajar a 0, se para; entre medias es su intensidad. |
| **Dimmer de una escena (sin lanzarla)** | Atenúa la escena la lance quien la lance (su grupo, un playback, un botón, una super escena). No la lanza ni la para. Su nivel se guarda con el show y dura mientras haya un fader o dial de dimmer en esa escena: si lo quitas, la escena vuelve al 100 %. |
| **Master de un grupo** | El master de ese grupo de Directo. |
| **Gran master** | El GM de la barra superior. |
| **Dimmer de fixtures** | El dimmer del mezclador de todos los fixtures o de un grupo de fixtures. |
| **Velocidad de movimiento de las cabezas móviles** | El canal de velocidad de pan/tilt de todas las cabezas móviles o de un grupo (arriba = más rápido). |
| **Velocidad de una escena** | La velocidad en vivo de esa escena. |
| **BPM** | El tempo del show, de 20 a 300. |

La diferencia entre los dos de escena: **«Lanzar una escena (su intensidad)»** es la escena misma, y la arranca y la para; **«Dimmer de una escena (sin lanzarla)»** solo la atenúa, también cuando la lanzan desde otro sitio. Se pueden usar a la vez: con el dimmer al 50 % y el fader que la lanza al 80 %, la escena sale al 40 %.

Pulsa **«Añadir otro elemento»** para sumar otro y el icono de la papelera para quitarlo. Si arrastras una escena a una celda de faders, se crea un fader que la lanza.

Con **Rango**, **De** … **a** … (de 0 a 100 %), el recorrido del fader cubre solo esa parte de lo que mueve, en todos sus elementos. Por ejemplo, la velocidad de una escena de 20 a 80 %, o un dimmer de 0 a 60 % para que no pase de ahí. Con «De» mayor que «a», o con **«Invertir»**, el fader va al revés. Un fader que lanza una escena la para en el extremo donde el valor es más bajo (abajo del todo; arriba del todo si está invertido), aunque «De» no sea 0.

El texto del fader muestra el valor real (el % o el BPM), no dónde está el fader. Si otro control lleva el valor fuera del rango, el fader se queda en el extremo.

Debajo de cada fader hay un botón **«Flash»**: lo lleva arriba del todo de su recorrido mientras lo pulsas y, al soltar, devuelve exactamente lo que había. Se quita o se pone con el interruptor «Botón flash: al máximo mientras se pulsa» de sus propiedades.

> **Consejo:** un fader con «Master de un grupo» de tus washes y «Dimmer de fixtures» de los PAR del público te da un solo fader para bajar todo el frontal.

## Diales y ruedas

Los diales controlan lo mismo que los faders (la tabla de arriba), con el mismo **Rango**. Cada dial puede ser:

- Un **dial**: recorre todo su rango en tres cuartos de vuelta. Doble clic lo lleva al principio de su recorrido (el valor de «De», o el de «a» si está invertido); si controla el BPM, a 90, o al extremo del rango más cercano si 90 queda fuera.
- Una **rueda incremental** («Rueda incremental: gira sin fin y cada muesca suma o resta un paso»): gira sin fin, y cada muesca suma o resta el **Paso** (de 1 a 25 %) a donde esté el valor. El paso es un % del valor, no del recorrido, y la rueda no sale del rango; al girarla hacia arriba con la escena parada, la arranca en «De». El arco de alrededor muestra el nivel.

Para girarlos, arrastra alrededor del centro con el ratón o el dedo, o arriba y abajo cerca del centro. Cada paso de la rueda del ratón lo gira un poco (en una rueda incremental, una muesca).

## Rueda de color y rejilla XY

- **Rueda de color**: toca la rueda para dar tono y saturación (blanco en el centro) a los fixtures de la sección, o pulsa una de las muestras de abajo.
- **Rejilla XY (pan y tilt)**: arrastra el punto para mover las cabezas. **«Centro»** las lleva al centro.

Las dos actúan como una edición en vivo del programador: van por encima de las escenas hasta que pulsas **«Liberar»**, y entonces las escenas vuelven a dar el color o la posición.

## Grupos y mezclador

Las secciones **Un grupo** y **Todos los grupos** muestran las columnas de [Directo](directo.md) con botones grandes, su anterior/siguiente, su botón de liberar y su master, sin opciones de edición. **Mezclador en vivo** muestra las tiras del mezclador ajustadas al ancho.

## Playbacks en el SHOW

La sección **Playbacks** muestra tiras iguales a las de la página [Playbacks](playbacks.md):

- **Página de playbacks**: **«Seguir la página actual»** (la barra de título muestra ◀ el nombre de la página ▶ para cambiarla) o una página fija.
- **Playbacks**: cuántos se ven (hasta 20) y **Primer playback**: desde cuál.
- Tres interruptores para mostrar u ocultar el **segundo botón (Pausa / Atrás)**, el **tercer botón (Flash)** y **la cue que suena y la siguiente**.

En el modo edición, suelta una escena sobre una tira para ponerla en ese playback.

## Utilidades

- **Reloj**: hora y fecha del ordenador.
- **Temporizador**: **«Iniciar»** / **«Pausa»** y **«Reiniciar»**. En cuenta atrás se pone naranja en el último minuto y rojo al llegar a cero. Sigue contando aunque cambies de página.
- **BPM y tap**: el tempo, **TAP** y **BeatGO**.
- **Notas**: el texto que escribas en sus propiedades (la escaleta, avisos para el operador…).
- **Sonando ahora**: lo que suena, con el botón de parar al lado de cada escena.
- **Niveles DMX**: una cuadrícula con los 512 canales del universo elegido.
- **Imagen**: entera o llenando la sección.

## Páginas

- En el modo edición, **«+ Página»** añade una página.
- Haz clic en el nombre de una página para verla. Las pestañas de página se ven si hay más de una.
- En el modo edición, doble clic en una pestaña la renombra; clic derecho abre **«Renombrar…»**, **«Mover a la izquierda»**, **«Mover a la derecha»** y **«Eliminar la página…»**. Al eliminar se avisa de cuántas secciones y controles se van con ella: no se puede deshacer.

## Llevar cualquier control al SHOW

**Alt+clic** en casi cualquier control del programa lo copia a la página del SHOW que tengas abierta: una escena de Directo, el master de un grupo, un botón del mezclador, el gran master, el blackout, una paleta de Programar, los botones y el fader de un playback… El programa avisa «Añadido al SHOW, página «…»».

El control nuevo va a la primera celda libre de una tabla de botones o de faders de esa página. Si no hay ninguna, una sección vacía se convierte en una (con el título «Touch»), o se divide la última sección.

## Pantalla completa y bloqueo

- **«Pantalla completa»** (F11) y **«Salir de pantalla completa»**.
- **«Bloquear»**: oculta la barra lateral y deja solo el SHOW, sin edición ni navegación. Para salir, **mantén pulsado «Desbloquear»** (una barra se llena bajo el botón). Si hay contraseña de bloqueo, la pide.

### Bloqueo y modo quiosco

- En Opciones › «SHOW y bloqueo», **«Abrir el SHOW a pantalla completa al arrancar»** abre directamente el SHOW. Con **«…y bloqueado»**, además arranca bloqueado: solo el SHOW, como un quiosco.
- Si bloqueas la **«Edición del SHOW»** con el candado de la barra superior, el botón «Editar» desaparece: las páginas se usan, pero no se cambian.

Consulta [Opciones](opciones.md#show-y-bloqueo).

## Teclas, mapeos y remoto

- Las **teclas** de los botones solo funcionan en la página Show. Por eso, en esta página las teclas G y T no hacen BeatGO ni Tap.
- En el modo Mapear, los botones y los faders del SHOW se pueden asignar a un controlador; un control MIDI puede mover un fader del SHOW entero, con todos sus elementos (consulta [Mapeos](mapeos.md)).
- Las páginas del SHOW se ven y se usan en el móvil o la tablet con el [control remoto](remoto.md).
