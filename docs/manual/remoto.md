# Control remoto

El **control remoto** muestra las páginas del [SHOW](show.md) en el navegador de un móvil o una tablet conectados a la misma red wifi que el ordenador. No hay que instalar nada: funciona en los dos sentidos, así que lo que tocas en el móvil cambia en el ordenador y lo que cambia en el ordenador se ve en el móvil.

## Activarlo

1. En el ordenador, abre **Opciones** (F9) › **Control remoto**.
2. Activa **«Control remoto activado»**.
3. Si quieres, escribe un **PIN**. Sin PIN, cualquiera que esté en la red puede usarlo.
4. Debajo aparecen las **direcciones** del ordenador (por ejemplo `http://192.168.1.20:8800/`), una por cada red a la que está conectado. El botón de copiar pone la dirección en el portapapeles.

## Conectar el móvil o la tablet

1. Conecta el móvil a la **misma red wifi** que el ordenador.
2. Abre el navegador y escribe una de las direcciones.
3. Si hay PIN, escríbelo y pulsa **OK**. Se pide una sola vez en cada dispositivo.

En Opciones se ve cuántos dispositivos están conectados.

> **Consejo:** añade la página a la pantalla de inicio del móvil: se abre como una aplicación, con el icono del programa.

> **Consejo:** si el móvil no conecta, comprueba que los dos están en la misma red y que el cortafuegos de Windows permite a DMX Total Control usar la red. Si otro programa usa el puerto, cámbialo en Opciones (el aviso aparece en rojo).

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
- **Playbacks**: las tiras con su nombre, la cue que suena y la siguiente, **GO**, **Atrás**, su fader y **Flash**. Las secciones que siguen la página actual tienen ◀ ▶ para cambiarla.
- **Rueda de color** y **rejilla XY**, con sus botones **Liberar** y **Centro**.
- **Reloj**, **BPM** (con **Tap** y **BeatGO**), **notas**, **sonando ahora** e **imagen**.

Los **temporizadores**, el **mezclador en vivo** y los **niveles DMX** aparecen como «Solo en el ordenador».

Si algún playback tiene algo asignado, el remoto añade una página **«Playbacks»** con las tiras de la página de playbacks actual.

El remoto usa el mismo [tema](opciones.md#general) que el programa y sale en el idioma del móvil si es español, portugués, francés, italiano o alemán (si no, en inglés).

## Si se corta la conexión

Si el móvil pierde la conexión, aparece «Sin conexión con DMX Total Control. Reintentando…» y vuelve a conectar solo. Lo que el dedo tenía pulsado (un flash, un botón) se suelta en el ordenador, para que nada se quede encendido.
