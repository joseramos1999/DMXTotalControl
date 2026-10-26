# Show

![A página Show](../img/show.png)

A página **Show** (F2) é a sua tela de operação: você a monta do seu jeito, com botões, faders, dials, grupos, playbacks, relógio, notas… Ela foi pensada para telas touch, para ficar em tela cheia durante o show e para ser usada também pelo celular com o [controle remoto](remoto.md). Ela é salva com o show.

Um SHOW tem **páginas**. Cada página é feita de **seções** que preenchem a tela. Cada seção pode ser dividida em duas metades, lado a lado ou uma acima da outra, quantas vezes você quiser, e cada seção mostra uma coisa: uma tabela de botões, uma tabela de faders, um grupo, um relógio…

## Começar

Com um show novo a página está vazia e oferece dois botões:

- **“Nova página”**: cria uma página com uma tabela de botões e entra no modo de edição.
- **“Partir dos meus grupos”**: cria a página “Grupos” com todos os seus grupos de Ao vivo em cima e, embaixo, uma tabela de faders “Masters” (o grand master e os masters dos primeiros grupos), uma seção de Ritmo e um relógio. Só aparece se você já tiver grupos.

## O modo de edição

Pressione **“Editar”** para montar a tela e **“Concluído”** para voltar a usá-la. No modo de edição:

- Clique em uma seção para **selecioná-la** (em uma tabela, em uma das células ou na etiqueta do canto superior direito). As propriedades dela aparecem no painel **“Propriedades”** da direita.
- Ao passar o mouse sobre uma seção, e na selecionada, aparece no canto superior direito uma etiqueta com o tipo e três botões: **dividir em duas metades, lado a lado**, **dividir em duas metades, uma acima da outra** e **remover esta seção** (em uma seção com título, eles ficam na barra de título). Os mesmos estão nas propriedades: “Lado a lado”, “Em cima e embaixo” e “Remover seção”.
- As seções mantêm a altura: não ganham uma barra ao editar. Com o painel de propriedades, elas só se estreitam até a largura em que as tabelas cabem sem rolar; se assim não couberem, a página rola para os lados com a barra de baixo (no máximo, fica tão larga quanto sem o painel).
- Arraste o **separador** entre duas metades para mudar o tamanho delas. Ele se ajusta sozinho a quartos, terços e metades.
- Uma seção **“Vazia”** mostra botões com todos os tipos: clique em um para escolher o que ela mostra. Se a seção for pequena e eles não couberem, fica um só botão, **“O que mostra ▾”**, com a lista.

### Tipos de seção

| Tipo | O que mostra |
|---|---|
| **Botões** | Uma tabela de botões (colunas × linhas). |
| **Faders** | Uma fileira de faders. |
| **Um grupo** | Um grupo de Ao vivo com botões grandes. |
| **Todos os grupos** | Todos os grupos de Ao vivo, um ao lado do outro. |
| **Playbacks** | Os playbacks da página Playbacks, com o GO, os botões e o fader. |
| **Mixer ao vivo** | O [mixer](directo.md#mixer) de Ao vivo. |
| **Relógio** | A hora, com segundos e data opcionais. |
| **Temporizador** | Um cronômetro ou uma contagem regressiva. |
| **Ritmo: GO e Stop** | **GO** e **Stop** grandes, como os da barra superior (mudam o que os botões das cenas fazem), e **BeatGO**. Os botões de cenas com passos, cuelists ou FX em “BPM do show” têm dentro a barra do ritmo. |
| **Notas** | Um texto seu, do tamanho que você quiser. |
| **Rodando agora** | O que está rodando, com um botão para pará-lo. |
| **Dials e rodas** | Uma tabela de dials ou rodas sem fim. |
| **Roda de cores** | Uma roda para dar cor a um grupo de fixtures. |
| **Grade XY (pan e tilt)** | Uma área para mover moving heads. |
| **Imagem** | Uma imagem (um logo, uma planta…). |
| **Níveis DMX** | Os 512 canais de um universo, ao vivo. |
| **Vazia** | Nada (no modo de edição, a lista de tipos). |

### Propriedades da seção

Conforme o tipo, o painel de propriedades mostra:

- **Mostra**: o tipo de seção.
- **Título**: o texto da barra de título. Sem título, a seção não tem barra, nem no modo de edição.
- **Fundo**: a cor de fundo da seção, entre as mesmas cores dos botões, ou **∅** (a do tema). Ela aparece como um tom, ao editar, ao tocar e no celular, e o texto em cima continua legível.
- **Cor de todos os botões** (de todos os faders, de todos os dials): dá essa cor a todos os da tabela de uma vez. As células vazias continuam vazias.
- **Colunas** e **Linhas**: o tamanho da tabela (até 32). Os botões, faders e dials preenchem as células. Em uma seção pequena demais para a tabela, as células não encolhem além do que dá para ler e tocar: a tabela rola (com a roda do mouse ou arrastando).
- **Preencher com as cenas de um grupo** (botões): coloca de uma vez um botão para cada cena desse grupo.
- **Grupo** (um grupo): qual ela mostra.
- **Fixtures** (roda de cores e grade XY): sobre qual grupo de fixtures elas atuam, ou “Todos os fixtures com cor” / “Todos os moving heads”.
- **Imagem** (imagem): o arquivo, e “Imagem inteira” ou “Preencher a seção”.
- **Segundos** e **Data** (relógio).
- **Contagem** (temporizador): “Crescente (cronômetro)” ou “Decrescente (contagem regressiva)”, com os **Minutos**.
- **Texto** e **Tamanho** (notas).
- **Universo** (níveis DMX).
- **Página de playbacks**, **Playbacks** e **Primeiro playback** (playbacks): veja [Playbacks no SHOW](show.md#playbacks-no-show).

## Botões

1. Selecione uma seção de botões e clique em uma célula vazia (a do **+**).
2. Pressione **“Adicionar um botão aqui”**.
3. Escolha a **Ação** e o que ela precisar (a cena, o grupo, a paleta…).

A forma mais rápida de preencher uma tabela é **arrastar cenas** da lista que fica sob as propriedades (“Arraste uma cena para uma célula:”) até as células: cada cena vira um botão que a dispara.

### Ações de um botão

| Ação | O que faz |
|---|---|
| **Reproduzir uma cena** | Dispara a cena no grupo dela (ou sozinha, se ela não estiver em nenhum grupo). Pressionado várias vezes seguidas, marca o [ritmo](directo.md#ritmo-de-uma-cena-com-passos) de uma cena com passos, de uma cuelist ou de uma cena FX com FX em “BPM do show”. Com **Stop** na barra superior, ele a para. |
| **Próxima cena de um grupo** / **Cena anterior de um grupo** | Em um grupo, ou em “Todos os grupos”. |
| **Liberar um grupo** | Um grupo, ou todos. |
| **Blackout** | Ativa ou desativa o blackout. |
| **Parar tudo** | Como o botão da barra superior. |
| **Tap tempo** | Marca o tempo do show (o dos FX e das cenas em modo BPM). |
| **BeatGO (próxima divisão)** | Como o botão BeatGO da barra superior. |
| **Uma paleta** | Aplica a paleta nos fixtures com os quais ela foi gravada (“Os próprios fixtures”) ou em um grupo de fixtures, como uma edição ao vivo do programador: fica por cima das cenas até você pressionar de novo. |
| **GO de cuelist** / **Voltar de cuelist** / **Liberar uma cuelist** | Sobre a cuelist que você escolher em **Cuelist**. |
| **Ir para uma cue…** | Salta para a **Cue** que você escolher dessa cuelist. |

Um botão trazido de outra página com Alt+clique mostra “Um controle de outra página” e, embaixo, o que ele faz (“Faz: …”).

### Opções de um botão

- **Rótulo**: o texto dele. Vazio (“Automático”), mostra o nome da cena, da paleta ou da ação.
- **Comportamento** (cena, blackout e paleta): **“Alternar: liga / desliga”** ou **“Flash: enquanto pressionado”**. Os botões flash têm um raio no canto. Em uma cena, no lugar de “Alternar” aparece **“Pressionar: conforme GO / Stop”**: ele a dispara ou a para conforme **GO** e **Stop** da barra superior.
- **Tecla**: uma tecla do teclado que o pressiona enquanto você está na página Show. A letra aparece no canto do botão.
- **Cor**, **Ícone** e **Imagem** de fundo (a imagem aparece atenuada enquanto o botão está desligado).

Um botão ligado é preenchido com a cor dele.

### Mover e remover

No modo de edição, arraste um botão, fader ou dial para outra célula da mesma seção para movê-lo. Para removê-lo, selecione-o e pressione **Del** (ou “Remover o botão” / “Remover o fader” nas propriedades). **Esc** tira a seleção.

## Faders

Um fader do SHOW pode controlar **várias coisas ao mesmo tempo**. Nas propriedades dele, em **Controla**, cada elemento é um destes:

| Elemento | O que o fader faz |
|---|---|
| **Disparar uma cena (sua intensidade)** | Ao subir a partir de 0, a cena inicia no grupo dela (respeitando se é exclusivo); ao descer até 0, ela para; no meio, é a intensidade dela. |
| **Dimmer de uma cena (sem dispará-la)** | Atenua a cena, seja quem for que a dispare (o grupo dela, um playback, um botão, uma super cena). Não a dispara nem a para. O nível dele é salvo com o show e dura enquanto houver um fader ou dial de dimmer nessa cena: se você o remover, a cena volta a 100 %. |
| **Master de um grupo** | O master desse grupo de Ao vivo. |
| **Grand master** | O GM da barra superior. |
| **Dimmer de fixtures** | O dimmer do mixer de todos os fixtures ou de um grupo de fixtures. |
| **Velocidade de movimento dos moving heads** | A velocidade do que move os moving heads (todos ou os de um grupo): os FX de pan/tilt deles, os do programador e os passos que os movem. Embaixo ×0,1, no meio a normal (×1), em cima ×10; o fader mostra o multiplicador e um clique duplo o volta para ×1. Um FX de pan/tilt e de cor só muda a velocidade do movimento; uma cuelist, a dos FX dela. O canal de velocidade dos motores não é tocado. |
| **Velocidade de uma cena** | A velocidade ao vivo dessa cena (×0,1 a ×10, como a de movimento). |
| **Disparar uma cena (sua velocidade)** | Como “Disparar uma cena (sua intensidade)”, mas o fader é a velocidade dela: embaixo de tudo a cena fica parada; ao subi-lo, ela inicia no grupo dela (respeitando se é exclusivo) e o curso é a velocidade dela, ×0,1 logo acima de embaixo, ×1 no meio e ×10 em cima. O texto diz “Parada” enquanto ela não toca, e o fader acompanha a cena se ela for parada ou disparada de outro lugar. A velocidade é só daquela vez: ao parar, a cena volta à velocidade que tinha (a do Ao vivo não muda). Um clique duplo a põe em ×1 (e a dispara se ela estava parada). Se um fader dispara várias cenas de um mesmo grupo exclusivo, elas não tiram o lugar uma da outra: toca a primeira. |
| **BPM** | O tempo do show, de 20 a 300. |

A diferença entre os dois de cena: **“Disparar uma cena (sua intensidade)”** é a própria cena, e a inicia e a para; **“Dimmer de uma cena (sem dispará-la)”** só a atenua, também quando ela é disparada de outro lugar. Os dois podem ser usados ao mesmo tempo: com o dimmer em 50 % e o fader que a dispara em 80 %, a cena sai a 40 %.

Pressione **“Adicionar outro elemento”** para somar outro e o ícone da lixeira para removê-lo. Se você arrastar uma cena para uma célula de faders, é criado um fader que a dispara.

Com **Faixa**, **De** … **a** … (de 0 a 100 %), o curso do fader cobre só essa parte do que ele move, em todos os seus elementos. Por exemplo, a velocidade de uma cena de 20 a 80 %, ou um dimmer de 0 a 60 % para que ele não passe disso. Com “De” maior que “a”, ou com **“Inverter”**, o fader funciona ao contrário. Um fader que dispara uma cena a para no extremo onde o valor é mais baixo (embaixo de tudo; em cima de tudo se estiver invertido), mesmo que “De” não seja 0.

O texto do fader mostra o valor real (o %, o BPM ou, se ele só controlar velocidades, o multiplicador ×), não onde o fader está. Se outro controle levar o valor para fora da faixa, o fader fica no extremo. Um fader que só controla velocidades (do movimento ou de cenas, uma ou várias) volta a ×1 com um clique duplo.

Embaixo de cada fader há um botão **“Flash”**: ele leva o fader ao topo do seu curso enquanto você o pressiona e, ao soltar, devolve exatamente o que havia. Ele é tirado ou colocado com o interruptor “Botão flash: no máximo enquanto pressionado” das propriedades.

> **Dica:** um fader com “Master de um grupo” dos seus washes e “Dimmer de fixtures” dos PAR do público dá a você um só fader para baixar toda a frontal.

## Dials e rodas

Os dials controlam o mesmo que os faders (a tabela acima), com a mesma **Faixa**. Cada dial pode ser:

- Um **dial**: percorre toda a faixa em três quartos de volta. Clique duplo o leva ao início do seu curso (o valor de “De”, ou o de “a” se estiver invertido); se ele controlar o BPM, a 90, ou ao extremo da faixa mais próximo se 90 ficar fora; se ele só controlar velocidades, a ×1.
- Uma **roda incremental** (“Roda incremental: gira sem fim, cada clique soma ou subtrai um passo”): gira sem fim, e cada clique soma ou subtrai o **Passo** (de 1 a 25 %) a partir de onde o valor estiver. O passo é um % do valor, não do curso, e a roda não sai da faixa; ao girá-la para cima com a cena parada, ela inicia a cena em “De”. O arco em volta mostra o nível.

Para girá-los, arraste em volta do centro com o mouse ou o dedo, ou para cima e para baixo perto do centro. Cada passo da roda do mouse os gira um pouco (em uma roda incremental, um clique).

## Roda de cores e grade XY

- **Roda de cores**: toque na roda para dar matiz e saturação (branco no centro) aos fixtures da seção, ou pressione uma das amostras de baixo.
- **Grade XY (pan e tilt)**: arraste o ponto para mover os moving heads. **“Centro”** os leva ao centro.

As duas atuam como uma edição ao vivo do programador: ficam por cima das cenas até você pressionar **“Liberar”**, e então as cenas voltam a dar a cor ou a posição.

## Grupos e mixer

As seções **Um grupo** e **Todos os grupos** mostram as colunas de [Ao vivo](directo.md) com botões grandes, o anterior/próximo, o botão de liberar e o master delas, sem opções de edição. **Mixer ao vivo** mostra as tiras do mixer ajustadas à largura.

Os grupos também se **recolhem** aqui, com a seta ao lado do nome: em **Todos os grupos**, o grupo vira uma faixa estreita e os outros dividem a largura; uma seção **Um grupo** que ocupa uma metade da página se recolhe à sua faixa (ou ao seu cabeçalho, se as metades estiverem uma acima da outra) e a outra metade cresce. As cenas dele continuam rodando, e isso é salvo com a página (não depende do que está recolhido em Ao vivo). Em um SHOW bloqueado, os grupos não se recolhem.

## Playbacks no SHOW

A seção **Playbacks** mostra tiras iguais às da página [Playbacks](playbacks.md):

- **Página de playbacks**: **“Acompanhar a página atual”** (a barra de título mostra ◀ o nome da página ▶ para mudá-la) ou uma página fixa.
- **Playbacks**: quantos aparecem (até 20) e **Primeiro playback**: a partir de qual.
- Três interruptores para mostrar ou ocultar o **segundo botão (Pausa / Voltar)**, o **terceiro botão (Flash)** e **a cue que ele reproduz e a próxima**.

No modo de edição, solte uma cena sobre uma tira para colocá-la nesse playback.

## Utilitários

- **Relógio**: hora e data do computador.
- **Temporizador**: **“Iniciar”** / **“Pausar”** e **“Zerar”**. Na contagem regressiva fica laranja no último minuto e vermelho ao chegar a zero. Continua contando mesmo que você mude de página.
- **Ritmo**: **GO**, **Stop** e **BeatGO**.
- **Notas**: o texto que você escrever nas propriedades (o roteiro, avisos para o operador…).
- **Rodando agora**: o que está rodando, com o botão de parar ao lado de cada cena.
- **Níveis DMX**: uma grade com os 512 canais do universo escolhido.
- **Imagem**: inteira ou preenchendo a seção.

## Páginas

- No modo de edição, **“+ Página”** adiciona uma página.
- Clique no nome de uma página para vê-la. As abas de página aparecem se houver mais de uma.
- No modo de edição, clique duplo em uma aba a renomeia; o botão direito abre **“Renomear…”**, **“Mover para a esquerda”**, **“Mover para a direita”** e **“Excluir página…”**. Ao excluir, o programa avisa quantas seções e controles vão com ela: isso não pode ser desfeito.

## Levar qualquer controle para o SHOW

**Alt+clique** em quase qualquer controle do programa o copia para a página do SHOW que estiver aberta: uma cena de Ao vivo, o master de um grupo, um botão do mixer, o grand master, o blackout, uma paleta de Programar, os botões e o fader de um playback… O programa avisa “Adicionado ao SHOW, página “…””.

O controle novo vai para a primeira célula livre de uma tabela de botões ou de faders dessa página. Se não houver nenhuma, uma seção vazia vira uma tabela (com o título “Touch”), ou a última seção é dividida.

## Tela cheia e bloqueio

- **“Tela cheia”** (F11) e **“Sair da tela cheia”**.
- **“Bloquear”**: oculta a barra lateral e deixa só o SHOW, sem edição nem navegação. Para sair, **mantenha pressionado “Desbloquear”** (uma barra se enche sob o botão). Se houver senha de bloqueio, ela é pedida.

### Bloqueio e modo quiosque

- Em Opções › “SHOW e bloqueio”, **“Abrir o SHOW em tela cheia ao iniciar”** abre diretamente o SHOW. Com **“…e bloqueado”**, além disso ele inicia bloqueado: só o SHOW, como um quiosque.
- Se você bloquear a **“Edição do SHOW”** com o cadeado da barra superior, o botão “Editar” desaparece: as páginas são usadas, mas não alteradas.

Veja [Opções](opciones.md#show-e-bloqueio).

## Teclas, mapeamentos e remoto

- As **teclas** dos botões só funcionam na página Show. Por isso, nesta página a tecla G não faz BeatGO.
- No modo Mapear, os botões e os faders do SHOW podem ser atribuídos a um controlador; um controle MIDI pode mover um fader do SHOW inteiro, com todos os seus elementos (veja [Mapeamentos](mapeos.md)).
- As páginas do SHOW são vistas e usadas no celular ou no tablet com o [controle remoto](remoto.md).
