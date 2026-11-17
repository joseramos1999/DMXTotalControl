# Directo

![La página Directo](img/directo.png)

**Directo** (F1) es la página para lanzar escenas durante el show. Las escenas se ordenan en **grupos**, que son columnas que funcionan en paralelo: cada grupo tiene sus escenas, su modo (exclusiva o mezcla), su tiempo de crossfade y su master.

En la cabecera de la página tienes:

- **◀ ▶**: escena anterior o siguiente en cada grupo que suena (también con Ctrl+← y Ctrl+→).
- **«Pausa»** / **«Reanudar»**: congela todo lo que suena y lo continúa.
- **«Liberar grupos»**: para las escenas de todos los grupos.
- **«Plegar grupos»** / **«Desplegar grupos»**: pliega todas las columnas a la vez para verlas todas; cuando ya están todas plegadas, las despliega.
- **«Mezclador»**: cambia a la vista del [mezclador](directo.md#mezclador). Un punto naranja avisa de que el mezclador está cambiando la salida.
- **«Nuevo grupo»**: añade una columna.

## Grupos

### Crear y organizar grupos

1. Pulsa **«Nuevo grupo»** (o el recuadro «Añadir grupo» al final de las columnas).
2. Haz clic en el nombre de la columna para cambiarlo (por ejemplo `Wash`, `Spots`, `Strobos`, `Efectos`) y pulsa Intro.

El botón del engranaje de cada columna abre su menú: **«Cambiar color»** (pasa al siguiente color), **«Mover a la izquierda»**, **«Mover a la derecha»** y **«Eliminar grupo»**. También puedes arrastrar el grupo por el asa que hay a la izquierda de su nombre y soltarlo sobre otro grupo para ocupar su lugar.

La flecha **‹** junto al nombre **pliega** el grupo: queda una tira estrecha con su nombre y cuántas escenas suenan, para dejar sitio a los demás. Sus escenas siguen sonando. La flecha **›** de la tira lo despliega. Se guarda con el show. Para plegarlos o desplegarlos todos a la vez, usa **«Plegar grupos»** / **«Desplegar grupos»** en la cabecera.

### Añadir y ordenar escenas

1. Pulsa **«Añadir escenas»** al final de la columna.
2. En la lista, haz clic en las escenas que quieras (se marcan con ✓). Puedes buscar por nombre o por carpeta y verlas por carpetas o por grupos.
3. Pulsa **«Añadir N»**. Con doble clic en una escena se añade directamente.

Para reordenar, pasa el ratón por una ficha y arrástrala por su asa (a la derecha de la ficha): suéltala sobre otra escena para ponerla delante, o en el hueco de la columna para ponerla al final. Puedes soltarla en **otro grupo** y se mueve con sus opciones. En el menú de la ficha (clic derecho) también tienes **«Subir»**, **«Bajar»** y **«Quitar del grupo»**.

> **Consejo:** al grabar desde Programar puedes meter la escena directamente en un grupo con «Añadir al grupo». Consulta [Grabar](programar.md#grabar).

## Lanzar escenas

Haz clic en una ficha para lanzar la escena (con **GO**, el modo en el que arranca el programa). Para liberarla, pulsa el cuadrado de la cabecera del grupo («Liberar este grupo») o cambia a **Stop** y púlsala. La ficha que suena se colorea con el color del grupo y tiene un punto que late.

En cada ficha se ve:

- El **nombre** y, debajo, el **tipo** de escena. En una cuelist que suena, la cue en la que está («Q 3 · Verso»). Detrás, el [ritmo marcado](directo.md#ritmo-de-una-escena-con-pasos) si lo tiene («120 BPM»).
- **P7**, **P2**…: la prioridad, solo cuando no es la normal (5). En naranja si es mayor que 5.
- Un **rayo**: es una escena flash.
- Una **barra partida en dos** abajo: una escena con pasos, una cuelist o un FX en «BPM del show» que suena; alterna en cada paso (en un FX, en cada pulso), a su ritmo.

### Exclusiva o mezcla

El botón de cada columna alterna entre:

- **«Exclusiva»**: solo suena una escena a la vez. Al lanzar otra, hace crossfade a la nueva. Los [faders del SHOW](show.md#faders) que lanzan escenas son la excepción: suman la suya, así que puedes subir varios del mismo grupo a la vez.
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

Una cuelist en un grupo es una ficha más: el clic la lanza (con **Stop**, la para). Su menú (clic derecho) añade **«GO (cue siguiente)»**, **«Atrás (cue anterior)»** y **«Liberar la cuelist»**. Para operarla con GO cómodamente, ponla en un [playback](playbacks.md).

### Ritmo de una escena con pasos

Las escenas con pasos (secuencias y chasers), las cuelists y las escenas FX con FX en «BPM del show» siguen el ritmo al que pulsas su ficha:

- Un clic la lanza (con **GO**, el modo en el que arranca el programa; ver más abajo).
- Con la escena sonando, pulsa su ficha **varias veces seguidas** al ritmo de la música, con menos de 2 segundos entre pulsaciones. Cada pulsación pasa al paso siguiente (en una cuelist, da un GO) y la escena sigue sola a ese ritmo: un paso en cada pulso, en lugar de sus propios tiempos. El ritmo es la media de las últimas pulsaciones. Una pulsación que llega un poco tarde, justo después del paso de ese pulso (menos de medio pulso), es ese mismo paso: no da otro, solo pone el ritmo a tiempo.
- Solo cuentan las pulsaciones con la escena sonando: la que la lanza empieza la cuenta. Así, cambiar de escena en un grupo exclusivo y volver no le pone ritmo a ninguna.
- Tras una pausa de más de 2 segundos, un clic da un paso sin cambiar su ritmo, y la escena sigue sonando. Para pararla, cambia a **Stop** y púlsala.
- Se reproduce el ritmo que marcaste, también con una **velocidad en vivo** puesta; si luego mueves la velocidad, el ritmo se acelera o se frena con ella. En pausa, las pulsaciones no dan pasos.
- El ritmo se guarda con la escena y es suyo: también lo sigue cuando suena en una super escena o en un playback. En sus [propiedades](directo.md#propiedades-de-la-escena) (y en [Escenas](escenas.md#cómo-suena-una-escena)) puedes escribirlo o pulsar **«Quitar»** para que vuelva a sus tiempos.
- En una **escena FX**, el ritmo es el de sus FX en **«BPM del show»**: la primera pulsación de una serie vuelve a empezar su ciclo (como al lanzarla), y cada una de las siguientes los lleva al pulso siguiente, un ciclo más allá por su multiplicador (una persecución pasa al fixture siguiente; a ×¼, avanzan un cuarto de ciclo). Varias seguidas les dan un ciclo en cada pulso (por su multiplicador), en lugar del BPM del show. Los FX con una velocidad propia no cambian. Sin ritmo marcado, o tras «Quitar», siguen el BPM del show. Si quitas «BPM del show» a todos sus FX, la escena pierde su ritmo marcado.

Funciona igual con sus botones del [SHOW](show.md), con sus mapeos de controladores y teclas en modo «Alternar» y con el [control remoto](remoto.md).

#### GO y Stop

Los botones **GO** y **Stop** de la barra superior (y de la sección «Ritmo» del SHOW) deciden lo que hace una pulsación de cualquier escena, para todo el que la pulse. Siempre hay uno de los dos encendido: el programa arranca siempre en **GO**, y al pulsar el otro cambia de modo:

- **GO** encendido: pulsar una escena la **lanza**; en una que ya suena, cada pulsación da un paso (en una cuelist, un GO; en una escena FX, la primera vuelve a empezar el ciclo de sus FX en «BPM del show» y las siguientes los llevan al pulso siguiente) y varias seguidas marcan su ritmo. Nunca la para, aunque pases más de 2 segundos sin pulsar: así marcas el tiempo tranquilo.
- **Stop** encendido: pulsar una escena la **para**. Una que no suena se queda como está.
- Para liberar las escenas de un grupo sin cambiar de modo, pulsa el cuadrado de su cabecera («Liberar este grupo»); **«Parar todo»** (Esc) lo para todo.

Las escenas flash siguen funcionando mientras las mantienes pulsadas, en cualquier modo.

Mientras suena, cada ficha de una escena con pasos, de una cuelist o de una escena FX con FX en «BPM del show» lleva abajo una **barra del ritmo** partida en dos: se enciende un lado y luego el otro en cada paso (en una cuelist, en cada cue; en una escena FX, en cada pulso, un ciclo de esos FX a ×1). Así ves a qué ritmo va cada una. La llevan también sus botones del SHOW, sus tiras de playback y sus botones en el móvil.

## Masters

- **Master del grupo**: el deslizador «Master» al pie de cada columna escala la intensidad de todas sus escenas.
- **Gran master**: el «GM» de la barra superior escala la luz de toda la salida: los dimmers y, en los fixtures sin dimmer, sus colores. No toca la mezcla de color (cian, magenta, amarillo, tono y saturación) ni los colores de un fixture o una cabeza con dimmer propio, que bajan con su dimmer. En Opciones decides si actúa solo sobre la intensidad o sobre todos los canales.
- **Dimmer en vivo**: cada escena tiene además su propio dimmer, en sus propiedades, por encima del master del grupo.

Los masters y el gran master no afectan a los canales que tienen desactivada la opción «Dimmer» en los [límites del fixture](patch.md#límites). Si es el dimmer de un fixture, ya no cuenta como su dimmer propio: el gran master baja entonces sus colores.

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
| **Ritmo marcado** | Pasos, cuelists y escenas FX con FX en «BPM del show»: el [ritmo marcado](directo.md#ritmo-de-una-escena-con-pasos) con su ficha, en BPM. Escríbelo o pulsa «Quitar» para volver a sus tiempos (en una escena FX, al BPM del show). Actúa al momento. |
| **Velocidad en vivo** | De ×0,1 a ×10 (escenas que se mueven: pasos, FX, cuelists…). |
| **Fase en vivo** / **Tamaño en vivo** | La fase (0–359°) y el tamaño (0–200 %) de los FX. |
| **Velocidad de fundido** | Acelera o frena sus fundidos, de ×0,1 a ×10. |

Los deslizadores en vivo actúan al momento, mientras los mueves, y cambian cómo suena la escena en ese grupo, no la escena grabada. El resto de campos se aplican al pulsar **«Aplicar»**.

El mismo menú de la ficha tiene **«Abrir en Escenas»**, **«Editar en el programador»** (escenas estáticas) y el submenú **«Prioridad: N»** para cambiarla sin abrir las propiedades.

## Prioridades

Cada escena tiene una prioridad de 0 a 10; las nuevas toman la que indiques en [Opciones](opciones.md#reproducción) (5 por defecto).

- La escena de **mayor prioridad manda en todos los canales que toca**, ceros incluidos: una escena roja de prioridad 8 sobre una azul de prioridad 5 da rojo, no magenta.
- Con la **misma prioridad**, en los canales de intensidad (dimmer, RGB…) se mezclan y gana el valor más alto; en el resto de canales gana el **último cambio**: la última escena lanzada o el último GO de una cuelist que cambió ese canal. Lo que una cuelist solo mantiene de cues anteriores no lo retoma.
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

Cuando la lanza un [fader del SHOW](show.md#faders), no libera nada, sea cual sea su modo: suma la suya. Con su botón, libera como siempre. Y mientras ese fader está subido, nada la para salvo bajarlo del todo.

**Protegida de liberaciones de** hace que la escena ignore esas liberaciones:

- **«Ninguno»**: se libera como siempre.
- **«Todo (solo se para a mano)»**: solo la paras tú.
- **«Su propio grupo»**: ignora las de su grupo.
- **«Los demás grupos»**: ignora las que vienen de otros grupos.

> **Consejo:** pon una escena de ambiente con protección «Todo (solo se para a mano)» y lanza encima escenas con «Todas las escenas»: el ambiente se queda.

## Pausa, liberar y parar

- **«Pausa»** congela todo lo que suena. **«Reanudar»** lo continúa.
- **«Liberar grupos»** para las escenas de todos los grupos; el cuadrado de cada columna, solo las de ese grupo. Durante 10 s, [«Recuperar lo que sonaba»](directo.md#recuperar-lo-que-sonaba) vuelve a lanzar lo que paró «Liberar grupos» (su botón, o un pad o un botón del SHOW en «Todos los grupos»); no se puede recuperar lo que se libera en un solo grupo (su cuadrado, o un pad o un botón del SHOW de ese grupo) ni con una acción de una cue.
- **«Parar todo»** (Esc), en la barra superior, para todo lo que suena, también fuera de los grupos. No vacía el programador. Ni «Parar todo» ni «Liberar grupos» paran las escenas que sujeta un [fader del SHOW](show.md#faders) subido: solo bajarlo del todo. También desarma los [efectos especiales](directo.md#efectos-especiales-protegidos), y durante 10 s se puede [recuperar lo que sonaba](directo.md#recuperar-lo-que-sonaba). Con [«Parar todo con fundido»](directo.md#parar-todo-con-fundido), las funciones bajan antes de pararse.
- **«Blackout»** (Ctrl+B) saca la salida a negro sin parar nada. Las cabezas no se mueven: el pan, el tilt, el zoom, el foco y los demás canales que no son luz se quedan donde están. También desarma los efectos especiales. Para bajar la luz poco a poco y acabar en Blackout, usa un [«Fundido a negro»](directo.md#fundido-a-negro).

## Seguridad en el directo

Para que un error o un fallo del ordenador no estropee el show: el [Modo show](directo.md#modo-show), los [efectos especiales protegidos](directo.md#efectos-especiales-protegidos), los [canales aparcados](directo.md#canales-aparcados), la [parada de show](directo.md#parada-de-show), [recuperar lo que sonaba](directo.md#recuperar-lo-que-sonaba), [parar todo con fundido](directo.md#parar-todo-con-fundido) y el [fundido a negro](directo.md#fundido-a-negro). Sus ajustes están en [Opciones › Seguridad en el directo](opciones.md#seguridad-en-el-directo).

> **Importante:** el ordenador nunca debe ser la única seguridad de una máquina de CO2, de llamas, de chispas frías o de confeti. Configura las máquinas y los nodos DMX para que se apaguen si pierden la señal DMX (no para que mantengan el último valor), pon una parada de emergencia o un pulsador de hombre muerto por hardware donde la máquina lo necesite y sigue las normas del lugar y las instrucciones del fabricante. Si el programa se cuelga o el ordenador se apaga, ningún software puede cortar un disparo: solo lo corta el equipo.

### Modo show

El **escudo** de la barra superior, justo después del candado, pone o quita el **Modo show** con un clic. Encendido se ve naranja, y la segunda línea del título empieza por «Modo show ·». Bloquear el SHOW (también al arrancar con «…y bloqueado») lo pone solo; desbloquearlo no lo quita. Con el SHOW bloqueado, el escudo se ve, pero no se puede pulsar. Es un ajuste del ordenador, no del show.

Con el Modo show puesto, estas acciones piden mantener pulsado un botón 1,5 s («Mantén para cerrar», «Mantén para abrir» o «Mantén para seguir»), aunque no haya cambios sin guardar:

- cerrar el programa (la × de la ventana, Alt+F4, la barra de tareas) e instalar una actualización;
- **Nuevo** y **Abrir** (también Ctrl+N y Ctrl+O), los shows recientes, los ejemplos y el show de práctica de la guía;
- abrir un show desde el Explorador de Windows.

Soltar antes de tiempo, **«Cancelar»** o Esc cancelan; Esc en esa ventana nunca para el show. También vale mantener pulsado Espacio o Intro (la repetición de la tecla no cuenta). Después, si hay cambios sin guardar, llega «¿Guardar los cambios?» como siempre. Al apagar Windows o cerrar la sesión con el Modo show puesto, la pregunta puede detener el apagado: cierra antes el programa.

Con el Modo show, si el portátil pasa a batería, una barra roja «Funcionando con batería: conecta el cargador» sale en todas las páginas (también bloqueado y a pantalla completa) hasta que vuelve el cargador; con un 20 % o menos añade «Batería al N %». La batería se mira cada 30 s y con cada aviso de energía de Windows. Sin el Modo show, un aviso lo dice una vez (y otra al quedar poca), y [Opciones](opciones.md#seguridad-en-el-directo) lo muestra.

Si el plan de energía de Windows tiene puesta la suspensión selectiva de USB, al arrancar una barra azul avisa de que la interfaz DMX puede pararse, con **«Cómo quitarla»** (abre [Suspensión selectiva de USB](opciones.md#suspensión-selectiva-de-usb)) y una × que la oculta hasta el próximo arranque. El programa solo lee estos ajustes de Windows: nunca los cambia.

### Efectos especiales protegidos

Un **efecto especial protegido** es un fixture que no puede disparar por error. Las máquinas de CO2, de llamas, de chispas frías y de confeti lo son solas, y cualquier fixture puede serlo a mano (consulta [Patch › Efectos especiales protegidos](patch.md#efectos-especiales-protegidos)). Llevan un escudo en la lista y en el plano del Patch.

- **Desarmados** (así arranca siempre el programa), todos sus canales salen a 0, suene lo que suene.
- **Armados**, solo salen desde **escenas** (también flash), **cuelists**, **chasers**, **secuencias**, **super escenas** y los valores base de las **escenas FX**. Así los disparas con un botón del SHOW en flash (dispara mientras lo mantienes), una ficha de Directo, un pad de la APC, un playback o el [móvil](remoto.md#seguridad-en-el-móvil).
- **Nunca** los disparan, ni armados: el programador, Identificar, Resaltar, el mezclador (Full, Flash, color, estrobo), los FX y sus macros, los Move FX, los Colour FX y los mappings del plano, los Curve FX, los scripts ni una entrada DMX que pasa a la salida. El gran master no los escala.
- Los [límites](patch.md#límites) solo cuentan mientras disparan: en reposo sale siempre 0. No inviertas los canales de una máquina de efectos: un canal de disparo invertido solo dispara con un valor que, invertido, encienda.
- Sus canales de pan y tilt (los de una cabeza de llamas) solo salen armados y desde esas funciones, sin tiempo máximo; desarmados, a 0. Todos los demás, menos el de seguridad, disparan y se les aplica el tiempo máximo (consulta [Patch › Efectos especiales protegidos](patch.md#efectos-especiales-protegidos)).

#### Armar efectos

El botón **«Armar efectos»** (con una llama) está en la barra superior, junto a «Blackout», solo cuando el show tiene efectos protegidos, también con el SHOW bloqueado. Un clic los arma y otro los desarma. Armados, el botón se pone rojo con el tiempo que falta para que se desarmen solos («ARMADOS · 9:59»), y su borde late mientras uno dispara. En una ventana más estrecha pone «Armar» o solo la llama: por debajo de 1450 px de ancho, «Armar efectos», «Blackout» y «Parar todo» quedan solo con su icono, y por debajo de unos 1200 px se oculta la cuenta de lo que está en marcha y el gran master se acorta.

Armados, una barra roja grande **«EFECTOS ARMADOS»** sale en todas las páginas (también bloqueado y a pantalla completa) con lo que dispara («Disparando: CO2 1») o lo que espera a que lo suelten, y un botón **«Desarmar»**.

La flecha **▾** de al lado muestra si están armados y cuánto falta para que se desarmen solos, **«Disparando ahora»**, **«Esperan a que se suelten»** (suelta su escena, su botón o su fader, a 0, y podrá disparar), **«Últimos disparos»** (nombre · hora · segundos · cómo terminó: soltado, cortado por su tiempo máximo, desarmado…), **«Abrir el registro»** y **«Ajustes de cada efecto: Patch»** (estos dos, no con el SHOW bloqueado).

No se arman si ningún fixture está protegido, con el Blackout puesto («El Blackout está puesto: quítalo para armar los efectos») ni con la parada de show puesta. También se arman con un [botón del SHOW](show.md#acciones-de-un-botón), un [mapeo](mapeos.md#controla) o el [móvil](remoto.md#seguridad-en-el-móvil).

#### Tiempo máximo y espera mínima

- Un disparo dura mientras su escena, su botón o su fader lo dan, como mucho su **tiempo máximo** (por defecto: CO2 3 s, llamas 2 s, chispas 5 s, confeti 2 s). Al llegar se corta, aunque sigas pulsando, y no vuelve hasta que lo sueltes.
- Entre un disparo y el siguiente hay una **espera mínima** (1 s por defecto) desde que se suelta. Una pulsación durante la espera no dispara después: suelta y vuelve a pulsar.
- Armar mientras una escena o un fader ya dan valor al efecto no lo dispara: espera a que lo sueltes. Desarmar mientras uno dispara lo corta, aunque vuelvas a armar enseguida (no vuelve hasta que lo sueltes); un control que solo arma no hace nada si ya están armados.
- El **canal de seguridad**, si la máquina tiene uno, solo sale dentro de sus valores mientras el efecto está armado y disparando; si no, 0. Por sí solo nunca dispara la máquina.
- Si el programa deja de enviar fotogramas (está muy ocupado), cada universo corta igual un disparo a su tiempo, como mucho 40 ms tarde.

#### Cuándo se desarman

Se desarman solos:

- con **Parar todo** (Esc, su botón, un pad, un botón del SHOW o el móvil) y con el **Blackout** (Ctrl+B, su botón, un botón del SHOW, un pad o el móvil; no el «Blackout» de una tira del mezclador);
- con la **parada de show**, un **fundido a negro** (al empezar) y **recuperar lo que sonaba**;
- al abrir o empezar **otro show**, y al **cerrar** el programa;
- tras unos **minutos sin disparos** (10 por defecto: «Desarmar los efectos sin uso tras», en [Opciones](opciones.md#seguridad-en-el-directo)), nunca mientras uno dispara;
- al quitar o desproteger el último efecto protegido, y **a mano**.

Un aviso lo dice («Efectos especiales desarmados: Parar todo»). Nunca se guardan armados. Un fader del SHOW que sujeta una escena con un CO2 la sigue sujetando después de Esc, pero el CO2 se queda a 0 hasta que bajas el fader del todo.

#### El registro de disparos

Cada sesión escribe un archivo en `%LOCALAPPDATA%\DMXTotalControl\DMX Total Control\effects-log` («Abrir el registro» abre la carpeta): cada disparo con su hora, el número y el nombre del fixture, cuánto duró y cómo terminó (soltado, cortado por su tiempo máximo, desarmado, parada de show, aparcado, quitado), y cuándo se armaron y se desarmaron los efectos y por qué.

> **Sin probar con equipos reales:** nada de esto se ha probado con máquinas de CO2, de llamas, de chispas ni de confeti, con sus canales de seguridad, con nodos Art-Net o sACN ni con lo que hacen al perder la señal DMX. Pruébalo con tu equipo, sin público y con la máquina sin cargar, antes de usarlo en un show.

### Canales aparcados

Un canal **aparcado** sale siempre con un valor fijo, por encima de las escenas, el programador, el mezclador, el gran master y Parar todo: el hazer al 30 % toda la noche, una cabeza rota apagada y quieta. Esc y Blackout nunca lo desaparcan; con el Blackout, sus intensidades se apagan, salvo con «También en blackout». Se aparca en el [Patch](patch.md#aparcar-canales) y en [Programar](programar.md#aparcar). Al abrir un show con canales aparcados se aplican al momento, y un aviso dice cuáles («2 fixtures aparcados: HAZER, Spot 3 (Patch)»).

### Parada de show

La **parada de show** es una luz de seguridad para parar el show y dar luz a la gente: por encima de todo, pone blanco abierto en las luces, apaga los estrobos y los láseres, deja quietas las cabezas y desarma los efectos especiales.

Para **ponerla o quitarla**, pulsa **la tecla Pausa dos veces** (en menos de 2 s; no en un show que la tiene mapeada, consulta [Mapeos](mapeos.md#teclado)), o **mantén pulsado 1 s** un botón del SHOW «Parada de show (mantener 1 s)», un pad o una tecla [mapeados](mapeos.md#controla), o la mano de la barra del [móvil](remoto.md#seguridad-en-el-móvil). Una pulsación más corta no hace nada, y una sola pulsación de la tecla Pausa solo avisa («Parada de show: pulsa Pausa otra vez para ponerla»). Mientras mantienes, una barra naranja «Parada de show: sigue manteniendo…» se llena. Puesta, una barra roja grande **«PARADA DE SHOW»** sale en todas las páginas (también bloqueado y a pantalla completa) con **«Mantén para quitar»** (mantenlo un segundo). Si el programa pierde el teclado mientras mantienes una tecla mapeada (otra ventana, un aviso de Windows), se suelta y no cambia nada.

Mientras está puesta:

- los efectos especiales se desarman («…desarmados: parada de show») y no se pueden armar («La parada de show está puesta: quítala para armar los efectos»);
- las luces (cabezas móviles, escáneres, PAR y washes, barras LED, blinders y dimmers) de los grupos elegidos van a **blanco abierto** al nivel elegido: el dimmer al nivel, el rojo, el verde, el azul y el blanco a tope (sin dimmer, los colores al nivel), CMY, ámbar y UV a 0, el shutter abierto, la rueda de color en blanco o abierta y los programas internos en «sin función»;
- los estrobos se apagan (sus intensidades a 0, o todos sus canales si no tienen) y los láseres van a 0 en todos sus canales (un láser DMX se apaga a 0: compruébalo con el tuyo);
- todos los shutters se abren (sin estrobo; un canal de estrobo «de lento a rápido» va a 0) y el estrobo del mezclador se para;
- las cabezas móviles se quedan donde están (pan y tilt, también los finos);
- el humo, los hazers, los ventiladores, las burbujas, la nieve, otros efectos, los gobos, los prismas y el zoom se quedan como están, y los efectos protegidos, a 0.

Gana al programador, al gran master, al Blackout (lo demás se queda a negro) y a los canales aparcados: un fixture aparcado apagado también se ilumina, y un hazer aparcado sigue aparcado. Los [límites](patch.md#límites) siguen contando (un dimmer invertido, un máximo). Qué fixtures ilumina y a qué nivel se elige en [Opciones › Parada de show](opciones.md#parada-de-show).

- **Parar todo** (Esc) durante la parada para las funciones de debajo, y la parada sigue.
- Al **quitarla**, el show queda como lo tocan ahora sus funciones (siguen sonando debajo) y vuelve el estrobo del mezclador. Los efectos siguen desarmados: ármalos a mano; un CO2 sujeto tiene que soltarse antes.
- **Nuevo**, **Abrir** y cerrar el programa la quitan. Nunca se guarda puesta: después de un cierre inesperado o de reiniciar, ponla otra vez.
- El registro de los efectos de la sesión apunta «Parada de show puesta» y «Parada de show quitada».

> **Sin probar con equipos reales:** el blanco abierto y los valores de shutter de los fixtures reales, que un láser quede apagado a 0, la tecla Pausa de los teclados reales, los LEDs de la APC y el móvil por una wifi real.

### Recuperar lo que sonaba

Después de **Parar todo** (Esc, su botón, un pad, un botón del SHOW o el móvil) o de **«Liberar grupos»** (todos los grupos, no uno solo), una barra azul bajo la barra superior dice lo que se paró («Parado: 4 escenas y 1 cuelist · Recuperar lo que sonaba (Ctrl+Z)»; «Liberado: …» tras «Liberar grupos»), con el botón **«Recuperar»**, una tira que se vacía en 10 segundos y una × que la oculta hasta el siguiente Parar todo. Sale también con el SHOW bloqueado. **Ctrl+Z** hace lo mismo en Directo y en el SHOW (no mientras lo editas ni mientras escribes), y la barra solo dice «(Ctrl+Z)» donde funciona. «Recuperar» no hace caso de un toque en su primer medio segundo (un segundo toque de pánico en Parar todo).

Vuelven, con un fundido de 0,5 s:

- la escena que tocaba cada grupo;
- las escenas lanzadas desde botones del SHOW, desde Escenas o desde playbacks;
- las cuelists, en la cue en la que estaban, con el nivel de su playback; una cuelist en pausa vuelve en pausa.

Los FX y los pasos empiezan su ciclo de nuevo, las funciones en beats usan su propio fundido y las cuelists «Timecode» siguen su timecode. No vuelven: los flashes que se mantenían (en Directo, el SHOW, los pads o los playbacks), las escenas que sujeta un fader del SHOW (nunca se pararon), el programador, el mezclador, el gran master, el armado de los efectos, los canales aparcados ni la parada de show.

Funciona una vez, y un aviso dice lo que volvió («Recuperado: 4 escenas y 1 cuelist»). Otro Parar todo dentro de los 10 s se suma al primero (no el segundo Esc durante el fundido de «Parar todo con fundido»); «Liberar grupos» guarda solo los grupos, y un Parar todo después se suma. Las funciones eliminadas mientras tanto se saltan, y un show nuevo o abierto lo borra.

Recuperar **nunca arma** los efectos especiales: si estaban armados (después de «Liberar grupos», que no desarma, o al volver a armarlos tras Parar todo), primero los desarma («Efectos especiales desarmados: Recuperar lo que sonaba»). Lo que vuelve con un CO2 espera a que lo sueltes y se queda a 0, también después de armar otra vez.

### Parar todo con fundido

Con **«Parar todo con fundido»** en [Opciones](opciones.md#seguridad-en-el-directo) (de 0 a 5 s; 0, al momento, como siempre), Parar todo baja las funciones en ese tiempo. Las escenas que sujetan los faders del SHOW y los canales aparcados se quedan; los efectos protegidos se apagan al momento (se desarman). Mientras funde, el botón «Parar todo» lleva una tira y dice «Fundiendo: Esc otra vez para al momento»: **Esc otra vez** lo para todo al momento, también lo que hayas lanzado durante el fundido. Después se puede recuperar lo que paró el primer Esc; lo que lanzaste durante el fundido no vuelve.

### Fundido a negro

Un **«Fundido a negro»** (un [botón del SHOW](show.md#acciones-de-un-botón) o un [mapeo](mapeos.md#controla), con su tiempo: 3 s por defecto) baja las intensidades en la salida en ese tiempo y luego pone el Blackout, por ejemplo al final de un set. Por debajo, la luz vuelve a estar a tope: al quitar el Blackout, vuelve.

- Actúa por encima del gran master y en los canales aparcados sin «También en blackout».
- No toca los aparcados con «También en blackout», el pan, el tilt ni otros canales que no son luz, ni los efectos protegidos.
- Al empezar, desarma los efectos especiales.
- Otra pulsación, o el Blackout, lo terminan al momento. Con el Blackout ya puesto, no hace nada.
- Mientras funde, el botón «Blackout» lleva una tira y dice «Fundido a negro: al final llega el Blackout (Blackout ahora: al momento)».

> **Sin probar con equipos reales:** el Modo show (la batería, el equipo despierto, la suspensión de USB), Recuperar, Parar todo con fundido y el Fundido a negro no se han probado con portátiles, interfaces DMX, LEDs de la APC ni máquinas de efectos reales.

## Mezclador

El mezclador actúa sobre lo que sale, por encima de cada escena y de la edición en vivo. Pulsa **«Mezclador»** para verlo: hay una tira para **«Todos los fixtures»** y una por cada [grupo de fixtures](patch.md#grupos-de-fixtures).

Cada tira tiene:

- Un **fader de dimmer** con su porcentaje.
- **«Full»**: el grupo a tope. **«Blackout»**: el grupo a negro, salvo los efectos especiales protegidos (ningún botón del mezclador los apaga ni los desarma).
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

Mientras algún mapeo use «La escena seleccionada», o en el modo Mapear, la ficha de la escena seleccionada (la última que pulsaste) lleva una barra fina en su borde inferior (consulta [Mapeos](mapeos.md#la-escena-seleccionada)).
