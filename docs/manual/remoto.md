# Control remoto

El **control remoto** muestra las páginas del [SHOW](show.md) en el navegador de un móvil o una tablet conectados a la misma red wifi que el ordenador. No hay que instalar nada: funciona en los dos sentidos, así que lo que tocas en el móvil cambia en el ordenador y lo que cambia en el ordenador se ve en el móvil.

## Activarlo

1. En el ordenador, abre **Opciones** (F9) › **Control remoto**.
2. Activa **«Control remoto activado»**.
3. Si quieres, escribe un **PIN**. Sin PIN, cualquiera que esté en la red puede usarlo.
4. Debajo aparecen las **direcciones** del ordenador (por ejemplo `http://192.168.1.20:8800/`), una por cada red a la que está conectado, con el nombre de la red delante. La wifi va primero. El botón de copiar pone la dirección en el portapapeles. Si el ordenador tiene **nombre en la red**, la primera es la del nombre, por ejemplo `http://dmxtotalcontrol.local:8800/` (más abajo).

## Conectar el móvil o la tablet

1. Conecta el móvil a la **misma red wifi** que el ordenador.
2. Abre el navegador y escribe una de las direcciones, o escanea el código QR (más abajo).
3. Si hay PIN, escríbelo y pulsa **OK**. Se pide una sola vez en cada dispositivo.

En Opciones se ve cuántos dispositivos están conectados.

> **Consejo:** añade la página a la pantalla de inicio del móvil: se abre como una aplicación, con el icono del programa.

> **Consejo:** si el móvil no conecta, comprueba que los dos están en la misma red y que el cortafuegos de Windows permite a DMX Total Control usar la red. Si otro programa usa el puerto, cámbialo en Opciones (el aviso aparece en rojo).

> **Consejo:** la primera vez que activas el control remoto, Windows pregunta si DMX Total Control puede usar la red. Permítelo en las redes **privadas**. Si Windows tiene la wifi como red **pública**, el móvil no llega al ordenador: cámbiala a privada en la configuración de red de Windows.

> **Consejo:** si la wifi del local no deja que los dispositivos se vean entre sí, activa en el ordenador la **zona con cobertura inalámbrica móvil** de Windows y conecta el móvil a ella. El remoto queda siempre en `http://192.168.137.1:8800/`.

## Escanear el código QR

Con el control remoto activado, debajo de las direcciones aparece un **código QR**. Con él, el remoto se abre sin escribir nada:

1. Conecta el móvil o la tablet a la **misma red wifi** que el ordenador.
2. Apunta al código con la cámara.
3. Toca el enlace que aparece.

- El código lleva la dirección que tiene el borde de color: al principio, la primera dirección numérica, que funciona en todos los móviles. Para usar otra, haz clic en su fila o en su botón de código QR.
- Si hay PIN, el código también lo lleva: el móvil entra sin pedirlo. El PIN no se queda en la barra de direcciones del navegador.
- Por eso, con PIN, cualquiera que haga una foto del código puede entrar.
- **«Más grande»** muestra el código en grande, para escanearlo a un par de metros.
- En el tema **Noche** el código se ve más tenue, para no deslumbrar.

## Encontrarlo por nombre

El router puede dar al ordenador otra dirección de un día para otro. El nombre no cambia: con el control remoto activado, el móvil abre `http://dmxtotalcontrol.local:8800/` aunque la dirección sea otra. Así, el icono de la pantalla de inicio sigue funcionando.

Si abres el remoto con el código QR o con una dirección numérica, el móvil comprueba en segundo plano si llega al ordenador por el nombre. Si llega, se pasa solo a `http://dmxtotalcontrol.local:8800/`, en la misma página y sin pedir el PIN. Añade el icono a la pantalla de inicio después de ese cambio: la barra de direcciones ya muestra el nombre.

- En Opciones, la dirección **«Por nombre»** va la primera de la lista.
- El nombre se cambia en **Nombre en la red**. Admite letras, números y guiones; las mayúsculas pasan a minúsculas y los espacios a guiones. Vacío, no hay nombre.
- Funciona en iPhone y iPad, y en móviles Android 12 o posteriores. En los más antiguos, usa una dirección numérica o el código QR.
- No funciona si el móvil tiene una VPN activada o usa los datos móviles.
- Si el móvil no llega por el nombre, se queda en la dirección numérica sin avisar.
- Si después el nombre deja de responder, el móvil vuelve a la dirección numérica, siempre que esa dirección responda.
- Si otro ordenador de la red ya usa el nombre (por ejemplo, otro DMX Total Control), este toma el siguiente, `dmxtotalcontrol-2`, y Opciones lo indica debajo del nombre.
- Si el puerto de red 5353 no está disponible, o si otros ordenadores usan ya el nombre y del `-2` al `-9`, aparece un aviso en rojo. Las direcciones numéricas siguen funcionando.

## Qué se ve

Arriba está la barra del remoto:

- El **nombre del show** y las **pestañas** de las páginas del SHOW. El móvil recuerda la última página que abriste.
- Un **punto** que indica si hay conexión con el ordenador.
- **BO**: blackout.
- El botón de **pantalla completa**.

Debajo, la página con sus secciones, como en el ordenador:

- **Botones**: se pulsan con el dedo, también los flash (mientras el dedo está encima). Se iluminan cuando están activos.
- **Faders**: se arrastran; los que tienen botón **Flash** lo llevan debajo.
- **Diales y ruedas**: se mueven arrastrando el dedo arriba y abajo. Los faders y los diales siguen el mismo [rango](show.md#faders) que en el ordenador y muestran el valor real.
- **Grupos**: sus escenas como botones y su master.
- **Playbacks**: las tiras con su nombre, la cue que suena y la siguiente, **GO**, **Atrás**, su fader y **Flash**. Un fader A/B muestra «A/B» y lo que lleva su fundido, con el borde discontinuo. Las secciones que siguen la página actual tienen ◀ ▶ para cambiarla.
- **Rueda de color** y **rejilla XY**, con sus botones **Liberar** y **Centro**.
- **Reloj**, **Ritmo** (**GO**, **Stop** y **BeatGO**; los botones de escenas con pasos, cuelists o FX en «BPM del show» llevan dentro su barra del ritmo), **notas**, **sonando ahora** e **imagen**.
- **Temporizador**: es el mismo que en el ordenador, con **Iniciar** / **Pausa** y **Reiniciar**. Si lo inicias en el móvil, cuenta también en el ordenador. Si lo pausas en el ordenador, se para en todos los móviles. En cuenta atrás se pone naranja en el último minuto y rojo al llegar a cero.
- **Niveles DMX**: los 512 canales del universo elegido, como en el ordenador: cada casilla brilla más cuanto mayor es su valor. El móvil los recibe unas 8 veces por segundo, solo mientras esa página está en pantalla.
- **Mezclador en vivo**: las mismas tiras que el [mezclador](directo.md#mezclador) del ordenador, una al lado de otra: «Todos los fixtures» y una por cada grupo de fixtures. Si no caben, se desplazan con el dedo. Cada tira tiene el dimmer, **Full**, **Blackout**, **Tono**, **Saturación**, **Restablecer color**, **Estrobo** con su velocidad, **Flash**, **Flash excl.** y **Full excl.**
  - **Flash** y **Flash excl.** funcionan mientras el dedo está encima. Se sueltan si el móvil pierde la conexión, se bloquea o cambias de aplicación.
  - Al soltarlos se apagan, aunque estuvieran fijos en el ordenador (Ctrl+clic).
  - El borde de la tira se pone naranja con un flash o un full, y rojo con blackout.
  - Lo que cambias en el móvil cambia en el ordenador, y al revés.

Si algún playback tiene algo asignado, el remoto añade una página **«Playbacks»** con las tiras de la página de playbacks actual.

El remoto usa el mismo [tema](opciones.md#general) que el programa y sale en el idioma del móvil si es español, portugués, francés, italiano o alemán (si no, en inglés).

## Si se corta la conexión

Si el móvil pierde la conexión, aparece «Sin conexión con DMX Total Control. Reintentando…» y vuelve a conectar solo. Lo que el dedo tenía pulsado (un flash, un botón) se suelta en el ordenador, para que nada se quede encendido.
