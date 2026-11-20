# Patch

![La página Patch](img/patch.png)

**Patch** (F5) tiene los fixtures de tu montaje: qué son, en qué universo y dirección están, su número, los grupos de fixtures y dónde están en el escenario.

En la cabecera eliges la vista y tienes las herramientas para añadir:

- **«Lista»**: todos los fixtures por dirección.
- **«Canales»**: los 512 canales de cada universo, con los fixtures encima.
- **«Plano 2D»**: el escenario visto desde arriba.
- **«Importar»**, **«Matriz LED»** y **«Añadir fixtures»**.
- Con fixtures aparcados, **«N aparcados · Desaparcar todo»**: cuántos hay y el botón para desaparcarlos todos (consulta [Aparcar canales](patch.md#aparcar-canales)).

## Añadir fixtures

1. Pulsa **«Añadir fixtures»**.
2. Elige el **Fabricante** (escribe en «Buscar fabricante…» para filtrar) y el **Modelo**.
3. En el panel **Patch**:
   - **Nombre**: el que verás en todo el programa (por defecto, el del modelo).
   - **Modo**: el modo DMX, con su número de canales.
   - **Cantidad**: cuántos iguales.
   - **Universo** y **Dirección**: la dirección se pone sola en la primera libre del universo.
   - **Separación**: canales libres entre un fixture y el siguiente.
   - Con más de uno, **«Crear un grupo con ellos»** y el nombre del **Grupo**.
4. Abajo se ve qué direcciones van a usar («Usa las direcciones 1 a 20»). Si no caben en lo que queda de universo, pasan al siguiente. Si el modo tiene más *DMX break* (un fixture GDTF con dos entradas DMX), un aviso te pide patchear también su modo «(DMX break 2)», en su propia dirección.
5. Pulsa **«Patchear fixture»** o **«Patchear N fixtures»**.

La ventana se queda abierta para seguir añadiendo, con la dirección ya en la siguiente libre. Si algo no se puede patchear (las direcciones están ocupadas, por ejemplo), el motivo aparece en rojo abajo. Pulsa **«Cerrar»** al terminar.

Para dimmers sueltos, elige **Generic › Generic Dimmer**: en lugar de modo, pides cuántos **Canales**.

**«Buscar en GDTF-Share…»**, abajo, abre [GDTF-Share](fixtures.md#gdtf-share): descarga el fixture y pulsa **«Patchearlo»** para dejarlo elegido aquí.

> **Consejo:** si no encuentras tu fixture, búscalo en GDTF-Share, descarga la [Open Fixture Library](fixtures.md#open-fixture-library), importa su GDTF o créalo en la página [Fixtures](fixtures.md).

## La lista

Columnas: casilla de selección, **#** (número), **NOMBRE**, **DIRECCIÓN** (universo . dirección), **CANALES**, **FIXTURE** (fabricante y modelo), **MODO**, **INVERTIR** (pan y tilt) y el botón de identificar. Tras el nombre, un **escudo** marca un [efecto especial protegido](patch.md#efectos-especiales-protegidos) y un **candado**, un fixture con canales [aparcados](patch.md#aparcar-canales).

- **«Agrupar por fixture»** los agrupa por modelo, con una casilla para seleccionar todos los de cada modelo. Un clic en la cabecera de un modelo lo pliega (oculta sus fixtures) o lo despliega.
- **Doble clic** en un fixture lo edita.

## Seleccionar

Las herramientas de la barra trabajan sobre la selección:

- **Clic** selecciona; **Ctrl+clic** añade o quita; **Mayús+clic** selecciona todos los fixtures desde el último que pulsaste hasta ese (en ese sentido, que es el orden de la selección), y **Ctrl+Mayús+clic** los añade. **«Todos»** y **«Ninguno»**. La casilla de una fila añade o quita su fixture (como Ctrl+clic); con Mayús, añade todos desde el último que pulsaste.
- **«Identificar»**: los fixtures seleccionados parpadean en blanco para encontrarlos en el escenario. **«Dejar de identificar»** lo para (también se para al salir de la página). El botón de cada fila identifica solo ese fixture. La lista se queda donde estaba. Los efectos especiales protegidos nunca se identifican (parpadear un CO2 lo dispararía): un aviso dice cuántos se quedaron fuera.
- **Invertir pan** e **invertir tilt** (los iconos de flechas): para los fixtures que lo tienen.
- **«Límites»**, el botón de la **llama** (**«Efecto especial»**), **«Editar»**, **«Redireccionar»**, **«Numerar»**, **«Crear grupo»** y **«Eliminar»**: se explican a continuación.

**Clic derecho** en una fila de la lista, o en un fixture del plano 2D sin mover el ratón, abre el menú del fixture: **«Efecto especial…»**, **«Identificar»**, **«Límites…»**, **«Proteger del gran master»**, **«Editar…»**, **«Aparcar como está ahora»**, **«Aparcar apagado»**, **«También en blackout»**, **«Desaparcar»** y **«Desaparcar todo»**. Si el fixture no estaba seleccionado, primero lo selecciona.

## Direcciones y universos

- **Editar un fixture**: selecciona uno y pulsa **«Editar»** (o doble clic). Puedes cambiar **Nombre**, **Modo**, **Universo**, **Dirección** y **Número**, y pulsar **«Aplicar»**.
- **Redireccionar varios**: selecciónalos y pulsa **«Redireccionar»**. Elige **Universo**, **Dirección inicial** y **Separación**: reciben direcciones seguidas, en el orden de la lista.
- **Arrastrar en la vista Canales**: elige el universo arriba y arrastra un fixture a otra dirección. La sombra se ve en verde si cabe y en rojo si los canales no están libres. Arriba se ve cuántos de los 512 canales están en uso. Doble clic en un fixture lo edita.

Los universos se crean y se conectan a las interfaces en [Salidas](salidas.md).

## Números de fixture

Cada fixture tiene un **número**, que es el que usa la [línea de comandos](programar.md#línea-de-comandos) de Programar (`1 THRU 8 @ FULL`). Un fixture nuevo recibe el número siguiente al último.

- Cambia uno en **«Editar»** › **Número**.
- Para numerar varios seguidos, selecciónalos, pulsa **«Numerar»** y escribe el **Primer número**: se numeran por orden de dirección.

## Grupos de fixtures

Los grupos de fixtures sirven para seleccionar de un clic en Patch y en Programar (`GROUP 2` en la línea de comandos), y cada uno tiene su tira en el [mezclador](directo.md#mezclador). También se usan en el SHOW (rueda de color, rejilla XY, paletas, dimmer y velocidad de movimiento) y en los mapeos.

1. Selecciona los fixtures.
2. Pulsa **«Crear grupo»** y escribe su nombre (por ejemplo, `Wash frontal` o `Spots`).

El panel **«Grupos de fixtures»** de la izquierda los muestra con su número de fixtures. Un clic en un grupo selecciona sus fixtures (en la vista «Plano 2D», lo aísla: consulta [Aislar un grupo en el plano](patch.md#aislar-un-grupo-en-el-plano)); **Ctrl+clic** o **Mayús+clic** los añade. Clic derecho:

- **«Renombrar»**.
- **«Subir»** y **«Bajar»**: lo mueven en la lista.
- **«Sustituir por la selección»**: el grupo pasa a tener los fixtures seleccionados.
- **«Guardar la selección aquí…»**: consulta [Selecciones](patch.md#selecciones).
- Patrones de selección, en el orden del grupo: **«Seleccionar los impares»**, **«Seleccionar los pares»**, **«Seleccionar en orden inverso»**, **«Seleccionar la primera mitad»**, **«Seleccionar la segunda mitad»**, **«Seleccionar de los extremos al centro»** y **«Seleccionar del centro a los extremos»**.
- **«Eliminar grupo»**.

**El orden de los grupos** lo eliges tú: arrastra un grupo por su asa (a la izquierda de su nombre) hasta otro sitio de la lista, o usa «Subir» y «Bajar». **«A–Z»**, en la cabecera del panel, los ordena alfabéticamente (los números, como números: «Truss 2» antes que «Truss 10»). Los grupos nuevos van al final. El orden se guarda con el show, y es el mismo en Programar y en los números de `GROUP` de la línea de comandos.

### Selecciones

Una **selección** es una parte ordenada de un grupo: por ejemplo, los spots de un grupo de seis tomados de los extremos al centro. El orden importa, porque los abanicos y los FX lo siguen.

1. Selecciona los fixtures en el orden que quieras (o con un patrón del menú del grupo).
2. Pulsa el **+** del grupo (o «Guardar la selección aquí…») y escribe un nombre.

Las selecciones aparecen bajo su grupo. Un clic selecciona sus fixtures en su orden, aquí y en Programar. Clic derecho: **«Renombrar»** y **«Eliminar la selección»**. La flecha a la izquierda del grupo oculta o muestra sus selecciones.

## Plano 2D

El plano muestra el escenario visto desde arriba, con **PÚBLICO** abajo. Sirve para seleccionar en Programar, para ver lo que sale y para los efectos de color colocados en el plano.

En la cabecera del plano:

- **«Ancho (m)»** y **«Fondo (m)»**: el tamaño del escenario, hasta 200 m. Lo agrandan o lo reducen por la derecha y por delante; un fixture que quede fuera de un escenario más pequeño se queda donde está.
- **«Colocar automáticamente»**: reparte todos los fixtures.

Fuera del escenario, en el medio de cada lado, hay un **«−»** y un **«+»**: quitan o añaden 1 m a ese lado (a la izquierda, a la derecha, al fondo o delante, del lado del público); mantenlos pulsados para varios metros. Los fixtures se quedan donde están en el escenario: a la izquierda y al fondo se mueven con ese lado, igual que el punto al que apunta [Follow](programar.md#follow-apuntar-a-un-punto), y la franja nueva aparece en ese lado. Mientras el ratón está sobre ellos, la vista del plano no cambia y los botones no se mueven, así que puedes pulsar varias veces seguidas; al apartarlo, el plano vuelve a ajustarse (si habías hecho zoom o movido la vista, se queda como está). El **«−»** de un lado solo funciona si ningún fixture está a 1 m o menos de él: muévelo antes (al pasar el ratón por encima dice por qué).

Las herramientas (trabajan sobre la selección y en su orden):

- **«En vivo»**: muestra el color, el dimmer, el estrobo y el gobo de cada fixture, y cada píxel de las barras y matrices.
- **«Lazo»**: selecciona dibujando alrededor de los fixtures en lugar de con un recuadro.
- **«Seleccionar»**: clic o recuadro para seleccionar.
- **«Mover»**: arrastra los fixtures (se mueve toda la selección) o la vista.
- **«Cuadrícula»**: «Libre», 10 cm, 25 cm, 50 cm o 1 m: donde caen los fixtures al moverlos.
- **«Aspecto»**: cómo se ven los fixtures seleccionados: «Redondo», «Cuadrado» o «Rectángulo», y su **«Icono»** (consulta [Iconos, colores y leyenda](patch.md#iconos-colores-y-leyenda)).
- **«Colocar»**: los coloca en «Línea», «Línea, de delante a atrás», «Círculo», «Rejilla, por filas», «Rejilla, en zigzag», «Rejilla, por columnas» o «Rejilla, en zigzag por columnas», con la **Separación (cm)** y las **Columnas** de al lado.
- **«Alinear»**: «Izquierda», «Centro», «Derecha», «Fondo», «Medio», «Frente», «Repartir a lo ancho» y «Repartir a lo hondo».
- **15°** (clic derecho: hacia el otro lado) y **90°**: giran la selección.
- **«Altura y orientación»**: dónde cuelgan las cabezas móviles seleccionadas, para [Follow](patch.md#altura-y-orientación-follow).

Las cabezas móviles llevan una pequeña flecha en el borde (una muesca, cuando el plano las muestra pequeñas): hacia dónde miran. Los nombres de los fixtures muy juntos se ocultan para no pisarse: acerca el zoom para leerlos. Un **escudo** arriba a la izquierda marca un efecto especial protegido (tenue; rojo con los efectos armados; con un anillo que late mientras dispara) y un **candado** arriba a la derecha, un fixture aparcado.

La **rueda** hace zoom, el **botón derecho** o el central arrastran la vista y un doble clic con ellos la centra. Los botones de la esquina también hacen zoom. Un clic derecho sin mover el ratón abre el [menú del fixture](patch.md#seleccionar).

> **Consejo:** coloca los fixtures en el orden en que quieres que corran los efectos: selecciónalos de izquierda a derecha y usa «Colocar › Línea».

### Iconos, colores y leyenda

Cada fixture lleva en el plano el icono de lo que es: cabeza móvil, escáner, wash o PAR, barra LED, estrobo, blinder, dimmer o convencional, láser, humo, hazer, chorro de CO2, llama, chispas frías, confeti, burbujas, nieve, ventilador, efecto u otro. Lo dice su tipo de fixture, salvo que el nombre de su modelo diga más: un «Effect» llamado «CO2 Jet» es un chorro de CO2 y un dimmer llamado «Blinder» es un blinder, pero una cabeza móvil llamada «Beam» o «Wash» sigue siendo una cabeza móvil. Un dimmer genérico, que puede estar en lugar de cualquier fixture, toma el icono de su propio nombre («CO2 1», «FLAME 3», «BLINDER1»). Las barras LED y las matrices muestran sus celdas; los demás fixtures con varias celdas (un blinder de varias lámparas, un estrobo de varios segmentos) llevan el icono al lado de sus celdas.

Para elegirlo a mano, selecciona los fixtures y usa **«Aspecto» › «Icono»**; **«Automático»** vuelve al suyo. El icono elegido se guarda con el show.

Cada tipo de fixture del show tiene además su color, en el icono y en el borde, el mismo cada vez que abres el show: dos tipos con el mismo icono se distinguen por el color. Un icono elegido a mano en un fixture de la biblioteca no cambia su color. En vivo, los fixtures encendidos muestran su luz y los apagados, su icono.

La **leyenda**, arriba a la izquierda del plano, lista los tipos de fixture con su icono, su color y cuántos hay (los dimmers genéricos, por lo que representan). Un clic en un tipo selecciona todos sus fixtures, también en el plano de Programar; **Ctrl+clic** o **Mayús+clic** los añade a la selección. Un clic en su título la pliega o la despliega, y se recuerda. El escenario se coloca al lado o debajo de la leyenda, para que no tape ningún fixture.

### Aislar un grupo en el plano

En la vista «Plano 2D», un clic en un grupo del panel «Grupos de fixtures» lo **aísla**: los demás fixtures quedan bloqueados y atenuados en el plano, y nada los selecciona ni los mueve: ni un clic, ni el recuadro, ni el lazo, ni la leyenda, ni «Mover» (un clic sobre uno no hace nada, y el recuadro puede empezar encima). Así es fácil seleccionar parte de un grupo en un plano lleno de fixtures y hacer grupos más pequeños con ella. De la selección se quedan solo los fixtures del grupo que ya estaban seleccionados. El grupo aislado lleva un borde grueso y un ojo, y junto a la leyenda del plano una etiqueta dice qué grupo ves y cuántos fixtures tiene.

- **«Todos»** selecciona todos los fixtures del grupo, igual que **Ctrl+clic** en el grupo. **Ctrl+clic** en otro grupo añade solo sus fixtures que también están en el aislado.
- En la leyenda, un clic en un tipo selecciona solo sus fixtures del grupo; los tipos que no tienen ninguno se ven atenuados.
- Un clic en otro grupo lo aísla en su lugar; un patrón de selección de su menú aísla antes ese grupo. Un clic en una selección selecciona siempre todos sus fixtures: si alguno está bloqueado, antes aísla el grupo de la selección o, si tiene fixtures de otros grupos, termina el aislamiento.
- Después de **«Crear grupo»**, el grupo sigue aislado: puedes seguir dividiéndolo en más grupos.
- Para terminar: otro clic en el grupo, la **×** de la etiqueta o **Esc**. También termina al pasar a «Lista» o «Canales», al abrir otro show o al eliminar el grupo. No se guarda con el show.

En las vistas «Lista» y «Canales», un clic en un grupo sigue seleccionando sus fixtures, como siempre.

### Altura y orientación (Follow)

Para que [Follow](programar.md#follow-apuntar-a-un-punto) apunte bien todas las cabezas móviles a un punto, cada una necesita saber dónde está. Coloca las cabezas en el plano donde están de verdad, selecciónalas y pulsa **«Altura y orientación»**. Lo que cambies se aplica al momento a todas las seleccionadas y se guarda con el show:

| Campo | Qué es |
|---|---|
| **Altura** | A cuántos metros del suelo del escenario está el eje de la cabeza (sin ajustar: 4 m colgada, 0 m de pie). |
| **Montaje** | **«Colgada»** (de una vara o un truss, boca abajo) o **«De pie»** (en el suelo). |
| **Orientación** | Hacia dónde va el haz con el pan en el centro y el tilt inclinado hacia delante. 0° es hacia el público y gira en el sentido de las agujas del reloj; es el mismo giro que el de los botones 15° y 90°, y la flecha del plano lo muestra. |
| **Recorrido de pan** / **de tilt** | Los grados que recorre cada eje. «Su definición» usa los de la definición del fixture; si no los trae se usan 540° y 270°, y se avisa en naranja. |
| **Pan al revés** / **Tilt al revés** | Si con Follow una cabeza gira o se inclina hacia el lado contrario. Solo afecta a Follow; las inversiones del Patch están en [Límites](patch.md#límites). |

«(distintas)» indica que las cabezas seleccionadas no tienen el mismo valor; al cambiarlo, todas toman el nuevo.

## Límites

Selecciona fixtures y pulsa **«Límites»**:

- **Dimmer**, **Pan** y **Tilt**: el rango permitido, de 0 a 100 %. Por ejemplo, un dimmer que no pase del 80 % o un tilt que no apunte al público.
- **«Invertir pan»**, **«Invertir tilt»** e **«Intercambiar pan y tilt»** (para cabezas colgadas al revés o giradas). Es una corrección física: hace que esas cabezas se muevan como las demás, y el espejo de un FX no la tiene en cuenta (consulta [El espejo](programar.md#el-espejo)). Con varios fixtures de los que solo algunos están invertidos, sale apagado con «(solo algunos)», como el botón de la barra, y al activarlo se invierten todos.
- **«Restablecer»**: sin límites ni ejes invertidos o intercambiados.
- **«Proteger del gran master»**: el gran master no baja ninguno de sus canales, actúe solo sobre la intensidad o sobre todos: la luz de sala, un servidor de medios, un hazer. Los masters de grupo, los dimmers de escena, el Blackout y el [fundido a negro](directo.md#fundido-a-negro) sí lo bajan. Con varios fixtures de los que solo algunos están protegidos, sale apagado con «(solo algunos)», y al activarlo se protegen todos. También está en el [menú del fixture](patch.md#seleccionar), marcado cuando todos los seleccionados lo están. En la lista, un **ancla** tras el nombre («Protegido del gran master: no baja con él»); en el plano 2D, del Patch y de Programar, un ancla abajo a la izquierda (en una barra, a su derecha). «Restablecer» no quita la protección. Se guarda con el show, en cada fixture, e [importar el patch de otro show](patch.md#importar-de-otro-show) la trae. QLC+ no la lee: allí el gran master baja el fixture.

Con un solo fixture, la lista **Canales** permite decidir para cada canal:

- **Fundidos**: si sigue los tiempos de fundido. Apagado, salta directamente a su valor (útil en ruedas de color y gobos).
- **Dimmer**: si lo escalan el gran master, los masters de grupo y los dimmers de escena. El gran master, además, solo escala la luz (consulta [Masters](directo.md#masters)). Si el fixture está protegido del gran master, solo lo escalan los masters de grupo y los dimmers de escena.

Los límites actúan en la salida: escenas, efectos y edición en vivo conservan sus valores, pero lo que sale los respeta. En un pan o un tilt de 16 bit, el límite va sobre los dos canales juntos, así que un movimiento lento sigue suave. Con varios fixtures se ven los valores del primero que tiene cada control, y lo que cambies se aplica a todos los que lo tienen.

## Efectos especiales protegidos

Un **efecto especial protegido** solo dispara con los efectos armados, desde escenas, cuelists y chasers, y cada disparo dura como mucho su tiempo máximo. Cómo se arman y qué los dispara se explica en [Directo](directo.md#efectos-especiales-protegidos).

**Cuáles lo son.** Las máquinas de **CO2**, **llamas**, **chispas frías** y **confeti** se protegen solas; las de **burbujas** y **nieve**, no (protégelas a mano si quieres). La clase sale de su tipo de fixture, de las palabras de su definición, del nombre de un dimmer genérico («CO2 1», «FLAME 3», «FUEGO 2», «PIRO 1») o del icono elegido en el plano. Las reglas automáticas solo protegen, nunca quitan la protección: cambiar el nombre o el icono nunca desprotege un fixture; un fixture que pasa a ser de una de esas clases se protege, y un aviso lo dice («CO2 4 es ahora un efecto especial protegido»); un CO2 que desprotegiste a mano sigue desprotegido aunque lo llames «CO2 9».

> **Límites:** «C02» escrito con un cero no se reconoce, y «FOCO2» no es un CO2. Una máquina que no se reconoce no está protegida: comprueba el escudo en la lista y protégela a mano.

**«Efecto especial»** (el botón de la llama, junto a «Límites», o «Efecto especial…» en el menú del fixture) abre sus ajustes para la selección:

- **«Efecto especial protegido»**: quitarlo pregunta antes («¿Quitar la protección?»). Sin ella, el fixture sale como cualquier otro canal: sin armar, sin tiempo máximo y también desde el programador, Identificar, el mezclador y los FX.
- **«Tiempo máximo de disparo»**: de 0,1 a 30 s (por defecto: CO2 3 s, llamas 2 s, chispas 5 s, confeti 2 s, el resto 3 s).
- **«Espera mínima entre disparos»**: de 0 a 60 s (1 s por defecto), desde que se suelta.
- **«Canal de seguridad»**: **«Ninguno»** o el canal del fixture que la máquina necesita para disparar, con sus **«Valores desde / hasta»** (de 0 a 255; «desde» no puede ser mayor que «hasta»). Ese canal solo sale dentro de esos valores mientras el efecto está armado y disparando («desde», si la escena no le da valor); si no, 0. Solo se elige cuando los fixtures seleccionados tienen los mismos canales, y un fixture de un solo canal no puede tener uno («… necesita otro canal para poder disparar»). Si el fixture cambia de modo o de definición y en su lugar queda otro canal, se quita el canal de seguridad, y un aviso lo dice («Se quitó el canal de seguridad de …»).
- **«Según su clase (…)»**: vuelve a la protección y a los tiempos de su clase, sin canal de seguridad.

**Qué canales disparan.** Todos menos el pan, el tilt y el de seguridad, sea cual sea su tipo (intensidad, efecto, altura…): a ellos se aplica el tiempo máximo. El pan y el tilt solo salen con los efectos armados y desde escenas, sin límite de tiempo, y desarmados a 0: una escena que solo apunta la máquina no la dispara. Una máquina que no tiene más que pan y tilt dispara con todos sus canales menos el de seguridad. Un fixture entre dos universos (de un show de QLC+) no dispara nunca: la ventana lo avisa; muévelo a un solo universo.

En la lista, un **escudo** tras el nombre; en el plano, un escudo arriba a la izquierda: tenue normalmente, rojo con los efectos armados y con un anillo que late mientras dispara. **Identificar** nunca los enciende. [Niveles DMX](salidas.md#niveles-dmx) y el plano en vivo muestran lo que sale de verdad: un CO2 desarmado se ve a 0 aunque su escena suene.

La protección se guarda con el show, en su sección de DMX Total Control, e [importar el patch de otro show](patch.md#importar-de-otro-show) la trae. Al abrir un show anterior o de QLC+, un aviso dice una vez qué fixtures son ahora efectos protegidos por su clase («2 fixtures de este show son ahora efectos especiales protegidos: …»); también una máquina de humo cuyo modelo dice «CO2» o un dimmer genérico llamado «Fuego…» o «Fire…». Para cambiarlo, usa «Efecto especial». QLC+ no lee esta protección: abierto en QLC+, el show dispara estos fixtures desde cualquier sitio, sin armar y sin tiempo máximo. Las versiones anteriores de DMX Total Control tampoco la leen: disparan estos fixtures sin armar y, al guardar el show, le quitan la protección a mano, los tiempos y el canal de seguridad. Usa esta versión en todos los ordenadores del show.

## Aparcar canales

Un canal **aparcado** sale siempre con un valor fijo, por encima de las escenas, las cuelists, los FX, el programador, el mezclador (Full, Flash…), el gran master y Parar todo: el hazer al 30 % toda la noche, una cabeza rota apagada y quieta, la luz de sala. Esc y Blackout nunca lo desaparcan.

En el [menú del fixture](patch.md#seleccionar) (clic derecho en la lista o en el plano):

- **«Aparcar como está ahora»**: todos sus canales en lo que sale ahora, así que el gran master, el mezclador y los límites de ese momento quedan dentro. No se puede durante la parada de show ni durante un fundido a negro (su luz quedaría aparcada para siempre).
- **«Aparcar apagado»**: los canales de intensidad (dimmers y colores) a 0 y los demás como están ahora; un fixture sin canales de intensidad (una máquina de humo o un hazer de la biblioteca), todo a 0. Durante la parada de show o un fundido a negro, los demás canales se aparcan en lo que el show envía por debajo.
- **«También en blackout»**: con el Blackout, sus intensidades aparcadas siguen con su valor. Sin ella, se apagan con el Blackout; el pan, el tilt, el gobo y los demás canales que no son luz siguen siempre aparcados.
- **«Desaparcar»** y **«Desaparcar todo»**.

En [Programar](programar.md#aparcar), **«Aparcar»** aparca los valores que el programador tiene en los fixtures seleccionados.

Con fixtures aparcados, la cabecera del Patch muestra **«N aparcados · Desaparcar todo»** (en una ventana estrecha, el candado, el número y el botón con solo su icono). En la lista, un **candado** tras el nombre («Aparcado: N canales con un valor fijo en la salida»); en el plano 2D, del Patch y de Programar, un candado arriba a la derecha. En [Niveles DMX](salidas.md#niveles-dmx), un candado en cada canal aparcado.

- Un efecto especial protegido solo se puede aparcar **apagado**: «Aparcar como está ahora» o «Aparcar» de Programar lo aparcan a 0 y lo dicen («Un efecto especial protegido solo se puede aparcar apagado»). Aparcado, no dispara nunca, aunque esté armado; un disparo cortado por aparcarlo sale en el registro como «aparcado». Un fixture aparcado encendido que pasa a estar protegido se aparca apagado; si después se desprotege, sigue aparcado a 0 hasta que lo desaparques. Desaparcado, solo dispara después de soltar su escena y volver a pulsarla.
- Los canales aparcados siguen al fixture si lo redireccionas. Cambiar el modo de un fixture quita sus canales aparcados («Se quitaron los canales aparcados de Spot 3: cambió de modo»), salvo en un efecto protegido, que sigue aparcado apagado. Eliminar un fixture quita los suyos. Un pan aparcado de un fixture con pan y tilt intercambiados sale por el canal del tilt, como el pan.
- La [parada de show](directo.md#parada-de-show) gana a los canales aparcados: un fixture aparcado apagado también se ilumina, y un hazer aparcado sigue aparcado. Un [fundido a negro](directo.md#fundido-a-negro) baja los aparcados sin «También en blackout».

Los canales aparcados se guardan con el show, en su sección de DMX Total Control (importar el patch de otro show no los trae). Al abrir un show con canales aparcados se aplican al momento, y un aviso dice cuáles («2 fixtures aparcados: HAZER, Spot 3 (Patch)»). Abrir otro show o uno nuevo los quita. Al cerrar el programa se sueltan (el escenario queda en reposo), pero siguen en el show. QLC+ no aparca canales: allí suenan como digan sus escenas, y la copia para QLC+ lo avisa («2 fixtures aparcados: QLC+ no aparca canales»). Las versiones anteriores de DMX Total Control no los aplican y, al guardar el show, los quitan, igual que los ajustes de la parada de show.

> **Sin probar con equipos reales:** aparcar no se ha probado con interfaces, nodos ni hazers reales.

## Matrices y tiras LED

**«Matriz LED»** patchea de una vez una matriz o una tira de píxeles:

1. **Nombre**, **Ancho (píxeles)** y **Alto (píxeles)** (alto 1 para una tira).
2. **Colores**: el orden de cada píxel (RGB, GRB, BGR, BRG, GBR, RBG o RGBW).
3. **Cableado**: «Filas», «Filas, en zigzag», «Columnas» o «Columnas, en zigzag».
4. **Píxel 1**: la esquina donde empieza («Arriba a la izquierda», etc.).
5. **Universo** y **Dirección**.
6. Comprueba el resumen y la vista previa del **orden de los píxeles**, y pulsa **«Patchear»**.

Se crea un fixture por línea de píxeles, un grupo para los efectos de color y su sitio en el plano. Si una línea necesita más de un universo, hazla más corta o cablea en el otro sentido.

## Importar de otro show

**«Importar»** trae a este show parte de otro (de DMX Total Control o de QLC+):

- **«Patch de otro show…»**: sus fixtures. Van a sus mismas direcciones si están libres; si no, a las siguientes. Traen su protección de efecto especial, sus tiempos, su canal de seguridad y su icono del plano (lo que diga el otro show nunca desprotege un CO2), pero no sus canales aparcados.
- **«Escenas de otro show…»**: sus escenas, asignadas a los fixtures de este show que coinciden (por dirección y modelo, por nombre o por modelo). Entran en grupos con el mismo nombre, con sus opciones en el grupo (tamaño, fase y velocidad en vivo, liberación, flash, dimmer…). Un grupo nuevo trae también su crossfade y su master; uno que ya existe en este show conserva los suyos.
- **«Patch y escenas de otro show…»**: las dos cosas.
- **«Patch de un programa de visualización (CSV)…»**: la tabla de fixtures que exporta tu programa de visualización; mira [Importar un patch desde un programa de visualización](patch.md#importar-un-patch-desde-un-programa-de-visualización).

## Importar un patch desde un programa de visualización

Si diseñas el montaje en un programa de visualización, exporta su tabla de fixtures (CSV) e impórtala: DMX Total Control patchea los mismos fixtures, en los mismos universos y direcciones, con sus nombres, sus números y su sitio en el plano.

1. Pulsa **«Importar» › «Patch de un programa de visualización (CSV)…»** y elige el archivo.
2. La ventana **«Importar un patch»** dice cuántos fixtures trae la tabla, de cuántos tipos y en qué universos. Nada cambia hasta que pulses **«Importar»**.
3. En **«Tipos de fixture»**, cada tipo de la tabla (fixture, modo y canales) dice en qué se convierte y cómo se encontró en la biblioteca: **«Exacto»** (el mismo modelo y modo), **«Por número de canales»** (el modo con esos canales), **«Nombre parecido»**, **«Recordado»** o **«Elegido a mano»**. Un **«Nombre parecido»** sale en naranja para que lo compruebes: es un modelo con un nombre parecido, no el mismo. Si no está en la biblioteca, se patchea como **«Dimmer genérico»** con sus canales, en naranja: todo queda en sus direcciones igualmente. Cada tipo muestra también el icono que tendrán sus fixtures en el plano; si se patchea como dimmer genérico, toma el que dice su nombre en la tabla (un «CO2 Jet» sale como chorro de CO2; un «Flame», como llama).
4. **«Cambiar»** elige otro fixture y modo (**«Elegir de la biblioteca…»**), un dimmer genérico o **«No importar»**. Lo que eliges a mano se recuerda para la próxima vez que importes ese tipo.
5. En **«Opciones»**:
   - **«Colocarlos en el plano del escenario»**, con la **Vista**: **«Desde arriba»** (el público abajo del plano) o **«Frontal»** (como ve el montaje el público: los fixtures más altos arriba). Se propone la frontal cuando el montaje cuelga a distintas alturas, y se recuerda la última que usaste. El plano crece para que quepa el montaje (nunca encoge). Cada importación que los coloca vuelve a centrar el montaje en el plano, también después de cambiar sus lados con «−» y «+».
   - **Grupos**: uno por cada tipo de fixture, uno por cada capa (si la tabla tiene más de una) y los de la columna «Groups» de la tabla.
6. Debajo de los tipos están los problemas, antes de importar: direcciones que ya usa otro fixture u otra fila (esas filas no se importan), filas sin universo o dirección, universos por encima de 128, direcciones que pasan de 512, un segundo DMX break (no se importa), números de fixture que ya tiene otro fixture o demasiado altos (reciben otro), un pan o tilt invertido que no es ni sí ni no, y los universos que se añaden al show.
7. Pulsa **«Importar N fixtures»**. Un aviso dice cuántos son nuevos y cuántos se han actualizado, y si queda algún problema.

**De cada fila** se importan el fixture y su modo, el universo y la dirección, el nombre (la columna «Unit») y el número de fixture: el de la columna «Channel», si no el de la otra columna de número de fixture, y si no, uno automático. La altura de cada cabeza móvil, si está de pie (girada boca arriba en la tabla) o colgada, y su pan o tilt invertido van a su [altura y orientación](patch.md#altura-y-orientación-follow), los coloques en el plano o no.

**Importar otra vez** la tabla (después de cambiar el montaje en el programa de visualización) actualiza los mismos fixtures: los reconoce por el identificador de cada fila de la tabla, o por el mismo nombre y fixture, y les cambia el modo, la dirección, el nombre, el número y el sitio en el plano. Los que ya no están en la tabla se quedan, salvo que marques **«Eliminar los N fixtures importados antes que ya no están en la tabla»** (debajo se ven sus nombres). Un icono que hayas elegido a mano después se conserva, salvo que cambie el fixture con el que se patchea su tipo.

> **Límites:** de cada fixture se importa solo su primer DMX break, y de las rotaciones solo se usa si la cabeza está boca arriba (la orientación no se importa). Visto desde arriba, los fixtures que cuelgan uno encima de otro quedan juntos en el plano: para verlos separados, usa la vista frontal.

## Eliminar fixtures

Selecciónalos y pulsa **«Eliminar»**. Se pregunta antes: sus valores se eliminan de todas las escenas.
