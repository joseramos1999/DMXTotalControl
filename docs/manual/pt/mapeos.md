# Mapeamentos

![A página Mapeamentos](../img/mapeos.png)

**Mapeamentos** (F8) serve para controlar o show de fora: controladores **MIDI**, **OSC** a partir de outro aplicativo, uma mesa por **entrada DMX** ou o **teclado**. Um mapeamento une um controle do seu dispositivo (um pad, um fader, uma tecla…) a algo do programa (uma cena, um master, um GO…). Os mapeamentos são salvos com o show.

A página tem três abas: **“Mapeamentos”**, **“Dispositivos”** e **“Monitor”**, e o botão **“Aprender”**.

## Dispositivos

Em **“Dispositivos”** aparecem os dispositivos de entrada que o programa encontra, com o tipo (MIDI, OSC ou DMX), o nome e o universo que eles usam. Primeiro aparecem os controladores (MIDI e OSC) e os dispositivos que você já usa, em **“Controladores e dispositivos em uso”**; depois, em **“Entradas DMX: interfaces e rede”**, as demais entradas DMX, com uma linha para cada placa de rede.

1. Conecte o controlador e, se ele não aparecer, pressione **“Buscar novamente”**.
2. Ative a **“Entrada”** dele.
3. Se o dispositivo tiver saída, ative **“Feedback (LEDs)”** para que o programa acenda os LEDs dele ou mova os faders motorizados dele.

Um mapeamento de um dispositivo que não está conectado é mantido: ele volta a funcionar quando você conecta o dispositivo.

## Modo Mapear

A forma mais rápida de criar mapeamentos é o **modo Mapear** (aprender):

1. Ative a entrada do seu dispositivo em “Dispositivos”.
2. Pressione o **alvo** da barra superior (ou **“Aprender”** nesta página). Os controles que podem ser mapeados ficam marcados em **roxo**; um número no canto diz quantos mapeamentos eles já têm.
3. Clique em um controle do programa: uma cena de Ao vivo, um master, um botão do mixer, um playback…
4. A barra roxa diz “Mova um controle do seu dispositivo ou pressione uma tecla para: …”. Mova o fader, pressione o pad ou pressione uma tecla.
5. O programa avisa “Mapeado: …”. Continue com o próximo controle.
6. Pressione **“Concluído”** (ou Esc) para sair.

No modo Mapear, **Shift+clique** continua usando o controle em vez de mapeá-lo. **Esc** cancela o controle pendente; outro Esc sai do modo. O botão “Mapeamentos” da barra roxa traz você a esta página.

Um controle do programa pode ter vários mapeamentos (por exemplo, um pad de um controlador e uma tecla).

> **Dica:** ao aprender um botão de uma Akai APC, o mapeamento já vem com os LEDs em verde enquanto roda.

### O que pode ser mapeado

- **Barra superior**: GO, Stop, BeatGO, grand master, Blackout e Parar tudo.
- **Ao vivo**: cada cena, ◀ ▶, Pausar e Liberar grupos do cabeçalho, e em cada grupo o liberar, o anterior, o próximo e o master.
- **Mixer**: em cada tira, o dimmer, o matiz, a saturação, o strobo e sua velocidade, e os botões Full, Blackout, Flash, Flash excl. e Full excl.
- **Programar**: as paletas.
- **Playbacks**: os nomes de página, ◀ ▶ de página, “Liberar tudo”, “Voltar”, o GO grande, e em cada playback o ×, o nome (selecionar), os três botões e o fader.
- **SHOW**: os botões e os faders (o fader inteiro, com todos os seus elementos), e ◀ ▶ das seções de playbacks.

## A lista de mapeamentos

Em **“Mapeamentos”**, cada linha mostra o controle do dispositivo, o que ele controla e o comportamento dele. Um ponto roxo indica um botão e um azul, um fader. Clique em um mapeamento para editá-lo à direita, ou pressione **“Adicionar”** para criar um vazio.

## Editar um mapeamento

### Controle

- **O tipo**: **MIDI**, **OSC**, **“Entrada DMX”** ou **“Teclado”**.
- **Dispositivo**: um específico ou **“Qualquer dispositivo”**.
- **MIDI**: **CC** ou **Nota**, o número (0–127) e o **Canal** MIDI (1–16).
- **OSC**: o caminho, por exemplo `/scene/1`.
- **Entrada DMX**: o **Canal** (1–512).
- **Teclado**: a tecla, por exemplo `Q` ou `F9`.
- **“Aprender o controle”**: pressione e mova o controle do dispositivo para preencher tudo o que está acima.

### Controla

O que o mapeamento faz. Os **botões**:

| Destino | O que faz |
|---|---|
| Disparar uma cena | A cena escolhida, no grupo dela. |
| Cena N de um grupo | A cena que está nessa posição do grupo (**Número da cena**). |
| Liberar um grupo / Próxima cena de um grupo / Cena anterior de um grupo | Em um grupo ou em “Todos os grupos”. |
| Blackout, Parar tudo, Pausar tudo, Tap tempo, BeatGO, GO, Stop | Como os botões deles (GO e Stop escolhem o modo, como na barra superior: sempre há um ligado). |
| Paleta | A paleta nos próprios fixtures dela ou em um grupo de fixtures. |
| Mixer: full, blackout, flash, flash exclusivo, full exclusivo, strobo | Na tira escolhida do mixer. |
| Playback: GO, voltar, pausa / voltar, flash, liberar, selecionar | Em um playback. |
| Playback: ir para uma cue | Salta para a **Cue** indicada (1 é a primeira da lista). |
| GO do playback selecionado / Voltar do playback selecionado | Como o GO grande e “Voltar” de Playbacks. |
| Página de playbacks N / próxima / anterior | Muda a página de playbacks. |
| Liberar todos os playbacks | Como “Liberar tudo”. |

Os **faders**:

| Destino | O que move |
|---|---|
| Grand master | O GM. |
| Master de um grupo | O master desse grupo. |
| Dimmer ao vivo de uma cena / Velocidade ao vivo de uma cena | Os controles deslizantes ao vivo dela. |
| BPM | O tempo. |
| Mixer: dimmer, matiz, saturação, velocidade do strobo | Na tira escolhida. |
| Fader do SHOW | Um fader do SHOW inteiro. |
| Playback: fader | O fader de um playback. |

Para os playbacks escolhe-se a **página** (“Página atual (acompanha)”, que acompanha a página que está à vista, ou uma fixa) e o número do **Playback**. Com **“Ou uma cuelist ou uma cena, onde estiver”** o mapeamento atua sobre essa cuelist ou cena em qualquer playback onde ela estiver. Em “ir para uma cue” aparece para qual cue ele leva agora (“Agora: cue 3 de Música 1”).

### Comportamento

Nos **botões**:

- **“Alternar: cada toque liga / desliga”**: cada toque liga ou desliga. Em uma cena, ele se chama **“Pressionar: conforme GO / Stop”**: ele a dispara ou a para conforme GO e Stop da barra superior.
- **“Flash: enquanto pressionado”**.
- **“Só ligar”** / **“Só desligar”**.

Os botões de playback atuam ao pressionar, exceto o flash, e não têm esta opção.

Nos **faders**:

- **“Absoluto: segue o fader”**.
- **“Incremental: um passo por toque”** (para botões que sobem ou descem).
- **“Relativo: encoder sem fim”** e **“Relativo: encoder, 1 sobe e 127 desce”** (encoders sem fim).
- **“De”** … **“a”**: a faixa, de 0 a 100 %. **“Inverter”** a inverte.
- **“Passo %”** e **“Em loop”** (incremental e relativo).
- **“Pegar o fader (sem motor)”** (absoluto): o fader do controlador só move o nível depois de alcançá-lo, sem saltos.

### Feedback

Com MIDI ou OSC, **“Feedback”** faz o programa **acender o botão do dispositivo** ou **mover o fader motorizado dele**. Só é enviado o que muda.

Para os LEDs dos botões MIDI:

1. Em **“LEDs de”**, escolha **“Qualquer controlador (números)”** ou o seu controlador: **Akai APC mini**, **Akai APC mini mk2** ou **Akai APC40 mkII**.
2. Para **“Ligado”**, **“Desligado”** e **“Em pausa”**, escolha a cor nas amostras (em uma APC) ou digite o valor (0–127) para qualquer outro controlador.
3. Nos pads RGB da APC mini mk2, escolha também como eles acendem: **“Fraco”**, **“Máximo”**, **“Pulsando”** ou **“Piscando”** (na APC40 mkII, **“Fixo”**). Sem escolher nada, “Ligado” acende em **“Máximo”** e “Desligado” em **“Fraco”**.

Os LEDs do controlador se apagam ao fechar o programa, ao apagar um mapeamento ou movê-lo para outro controle, ao abrir outro show ou ao desativar o dispositivo ou o feedback dele.

### Duplicar e excluir

**“Duplicar”** cria uma cópia do mapeamento (útil para outro dispositivo ou canal) e **“Excluir”** o apaga.

## Teclado

Os mapeamentos de teclado funcionam em qualquer página e têm preferência sobre os atalhos do programa que usem a mesma tecla. Eles não disparam enquanto você digita em um campo de texto (em Programar, enquanto a linha de comando tem o teclado) nem com Ctrl ou Alt pressionados. Os botões do SHOW têm também sua própria tecla, que funciona só na página Show (veja [Show](show.md#opções-de-um-botão)).

## Modelos para Akai APC

**“Mapear tudo”** atribui de uma vez um controlador inteiro. Ele fica em cada dispositivo MIDI da aba “Dispositivos”:

1. Pressione **“Mapear tudo”** no seu controlador.
2. Escolha o modelo:
   - **“Akai APC mini → Grupos”** / **“Akai APC mini mk2 → Grupos”**
   - **“Akai APC mini → Playbacks”** / **“Akai APC mini mk2 → Playbacks”**
   - **“Akai APC40 mkII → Playbacks”**
3. Leia o que cada controle faz e pressione **“Mapear”**. Os mapeamentos que esse dispositivo tinha são substituídos; se a entrada dele estava desligada, ela é ativada.

**→ Grupos** (APC mini e mk2): cada coluna de pads dispara as cenas de um grupo (verde: rodando; amarelo: pronta; apagado: não há cena). Os faders são os masters dos grupos e o último, o grand master. Os botões abaixo dos pads liberam os grupos; os da direita são blackout, parar tudo, pausa, tap, BeatGO, anterior, próxima e flash.

**→ Playbacks** (APC mini e mk2): cada coluna é um playback 1-8 da página de playbacks atual: o fader dele (pega o nível) e, de baixo para cima, GO, Pausa/Voltar, Flash e as 5 primeiras cues dele (a cue 1 no topo). O botão abaixo dos pads o libera. O último fader é o grand master; os botões da direita são página anterior e próxima, GO e Voltar do playback selecionado, blackout, tap, BeatGO e parar tudo.

**→ Playbacks** (APC40 mkII): cada faixa é um playback 1-8 da página atual: o fader dele, GO no botão da faixa, Pausa/Voltar no S, Flash no botão de armar gravação, selecionar e liberar, e as 5 primeiras cues dele nos clipes. Os botões de cena vão para as páginas 1-5, os de banco mudam a página e o grupo, e Play e Record são GO e Voltar do playback selecionado. Os knobs de device são os faders dos playbacks 9-16 e os knobs de faixa, os masters dos primeiros grupos.

> **Dica:** depois de “Mapear tudo”, cada controle aparece como um mapeamento normal na lista: você pode mudar qualquer um, ou apagá-lo, sem mexer nos outros.

## Monitor

**“Monitor”** mostra as **mensagens de entrada e saída**: a hora, **IN** ou **OUT**, o dispositivo (ou “Teclado”), a mensagem e o valor dela. Mova um controle de um dispositivo ativado para ver o que ele envia. **“Limpar”** esvazia a lista.

A luz verde do alvo, na barra superior, pisca a cada mensagem que chega.

## Alt+clique

Fora do modo Mapear, **Alt+clique** em um controle que pode ser mapeado o leva para o SHOW: veja [Show](show.md#levar-qualquer-controle-para-o-show).
