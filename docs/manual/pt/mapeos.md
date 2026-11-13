# Mapeamentos

![A página Mapeamentos](../img/mapeos.png)

**Mapeamentos** (F8) serve para controlar o show de fora: controladores **MIDI**, **OSC** a partir de outro aplicativo, uma mesa por **entrada DMX** ou o **teclado**. Um mapeamento une um controle do seu dispositivo (um pad, um fader, uma tecla…) a algo do programa (uma cena, um master, um GO…). Os mapeamentos são salvos com o show.

A página tem três abas: **“Mapeamentos”**, **“Dispositivos”** e **“Monitor”**, e o botão **“Aprender”**.

## Dispositivos

Em **“Dispositivos”** aparecem os dispositivos de entrada que o programa encontra, com o tipo (MIDI, OSC ou DMX), o nome e o universo que eles usam. Primeiro aparecem os controladores (MIDI e OSC) e os dispositivos que você já usa, em **“Controladores e dispositivos em uso”**; depois, em **“Entradas DMX: interfaces e rede”**, as demais entradas DMX, com uma linha para cada placa de rede.

1. Conecte o controlador e, se ele não aparecer, pressione **“Buscar novamente”**.
2. Ative a **“Entrada”** dele.
3. Se o dispositivo tiver saída, ative **“Feedback (LEDs)”** para que o programa acenda os LEDs dele ou mova os faders motorizados dele.

Numa Akai APC (APC mini, APC mini mk2 ou APC40 mkII) com a entrada e o feedback ativados aparece **“Preparar”**. Ele coloca o controlador no modo que os mapeamentos usam e envia de novo todos os LEDs. Use depois de desligar e ligar o controlador ou de mudar o modo dele.

Um mapeamento de um dispositivo que não está conectado é mantido: ele volta a funcionar quando você conecta o dispositivo.

A entrada de um dispositivo MIDI também traz o timecode MIDI (MTC) dele para as [cuelists “Timecode”](escenas.md#opções-da-cuelist) que o seguem. Basta ativar a **“Entrada”** dele; não é preciso nenhum mapeamento.

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

As teclas e os botões do controlador das listas de cues do QLC+ podem vir como mapeamentos, e os das suas cuelists vão para a cópia para o QLC+ (veja [Cenas](escenas.md#cuelists-e-qlc)).

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
| Disparar uma cena | A cena escolhida (ou “A cena selecionada”), no grupo dela. |
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
| Dimmer ao vivo de uma cena / Velocidade ao vivo de uma cena | Os controles deslizantes ao vivo da cena escolhida (ou “A cena selecionada”). |
| BPM | O tempo. |
| Mixer: dimmer, matiz, saturação, velocidade do strobo | Na tira escolhida. |
| Fader do SHOW | Um fader do SHOW inteiro. |
| Playback: fader | O fader de um playback. |
| Velocidade das cues (todas as cuelists) | A [velocidade de todas as cuelists](playbacks.md#velocidade-das-cues): ×0,1 embaixo, ×1 no meio, ×10 em cima. |
| Tempo ao vivo de um playback / Tempo ao vivo do playback selecionado | O [tempo da cue](playbacks.md#tempo-ao-vivo) que está em fade. Como fader: no meio, o tempo com que ela começou; embaixo ×0,1 e em cima ×10. Como encoder ou com toques: 10 % a mais ou a menos por passo. |

Para os playbacks escolhe-se a **página** (“Página atual (acompanha)”, que acompanha a página que está à vista, ou uma fixa) e o número do **Playback**. Com **“Ou uma cuelist ou uma cena, onde estiver”** o mapeamento atua sobre essa cuelist ou cena em qualquer playback onde ela estiver. Em “ir para uma cue” aparece para qual cue ele leva agora (“Agora: cue 3 de Música 1”).

### A cena selecionada

Em “Disparar uma cena”, “Dimmer ao vivo de uma cena” e “Velocidade ao vivo de uma cena”, a lista de cenas começa com **“A cena selecionada”**. Esse mapeamento atua sobre a última cena que você tocou.

- **Selecionam uma cena**: tocá-la em Ao vivo, em um botão do SHOW, em um controlador ou no celular, e dispará-la com “Próxima cena de um grupo” ou “Cena anterior de um grupo”.
- **Não a mudam**: os faders do SHOW, os playbacks, as ações das cues nem “Testar” em Cenas.
- Ela continua selecionada quando para. Não é salva: ao abrir um show ou começar um novo não há nenhuma.
- Embaixo da lista aparece qual é agora (“Agora: Verso”). Sem nenhuma, o mapeamento não faz nada e o LED dele fica apagado.
- Um botão dispara, para ou faz flash da cena selecionada no momento em que você o toca, no grupo dela e conforme GO / Stop. O LED dele mostra o estado dessa cena.
- Em um fader absoluto, **“Pegar o fader (sem motor)”** é ativado: ao mudar de cena, o nível não salta. O fader assume quando alcança o nível da nova cena. Você pode desativá-lo.
- Em Ao vivo, a cena selecionada tem uma barra fina na borda inferior, enquanto algum mapeamento usar “A cena selecionada” ou no modo Mapear.

Exemplo: os pads da APC disparam cenas e, assim, as selecionam. Um botão giratório vai para “Velocidade ao vivo de uma cena” › “A cena selecionada”. Ele muda a velocidade da última cena que você tocou.

### Comportamento

Nos **botões**:

- **“Alternar: cada toque liga / desliga”**: cada toque liga ou desliga. Em uma cena, ele se chama **“Pressionar: conforme GO / Stop”**: ele a dispara ou a para conforme GO e Stop da barra superior.
- **“Flash: enquanto pressionado”**.
- **“Só ligar”** / **“Só desligar”**.

Os botões de playback atuam ao pressionar, exceto o flash, e não têm esta opção.

O flash de um playback usa o ataque e a queda desse playback (veja [Propriedades de um playback](playbacks.md#propriedades-de-um-playback)). Com “Ou uma cuelist ou uma cena, onde estiver”, usa os do primeiro playback que a tem; se ela não está em nenhum playback, é na hora. Com “Alternar”, um toque durante a queda o faz subir de novo.

Nos **faders**:

- **“Absoluto: segue o fader”**.
- **“Incremental: um passo por toque”** (para botões que sobem ou descem).
- **“Relativo: encoder sem fim”** e **“Relativo: encoder, 1 sobe e 127 desce”** (encoders sem fim).
- **“De”** … **“a”**: a faixa, de 0 a 100 %. **“Inverter”** a inverte.
- **“Passo %”** e **“Em loop”** (incremental e relativo).
- **“Pegar o fader (sem motor)”** (absoluto): o fader do controlador só move o nível depois de alcançá-lo, sem saltos. Com um fader motorizado não é necessário. No fader A/B de um playback, o primeiro valor que chega só diz onde ele está; depois, o fader do controlador tem que alcançar o da tela.

### Feedback

Com MIDI ou OSC, **“Feedback”** faz o programa **acender o botão do dispositivo** ou **mover o fader motorizado dele**. Só é enviado o que muda.

Um fader motorizado (absoluto) não se move enquanto você o move. O programa espera até que você o deixe parado por 0,4 s e só o corrige se o destino dele ficou em outro valor, por exemplo porque outra coisa o mudou. Enquanto você não o toca, a tela, outra página de playbacks ou outro controle o movem na hora. Os encoders e os botões não esperam.

Para os LEDs dos botões MIDI:

1. Em **“LEDs de”**, escolha **“Qualquer controlador (números)”** ou o seu controlador: **Akai APC mini**, **Akai APC mini mk2** ou **Akai APC40 mkII**.
2. Para **“Ligado”**, **“Desligado”**, **“Em pausa”** e, nas cenas, **“Em fade”**, escolha a cor nas amostras (em uma APC) ou digite o valor (0–127) para qualquer outro controlador.
3. Nos pads RGB, escolha também como eles acendem: **“Fraco”**, **“Máximo”**, **“Pulsando”** ou **“Piscando”** na APC mini mk2, e **“Fixo”**, **“Pulsando”** ou **“Piscando”** na APC40 mkII. Pulsando ou piscando, o pad alterna entre a cor e apagado. Sem escolher nada, “Ligado” acende em **“Máximo”** (**“Fixo”** na APC40 mkII), “Desligado” em **“Fraco”** (**“Fixo”** na APC40 mkII) e “Em fade” em **“Pulsando”**.

**“Em fade”** (em “Disparar uma cena” e “Cena N de um grupo”): ative **“Cor própria durante o fade”** e o botão mostra essa cor enquanto a cena entra com o fade dela e enquanto sai com o seu. Em uma cuelist, mostra enquanto a cue dela faz o fade. Sem ela, ao entrar o botão fica como “Ligado” e ao sair como “Desligado”. Uma cena em pausa ou esperando fica como “Em pausa”.

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

**→ Playbacks** (APC mini e mk2): cada coluna é um playback 1-8 da página de playbacks atual: o fader dele (pega o nível) e, de baixo para cima, GO, Pausa/Voltar, Flash e as 5 primeiras cues dele (a cue 1 no topo). O botão abaixo dos pads o libera. O último fader é o grand master; os botões da direita são página anterior e próxima, GO e Voltar do playback selecionado, blackout, tap, BeatGO e parar tudo. Se o fader de um playback estiver em A/B (veja [O fader](playbacks.md#o-fader)), cada percurso do fader da APC é um GO cujo fade você faz à mão.

**→ Playbacks** (APC40 mkII): cada faixa é um playback 1-8 da página atual: o fader dele, GO no botão da faixa, Pausa/Voltar no S, Flash no botão de armar gravação, selecionar e liberar, e as 5 primeiras cues dele nos clipes (verde: rodando; verde pulsando: a próxima; azul fraco: as outras). Os botões de cena vão para as páginas 1-5, os de banco mudam a página e o grupo, e Play e Record são GO e Voltar do playback selecionado. Os knobs de device são os faders dos playbacks 9-16 e os knobs de faixa, os masters dos primeiros grupos.

A APC mini mk2 entra sozinha no modo Session, e a APC40 mkII no modo Ableton: quando o programa inicia, quando você abre um show, quando ativa o feedback dela e com “Mapear tudo”. Nos modos Nota ou Bateria, os pads da mk2 não chegam aos mapeamentos.

> **Dica:** depois de “Mapear tudo”, cada controle aparece como um mapeamento normal na lista: você pode mudar qualquer um, ou apagá-lo, sem mexer nos outros.

## Monitor

**“Monitor”** mostra as **mensagens de entrada e saída**: a hora, **IN** ou **OUT**, o dispositivo (ou “Teclado”), a mensagem e o valor dela. Mova um controle de um dispositivo ativado para ver o que ele envia. **“Limpar”** esvazia a lista.

O Monitor mostra como **OUT** as mudanças de modo que o programa envia: “SysEx · modo Session” na APC mini mk2 e “SysEx · modo Ableton” na APC40 mkII.

A luz verde do alvo, na barra superior, pisca a cada mensagem que chega.

## Alt+clique

Fora do modo Mapear, **Alt+clique** em um controle que pode ser mapeado o leva para o SHOW: veja [Show](show.md#levar-qualquer-controle-para-o-show).
