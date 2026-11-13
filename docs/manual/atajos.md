# Atajos de teclado

Todas las teclas y combinaciones del programa, por página. Las teclas que asignes tú en [Mapeos](mapeos.md#teclado) y las teclas de los botones del [SHOW](show.md#opciones-de-un-botón) se suman a estas. Una tecla asignada en Mapeos tiene preferencia sobre el atajo del programa que use la misma tecla.

## En cualquier página

| Tecla | Qué hace |
|---|---|
| **F1** | Directo |
| **F2** | Show |
| **F3** | Programar |
| **F4** | Escenas |
| **F5** | Patch |
| **F6** | Fixtures |
| **F7** | Salidas |
| **F8** | Mapeos |
| **F9** | Opciones |
| **F10** | Playbacks |
| **F11** | Pantalla completa (y volver a la ventana) |
| **F12** | Abre este manual en la página en la que estás (también el botón **«?»** de la barra superior) |
| **Ctrl+N** | Nuevo show |
| **Ctrl+O** | Abrir un show |
| **Ctrl+S** | Guardar el show (la primera vez, pide nombre) |
| **Ctrl+Mayús+S** | Guardar el show con otro nombre («Guardar como…») |
| **Ctrl+B** | Blackout (encender y apagar) |
| **Esc** | Parar todo. En el modo Mapear, cancela el control pendiente o sale del modo; con «Grabar» armado en Playbacks, lo desarma |
| **Ctrl+→** | Escena siguiente en cada grupo que suena |
| **Ctrl+←** | Escena anterior en cada grupo que suena |
| **G** | BeatGO (menos en la página Show) |

Si una página está bloqueada, su tecla pide la contraseña (si la hay) y desbloquea. Con el SHOW bloqueado (modo quiosco) las teclas de página no funcionan, y con el Setup bloqueado tampoco Ctrl+N, Ctrl+O ni Ctrl+Mayús+S.

T, G y las teclas que asignes en Mapeos no actúan mientras escribes en un campo de texto, como la línea de comandos de Programar.

## Playbacks

| Tecla | Qué hace |
|---|---|
| **Espacio** | GO del playback seleccionado (un GO por pulsación) |
| **Mayús+Espacio** | Atrás en el playback seleccionado |
| **RePág** | Página de playbacks anterior |
| **AvPág** | Página de playbacks siguiente |
| **Esc** | Desarma «Grabar» |

Funcionan en la página Playbacks siempre que no estés escribiendo en un campo de texto.

## Programar

| Tecla | Qué hace |
|---|---|
| Escribir | Lo que escribes en la página va a la línea de comandos |
| **Intro** | Ejecuta el comando |
| **↑** / **↓** | Los últimos comandos |
| **Esc** | Vacía la línea de comandos (con la línea vacía, Parar todo) |

Consulta [la línea de comandos](programar.md#línea-de-comandos).

## Show

| Tecla | Qué hace |
|---|---|
| La tecla de un botón | Pulsa ese botón (mientras la mantienes, si es flash) |
| **Supr** / **Retroceso** | En el modo edición, quita el control de la celda seleccionada |
| **Esc** | En el modo edición, quita la selección |

## Super escenas

| Tecla o gesto | Qué hace |
|---|---|
| **Espacio** | Reproducir o parar desde el cursor |
| **Supr** / **Retroceso** | Elimina los bloques seleccionados |
| **Ctrl+A** | Selecciona todos los bloques de las pistas a la vista |
| **Ctrl+D** | Duplica los bloques seleccionados justo detrás |
| **Ctrl+C** | Copia los bloques seleccionados |
| **Ctrl+X** | Corta los bloques seleccionados (los de pistas bloqueadas solo se copian) |
| **Ctrl+V** | Pega los bloques copiados en el cursor, desde la pista seleccionada |
| **Ctrl+Z** | Deshace el último cambio (no mientras escribes en un campo) |
| **Ctrl+Y** / **Ctrl+Mayús+Z** | Rehace lo deshecho |
| **Ctrl+clic** en un bloque | Lo añade a la selección o lo quita |
| **Mayús+clic** en un bloque | Selecciona desde el último bloque elegido hasta este |
| Arrastrar en una zona vacía de las pistas | Dibuja un recuadro que selecciona los bloques que toca (con **Ctrl** o **Mayús**, los suma) |
| Clic derecho en una zona vacía de una pista | «Pegar aquí» y «Seleccionar todo» |
| **Ctrl+rueda** | Zoom en la línea de tiempo |
| **Mayús+rueda** | Desplazarse en el tiempo |
| Doble clic en la regla | Reproducir desde ahí |

Las teclas de los bloques no actúan mientras escribes en un campo. Consulta [Seleccionar varios bloques](escenas.md#seleccionar-varios-bloques) y [Copiar y pegar](escenas.md#copiar-y-pegar).

## Faders y deslizadores

Después de hacer clic en un fader o un deslizador:

| Tecla o gesto | Qué hace |
|---|---|
| **↑** / **→** | Sube un paso |
| **↓** / **←** | Baja un paso |
| **RePág** / **AvPág** | Sube o baja diez pasos |
| Doble clic | Lo devuelve a su valor de reposo (normalmente, cero) |

## Cuelists y pasos (Escenas)

| Tecla o gesto | Qué hace |
|---|---|
| **Ctrl+clic** / **Mayús+clic** | Selecciona varias cues o pasos para cambiar sus tiempos a la vez |
| Doble clic en una cue o un paso | Lo carga en el programador |
| Doble clic en el número de una cue | Cambia su número |
| Rueda sobre un tiempo pulsado | ±0,1 s (con **Mayús**, ±1 s) |
| **Ctrl+Z** | Deshace la última edición de la cuelist abierta (en un campo con texto escrito, primero ese texto) |
| **Ctrl+Y** / **Ctrl+Mayús+Z** | Rehace lo deshecho |
| **Esc** en un campo | Deja el valor como estaba |

## Ratón y modificadores

| Gesto | Dónde | Qué hace |
|---|---|---|
| **Alt+clic** | Casi cualquier control | Lo lleva a la página del SHOW abierta |
| **Mayús+clic** | Modo Mapear | Usa el control en lugar de mapearlo |
| **Ctrl+clic** | Botones Flash del mezclador | Dejan el flash encendido hasta el siguiente clic |
| **Ctrl+clic** / **Mayús+clic** | Fixtures, grupos y plano 2D (Programar y Patch), lista de escenas, bloques de una super escena | Añade o quita de la selección |
| Rueda | Plano 2D | Zoom |
| Arrastrar con el botón derecho o central | Plano 2D | Mueve la vista (doble clic: la centra) |
| Clic derecho | Botón 15° del plano 2D | Gira hacia el otro lado |
| Clic derecho | Botón del SHOW (en directo) | Para lo que lanza (su escena, su grupo, su playback o su paleta), también con GO |
| Mantener pulsado «Desbloquear» | Show bloqueado | Desbloquea |
