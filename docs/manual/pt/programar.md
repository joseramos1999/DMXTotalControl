# Programar

![A página Programar](../img/programar.png)

**Programar** (F3) é o programador: você seleciona fixtures, dá a eles dimmer, cor, posição, gobos ou qualquer outro canal, aplica FX e paletas, e grava tudo como cena, cuelist ou cue.

O que você define no programador **sai na hora e manda sobre tudo** (cenas, grupos e playbacks) até você limpá-lo. O subtítulo da página lembra você disso: “N canais no programador — eles se sobrepõem aos playbacks até você limpá-los”.

A página tem, de cima para baixo: o cabeçalho com seus botões, a [linha de comando](programar.md#linha-de-comando), o painel de **fixtures** à esquerda e, à direita, o painel **Rápido** e o painel de **Features / Faders / Paletas / FX**.

## O cabeçalho

- **“Beam on”**: dimmer no máximo e shutter aberto na seleção.
- **“Beam off”**: dimmer em zero na seleção.
- **“Centralizar”**: pan e tilt no centro.
- **“Cego”**: veja [Cego](programar.md#cego).
- **“Redefinir tudo”**: libera todas as edições ao vivo (valores e FX). As cenas voltam a aparecer.
- **“Redefinir seleção”**: libera só as dos fixtures selecionados.
- **“Snapshot”** e **“Gravar…”**: veja [Gravar](programar.md#gravar).

Em uma janela estreita alguns botões mostram só o ícone; ao passar o mouse aparece o nome deles.

## Selecionar fixtures

O painel **“Fixtures (N sel.)”** mostra um cartão por fixture com o nome, o número (para a linha de comando), o endereço (universo.endereço) e o tipo dele.

- **Clique** em um cartão: seleciona só esse.
- **Ctrl+clique**: adiciona-o à seleção ou o tira dela. **Shift+clique**: seleciona todos os cartões desde o último que você clicou até esse (nesse sentido, que é a ordem da seleção); **Ctrl+Shift+clique** os adiciona.
- **“Todos”** e **“Nenhum”**, no cabeçalho do painel.
- **“Inverter”**: seleciona os que não estavam selecionados.
- **½**, **⅓**, **¼**: deixa um a cada dois, três ou quatro.
- **◀ ▶**: desloca a seleção para a esquerda ou a direita.

**A ordem importa**: a ordem em que você seleciona os fixtures é a ordem que seguem os leques, os FX e os Colour FX. No plano 2D cada fixture selecionado mostra seu número de ordem.

> **Dica:** você também pode selecionar digitando: `1 THRU 8` e Enter. Veja [a linha de comando](programar.md#linha-de-comando).

### Grupos de fixtures e seleções

Acima dos cartões aparecem os [grupos de fixtures](patch.md#grupos-de-fixtures) do Patch como etiquetas redondas:

- **Clique** em um grupo: seleciona os fixtures dele. **Ctrl+clique** ou **Shift+clique**: adiciona-os.
- A etiqueta se ilumina quando todos os fixtures dela estão selecionados. Ao passar o mouse aparece o número dela para a linha de comando (“GROUP 2 na linha de comando”).
- As **seleções** de um grupo (partes ordenadas, com seu ícone) selecionam os fixtures delas na sua ordem.
- Com muitos grupos aparecem três linhas de etiquetas, que rolam com a roda do mouse: **“Todos os grupos (N)”** as abre (até a metade do painel) e **“Menos”** as recolhe. Assim os cartões dos fixtures continuam à vista.

### Plano 2D

O botão da grade no cabeçalho do painel (“Plano 2D”) troca os cartões pelo plano do palco:

- **Clique** em um fixture o seleciona; **Ctrl+clique** ou **Shift+clique** o adiciona ou o tira.
- **Arraste** sobre o palco vazio para selecionar com um retângulo. Um clique no vazio tira a seleção.
- A **roda** do mouse faz zoom; arrastar com o **botão direito** ou o do meio move a vista, e um clique duplo com esses botões a centraliza de novo.

Em Programar o plano está sempre **ao vivo**: cada fixture mostra sua cor, seu dimmer, seu strobo (pisca) e seu gobo, e cada pixel das barras e matrizes. Os fixtures são posicionados no [plano 2D do Patch](patch.md#plano-2d).

## O painel Rápido

### Dimmer

O fader **Dimmer** e os botões **100%**, **75%**, **50%**, **25%** e **0%** dão intensidade à seleção.

### Cor

Clique em uma das 16 amostras, ou mova **Matiz** e **Sat**. A cor é misturada nos canais RGB (a parte branca vai para o canal branco se o fixture o tiver) ou CMY de cada fixture. Para as rodas de cores, use os presets delas em [Features](programar.md#features) › Cor.

> **Dica:** para um leque de dimmer, use a linha de comando: `@ 0 THRU 100` vai de 0 % no primeiro a 100 % no último. O leque da posição fica em [Features › Posição](programar.md#posição-grade-xy-e-leque).

## Features

**Features** mostra os canais da seleção por abas: **Dimmer**, **Cor**, **Posição**, **Gobo**, **Feixe**, **Foco** e **Outros**. As abas sem canais na seleção ficam desativadas.

Cada canal é um cartão com o nome, o valor em %, um controle deslizante e os **presets** dele como botões (com a cor ou a imagem do gobo, se o perfil os trouxer). Clique em um preset para aplicá-lo. O cartão fica marcado em laranja quando o canal está no programador; o **×** dele o libera.

Os canais são agrupados **por função**, não por nome: “Red”, “R” ou “Vermelho” de fixtures diferentes são o mesmo controle, e o mesmo vale para dimmer, pan/tilt, CMY, roda de cores, gobo, shutter, iris, zoom, foco, prisma… Cada cabeça de uma barra mantém seus canais.

### Cor: seletor e favoritos

Na aba **Cor**, o quadro da esquerda dá o matiz (de um lado ao outro) e a saturação (de cima para baixo). Arraste a cruz para posicioná-la, ou arraste em outro lugar do quadro para um ajuste fino.

Embaixo, **“Favoritos”**: o **+** salva a cor atual; um clique em um favorito a aplica; o botão direito o remove. Os favoritos servem para todos os shows.

### Posição: grade XY e leque

A aba **Posição** só move o **pan** e o **tilt** (com seus canais finos, em 16 bits). A rotação contínua de pan ou tilt e a velocidade dela ficam em **Outros**.

- Arraste a **cruz** para posicioná-la, ou arraste em outro lugar da grade para um movimento lento e fino.
- A barra da esquerda é o **tilt** e a de baixo, o **pan**.
- Com vários fixtures, cada um é um ponto na grade.
- **“Centralizar”** leva pan e tilt ao centro.

À direita fica o **Leque**: abre a posição da seleção, na ordem dela, cada fixture a partir de onde estava.

- **“Aberto”**: de uma ponta à outra (o primeiro para um lado, o último para o outro, o do meio parado).
- **“Em V”**: as pontas se abrem e o centro fica parado.
- **“A partir do primeiro”**: o primeiro parado e cada um um pouco mais.
- **Pan** e **Tilt**: quanto cada um se abre (de −100 a 100 %; clique duplo, para 0). Em 100 %, as pontas se movem meia grade.
- **“Sem leque”**: os fixtures voltam para onde estavam.
- O botão **“Leque”** sob a grade faz com que arrastar nela abra o leque em vez de mover: para os lados, o pan; para cima e para baixo, o tilt.

Se depois você mover a cruz, o centro se move e o leque se mantém. Se a posição mudar por outro caminho (uma paleta, outra seleção, “Redefinir tudo”), o leque volta a 0 e recomeça de onde os fixtures estiverem.

### Follow: apontar para um ponto

O botão **“Follow”** sob a grade troca o leque por um plano do palco. Clique ou arraste sobre ele: **todos os moving heads selecionados apontam para esse ponto**, cada um com seu pan e seu tilt, e uma linha vai de cada um até o ponto.

- **“Altura do ponto”**: 0 m é o chão; suba-a para apontar, por exemplo, para o rosto de quem canta (1,6 m).
- Cada cabeça calcula seu pan e seu tilt com seu lugar no plano, sua altura, se está pendurada ou em pé e para onde aponta. Tudo isso é ajustado em [Patch › Plano 2D › “Altura e orientação”](patch.md#altura-e-orientação-follow).
- Ao arrastar, cada cabeça vai pelo caminho curto a partir de onde apontava, sem dar a volta longa.
- Os valores ficam no programador como qualquer pan e tilt: grave-os em uma cena, uma cue ou uma paleta.
- Embaixo é indicado quantos apontam, quais não alcançam (ficam o mais perto que podem), quais não trazem o curso de pan e tilt na definição (são usados 540° e 270°) e os scanners, que não se movem (o espelho deles não segue esta geometria).
- As inversões, os limites e a troca de pan e tilt do [Patch](patch.md#limites) já são levados em conta.

Se você mover a cruz, o leque ou “Centralizar”, ou mudar a seleção, o ponto desaparece.

> **Não testado com equipamentos reais:** cada fabricante orienta seus moving heads do seu jeito. Se um apontar errado, ajuste a orientação dele ou marque “Pan invertido” ou “Tilt invertido” em “Altura e orientação”.

## Faders

**Faders** mostra cada canal da seleção como um fader vertical, com o nome, o grupo e o valor (0–255). O botão da lista abre os **presets** dele (“Nome (mín–máx)”) e o **×** o libera.

> **Dica:** depois de clicar em um fader ou um controle deslizante, as setas ↑ ↓ o movem de um em um e PgUp / PgDn de dez em dez. Clique duplo o devolve ao valor de repouso (normalmente, zero).

## Cego

Com **“Cego”** ativo, o programador edita **sem enviar nada para as luzes**: o palco continua mostrando o que está rodando e você prepara o próximo visual. O subtítulo avisa: “Cego: as luzes não mostram o que você programa”. Você pode gravar como sempre. Ao desativá-lo, o que está no programador sai.

## Destacar e localizar

Na barra da linha de comando:

- **“Destacar”**: a seleção no máximo em branco aberto, por cima de tudo, para ver quais fixtures são. Não é gravado. Pressione de novo para desativar.
- **“Localizar”**: coloca a seleção no máximo, em branco aberto, centralizada e com o feixe aberto, **no programador** (esses valores são gravados).
- **◀ ▶** (PREV / NEXT): percorre a seleção **um por vez**: só um fixture fica selecionado e aparece em qual você está (“2/8”). **“Todos”** volta à seleção inteira (ALL).

## Paletas

Uma **paleta** guarda um valor de intensidade, posição, cor ou feixe para cada fixture. As cenas e cues gravadas com uma paleta a usam **por referência**: se você atualiza a paleta, elas mudam sozinhas (as edições ao vivo também).

Em **Paletas** há quatro colunas: **Intensidade**, **Posição**, **Cor** e **Feixe**.

### Gravar uma paleta

1. Selecione fixtures e dê o valor a eles (por exemplo, uma cor).
2. Na coluna **Cor**, pressione **“+ Gravar”**.

A paleta é criada com um nome e um número (“Cor 1”, COL 1). A partir desse momento, o que você gravar com ela a segue.

### Usar uma paleta

Selecione fixtures e clique na paleta: ela é aplicada à seleção. Uma paleta fica marcada quando está aplicada na seleção (mais forte se estiver em toda ela). Em um fixture que não foi gravado na paleta, ela usa os valores de outro do mesmo tipo ou os atributos dele: uma cor é misturada em RGB ou CMY, ou vai para a posição mais parecida da roda de cores.

Também: `COL 2`, `POS 1`, `BEAM 3` ou `INT 4` na linha de comando.

### O menu de uma paleta

Botão direito em uma paleta:

- **“Aplicar à seleção”**.
- **“Carregar seus fixtures no programador”**.
- **“Atualizar com o programador”**: adiciona ou muda os valores do programador na paleta. As cenas que a usam também mudam.
- **“Gravar por cima”**: grava a paleta de novo só com o que há agora.
- **“Renomear…”**, **“Alterar o número…”** e **“Excluir”** (as cenas ficam com os valores).

Um ícone de link na paleta indica que há cenas que a usam; o texto ao passar o mouse diz quantas.

> **Dica:** as paletas podem ser colocadas no [SHOW](show.md) (ação “Uma paleta”), no [controle remoto](remoto.md) e em um [mapeamento](mapeos.md), sobre os fixtures delas ou sobre um grupo, como edição ao vivo. Alt+clique em uma paleta a leva para o SHOW.

## FX

Um **FX** move um parâmetro da seleção (o dimmer, o vermelho, o zoom, pan e tilt juntos…) em torno do valor dele no programador ou, se não tiver valor, em torno do que as cenas abaixo enviam. Ele roda na hora e é gravado com a cena.

Em **FX** há três zonas: os parâmetros e os FX rodando à esquerda, os controles do FX no centro e as macros à direita.

### Aplicar um FX

1. Selecione fixtures.
2. Em **Parâmetro**, escolha o que mover. Por padrão é escolhido o último parâmetro que você tocou. Os que já têm um FX na seleção têm a marca “FX”.
3. Pressione **“Iniciar FX”**, ou clique em uma **macro**.
4. Ajuste os controles: o FX muda ao vivo.

### Os controles do FX

- **A figura ou o modo** (o desenho da esquerda): clique nele para escolher outro. Para pan/tilt há **12 figuras** (círculo, oito, quadrado, triângulo, diamante, linha de pan, linha de tilt, diagonal, trevo, lissajous, ballyhoo e cruz). Para o resto, **22 modos**: a linha pontilhada é o valor base e a onda vai por cima, por baixo ou dos dois lados; os modos 9 a 12 são para perseguições, com passo.
- **Amplitude** (de 0 a 250 %). Em pan/tilt, **“Amplitude pan”** e **“Amplitude tilt”** separadamente.
- **Velocidade**, em BPM. **½** e **×2** a dividem ou a dobram. **“BPM do show”** faz com que ela siga o tempo do show (um fader ou dial de BPM do SHOW, ou um mapeamento de BPM ou Tap). Gravado em uma cena FX, ele segue em vez disso o ritmo que você marcar tocando o botão da cena, se ela tiver um ([Ao vivo](directo.md#ritmo-de-uma-cena-com-passos)). **“Multiplicador”**: de ×¼ a ×8.
- Só pan/tilt: **“Inverter pan”**, **“Inverter tilt”**, **“Ao contrário”** e **“Girar”** (0–359°).

### Timing do FX

Como o FX é distribuído entre os fixtures:

- **“Onda por X”**: a forma se espalha por X fixtures (ou conjuntos), cada um um pouco depois.
- **“Passo por X”**: um fixture (ou conjunto) depois do outro, de X em X.
- **“Agrupar”**: “Não”, “A cada X”, “Blocos de X”, “Dividir em X”, “Espelho” ou “Por grupo” (de fixtures).
- **“Ordem”**: “Seleção”, “Invertido”, “Aleatório” (com um botão para outra ordem aleatória) ou “Por número”.
- **“Espera”**: tempo na base depois de cada passada.
- **“Fase”**.

Embaixo, uma prévia animada desenha cada fixture enquanto ele se move.

### FX rodando, vincular e parar

- **“FX rodando (N)”** lista os FX do programador. Um clique seleciona os fixtures e o parâmetro de um FX; o **×** o para.
- **“Vincular”**: os FX vinculados compartilham a velocidade e o timing, e param juntos.
- **“Parar”** para o FX do parâmetro escolhido; **“Parar todos os FX”**, todos.

“Redefinir tudo” também tira os FX do programador. “Parar tudo” da barra superior não os tira: eles são edições ao vivo.

### FX próprios de cada fixture

Um FX continua sendo **um só** mesmo que você mude parte dos fixtures dele. Se você selecionar só alguns dos fixtures de um FX e mudar a **forma, o tamanho, o sentido (“Ao contrário”), as inversões, o giro ou a fase**, a mudança é **própria desses fixtures**, sobre o mesmo relógio: eles continuam sincronizados com os outros. A **velocidade e o timing** são sempre do FX inteiro.

- O painel avisa: “Este FX também está em outros fixtures…”.
- Com todos os fixtures dele selecionados, a mudança é do FX inteiro.
- Se você selecionar fixtures que não estavam no FX, eles se juntam a ele. Dois FX do mesmo parâmetro selecionados juntos se fundem em um, e cada fixture mantém o que fazia.
- Em “FX rodando” aparece quantos têm algo próprio (“N próprios”).
- **“Como o resto”** tira o que é próprio dos selecionados: eles voltam a fazer o mesmo que o FX.

### O espelho

Se os fixtures que vão ao contrário (ou com o pan ou o tilt invertido próprio) estiverem **todos em uma metade** do FX, cada um segue a onda do seu par do outro lado, então a onda também sai em espelho. O painel indica isso com “Espelho: …”.

Exemplo com 6 beams em círculo com onda:

1. Selecione os 6, escolha **Pan / tilt**, pressione **“Iniciar FX”**, figura **Círculo** e **“Onda por X”** em “Todos”.
2. Selecione só os 3 últimos e ative **“Ao contrário”**.

Os 3 últimos giram ao contrário e seguem a onda do 3.º, do 2.º e do 1.º: um espelho sincronizado. Se você colocar ao contrário fixtures alternados (2, 4 e 6) não é um espelho: cada um mantém seu lugar na onda.

### Macros de FX

À direita, as **macros de FX** de fábrica por categorias: **Posição** (círculo, círculo em onda, oito, ballyhoo, varredura de pan, can-can, pan em espelho, quadrado, triângulo, trevo), **Intensidade** (onda de dimmer, sequência, aleatório, pulso, respiração, strobo, de fora para dentro), **Cor** (arco-íris, vermelho / azul, fogo, ciano / magenta) e **Feixe** (zoom, iris, foco). Um clique aplica a macro à seleção.

**“Salvar como macro…”** salva os FX do programador como uma macro sua, em **“Minhas”**, para todos os shows. Botão direito em uma macro sua: **“Renomear”** ou **“Excluir”**.

## Linha de comando

A linha de comando funciona como a de uma mesa de luz. Ao abrir Programar ela recebe o teclado: digite e pressione **Enter**. Ao lado aparece o que ela fez, ou por que não entendeu.

| Comando | O que faz |
|---|---|
| `1 THRU 8` | Seleciona os fixtures 1 a 8 (também `1 > 8` ou `1 / 8`). `8 THRU 1`, em ordem inversa. |
| `1 + 3 - 2` | Soma e tira. `1,3,5` é o mesmo que `1 + 3 + 5`. Começar com `+ 5` ou `- 5` muda a seleção que já existe. |
| `GROUP 2` | O grupo de fixtures 2 (na ordem do Patch). `GROUP 1 THRU 3`: os grupos 1 a 3. |
| `ALL` | Todos os fixtures, por ordem de número (depois de NEXT / PREV, volta à seleção inteira). |
| `@ 50` | Dimmer em 50 %. `@ 5` é 50 % (um dígito são dezenas). |
| `@ FULL` / `@ OUT` | Dimmer no máximo ou em zero. `@ @` também é no máximo. |
| `@ 0 THRU 100` | Leque de dimmer, de 0 % no primeiro a 100 % no último. |
| `@ + 10` / `@ - 10` | Sobe ou desce o dimmer 10 %. |
| `POS 1`, `COL 2`, `BEAM 3`, `INT 4` | Aplica a paleta de posição, cor, feixe ou intensidade com esse número. |
| `LOCATE` | Localizar. |
| `HIGHLIGHT` | Destacar (alterna). |
| `NEXT` / `PREV` | Um por vez pela seleção. |
| `REL` | Libera as edições da seleção. |
| `CLEAR` | Esvazia o programador. |

É possível encadear: `1 THRU 4 @ FULL COL 2` seleciona, coloca o dimmer no máximo e aplica a paleta de cor 2.

As palavras também funcionam em espanhol: HASTA, GRUPO, TODO, LLENO, TOPE, CERO, INTENSIDAD, POSICION, COLOR, HAZ, LIMPIAR, RESALTAR, LOCALIZAR, SIGUIENTE, ANTERIOR, LIBERAR, IR, ATRÁS, GRABAR, ACTUALIZAR, ELIMINAR… e abreviadas: THR, FL, GRP (ou G), CLR, LOC, HI, SIG, ANT, REL.

### Comandos de cuelists

Atuam sobre a **cuelist selecionada**: a do playback selecionado, a que você abriu em Cenas ou a última que você criou.

| Comando | O que faz |
|---|---|
| `GO` | A próxima cue. |
| `BACK` | A cue anterior. |
| `GO CUE 3` | Salta para a cue 3. |
| `CUE 3` | Carrega a cue 3 no programador (o que o palco mostra nela) para alterá-la. |
| `REC` | Grava a próxima cue. `REC CUE 2.5` grava a cue 2.5 (entre a 2 e a 3). |
| `UPD` | Atualiza a cue carregada no programador ou, se não houver nenhuma, a que está rodando. `UPD CUE 4`, a cue 4. |
| `DELETE CUE 4` | Exclui a cue 4 mantendo o visual das seguintes. |
| `PB 3` | Seleciona o playback 3 da página atual (e a cuelist dele). |

Se um comando para no meio (por exemplo, a paleta não existe), o que ele já fez fica feito e na linha fica só a parte que falta corrigir.

### Histórico e teclado na tela

- **↑ ↓** percorrem os últimos comandos.
- **Esc** esvazia a linha (com a linha vazia, Esc é “Parar tudo”).
- O botão do **teclado** abre um teclado na tela para telas touch, com números, THRU, @, FULL, OUT, GROUP, INT, POS, COL, BEAM, NEXT, PREV, ALL, LOCATE, CLR e as teclas de cuelists (CUE, o ponto decimal, REC, UPD, GO, BACK, DELETE, PB). Ele fica aberto enquanto você escolhe fixtures.

## Gravar

### A janela “Gravar”

Pressione **“Gravar…”**:

1. Em **Gravar em**, escolha:
   - **“Nova cena”**: uma cena estática ou, se o programador tiver FX, uma cena FX com seus valores base e seus FX.
   - **“Nova cuelist”**: o programador será a cue 1 de uma nova cuelist. Com **“Colocá-la no primeiro playback livre”** marcada, ela também aparece em Playbacks.
   - **Uma cuelist que já existe**: o programador é gravado como uma cue dela. A cuelist selecionada aparece escolhida por padrão.
2. Digite o **Nome** (e, em uma cue, o número em **Cue**: vazio, o próximo; `2.5` fica entre a 2 e a 3).
3. Para uma cena ou uma cuelist nova: **“Adicionar ao grupo”** (ou “Sem grupo”) e **“Prioridade (0-10)”**.
4. Para uma cue: **“Só esta cue (o que ela muda não passa para as seguintes)”** e **“Todos os valores (cue de bloqueio)”** (também os valores iguais ao que ela herda, para que fique igual façam o que fizerem as cues anteriores).
5. Pressione **“Gravar”** (ou “Gravar a cue N”).

Uma cue guarda só o que o programador muda em relação ao que as cues anteriores deixam no palco. Se o número de cue já existir, o programa pergunta o que fazer: **“Mesclar”** (o programador entra na cue), **“Substituir”** (a cue guarda só o que está no programador; mantém o nome e os tempos dela) ou **“Inserir depois”** (uma nova cue entre esta e a seguinte).

**“Limpar o programador depois de gravar”** esvazia o programador depois de cada gravação e atualização. A opção fica memorizada.

### Snapshot

O menu **“Snapshot”** tem:

- **“Salvar as edições em uma cena”**: coloca os valores do programador em uma cena estática que já existe.
- **“Tudo o que está no palco, em uma nova cena…”**: tudo o que as luzes mostram agora (cenas, edições ao vivo e o mixer, sem o grand master) vira uma nova cena estática, em cada fixture com luz. Se “Destacar” estiver ativo, é preciso desativá-lo antes (“Desativar destaque”).
- **“Só as edições, em uma nova cena…”**.

### Editar cenas e cues

- **Uma cena estática ou FX**: na página Ao vivo ou em Cenas, **“Editar no programador”** a carrega no programador. Mude o que quiser e, em [Cenas](escenas.md), pressione **“Salvar o programador nesta cena”**.
- **Uma cue**: ao carregá-la (clique duplo na cue em Cenas, “Carregar”, ou `CUE 3`), aparece uma barra “Editando a cue 3 de “Música 1””. Mude os valores e pressione **“Atualizar ▾”**: **“Atualizar a cue 3 (as seguintes acompanham)”** ou **“Só esta cue (as seguintes mantêm o visual)”**. O **×** para de editar a cue (o programador mantém os valores).

## Limpar o programador

- **“Redefinir tudo”** (ou `CLEAR`): libera todo o programador, valores e FX.
- **“Redefinir seleção”** (ou `REL`): só os fixtures selecionados.
- O **×** de um cartão ou de um fader: só esse canal.

O que acontece com a edição ao vivo ao disparar uma cena, e a prioridade dela frente às cenas, é decidido em [Opções](opciones.md#reprodução).
