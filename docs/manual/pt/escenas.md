# Cenas

![A página Cenas](../img/escenas.png)

**Cenas** (F4) é a biblioteca de tudo o que você gravou: cenas estáticas, passos, FX, efeitos de cor, racks, super cenas e cuelists. À esquerda fica a lista; à direita, o editor da cena escolhida, diferente conforme o tipo.

Quase tudo é criado a partir do programador: os efeitos novos pegam os fixtures selecionados em [Programar](programar.md) e os passos e cues, os valores dele.

## A lista de cenas

- **“Buscar…”**: filtra por nome (ou pelo nome da pasta).
- **“Pastas”** / **“Grupos”**: ver as cenas por [pastas](escenas.md#pastas) ou pelos grupos de Ao vivo em que estão (“Sem grupo” para o resto).
- **Recolher tudo** / **Expandir tudo**: o botão da direita.
- **Filtro por tipo**: “Todos”, “Estática”, “Passos”, “Cuelists”, “FX”, “Colour FX”, “Racks” e “Super cenas”.
- **Ctrl+clique** e **Shift+clique** selecionam várias cenas para movê-las para uma pasta (“Para pasta…”).

Cada cena tem uma tarja com a cor do seu tipo. Clique em uma para abri-la no editor.

## Criar uma cena

Pressione **“Nova”**:

| Opção | O que cria |
|---|---|
| **Estática (a partir do programador)** | Uma cena estática com os valores do programador. |
| **Passos (programador = primeiro passo)** | Uma cena de passos. |
| **Cuelist (programador = cue 1)** | Uma cuelist. |
| **FX (no programador)** | Leva você a Programar › FX: os FX são feitos lá e gravados com “Gravar…”. |
| **Colour FX (fixtures selecionados)** | Um efeito de cor sobre os fixtures selecionados, na ordem deles. |
| **Mapeamento no plano 2D (fixtures selecionados)** | Um efeito de cor posicionado onde os fixtures estão no plano 2D. |
| **Rack de efeitos** | Um rack vazio. |
| **Super cena (linha do tempo com áudio)** | Uma super cena vazia. |
| **Importar de um arquivo…** | Cenas e efeitos exportados de outro show (veja [Importar e exportar](escenas.md#importar-e-exportar)). |
| **Cue lists do QLC+…** | Cuelists com os chasers, as sequências e as listas de cues de um show do QLC+ (veja [Cue lists do QLC+](escenas.md#cue-lists-do-qlc)). |

## O cabeçalho do editor

- **O nome**: clique para alterá-lo.
- **A pasta** (ícone de pasta): mover a cena para uma pasta.
- **“Testar”** / **“Parar”**: reproduz a cena agora, fora dos grupos. As cuelists têm seu próprio GO.
- **“Adicionar ao grupo…”**: adiciona a cena a um grupo de Ao vivo.
- **Converter em passos** (racks, FX, Colour FX, Move FX, Curve FX e chasers): grava um ciclo em uma nova cena de passos, só com os canais aos quais a cena dá um valor: os outros, mesmo que tenham um valor padrão (o pan ou o zoom no centro de muitos moving heads), não entram nos passos. Se um FX tem atrasos, grava o ciclo quando todos os fixtures dele já arrancaram.
- **Converter em cuelist** (passos e chasers): veja [Cuelists](escenas.md#cuelists).
- **“QLC+”** (cuelists): salva uma cópia do show para o QLC+; veja [Cuelists e QLC+](escenas.md#cuelists-e-qlc).
- **Exportar**, **“Duplicar”** e **“Excluir”** (a cena também é removida dos seus grupos e racks).

## Como a cena é reproduzida

Abaixo do cabeçalho fica o painel de reprodução. Ele mostra o que faz sentido para cada tipo:

- **“Fade in”** e **“Fade out”**.
- **“Tempo de ciclo”** (ou **“Tempo por passo”** em um Colour FX) e **“Velocidade”** (de um ciclo por minuto a dez por segundo; clique duplo: um ciclo por segundo), com **×½** e **×2**.
- **“Fade escalonado”** (estáticas e passos): cada fixture começa o fade este tempo depois do anterior, na ordem de seleção.
- **“Comandado por”**: **“Duração”** (os tempos normais), **“BPM”** (os tempos passam a beats e seguem o tempo do show, também os fades de cada passo; o tempo de crossfade de um grupo do Ao vivo e os fades dos blocos de uma super cena continuam em segundos), **“BeatGO”** ou **“Pulso”** (avança com o botão BeatGO ou com os picos da entrada de áudio). Com BeatGO ou Pulso, **“Divisões”** (/2, ×2).
- **“Ritmo marcado”** (sequências, chasers e cenas FX com FX em “BPM do show”): o ritmo marcado tocando o botão dela várias vezes seguidas ([Ao vivo](directo.md#ritmo-de-uma-cena-com-passos)), em BPM. Enquanto ela o tiver, os passos dela vão nesse ritmo e não pelos seus tempos; em uma cena FX, os FX dela em “BPM do show” fazem um ciclo a cada batida em vez de seguir o BPM do show. **“Remover”** volta aos tempos dela (ou ao BPM do show). Com “BPM”, um beat é um passo desse ritmo: um fade de 0,5 b dura meio passo.
- **“Início”**: **“Reproduzindo”** ou **“Em pausa, pressione de novo para reproduzir”**.
- **“Reproduz”**: **“Sempre”** ou **“N vezes”**, **“e depois”** **“Liberá-la”** ou **“Manter o último quadro”**, **“e reproduzir”** “Nada”, “Próxima cena do grupo”, “Cena anterior do grupo” ou qualquer outra cena.

## Tipos de cena

### Estática

Guarda valores fixos. Mostra quantos canais e fixtures ela tem e as paletas que usa (“Usa estas paletas: quando elas mudam, a cena também muda”).

Para alterá-la:

1. Pressione **“Editar no programador”**: ela é carregada em Programar.
2. Mude o que quiser.
3. Volte para Cenas e pressione **“Salvar o programador nesta cena”**.

Se você salvar FX em uma cena estática, ela passa a ser uma cena FX.

### Passos

Uma sucessão de visuais, cada um com seu **fade in**, sua **espera** e seu **fade out**.

- **“Adicionar passo”**: o programador vira um novo passo depois do selecionado.
- **“Salvar no passo”**: salva o programador no passo selecionado.
- **“Carregar passo”** (ou clique duplo em um passo): leva-o ao programador para vê-lo e alterá-lo.
- **Duplicar**, **▲ ▼** para reordenar e **excluir** o passo.
- O botão **∞** da espera faz o passo esperar um GO (“Até o GO”).

**Ctrl+clique** e **Shift+clique** selecionam vários passos: ao mudar um tempo, ele muda em todos. Os passos usam as paletas **por referência**, como as cenas e as cues: se você atualizar uma paleta, os passos gravados com ela mudam (o que está tocando, na hora). O ícone de paleta de um passo diz quais ele usa.

### FX

Uma cena FX tem **valores base** e uma ou várias **camadas de FX**. Ela é criada gravando a partir do programador com FX (veja [FX](programar.md#fx)).

- À esquerda, a lista de FX: parâmetro, fixtures e velocidade (em “BPM do show”, o ritmo marcado da cena, se ela tiver um). A lixeira remove uma camada.
- À direita, os controles da camada escolhida (os mesmos que em Programar) e a prévia animada dela. As mudanças aparecem ao vivo enquanto a cena roda.
- **“Vincular”**: as camadas vinculadas compartilham velocidade e timing.
- Se alguns fixtures fazem o FX do seu jeito, há um aviso: se você mudar aqui a forma, o tamanho, o sentido ou a fase, todos farão o mesmo.
- **“Por fixture”**: a defasagem e o atraso de cada fixture, também ao vivo. Aqui “Distribuir…” e “Sem defasagem nem atraso” valem para todos (veja [Defasagem e atraso por fixture](programar.md#defasagem-e-atraso-por-fixture)).
- Para adicionar camadas ou alterar os valores base: **“Editar no programador”** e depois **“Salvar o programador nesta cena”**.

### Colour FX e mapeamentos no plano 2D

Um **Colour FX** é um efeito de pixel (mais de 60: degradês, fogo, meteoros, strobos, ondas…) que percorre os fixtures na ordem de seleção.

- **“Efeito”**: qual.
- **“Atua sobre”**: “Cor (RGB)”, “Branco”, “Âmbar”, “UV”, “Dimmer (valor)” ou “Shutter”.
- **“Cores”**: até 5, conforme o efeito. Clique em uma cor para mudá-la (amostras ou matiz), ou “Sem cor”.
- **“Texto”**: nos efeitos de texto, o texto que percorre os fixtures.
- **“Imagem”** e **“Animação”**: nos efeitos de imagem, o arquivo e “Fixa”, “Rolar para os lados”, “Rolar para cima e para baixo” ou “Animada (GIF)”.
- **“Disposição”**: **“Em fila”** (na ordem de seleção) ou **“No plano 2D”** (onde estão os fixtures).
- **“Parâmetros”**: os próprios de cada efeito.
- **“Usar a seleção do programador”**: troca os fixtures dele pelos selecionados.

No topo aparece a cor real de cada cabeça enquanto ele roda (pressione “Testar” para vê-la).

Um **mapeamento no plano 2D** é um Colour FX que já nasce com a disposição do plano: o efeito, o texto ou a imagem são posicionados sobre o palco tal como os fixtures estão.

### Move FX e Curve FX

São efeitos do módulo antigo: os **Move FX** movem pan/tilt com uma forma e os **Curve FX** dão uma onda (seno, triângulo, rampas, quadrada, aleatória, strobo…) a canais. Você os encontrará em shows antigos ou em shows do QLC+; eles já não são criados em “Nova”.

Eles continuam rodando igual e têm seu editor (forma, largura, altura, rotação, posição, fase, distribuição entre fixtures, simétrico, relativo…). Um aviso oferece **“Converter em FX”**: eles passam ao editor de FX, também no programador, e os grupos, botões do SHOW e mapeamentos os mantêm. A fase de um Move FX passa a ser a “Fase” do FX, e a defasagem de início de cada fixture, a “Defasagem” dele ([Defasagem e atraso por fixture](programar.md#defasagem-e-atraso-por-fixture)).

### Rack de efeitos

Várias cenas e efeitos que rodam juntos como um só, cada um nos seus fixtures.

- Em **“Adicionar”** (à direita), clique em uma cena para colocá-la no rack.
- Em **“No rack (N)”**, o **×** a remove.

Um rack não pode conter uma cena que o reproduza.

### Chasers

Os chasers de shows do QLC+ rodam normalmente, mas não têm editor: converta-os em passos (grava um ciclo) ou em cuelist com os botões do cabeçalho.

## Super cenas

Uma **super cena** é uma linha do tempo com faixas de cenas e de áudio. Ela é disparada como qualquer cena (em um grupo, no SHOW, com um mapeamento). O editor dela ocupa a página inteira; o botão da lista, no canto superior esquerdo, volta à lista de cenas.

### A barra de ferramentas

- **“Reproduzir”** / **“Parar”** (**Espaço**): a partir do cursor. **⏮** volta ao início. Ao lado aparece o tempo (e o compasso).
- As setas **Desfazer** e **Refazer** (**Ctrl+Z** / **Ctrl+Y**). Consulte [Desfazer](escenas.md#desfazer).
- **“Colar”** (**Ctrl+V**): os blocos copiados, no cursor e a partir da faixa selecionada. Consulte [Copiar e colar](escenas.md#copiar-e-colar).
- **“Tempo”** / **“Compassos”**: a grade. Em compassos, o **BPM**, a fórmula de compasso (**4/4**, **3/4**, **2/4**) e **“Compasso 1 aqui”** para que o primeiro compasso comece no cursor e coincida com a música.
- **“Ajustar”**: os blocos e pontos se ajustam à grade.
- **“Fim aqui”**: a super cena termina no cursor (o **×** remove o marcador: ela termina com o último bloco). **“Loop”**: recomeça.
- **“+ Faixa”** e **“+ Áudio”** (um arquivo de áudio na faixa selecionada, no cursor).
- **“Valores estáticos”**: **“Pegar os valores do programador”** (rodam enquanto a super cena está em andamento) ou **“Limpar os valores estáticos”**.
- **Zoom** − e +.

### Colocar cenas na linha do tempo

1. Pressione **“+ Faixa”** se não houver nenhuma.
2. Arraste uma cena da lista da esquerda para uma faixa. Ou selecione uma faixa, posicione o cursor e dê um **clique duplo** na cena: ela é colocada no cursor.

Os blocos podem se sobrepor.

Uma cuelist **“Timecode”** com relógio próprio num bloco segue a linha do tempo: o 00:00 do seu relógio é o início do bloco. Coloque-a em outra faixa, junto ao bloco de áudio da música e a partir do mesmo ponto: cada cue soa no seu momento da música. Se você reproduzir a partir de outro ponto, a cuelist vai direto à cue daquela hora. Esticada, o relógio dela vai mais rápido ou mais devagar para que a cuelist dure o bloco.

### Trabalhar com os blocos

- **Arraste** um bloco para movê-lo, também para outra faixa. Arraste as **bordas** dele para mudar a duração.
- Com um só bloco selecionado, as **alças** dos cantos superiores são os fades de entrada e de saída dele.
- Ao alongar um bloco, a cena **se repete em loop** (aparecem marcas a cada volta) ou **se estica** para que um ciclo dure o bloco inteiro. Isso se muda no menu do bloco ou na barra de baixo (“Em loop” / “Esticada”).
- **Botão direito**: “Repetir a cena no bloco” / “Esticar a cena para caber no bloco”, “Copiar”, “Recortar”, “Duplicar”, “Bloquear” / “Desbloquear”, “Abrir a cena” e “Excluir”. Sobre um bloco da seleção, o menu age sobre a seleção inteira; “Abrir a cena” abre a do bloco clicado.
- **Clique duplo** em um bloco abre a cena dele. **Del** (ou **Backspace**) exclui os blocos selecionados; os de faixas bloqueadas ficam. **Ctrl+D** os duplica logo depois: cada cópia na sua faixa.
- A barra de baixo mostra o bloco selecionado: **Início**, **Duração**, **Fade in**, **Fade out**. Com vários, diz quantos são. Com um ou vários, tem **Copiar**, **Recortar**, **Duplicar** e **Excluir**.

### Selecionar vários blocos

- **Clique** em um bloco: só ele. A faixa dele passa a ser a faixa selecionada.
- **Ctrl+clique** adiciona um bloco à seleção ou o tira.
- **Shift+clique** seleciona do último bloco escolhido até este: os blocos das faixas entre os dois, do início mais cedo ao fim mais tarde.
- **Arrastar em uma área vazia** das faixas desenha um retângulo que seleciona os blocos que ele toca. Com **Ctrl** ou **Shift**, eles se somam à seleção.
- **Ctrl+A** seleciona todos os blocos das faixas visíveis. Os de uma faixa que você oculta saem da seleção.
- Um clique em uma área vazia tira a seleção (e move o cursor). Com Ctrl ou Shift, a seleção fica.

Ao **arrastar** um bloco selecionado, a seleção inteira se move:

- Todos se movem o mesmo no tempo: o bloco arrastado se ajusta à grade e os outros mantêm as distâncias. O primeiro não passa de 0.
- Sobem ou descem o mesmo número de faixas. As faixas para onde vão se iluminam.
- Os blocos bloqueados e os de faixas bloqueadas ficam onde estão.
- Se algum cairia fora das faixas ou em uma faixa bloqueada, eles se movem só no tempo.
- As bordas e os fades mudam só o bloco sob o mouse.

Clicar em um bloco da seleção e soltar sem movê-lo deixa só ele selecionado. Cada ação sobre a seleção é um só passo de [Desfazer](escenas.md#desfazer) (“Mover 3 blocos”).

### Copiar e colar

- **Ctrl+C** copia os blocos selecionados. **Ctrl+X** os recorta: copia e os tira. Os de faixas bloqueadas são copiados, mas ficam.
- **Ctrl+V** ou **“Colar”** os cola no cursor, a partir da faixa selecionada (sem faixa selecionada, a partir da primeira).
- **Botão direito** em uma área vazia de uma faixa: **“Colar aqui”** cola nesse ponto e nessa faixa. **“Selecionar tudo”** faz o mesmo que **Ctrl+A**.
- O primeiro bloco cai no cursor e os outros mantêm as distâncias no tempo. O da faixa mais alta vai para a faixa selecionada e os outros mantêm as distâncias em faixas.
- Se faltarem faixas, elas são adicionadas no final. Uma faixa oculta que recebe blocos é mostrada.
- Os blocos colados mantêm a duração, os fades, o loop ou esticado e a cor. Chegam desbloqueados, como com Duplicar.
- O que você cola fica selecionado e o cursor passa para o final: pressione **Ctrl+V** de novo e a próxima cópia vai logo depois.
- Você pode colar em outra super cena do mesmo show: abra-a e cole. As cenas não são copiadas: o bloco colado reproduz a mesma cena.
- Um bloco de áudio colado na mesma super cena reproduz o mesmo áudio. Em outra super cena ele tem o seu próprio áudio, lido do mesmo arquivo: excluí-lo lá nunca leva o original. Se você recortou o bloco, o áudio dele é lido de novo do arquivo.
- Nada é colado em uma faixa bloqueada, nem uma cena que reproduz esta super cena (por exemplo, um rack que a contém). Um aviso diz isso.
- O que você copia fica guardado enquanto o show está aberto. Abrir outro show ou começar um novo o esvazia. Uma cena que você exclui da lista sai do que foi copiado.
- Colar e recortar são um só passo de [Desfazer](escenas.md#desfazer). O que é colado com a super cena em andamento toca a partir do próximo “Reproduzir”.

### Áudio

Os blocos de áudio (wav, flac, ogg, aiff e mp3 se o sistema suportar) mostram sua **forma de onda**, que é calculada em segundo plano. É preciso uma faixa desbloqueada para adicioná-lo.

### Faixas e automação

No cabeçalho de cada faixa:

- **M**: silenciar (a faixa não toca).
- **Cadeado**: bloquear (os blocos e pontos dela não se movem).
- **D**: a linha de automação do **dimmer** (em uma faixa de áudio, o volume).
- **φ**: a linha de automação da **fase** dos seus efeitos.
- **⋯**: “Renomear” (também com clique duplo no nome), “Mover para cima”, “Mover para baixo”, “Limpar a automação desta linha” e “Excluir a faixa”.
- A seta da direita oculta ou mostra a faixa.

Em uma linha de automação: **clique** adiciona um ponto, **arrastar** o move, **clique duplo** o apaga e **botão direito** alterna entre reta e curva suave. Sem pontos, o dimmer fica no máximo e a fase em 0.

### Mover-se pela linha do tempo

- Clicar ou arrastar na **régua** move o cursor; **clique duplo** na régua reproduz a partir dali. Clicar em uma faixa vazia também move o cursor.
- **Ctrl+roda**: zoom. **Shift+roda**: deslocar-se no tempo. A roda sozinha sobe e desce pelas faixas.

### Desfazer

- **Ctrl+Z** desfaz a última alteração. **Ctrl+Y** (ou **Ctrl+Shift+Z**) a refaz. As setas da barra de ferramentas fazem o mesmo; ao passar o mouse, dizem qual alteração desfazem ou refazem.
- Cada super cena tem seu próprio histórico, de até 100 passos. Ele dura enquanto o show está aberto: salvar não o apaga, abrir outro show ou começar um novo, sim.
- Contam as alterações de faixas, blocos, automação, grade, BPM, fim, loop e valores estáticos. Várias alterações do mesmo campo da barra de baixo (ou do BPM) em menos de um segundo são um único passo.
- Não contam ocultar ou mostrar faixas, o zoom, o cursor, “Ajustar” nem a seleção.
- Enquanto você digita em um campo, Ctrl+Z desfaz o que foi digitado no campo.
- Funciona com a super cena em andamento. Um bloco retirado para de soar; um bloco que volta soa a partir do próximo “Reproduzir”.
- Um bloco de áudio volta com o seu áudio, lido de novo do arquivo.
- Os blocos de uma cena apagada da lista não voltam.

## Cuelists

Uma **cuelist** é uma lista de cues que avançam com GO. Ela funciona com **tracking**: cada cue guarda **só o que muda** e o resto continua das cues anteriores. Um GO só muda os canais que mudam: sem quedas de luz e sem refazer o que não precisa.

Com outras cuelists e cenas da mesma prioridade, cada canal que não é de intensidade (posição, gobo, roda de cores…) vem do último que o mudou: o GO que o muda o toma, e o que a cuelist só mantém de cues anteriores ela não retoma. Ao liberá-lo, o canal faz um fade para quem o tinha antes. Veja [Prioridades](directo.md#prioridades).

As cuelists são gravadas a partir do programador, operadas em [Playbacks](playbacks.md) e editadas aqui.

### Opções da cuelist

No topo do editor:

- **“Tipo”**: como a cuelist é reproduzida. Uma linha ao lado dos botões diz o que o tipo escolhido faz. Em Playbacks e neste editor, uma cuelist de outro tipo mostra a etiqueta dela: **SUB**, **CHASE**, **TC** ou **OVR**.
  - **“Normal”** (padrão): cada GO reproduz a próxima cue.
  - **“Chase”**: as cues seguem umas às outras sozinhas, em loop, a uma velocidade (veja abaixo).
  - **“Timecode”**: cada cue é reproduzida no seu tempo em um relógio (veja abaixo).
  - **“Submaster”**: o fader do playback dela manda (veja abaixo).
  - **“Override”**: o fader dela faz um crossfade de tudo o que ela controla entre o que está abaixo e a cue dela (veja abaixo).
- **“Tracking”**: ligado, cada cue guarda só o que muda. Desligado, cada cue por conta própria.
- **“Após a última cue”**: **“Loop”** (volta à primeira), **“Liberar”** ou **“Ficar”**.
- **“Tempo de liberação”**: o fade ao liberar a cuelist.
- **“Ao liberar, voltar à cue 1”**: depois de liberá-la, o GO recomeça pela primeira cue.
- **“Liberada pelo GO de outra cuelist”**: uma música que acaba quando a próxima começa.
- **“Fade das novas cues”**: o fade da primeira cue gravada; as seguintes pegam os tempos da anterior.
- **“Voltar”**: os tempos do BACK: **“Tempos da cue”** ou **“Tempo fixo”**.
- **“Prioridade”**.
- **“GO no ritmo”**: “Não”, “BeatGO” ou “Pulso” dão um GO em cada divisão.
- **“Ritmo marcado”**: o ritmo marcado tocando o botão dela várias vezes seguidas, em BPM: um GO em cada batida. Enquanto ela o tiver, os disparos Seguir e Esperar dela não contam. **“Remover”** volta aos tempos dela.
- **“Mover no escuro”**: quando uma cue termina, os fixtures no escuro que a próxima cue acende já assumem sua posição, cor e feixe. No GO eles só sobem de intensidade. **“Depois de”** é a espera após o fim da cue (0 s por padrão). **“Tempo”** é quanto demoram esses movimentos (2 s por padrão).
  - Um fixture que outro playback ou a própria cue ilumina não se move.
  - Em um fixture sem dimmer, o RGB dele é a sua luz: só se preparam os outros atributos dele.
  - Os canais de um FX não se preparam: o FX começa no GO dela.
  - Se você escolher outra cue para o próximo GO (“Próximo” em [Playbacks](playbacks.md)), prepara-se essa.
  - Não vale em uma cuelist “Chase” nem “Override”.

Em uma janela estreita, parte das opções se recolhe: **“Mais opções”** as mostra.

Uma cuelist **“Submaster”** é comandada pelo fader do [playback](playbacks.md#o-fader) dela:

- Ao subir o fader a partir do zero, ela inicia na próxima cue, no nível do fader e nunca no máximo.
- Ao baixá-lo até zero, ela é liberada com o tempo de liberação dela. Isso pode ser mudado nas [propriedades](playbacks.md#propriedades-de-um-playback) do playback.
- Enquanto ela toca, GO e Voltar mudam de cue no nível do fader.
- Liberada, GO e Voltar só escolhem a cue em que ela vai iniciar (Pausa/Voltar e “Ir” também). O playback a mostra como a próxima (“▸ 3”). Após a última cue, GO volta à primeira; com “Ficar”, fica na última.
- Se você a liberar com o fader em cima, baixe o fader até zero e suba-o de novo para iniciá-la.
- O flash a leva ao máximo enquanto você o pressiona.
- Em um grupo de Ao vivo e em um botão do SHOW ela toca como uma cuelist normal. Também quando não está em nenhum playback ou quando o fader dela não a inicia (“O fader o inicia” desligado).

Uma cuelist **“Chase”** percorre as cues dela sozinha, em loop:

- **“Velocidade”**: um passo em cada batida, em BPM (de 10 a 600; 120 por padrão). Com **“BPM do show”**, segue o tempo do show. **×¼**, **×½**, **×1**, **×2** e **×4** a multiplicam.
- **“Crossfade”**: a parte do passo que a mudança dura, em %. Com 0 (o padrão), é um corte. Com **“Tempos da cue”**, cada cue usa o próprio atraso e o próprio fade.
- **“Sentido”**: **“Para a frente”**, **“Ao contrário”**, **“Ida e volta”** (dá a volta na primeira e na última cue) ou **“Aleatório”** (nunca repete um passo duas vezes seguidas).
- GO passa ao passo seguinte na hora, e os seguintes contam a partir dali. Voltar vai um passo no outro sentido. Pausa a segura onde está.
- Um **“Ritmo marcado”** manda sobre a velocidade dela; **“Remover”** volta a ela. Com BeatGO ou Pulso, ela só avança em cada divisão.
- A [velocidade das cues](playbacks.md#velocidade-das-cues) e a velocidade ao vivo também a aceleram ou a freiam.
- O tracking continua valendo: um passo pode mudar só a cor e manter o resto.
- Ela sempre recomeça: não tem a coluna **DISPARO** nem **“Após a última cue”**, e os vínculos das cues não contam. Com um crossfade em %, os tempos das cues aparecem atenuados: não são usados.
- Em uma super cena, um bloco dura os passos dela vezes a duração de um passo. Em “Ida e volta”, a ida e a volta.

Em uma cuelist **“Override”**, o fader do [playback](playbacks.md#o-fader) dela a mistura com o que está abaixo:

- O fader faz um crossfade de tudo o que ela controla: intensidades, cores, posições e gobos. Na metade, os moving heads ficam a meio caminho. Em zero, tudo fica como se ela não estivesse lá.
- No máximo, a cue dela substitui o que está abaixo, também nas intensidades: um azul sobre um vermelho dá azul, não magenta.
- As cues, os GO e os fades funcionam como em uma cuelist normal. O fade de uma cue e o fader se multiplicam.
- Ao liberá-la, tudo volta com um fade para o que está abaixo, no tempo de liberação dela. Com o fader em zero ela continua carregada, como qualquer cuelist.
- Ao escolher “Override”, uma cuelist com a prioridade padrão (5) passa para 10. O programador continua acima dela.
- Uma cuelist ou uma cena da mesma prioridade disparada depois dela se mistura por cima como sempre: nas intensidades vence o valor mais alto.
- Um canal com **Fades** desligado nos [limites do fixture](patch.md#limites) salta para o valor dele assim que o fader sai do zero.
- Os FX dela rodam como em uma cuelist normal: o fader não faz esse crossfade neles.
- Se a cuelist já está rodando, a mudança para “Override” (ou de “Override”) vale a partir do próximo início dela.
- Usos típicos: um blinder ou a luz da plateia, uma posição fixa para um solo, uma luz de emergência em um fader.

Uma cuelist **“Timecode”** lança cada cue na sua hora em um relógio:

- Cada cue tem a sua **HORA** (horas:minutos:segundos.centésimos). A coluna **HORA** substitui **DISPARO**. Digite a hora (“00:01:23.40”, “1:23.4” ou “83.4”) ou use a roda. Se você a apagar, a cue fica sem hora e o relógio não a lança.
- **“Relógio”**: **“Relógio próprio”** ou **“Timecode MIDI (MTC)”** (veja abaixo). Com o relógio próprio, o GO lança a cuelist e põe o relógio em marcha em 00:00, ou na hora da cue em que ela está (a escolhida com “Ir para a cue” ou, com “Ao liberar, voltar à cue 1” desligado, a seguinte depois de liberá-la).
- Soa a última cue da lista cuja hora já chegou. Antes da hora da primeira cue, a cuelist não mostra nada.
- Ao passar para a cue seguinte, a cue faz o seu fade. Um salto (para trás ou de várias cues) vai direto ao visual daquela hora, sem fade.
- O GO passa para a cue seguinte com o seu fade, e o relógio salta para a hora dela. Voltar vai para a cue anterior e a sua hora, sem fade (com um tempo fixo em **“Voltar”**, nesse tempo). “Ir para a cue” põe o relógio na hora dela. Com uma cue sem hora, o relógio continua como ia.
- Pausa para o relógio. A [velocidade das cues](playbacks.md#velocidade-das-cues) e a velocidade ao vivo o aceleram ou o freiam.
- Os seus Seguir, Esperar e vínculos não contam. “Após a última cue” vale para o GO na última cue.
- O cabeçalho e o playback dela mostram o relógio: **“TC 00:01:23.4”**.
- **“Pôr no relógio”** (o relógio da barra de ferramentas) põe a cue selecionada na hora do relógio enquanto a cuelist soa.
- Uma cue gravada enquanto o relógio corre recebe a hora dele. Com a cuelist liberada ou em pausa, recebe a hora da cue anterior mais 5 s.
- Um triângulo avisa de uma cue cuja hora é anterior à da cue de antes. Das duas, só soa a última.
- Ao escolher “Timecode”, as cues sem hora recebem uma: 00:00 a primeira e 5 s a mais que a anterior as outras.
- Numa [super cena](escenas.md#super-cenas), o relógio é a linha do tempo.

Com **“Timecode MIDI (MTC)”**, a cuelist segue o timecode que uma DAW, um reprodutor de vídeo ou um gerador de timecode envia para uma entrada MIDI:

- Ative a entrada do dispositivo MIDI em [Mapeamentos › Dispositivos](mapeos.md#dispositivos). Não é preciso nenhum mapeamento.
- **“Deslocamento”**: o timecode em que fica o 00:00 da cuelist, em horas:minutos:segundos:quadros (por exemplo “01:00:00:00”). Antes dessa hora, a cuelist não mostra nada.
- Ao lado aparece o timecode que chega, **“MTC 01:00:12:05 · 25 fps”**, ou **“Sem timecode MIDI”**.
- Taxas de quadros: 24, 25, 29,97 drop-frame e 30.
- Ao lançá-la, a cuelist espera o timecode e vai direto à cue daquela hora. Se for a primeira cue, a cue faz o seu fade.
- Se o timecode parar ou cair, a cuelist fica na sua cue. Se ele voltar de outra hora (um retrocesso ou um salto), a cuelist vai direto à cue daquela hora, sem fade.
- GO, Voltar e “Ir para a cue” não fazem nada: quem manda é o timecode. O GO na cuelist liberada a lança.
- A velocidade das cues não move o relógio; só muda os fades.
- Também numa super cena, o relógio é o timecode.

### A tabela de cues

| Coluna | O que é |
|---|---|
| **CUE** | O número. Aceita decimais: a 2.5 fica entre a 2 e a 3. Clique duplo para alterá-lo. Um lápis indica que a cue está carregada no programador. |
| **NOME** | O nome dela. |
| **DISPARO** | **GO** (espera um GO), **Seguir** (quando a cue anterior termina, mais um tempo) ou **Esperar** (um tempo depois do GO da cue anterior). |
| **HORA** | Só numa cuelist “Timecode”, no lugar de DISPARO: a hora da cue no relógio. Um triângulo avisa se ela é anterior à da cue de antes. |
| **ATRASO** | Quanto a cue espera antes de começar o fade. |
| **FADE** | O tempo de fade. |
| **DESCIDA** | O fade dos dimmers que descem. “=” é igual ao fade. |
| **MUDANÇAS** | Quantos canais ela muda, e as marcas **FX**, **[R]** (valores liberados), o ícone de paleta se ela usa paletas, um relógio se ela tem tempos próprios ou uma curva, um elo se ela tem um vínculo e um raio se ela tem ações (passe o mouse sobre eles para vê-los). |
| **S** | **Pular** (só esta cue): o que ela muda não passa para as seguintes. |
| **B** | **Bloqueio**: Gravar e Atualizar guardam também os valores iguais aos que ela herda. |
| **MIB** | Só com “Mover no escuro”: a cue é preparada enquanto seus fixtures estão no escuro. Desmarcada, os movimentos dela são vistos no seu GO. |
| **NOTA** | Uma nota para você. |

Para mudar um tempo, clique nele e digite os segundos, ou use a **roda** do mouse sobre o tempo clicado: ±0,1 s (com Shift, 1 s). Com **Ctrl+clique** ou **Shift+clique** você seleciona várias cues e muda os tempos, o disparo ou as marcas delas juntos.

A cue que está rodando tem uma barra verde com o avanço do fade; a próxima, uma borda verde. No topo aparece o estado (“Q 3 · Verso ▸ 4”, com “↺ 1/3” se a próxima é o salto de um vínculo, ou “Liberada · GO reproduz 1”) e os botões **Voltar**, **GO** e **liberar**, os mesmos do playback dela.

Enquanto uma cue está em fade, o cabeçalho mostra também **“Tempo ao vivo”**: o tempo dessa cue, que você pode mudar enquanto ela roda (digitando outro, com **“×½”**, **“×2”** ou **“Agora”**), e o botão para gravá-lo na cue. Funciona como em [Playbacks](playbacks.md#tempo-ao-vivo).

### Gravar e editar cues

- **“Gravar a cue N”**: o programador vira uma nova cue depois da selecionada, só com o que muda em relação ao que ela herda.
- **“Atualizar ▾”**: mescla o programador na cue selecionada: **“Atualizar a cue N (as seguintes acompanham)”** ou **“Só esta cue (as seguintes mantêm o visual)”**.
- **“Carregar ▾”** (ou clique duplo em uma cue): leva a cue ao programador: **“O que o palco mostra na cue N”** ou **“Só o que a cue N muda”**. Depois, atualize-a pela barra “Editando a cue…” de Programar.
- **“Tirar ▾”**: com os canais do programador, **“Tirar da cue N os canais do programador (voltam a herdar)”** ou **“[Liberar] os canais do programador na cue N”** (a partir dessa cue eles voltam ao que há por baixo: outras cenas ou o valor de repouso).
- **Copiar** para o final: **“Copiar o que a cue N muda”** ou **“Copiar todo o visual da cue N (cue de bloqueio)”**.
- **“Número…”**: outro número para a cue selecionada. Mover uma cue muda o que as cues seguintes herdam.
- **Excluir**: com **“Manter o visual das próximas cues”** marcada, o que a cue muda passa para a próxima e as seguintes ficam iguais.

Em uma lista estreita (ou em um idioma com palavras longas), “Atualizar”, “Carregar” e “Tirar” ficam só com o ícone e a seta **▾**; ao passar o mouse aparece o nome deles.

O botão direito em uma cue tem **“Ir para a cue N”**, as duas formas de carregar, **“Atualizar com o programador”**, **“Tempos…”**, **“Vínculo e ações…”**, **“Número…”** e **“Excluir…”**.

As cues podem usar **paletas por referência** e ter **camadas de FX**, que são herdadas de cue em cue.

### A planilha de tracking

**“Detalhes da cue”** tem três abas: **“Tracking”**, **“Tempos”** e **“Vínculo e ações”**. Se a janela for larga, o painel aparece sozinho ao lado das cues.

**“Tracking”** mostra o que o palco tem na cue selecionada, fixture por fixture (“No palco: cue N”).

- As etiquetas coloridas dizem o que é cada valor: **“Mudanças”** (esta cue o muda), **“Herdados”** (vem de uma cue anterior, que é indicada: “Q2”), **“Bloqueio”** e **“Liberados”** (riscados, com [R]).
- No topo aparecem as paletas que a cue usa.
- Clique nos canais para escolhê-los e pressione **“Liberar”** (coloca-os em [RELEASE] a partir desta cue) ou **“Tirar”** (voltam a herdar das cues anteriores).

### Tempos da cue

Dentro de uma cue, cada atributo e cada fixture podem ter seu próprio atraso e seu próprio fade. Selecione uma cue (ou várias) e abra **“Detalhes da cue”** › **“Tempos”**, ou clique com o botão direito › **“Tempos…”**. Com várias cues selecionadas, as mudanças valem para todas.

- **“Tempos por atributo”**: **Intensidade**, **Cor**, **Posição** e **Feixe**, cada um com ATRASO e FADE. Vazio ou “=” é o tempo da cue. O strobo vai com Intensidade; gobo, prisma, zoom, foco e iris vão com Feixe.
- **“Tempos por fixture”**: **“+ Fixtures selecionados”** adiciona os fixtures selecionados em Programar, com os tempos da cue. Depois mude os tempos deles. **×** remove um e **“Remover”** remove todos.
- O tempo de um fixture vence o do seu atributo, e o de um atributo vence o da cue.
- Os dimmers que descem usam o fade do seu fixture, se ele tiver um. Se não, a DESCIDA da cue; com “=”, o fade de Intensidade.
- Uma cue dura o seu tempo mais longo: Seguir, a barra verde e a duração em uma super cena contam com ele. Também conta o tempo de um atributo que a cue não muda.
- Um só tempo para tudo, como antes: o fade de entrada de um grupo ou de um bloco de super cena na primeira cue, e um tempo fixo de **“Voltar”**. Com **“Tempos da cue”**, o BACK usa todos os tempos da cue para a qual volta.
- As camadas de FX entram com o fade da cue.
- Uma cue nova pega os tempos de atributo da cue anterior, mas não os dos seus fixtures. Copiar uma cue copia todos os seus tempos.

Exemplo: a cue 2 muda cor e posição com fade 0, e Posição tem atraso de 1 s e fade de 3 s. Com GO, a cor muda na hora; um segundo depois, os moving heads se movem durante 3 s.

**Curvas.** **“Curva dos fades”** diz como avançam os fades da cue:

- **Linear** (padrão): a mesma velocidade o tempo todo.
- **Suave**: começa e termina devagar.
- **Início lento**: quase não se move no começo e acelera no fim.
- **Início rápido**: avança quase de uma vez e depois se acomoda.

A curva vale para tudo o que a cue faz em fade: dimmers que sobem e que descem, cores, movimentos e os canais que ela libera. Cada atributo também tem sua **CURVA**; “=” é a da cue. O tempo de liberação da cuelist, um tempo fixo de **“Voltar”**, o fade de entrada de um grupo e as camadas de FX são sempre lineares. O relógio da coluna MUDANÇAS também aparece se a curva da cue não é linear.

**Leque de tempos.** Em **“Tempos por fixture”**, **“Leque”** distribui um atraso ou um fade entre vários fixtures:

- Escolha **Atraso** ou **Fade**, e os tempos **de** e **a**.
- Escolha a forma: **Um após o outro**, **Do centro** ou **Das pontas**.
- Escolha os fixtures: **“Selecionados em Programar”**, na ordem em que você os selecionou, ou **“Desta cue, por número”**.
- **“Aplicar”** dá a cada fixture o seu tempo nas cues selecionadas. O outro tempo dele não muda.

Com 5 fixtures e um atraso de 0 a 2 s, **Um após o outro** dá 0; 0,5; 1; 1,5 e 2 s. **Do centro** dá 2; 1; 0; 1 e 2 s. **Das pontas** dá 0; 1; 2; 1 e 0 s. Se “de” é maior que “a”, a ordem se inverte. O leque escreve tempos fixos, não uma regra: depois você pode mudar cada linha.

### Vínculos e ações

Selecione uma cue e abra **“Detalhes da cue”** › **“Vínculo e ações”**, ou clique com o botão direito › **“Vínculo e ações…”**. Com várias cues selecionadas, você edita a última que escolheu.

**Vínculo.** **“Depois desta cue”** diz qual cue toca depois dela: **“A cue seguinte”** (o normal) ou outra cue da lista. **“Vezes”** diz quantas vezes o vínculo salta antes de seguir; 0 é sempre. Um vínculo novo começa com 1.

- Com a cue 5 → cue 2, 3 vezes, os GOs tocam 1 2 3 4 5, 2 3 4 5, 2 3 4 5, 2 3 4 5 e 6: as cues 2 a 5 tocam 4 vezes. Embaixo, uma frase explica: “As cues 2 a 5 tocam 4 vezes e depois a 6.”
- Com 0 (sempre), as cues se repetem até você ir para outra cue.
- Um vínculo para a frente pula cues; um vínculo para a própria cue a repete.
- Vale o disparo da cue para a qual ele salta: se a cue 2 tem Seguir, o loop avança sozinho.
- Essa cue é a próxima em todos os lugares: a borda verde da tabela, o topo (“▸ 2 ↺ 1/3”: o primeiro salto de três; só “↺” se for sempre), os playbacks, o SHOW, o celular e os LEDs do controlador.
- O BACK e “Ir para a cue” não contam saltos. Liberar a cuelist, ou iniciá-la de novo, começa a contagem de novo.
- Feitos todos os saltos, a lista segue depois da cue. Na próxima vez que chega a ela, a contagem começa de novo.
- Na última cue, o vínculo vem antes de **“Após a última cue”**.
- Em uma cuelist “Chase” ou “Timecode”, os vínculos não contam: a aba diz isso. O vínculo fica guardado para quando a lista voltar a ser “Normal”.
- Se você excluir a cue para a qual ele salta, o vínculo some e um aviso diz isso.

**Ações ao começar.** **“+ Adicionar uma ação ▾”** adiciona o que a cue faz quando começa:

- **“GO de uma cuelist”**, **“Ir para uma cue…”** de outra cuelist e **“Liberar uma cuelist”**.
- **“Iniciar uma cena”** e **“Parar uma cena”**, no grupo dela, se tiver um (como os botões do SHOW). Não mudam a cena selecionada.
- **“Liberar um grupo”** (ou todos) e **“Liberar todos os playbacks”**.
- **“Definir o BPM”**, de 20 a 300.

Cada linha se muda ali mesmo; o **×** a remove. As ações acontecem em ordem quando a cue começa com GO, Seguir, Esperar ou “Ir para a cue”, também a primeira cue ao iniciar a cuelist. Não acontecem com o BACK, quando uma edição aplica a cue de novo nem quando uma super cena começa a cuelist no meio.

- A própria cuelist não aparece: para isso existe o vínculo.
- Uma ação sobre uma cena ou cuelist excluída mostra “(excluída)” e não faz nada.
- Se duas cuelists iniciam uma à outra, o loop para: uma cuelist iniciada por uma ação não inicia, com as ações dela, uma cuelist anterior nessa cadeia. Além disso, a mesma cue não repete as ações antes de 200 ms.
- Copiar uma cue não copia o vínculo nem as ações dela.

### Desfazer

As setas **Desfazer** e **Refazer**, depois da lixeira, desfazem e refazem as edições da cuelist, uma a uma. **Ctrl+Z** e **Ctrl+Y** (ou **Ctrl+Shift+Z**) também. Ao passar o mouse elas dizem o que desfazem: “Desfazer: Gravar a cue 3”.

- Tudo o que muda as cues ou as opções da cuelist pode ser desfeito: gravar (também a partir de Programar, de um playback ou da linha de comando), atualizar, tirar, liberar, copiar, excluir, o número, os tempos, o disparo, o nome, a nota, as marcas S e B, o vínculo, as ações e as opções do topo, exceto “GO no ritmo” e “Ritmo marcado”.
- Cada cuelist guarda os seus 50 últimos passos.
- Vários giros da roda do mouse (ou valores digitados) no mesmo tempo, com menos de 2 segundos entre eles, são um só passo. Se você voltar ao valor de partida, o passo desaparece.
- Depois de desfazer, fica selecionada a primeira cue que muda. Se a cuelist estiver tocando, o palco muda na hora.
- Em um campo com texto digitado, Ctrl+Z desfaz primeiro esse texto.
- Criar a cuelist e converter passos em cuelist não podem ser desfeitos. Desfazer mantém os valores atuais das paletas e não recupera os canais de fixtures excluídos.
- Uma prioridade ou um tempo de liberação alterados em outro lugar (Ao vivo, Playbacks) ficam como estão.
- O histórico não é salvo com o show. Novo e Abrir o esvaziam, e excluir a cuelist também.

### Converter passos ou um chaser em cuelist

Em uma cena de passos, o botão **“Converter em cuelist”** do cabeçalho transforma cada passo em uma cue com o que muda em relação ao anterior; os passos com tempo se encadeiam com **Esperar**. Os grupos, botões do SHOW e mapeamentos dela passam a reproduzir a cuelist. Os passos deixam de existir. As cues continuam usando as paletas dos passos.

Em um chaser, cada cue leva a cena do seu passo (os Move FX viram FX) e libera o que o passo anterior tinha e este não; os passos que não são cenas nem efeitos ficam em uma cue vazia com uma nota. As cenas dele ficam.

Um ping-pong vai e volta; uma ordem aleatória toca em ordem. Os tempos em beats passam a segundos com o BPM desse momento. Os passos de uma cópia para o QLC+ voltam com o número, o nome e a nota de cada cue.

### Cuelists e QLC+

O QLC+ não lê cuelists: se você abrir este show no QLC+, elas não aparecem. Para levá-las ao QLC+, pressione **“QLC+”** no cabeçalho de uma cuelist. **“Salvar uma cópia para o QLC+”** grava outro `.qxw` em que cada cuelist marcada é uma lista de passos do QLC+:

- **Sequence** (padrão): um único item no QLC+.
- **Chaser**: uma cena por cue, em uma pasta com o nome da cuelist.

Com **“Uma lista de cues para cada uma no Console Virtual”** marcada, a cópia abre no Console Virtual do QLC+, com uma moldura “Cuelists” e uma lista de cues por cuelist: “Próximo” é o GO, “Anterior” o BACK e “Stop” a libera. O seu show não muda.

A caixa **“Com suas teclas e botões do controlador”** dá a cada lista de cues as teclas e os botões do controlador da sua cuelist: GO, Voltar, Pausa/Voltar e Liberar passam a “Próximo”, “Anterior”, “Reproduzir” e “Stop”. Contam os mapeamentos da cuelist e os do playback que a tem na página atual. Cada botão do QLC+ leva uma tecla e uma entrada. Os controles OSC e os de um dispositivo não ativado não passam; a janela avisa.

Cada passo guarda tudo o que o palco mostra nessa cue, e a nota dele diz o número e o nome (“Q2.5 · Verso”). O fade da cue vira o fade de entrada do passo, e a descida, o de saída. GO deixa o passo esperando “Próximo”; Esperar e Seguir viram um tempo. Em uma cuelist “Chase”, os passos vão sozinhos, em loop e no seu sentido: cada um dura um passo da sua velocidade, com o seu fade. Em uma cuelist “Timecode”, cada passo dura até a hora da cue seguinte. Os passos levam o ID da cuelist: as super cenas e os racks que a reproduzem reproduzem os passos.

Abaixo de cada cuelist, a janela avisa o que não passa:

- “Chase” com “BPM do show” ou com um ritmo marcado: os passos mantêm um BPM fixo, pelo seu multiplicador: o do show naquele momento com “BPM do show”, senão o da cuelist. Não seguem o ritmo marcado.
- “Timecode”: os passos correm desde o primeiro GO; não seguem o timecode MIDI nem a linha do tempo de uma super cena. O primeiro passo toca com o GO, não na hora da sua cue. Em uma cue sem hora, param até um GO.
- “Submaster”: os passos começam com um GO, não com o fader.
- “Override”: os passos se misturam com o que está abaixo como os outros.
- Os FX: só seus valores base.
- O atraso, também o de um atributo ou de um fixture: o passo começa na hora.
- Uma descida diferente do fade: só aproximada.
- Os tempos por atributo e por fixture: só o fade da cue.
- As curvas dos fades: só lineares.
- Um Esperar mais curto que o fade, ou em “Chase” e “Timecode” um passo mais curto que o seu fade: o passo seguinte espera o fade acabar.
- Os vínculos: os passos tocam em ordem. Se a última cue tem um vínculo sempre com a primeira, os passos recomeçam.
- As ações das cues: não são feitas.
- Em Sequence, os valores liberados vão para o seu valor de repouso. Em Chaser, ficam por conta do QLC+.
- Com “Ficar”, GO na última cue para os passos.
- GO no ritmo e ritmo marcado: os passos não seguem o ritmo.
- “Mover no escuro”: os fixtures se movem enquanto sobem.
- “Liberada pelo GO de outra cuelist”: essas listas vão para uma moldura “Cuelists (uma de cada vez)”, onde lançar uma para as outras.

Uma cuelist que já tem sua lista de cues no Console Virtual (um show do QLC+ cujos passos você converteu em cuelist) não ganha outra: a de antes a lança de novo. Teste a cópia no QLC+ antes de usá-la ao vivo: o QLC+ faz os fades do jeito dele.

### Cue lists do QLC+

**“Nova” › “Cue lists do QLC+…”** converte em cuelists os chasers, as sequências e as listas de cues do Console Virtual de um show do QLC+: do show aberto ou de outro (“De outro show…”). Marque as que você quer converter. Primeiro aparecem as listas de cues do Console Virtual, marcadas; depois, os chasers e as sequências que nenhuma lista usa. As que já têm uma cuelist com o nome delas aparecem sem marcar.

Cada passo é uma cue com o que muda; os passos com tempo se encadeiam com Esperar e os que esperam são GO. Uma lista de cues do Console Virtual dá o nome dela à cuelist.

- **“Substituir os originais”** (só no show aberto): a cuelist ocupa o lugar e o ID deles. Os grupos, botões do SHOW e mapeamentos deles passam para a cuelist, mas o QLC+ não os vê mais. Sem marcar, os originais ficam e o show continua tocando igual no QLC+.
- **“Colocá-las nos playbacks livres desta página”**. Se não couberem todas, o aviso diz quantas ficam de fora.
- **“Trazer suas teclas e botões do controlador como mapeamentos”**: “Próximo”, “Anterior”, “Reproduzir” e “Stop” de cada lista de cues passam a GO, Voltar, Pausa/Voltar e Liberar da sua cuelist. Não passam os controles OSC, as teclas com Ctrl, Alt ou Shift, as entradas de um universo sem controlador neste show nem os controles que já têm um mapeamento. O aviso diz quantos mapeamentos são criados e quais controles ficam de fora. Um botão de uma Akai APC vem com os seus LEDs, como ao aprendê-lo.

De outro show só entram as cuelists, nos fixtures deste show que coincidem com os dele (primeiro o mesmo endereço e o mesmo modelo). O aviso diz quantos fixtures não estão.

Não passam: a ordem aleatória (tocam em ordem), os beats (passam a segundos com o BPM atual), o fader lateral das listas de cues e os passos que não são cenas nem efeitos. As listas de cues no formato antigo do QLC+ não podem ser convertidas: abra e salve-as antes no QLC+.

Ao abrir um show do QLC+ com listas de cues e sem cuelists, o aviso “Aberto” lembra onde convertê-las.

## Pastas

As pastas organizam a lista de cenas (também no SHOW, nas super cenas, nos racks e ao adicionar cenas a um grupo).

- **Criar**: o botão de pasta com **+** ao lado da busca, ou **“Nova pasta…”** no menu de uma cena.
- **Colocar cenas**: arraste-as sobre a pasta, ou botão direito › **“Mover para a pasta”**. Funciona com várias cenas selecionadas.
- **Tirar**: botão direito › **“Tirar da pasta”**.
- **Renomear ou excluir**: o botão **⋯** da pasta (ou botão direito): **“Renomear a pasta”** e **“Excluir a pasta (as cenas dela ficam)”**.
- Clique no cabeçalho de uma pasta para recolhê-la ou expandi-la.

As pastas são salvas com o show.

## Importar e exportar

- **Exportar**: o botão de exportar do cabeçalho salva a cena em um arquivo `.dtcfx` (“Efeitos do DMX Total Control”) para carregá-la em outros shows. Funciona também com racks.
- **Importar**: **“Nova” › “Importar de um arquivo…”**. Os fixtures são pareados com os deste show; se algum do arquivo não estiver aqui, a parte dele é omitida e há um aviso.
- **As cenas de outro show inteiro**: em [Patch › Importar](patch.md#importar-de-outro-show), “Cenas de outro show…”.
- **Para o QLC+**: “QLC+” no cabeçalho de uma cuelist ([Cuelists e QLC+](escenas.md#cuelists-e-qlc)).
- **Do QLC+**: “Nova” › “Cue lists do QLC+…” ([Cue lists do QLC+](escenas.md#cue-lists-do-qlc)).
