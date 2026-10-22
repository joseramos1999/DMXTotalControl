# Fixtures

![La página Fixtures](img/fixtures.png)

Para patchear un aparato, el programa necesita su **definición**: qué canales tiene, qué hace cada uno y en qué modos trabaja. Esta página explica de dónde salen las definiciones y cómo crear las tuyas en la página **Fixtures** (F6).

## La biblioteca de fixtures

Al añadir fixtures en el [Patch](patch.md#añadir-fixtures) eliges entre todas estas definiciones:

- **La biblioteca incluida**: miles de fixtures de muchos fabricantes, en formato `.qxf`.
- **Los genéricos**: 16 fixtures básicos en el fabricante **Generic** (PAR LED RGB, RGBA, RGBW, RGBWA, RGBWA+UV y blanco, barra LED de 8 segmentos, cabezas spot, beam y wash, scanner, estrobo, blinder, láser, máquina de humo y hazer), además del **Generic Dimmer**.
- **La Open Fixture Library**, si la descargas.
- **Los fixtures GDTF** que importes.
- **Tus fixtures**: los que creas en esta página o importas.

En [Opciones › Biblioteca de fixtures](opciones.md#biblioteca-de-fixtures) se ve cuántas definiciones hay disponibles y está todo lo necesario para ampliarla.

> **Consejo:** para que Programar, los FX y las paletas reconozcan la función de cada canal (dimmer, rojo, pan, gobo…), la definición tiene que traer el grupo o el preset del canal. Las de la biblioteca lo traen; en las tuyas, elige bien el preset de cada canal.

## Crear un fixture

La página **Fixtures** es el creador de fixtures. A la izquierda, **«Mis fixtures»** lista los que has creado; a la derecha, el editor.

1. Pulsa **«Nuevo fixture»**, o **«Partir de la biblioteca…»** para empezar desde una copia de uno que ya existe (se llama igual con «(copia)»).
2. En **Fixture**, escribe el **Fabricante** (por ejemplo `Generic` o la marca), el **Modelo** (`PAR 18x10W RGBWA`) y elige el **Tipo**.
3. Añade los **canales** (consulta abajo).
4. Prepara los **modos**.
5. Pulsa **«Guardar fixture»**.

El fixture queda disponible al momento en el Patch, dentro de su fabricante. Arriba se ve su estado: «Cambios sin guardar», «Guardado en tus fixtures» o «Sin guardar todavía». Si abres otro fixture con cambios sin guardar, se pregunta si descartarlos («Seguir editando» / «Descartar cambios»).

### Canales

En **Canales**, el botón **«Añadir»** ofrece los canales más habituales (dimmer, rojo, verde, azul, blanco, ámbar, UV, pan y pan fino, tilt y tilt fino, estrobo, rueda de color, rueda de gobos, velocidad de pan/tilt) y **«Canal personalizado»**.

En cada canal:

- Su **nombre** y su **preset** (el desplegable): el preset dice qué hace el canal. Cambiar de preset cambia también el nombre, salvo que le hayas puesto uno propio.
- La papelera lo elimina.

Un canal **«Personalizado»** tiene además:

- **Grupo**: su función general (intensidad, color, gobo, haz…).
- **Color**: en un canal de intensidad, qué color es (rojo, verde…) o «Ninguno» para un dimmer.
- **Fino**: es el canal fino (16 bits) de otro.
- **«Rangos (N)»**: qué hace cada rango de valores DMX, por ejemplo 0-7 Cerrado, 8-255 Estrobo. Pulsa **«Añadir rango»** y escribe su **Descripción**. Los rangos no se pueden solapar. Son los presets que luego verás como botones en Programar.

Los canales nuevos se añaden al final del modo que tengas abierto.

### Modos

En **Modos**, **«Añadir»** crea un modo con todos los canales. Cada pestaña es un modo:

- **Nombre** del modo y la papelera para eliminarlo (siempre queda uno).
- La lista es el **orden DMX** del modo: marca los canales que usa y muévelos con **▲ ▼** hasta su posición. El número de la izquierda es su canal DMX dentro del modo.

### Eliminar un fixture tuyo

Abre el fixture en «Mis fixtures» y pulsa **«Eliminar»**. Desaparece de la biblioteca en el próximo arranque.

Tus fixtures se guardan como archivos `.qxf` en la carpeta de fixtures de usuario (en Opciones › «Carpeta de fixtures de usuario» › «Abrir carpeta»).

## Open Fixture Library

La Open Fixture Library es una biblioteca abierta con miles de fixtures adicionales (licencia MIT).

1. Abre **Opciones** (F9).
2. En **Biblioteca de fixtures › Open Fixture Library**, pulsa **«Descargar»**. Hace falta internet.
3. Espera a que termine la barra de progreso («Cancelar» la para).

Al terminar se dice cuántos fixtures se han descargado y cuántos son nuevos. Después, en el mismo sitio se ve cuántos tiene y cuándo se descargó.

Para ponerla al día, pulsa **«Actualizar»** (el mismo botón). Si en Opciones está activado «Buscar actualizaciones al arrancar», el programa mira una vez al día en GitHub si la biblioteca tiene fixtures nuevos o mejorados desde tu descarga, te avisa y resalta el botón; «Buscar actualizaciones» lo comprueba en el momento. La actualización:

- añade los fixtures nuevos (debajo se listan);
- cambia los que han mejorado, también los que tengas patcheados (si su modo nuevo no cabe donde están, se quedan con la versión anterior hasta que los vuelvas a patchear);
- conserva los que la biblioteca ya no tiene, por si un show los usa;
- no toca tus fixtures ni los de la biblioteca incluida: si tienen el mismo fabricante y modelo, se queda el tuyo (o el incluido).

Una descarga que no se puede leer, o que se corta, no cambia nada.

## Importar GDTF

GDTF es el formato abierto que publican los fabricantes en GDTF-Share (con una cuenta gratuita).

1. Descarga los `.gdtf` de tus aparatos (o el `.zip` de la descarga múltiple de GDTF-Share). El botón **«GDTF-Share»** de Opciones abre su web.
2. En **Opciones › Biblioteca de fixtures › Fixtures GDTF**, pulsa **«Importar GDTF…»** y elige los archivos, carpetas o el `.zip`.
3. Se convierten en segundo plano, con una barra de progreso. **«Parar»** detiene la importación.

Al terminar se indica cuántos se han convertido y, si alguno no se ha podido leer, por qué. Si el mismo fixture ya está en otra biblioteca, el de GDTF se añade como «(GDTF)» y puedes usar los dos. Importarlo otra vez lo actualiza.

De cada modo se usa el primer *DMX break*; los canales virtuales (sin DMX) se omiten.

Algunos canales cambian de función según el valor de otro canal (por ejemplo, la rotación del prisma solo gira con el prisma en «rotación»). En ellos se omiten las funciones que no hacen nada y, donde hay varias, cada tramo lleva sus nombres juntos (por ejemplo, «0 deg / Stop»). Para verlo en un fixture importado antes, importa su `.gdtf` otra vez.

## Importar archivos de fixtures

**Opciones › Importar archivos de fixtures › «Importar…»** admite definiciones `.qxf` (el formato de QLC+), archivos `.gdtf` o un `.zip` que los contenga. Se añaden a la biblioteca y se indica cuántos son nuevos.

### Importar mis fixtures de QLC+

Si creaste fixtures en QLC+ en este ordenador, **Opciones › «Importar mis fixtures de QLC+» › «Importar»** los copia a tus fixtures.

## Formatos que no se importan

Las bibliotecas de otros programas en formatos cerrados o cifrados no se pueden importar. La mayoría de los fixtures están en la Open Fixture Library o en GDTF-Share; los que falten, créalos en esta página.
