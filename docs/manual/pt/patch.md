# Patch

![A página Patch](../img/patch.png)

**Patch** (F5) tem os fixtures da sua montagem: o que são, em qual universo e endereço estão, o número deles, os grupos de fixtures e onde estão no palco.

No cabeçalho você escolhe a vista e tem as ferramentas para adicionar:

- **“Lista”**: todos os fixtures por endereço.
- **“Canais”**: os 512 canais de cada universo, com os fixtures por cima.
- **“Plano 2D”**: o palco visto de cima.
- **“Importar”**, **“Matriz de LED”** e **“Adicionar fixtures”**.
- Com fixtures estacionados, **“N estacionados · Desestacionar tudo”**: quantos são e o botão para desestacionar todos (veja [Estacionar canais](patch.md#estacionar-canais)).

## Adicionar fixtures

1. Pressione **“Adicionar fixtures”**.
2. Escolha o **Fabricante** (digite em “Buscar fabricante…” para filtrar) e o **Modelo**.
3. No painel **Patch**:
   - **Nome**: o que você verá em todo o programa (por padrão, o do modelo).
   - **Modo**: o modo DMX, com o número de canais dele.
   - **Quantidade**: quantos iguais.
   - **Universo** e **Endereço**: o endereço é definido sozinho no primeiro livre do universo.
   - **Intervalo**: canais livres entre um fixture e o seguinte.
   - Com mais de um, **“Criar um grupo com eles”** e o nome do **Grupo**.
4. Embaixo aparece quais endereços eles vão usar (“Usa os endereços 1 a 20”). Se não couberem no que resta do universo, eles passam para o seguinte. Se o modo tiver mais *DMX breaks* (um fixture GDTF com duas entradas DMX), um aviso pede para patchear também o modo “(DMX break 2)” dele, no seu próprio endereço.
5. Pressione **“Patchear fixture”** ou **“Patchear N fixtures”**.

A janela fica aberta para continuar adicionando, com o endereço já no próximo livre. Se algo não puder ser patcheado (os endereços estão ocupados, por exemplo), o motivo aparece em vermelho embaixo. Pressione **“Fechar”** ao terminar.

Para dimmers avulsos, escolha **Generic › Generic Dimmer**: em vez de modo, você informa quantos **Canais**.

**“Buscar no GDTF-Share…”**, embaixo, abre o [GDTF-Share](fixtures.md#gdtf-share): baixe o fixture e pressione **“Patchear”** para deixá-lo escolhido aqui.

> **Dica:** se não encontrar seu fixture, busque-o no GDTF-Share, baixe a [Open Fixture Library](fixtures.md#open-fixture-library), importe o GDTF dele ou crie-o na página [Fixtures](fixtures.md).

## A lista

Colunas: caixa de seleção, **#** (número), **NOME**, **ENDEREÇO** (universo . endereço), **CANAIS**, **FIXTURE** (fabricante e modelo), **MODO**, **INVERTER** (pan e tilt) e o botão de identificar. Depois do nome, um **escudo** marca um [efeito especial protegido](patch.md#efeitos-especiais-protegidos) e um **cadeado**, um fixture com canais [estacionados](patch.md#estacionar-canais).

- **“Agrupar por fixture”** os agrupa por modelo, com uma caixa para selecionar todos os de cada modelo. Um clique no cabeçalho de um modelo o recolhe (oculta os fixtures dele) ou o expande.
- **Clique duplo** em um fixture o edita.

## Selecionar

As ferramentas da barra trabalham sobre a seleção:

- **Clique** seleciona; **Ctrl+clique** adiciona ou tira; **Shift+clique** seleciona todos os fixtures desde o último que você clicou até esse (nesse sentido, que é a ordem da seleção), e **Ctrl+Shift+clique** os adiciona. **“Todos”** e **“Nenhum”**. A caixa de seleção de uma linha adiciona ou tira o fixture dela (como Ctrl+clique); com Shift, adiciona todos desde o último que você clicou.
- **“Identificar”**: os fixtures selecionados piscam em branco para que você os encontre no palco. **“Parar de identificar”** interrompe isso (também para ao sair da página). O botão de cada linha identifica só esse fixture. A lista fica onde estava. Os efeitos especiais protegidos nunca são identificados (piscar um CO2 o dispararia): um aviso diz quantos ficaram de fora.
- **Inverter pan** e **inverter tilt** (os ícones de setas): para os fixtures que os têm.
- **“Limites”**, o botão da **chama** (**“Efeito especial”**), **“Editar”**, **“Reendereçar”**, **“Renumerar”**, **“Criar grupo”** e **“Excluir”**: são explicados a seguir.

**Clique com o botão direito** em uma linha da lista, ou em um fixture do plano 2D sem mover o mouse, para abrir o menu do fixture; se o fixture não estava selecionado, ele é selecionado antes. No topo ele diz sobre o que atua: o fixture (o número, o nome, o endereço e o tipo) ou quantos estão selecionados e quais. Abaixo, com os mesmos ícones da barra: **“Identificar”** (enquanto há fixtures piscando, **“Parar de identificar”**, com quantos são); em **Ajustes**, **“Limites…”**, **“Efeito especial…”** (com o escudo da lista se há efeitos protegidos), **“Editar…”** (com um só fixture; com vários diz “Um de cada vez”) e o interruptor **“Proteger do grand master”**; em **Estacionar**, que à direita diz quantos da seleção estão estacionados, **“Estacionar como está agora”**, **“Estacionar apagado”** e, com algum fixture estacionado na seleção, o interruptor **“Também em blackout”** e **“Desestacionar”**; e em **Todo o show**, com fixtures estacionados, **“Desestacionar tudo”**, que desestaciona todos os do show, não só os selecionados. Um interruptor pela metade, com quantos o têm (3/4), quer dizer que só alguns dos selecionados o têm: ao clicar nele, ele é ativado em todos. Com o mouse parado sobre a maioria das entradas, uma nota ao lado do menu diz o que fazem.

## Endereços e universos

- **Editar um fixture**: selecione um e pressione **“Editar”** (ou clique duplo). Você pode mudar **Nome**, **Modo**, **Universo**, **Endereço** e **Número**, e pressionar **“Aplicar”**.
- **Reendereçar vários**: selecione-os e pressione **“Reendereçar”**. Escolha **Universo**, **Endereço inicial** e **Intervalo**: eles recebem endereços consecutivos, na ordem da lista.
- **Arrastar na vista Canais**: escolha o universo no topo e arraste um fixture para outro endereço. A sombra aparece em verde se couber e em vermelho se os canais não estiverem livres. No topo aparece quantos dos 512 canais estão em uso. Clique duplo em um fixture o edita.

Os universos são criados e conectados às interfaces em [Saídas](salidas.md).

## Números de fixture

Cada fixture tem um **número**, que é o que a [linha de comando](programar.md#linha-de-comando) de Programar usa (`1 THRU 8 @ FULL`). Um fixture novo recebe o número seguinte ao último.

- Mude um em **“Editar”** › **Número**.
- Para numerar vários seguidos, selecione-os, pressione **“Renumerar”** e digite o **Primeiro número**: eles são numerados por ordem de endereço.

## Grupos de fixtures

Os grupos de fixtures servem para selecionar com um clique no Patch e em Programar (`GROUP 2` na linha de comando), e cada um tem sua tira no [mixer](directo.md#mixer). Eles também são usados no SHOW (roda de cores, grade XY, paletas, dimmer e velocidade de movimento) e nos mapeamentos.

1. Selecione os fixtures.
2. Pressione **“Criar grupo”** e digite o nome (por exemplo, `Wash frontal` ou `Spots`).

O painel **“Grupos de fixtures”** da esquerda os mostra com o número de fixtures de cada um. Um clique em um grupo seleciona os fixtures dele (na vista “Plano 2D”, o isola: veja [Isolar um grupo no plano](patch.md#isolar-um-grupo-no-plano)); **Ctrl+clique** ou **Shift+clique** os adiciona. Botão direito:

- **“Renomear”**.
- **“Mover para cima”** e **“Mover para baixo”**: movem o grupo na lista.
- **“Substituir pela seleção”**: o grupo passa a ter os fixtures selecionados.
- **“Salvar a seleção aqui…”**: veja [Seleções](patch.md#seleções).
- Padrões de seleção, na ordem do grupo: **“Selecionar os ímpares”**, **“Selecionar os pares”**, **“Selecionar em ordem inversa”**, **“Selecionar a primeira metade”**, **“Selecionar a segunda metade”**, **“Selecionar das pontas para o centro”** e **“Selecionar do centro para as pontas”**.
- **“Excluir grupo”**.

**A ordem dos grupos** quem escolhe é você: arraste um grupo pela alça (à esquerda do nome) até outro lugar da lista, ou use “Mover para cima” e “Mover para baixo”. **“A–Z”**, no cabeçalho do painel, ordena os grupos em ordem alfabética (os números como números: “Truss 2” antes de “Truss 10”). Os grupos novos vão para o final. A ordem é salva com o show e é a mesma em Programar e nos números de `GROUP` da linha de comando.

### Seleções

Uma **seleção** é uma parte ordenada de um grupo: por exemplo, os spots de um grupo de seis tomados das pontas para o centro. A ordem importa, porque os leques e os FX a seguem.

1. Selecione os fixtures na ordem que quiser (ou com um padrão do menu do grupo).
2. Pressione o **+** do grupo (ou “Salvar a seleção aqui…”) e digite um nome.

As seleções aparecem sob o grupo delas. Um clique seleciona os fixtures dela na sua ordem, aqui e em Programar. Botão direito: **“Renomear”** e **“Excluir a seleção”**. A seta à esquerda do grupo oculta ou mostra as seleções dele.

## Plano 2D

O plano mostra o palco visto de cima, com **PÚBLICO** embaixo. Ele serve para selecionar em Programar, para ver o que sai e para os efeitos de cor posicionados no plano.

No cabeçalho do plano:

- **“Largura (m)”** e **“Profundidade (m)”**: o tamanho do palco, até 200 m. Aumentam ou diminuem o palco pela direita e pela frente; um fixture que fique fora de um palco menor fica onde está.
- **“Organizar automaticamente”**: distribui todos os fixtures.

Fora do palco, no meio de cada lado, há um **“−”** e um **“+”**: removem ou adicionam 1 m nesse lado (à esquerda, à direita, no fundo ou na frente, do lado do público); mantenha-os pressionados para vários metros. Os fixtures ficam onde estão no palco: à esquerda e no fundo eles se movem com esse lado, assim como o ponto para onde o [Follow](programar.md#follow-apontar-para-um-ponto) aponta, e a nova faixa aparece nesse lado. Enquanto o mouse está sobre eles, a vista do plano não muda e os botões não se movem, então você pode clicar várias vezes seguidas; ao afastar o mouse, o plano se ajusta de novo (se você tinha dado zoom ou movido a vista, ela fica como está). O **“−”** de um lado só funciona se nenhum fixture estiver a 1 m ou menos dele: mova-o antes (passe o mouse sobre o botão para ver o motivo).

As ferramentas (trabalham sobre a seleção e na ordem dela):

- **“Ao vivo”**: mostra a cor, o dimmer, o strobo e o gobo de cada fixture, e cada pixel das barras e matrizes.
- **“Laço”**: seleciona desenhando ao redor dos fixtures em vez de com um retângulo.
- **“Selecionar”**: clique ou retângulo para selecionar.
- **“Mover”**: arraste os fixtures (a seleção inteira se move) ou a vista.
- **“Grade”**: “Livre”, 10 cm, 25 cm, 50 cm ou 1 m: onde os fixtures caem ao serem movidos.
- **“Aparência”**: como os fixtures selecionados aparecem: “Redondo”, “Quadrado” ou “Retângulo”, e o seu **“Ícone”** (veja [Ícones, cores e legenda](patch.md#ícones-cores-e-legenda)).
- **“Organizar”**: posiciona-os em “Linha”, “Linha, da frente para o fundo”, “Círculo”, “Grade, por linhas”, “Grade, em zigue-zague”, “Grade, por colunas” ou “Grade, em zigue-zague por colunas”, com o **Espaçamento (cm)** e as **Colunas** ao lado.
- **“Alinhar”**: “Esquerda”, “Centro”, “Direita”, “Fundo”, “Meio”, “Frente”, “Distribuir na largura” e “Distribuir na profundidade”.
- **15°** (botão direito: para o outro lado) e **90°**: giram a seleção.
- **“Altura e orientação”**: onde estão pendurados os moving heads selecionados, para o [Follow](patch.md#altura-e-orientação-follow).

Os moving heads têm uma pequena seta na borda (um entalhe, quando o plano os mostra pequenos): para onde eles apontam. Os nomes dos fixtures muito próximos ficam ocultos para não se sobreporem: aproxime o zoom para lê-los. Um **escudo** no canto superior esquerdo marca um efeito especial protegido (fraco; vermelho com os efeitos armados; com um anel que pulsa enquanto dispara) e um **cadeado** no canto superior direito, um fixture estacionado.

A **roda** do mouse faz zoom, o **botão direito** ou o do meio arrastam a vista e um clique duplo com eles a centraliza. Os botões do canto também fazem zoom. Um clique com o botão direito sem mover o mouse abre o [menu do fixture](patch.md#selecionar).

> **Dica:** posicione os fixtures na ordem em que você quer que os efeitos corram: selecione-os da esquerda para a direita e use “Organizar › Linha”.

### Ícones, cores e legenda

Cada fixture mostra no plano o ícone do que ele é: moving head, scanner, wash ou PAR, barra de LED, strobo, blinder, dimmer ou convencional, laser, fumaça, hazer, jato de CO2, chama, faíscas frias, confete, bolhas, neve, ventilador, efeito ou outro. Quem diz é o tipo do fixture, a não ser que o nome do modelo diga mais: um “Effect” chamado “CO2 Jet” é um jato de CO2 e um dimmer chamado “Blinder” é um blinder, mas um moving head chamado “Beam” ou “Wash” continua sendo um moving head. Um dimmer genérico, que pode estar no lugar de qualquer fixture, recebe o ícone do seu próprio nome (“CO2 1”, “FLAME 3”, “BLINDER1”). As barras de LED e as matrizes mostram as suas células; os outros fixtures com várias células (um blinder de várias lâmpadas, um strobo de vários segmentos) têm o ícone ao lado das suas células.

Para escolhê-lo à mão, selecione os fixtures e use **“Aparência” › “Ícone”**; **“Automático”** volta ao dele. O ícone escolhido é salvo com o show.

Cada tipo de fixture do show tem também a sua cor, no ícone e na borda, a mesma toda vez que você abre o show: dois tipos com o mesmo ícone se distinguem pela cor. Um ícone escolhido à mão em um fixture da biblioteca não muda a sua cor. Ao vivo, os fixtures acesos mostram a sua luz e os apagados, o seu ícone.

A **legenda**, no canto superior esquerdo do plano, lista os tipos de fixture com o seu ícone, a sua cor e quantos são (os dimmers genéricos, pelo que representam). Um clique em um tipo seleciona todos os fixtures dele, também no plano de Programar; **Ctrl+clique** ou **Shift+clique** os adiciona à seleção. Um clique no título dela a recolhe ou a expande, e isso é lembrado. O palco fica ao lado ou abaixo da legenda, para que ela não cubra nenhum fixture.

### Isolar um grupo no plano

Na vista “Plano 2D”, um clique em um grupo do painel “Grupos de fixtures” o **isola**: os outros fixtures ficam bloqueados e esmaecidos no plano, e nada os seleciona nem os move: nem um clique, nem o retângulo, nem o laço, nem a legenda, nem “Mover” (um clique em um deles não faz nada, e o retângulo pode começar em cima dele). Assim fica fácil selecionar parte de um grupo em um plano cheio de fixtures e fazer grupos menores com ela. Da seleção ficam só os fixtures do grupo que já estavam selecionados. O grupo isolado tem uma borda grossa e um olho, e ao lado da legenda do plano uma etiqueta diz qual grupo você vê e quantos fixtures ele tem.

- **“Todos”** seleciona todos os fixtures do grupo, assim como **Ctrl+clique** no grupo. **Ctrl+clique** em outro grupo adiciona só os fixtures dele que também estão no isolado.
- Na legenda, um clique em um tipo seleciona só os fixtures dele no grupo; os tipos que não têm nenhum aparecem esmaecidos.
- Um clique em outro grupo isola esse no lugar; um padrão de seleção do menu dele isola antes esse grupo. Um clique em uma seleção sempre seleciona todos os fixtures dela: se algum estiver bloqueado, antes isola o grupo da seleção ou, se ela tiver fixtures de outros grupos, termina o isolamento.
- Depois de **“Criar grupo”**, o grupo continua isolado: você pode continuar dividindo-o em mais grupos.
- Para terminar: outro clique no grupo, o **×** da etiqueta ou **Esc**. Também termina ao passar para “Lista” ou “Canais”, ao abrir outro show ou ao excluir o grupo. Não é salvo com o show.

Nas vistas “Lista” e “Canais”, um clique em um grupo continua selecionando os fixtures dele, como sempre.

### Altura e orientação (Follow)

Para que o [Follow](programar.md#follow-apontar-para-um-ponto) aponte bem todos os moving heads para um ponto, cada um precisa saber onde está. Posicione os moving heads no plano onde eles estão de verdade, selecione-os e pressione **“Altura e orientação”**. O que você mudar é aplicado na hora a todos os selecionados e salvo com o show:

| Campo | O que é |
|---|---|
| **Altura** | A quantos metros do chão do palco está o eixo da cabeça (sem ajustar: 4 m pendurada, 0 m em pé). |
| **Montagem** | **“Pendurada”** (de uma vara ou um truss, de cabeça para baixo) ou **“Em pé”** (no chão). |
| **Orientação** | Para onde vai o feixe com o pan no centro e o tilt inclinado para a frente. 0° é para o público e gira no sentido horário; é o mesmo giro dos botões 15° e 90°, e a seta do plano o mostra. |
| **Curso do pan** / **do tilt** | Os graus que cada eixo percorre. “A da definição” usa os da definição do fixture; se ela não os traz, são usados 540° e 270°, e há um aviso em laranja. |
| **Pan invertido** / **Tilt invertido** | Se com o Follow uma cabeça gira ou se inclina para o lado contrário. Só afeta o Follow; as inversões do Patch estão em [Limites](patch.md#limites). |

“(diferentes)” indica que os moving heads selecionados não têm o mesmo valor; ao alterá-lo, todos recebem o novo.

## Limites

Selecione fixtures e pressione **“Limites”**:

- **Dimmer**, **Pan** e **Tilt**: a faixa permitida, de 0 a 100 %. Por exemplo, um dimmer que não passe de 80 % ou um tilt que não aponte para o público.
- **“Inverter pan”**, **“Inverter tilt”** e **“Trocar pan e tilt”** (para moving heads pendurados ao contrário ou girados). É uma correção física: faz esses moving heads se moverem como os outros, e o espelho de um FX não a leva em conta (consulte [O espelho](programar.md#o-espelho)). Com vários fixtures dos quais só alguns estão invertidos, aparece desligado com “(só alguns)”, como o botão da barra, e ao ativá-lo todos se invertem.
- **“Redefinir”**: sem limites nem eixos invertidos ou trocados.
- **“Proteger do grand master”**: o grand master não baixa nenhum dos canais dele, atue só sobre a intensidade ou sobre todos: a luz da plateia, um servidor de mídia, um hazer. Os masters de grupo, os dimmers de cena, o Blackout e o [fade para preto](directo.md#fade-para-preto) ainda o baixam. Com vários fixtures dos quais só alguns estão protegidos, aparece desligado com “(só alguns)”, e ao ligá-lo todos ficam protegidos. Também está no [menu do fixture](patch.md#selecionar), em **Ajustes**, como interruptor: ligado quando todos os selecionados estão protegidos e pela metade, com quantos (3/4), quando só alguns estão; ao clicar nele pela metade, todos são protegidos. Na lista, uma **âncora** depois do nome (“Protegido do grand master: não desce com ele”); no plano 2D, do Patch e de Programar, uma âncora no canto inferior esquerdo (numa barra, à direita dela). “Redefinir” não tira a proteção. Ela é salva com o show, em cada fixture, e [importar o patch de outro show](patch.md#importar-de-outro-show) a traz. O QLC+ não a lê: lá o grand master baixa o fixture.

Com um só fixture, a lista **Canais** permite decidir para cada canal:

- **Fades**: se ele segue os tempos de fade. Desligado, salta direto para o valor (útil em rodas de cor e de gobos).
- **Dimmer**: se o grand master, os masters de grupo e os dimmers de cena o escalam. O grand master, além disso, só escala a luz (veja [Masters](directo.md#masters)). Se o fixture está protegido do grand master, só os masters de grupo e os dimmers de cena o escalam.

Os limites atuam na saída: cenas, efeitos e edição ao vivo mantêm seus valores, mas o que sai os respeita. Em um pan ou tilt de 16 bits, o limite vai sobre os dois canais juntos, então um movimento lento continua suave. Com vários fixtures aparecem os valores do primeiro que tem cada controle, e o que você mudar é aplicado a todos os que o têm.

## Efeitos especiais protegidos

Um **efeito especial protegido** só dispara com os efeitos armados, a partir de cenas, cuelists e chasers, e cada disparo dura no máximo o seu tempo máximo. Como armá-los e o que os dispara está em [Ao vivo](directo.md#efeitos-especiais-protegidos).

**Quais são.** As máquinas de **CO2**, **chamas**, **faíscas frias** e **confete** são protegidas sozinhas; as de **bolhas** e **neve**, não (proteja-as manualmente se quiser). A classe vem do tipo de fixture, das palavras da definição dele, do nome de um dimmer genérico (“CO2 1”, “FLAME 3”, “FOGO 2”, “PIRO 1”) ou do ícone escolhido no plano. As regras automáticas só protegem, nunca tiram a proteção: mudar o nome ou o ícone nunca desprotege um fixture; um fixture que passa a ser de uma dessas classes é protegido, e um aviso diz isso (“CO2 4 agora é um efeito especial protegido”); um CO2 que você desprotegeu manualmente continua desprotegido mesmo que o chame de “CO2 9”.

> **Limites:** “C02” escrito com um zero não é reconhecido, e “FOCO2” não é um CO2. Uma máquina que não é reconhecida não está protegida: confira o escudo na lista e proteja-a manualmente.

**“Efeito especial”** (o botão da chama, ao lado de “Limites”, ou “Efeito especial…” no menu do fixture) abre os ajustes para a seleção:

- **“Efeito especial protegido”**: desligá-lo pergunta antes (“Tirar a proteção?”). Sem ela, o fixture sai como qualquer outro canal: sem armar, sem tempo máximo e também a partir do programador, de Identificar, do mixer e dos FX.
- **“Tempo máximo de disparo”**: de 0,1 a 30 s (por padrão: CO2 3 s, chamas 2 s, faíscas 5 s, confete 2 s, os outros 3 s).
- **“Espera mínima entre disparos”**: de 0 a 60 s (1 s por padrão), desde que se solta.
- **“Canal de segurança”**: **“Nenhum”** ou o canal do fixture de que a máquina precisa para disparar, com os seus **“Valores de / até”** (de 0 a 255; “de” não pode ser maior que “até”). Esse canal só sai dentro desses valores enquanto o efeito está armado e disparando (“de”, se a cena não lhe dá valor); senão, 0. Só pode ser escolhido quando os fixtures selecionados têm os mesmos canais, e um fixture de um só canal não pode ter um (“… precisa de outro canal para poder disparar”). Se o fixture muda de modo ou de definição e fica outro canal no lugar dele, o canal de segurança é tirado, e um aviso diz isso (“O canal de segurança de … foi tirado”).
- **“Conforme a sua classe (…)”**: volta à proteção e aos tempos da sua classe, sem canal de segurança.

**Quais canais disparam.** Todos menos o pan, o tilt e o de segurança, seja qual for o tipo (intensidade, efeito, altura…): a eles se aplica o tempo máximo. O pan e o tilt só saem com os efeitos armados e a partir de cenas, sem limite de tempo, e em 0 desarmados: uma cena que só aponta a máquina não a dispara. Uma máquina que só tem pan e tilt dispara com todos os canais menos o de segurança. Um fixture entre dois universos (de um show do QLC+) nunca dispara: a janela avisa; mova-o para um só universo.

Na lista, um **escudo** depois do nome; no plano, um escudo no canto superior esquerdo: fraco normalmente, vermelho com os efeitos armados e com um anel que pulsa enquanto dispara. **Identificar** nunca os acende. [Níveis DMX](salidas.md#níveis-dmx) e o plano ao vivo mostram o que sai de verdade: um CO2 desarmado aparece em 0 mesmo com a cena dele tocando.

A proteção é salva com o show, na seção do DMX Total Control, e [importar o patch de outro show](patch.md#importar-de-outro-show) a traz. Ao abrir um show anterior ou do QLC+, um aviso diz uma vez quais fixtures agora são efeitos protegidos pela classe (“2 fixtures deste show agora são efeitos especiais protegidos: …”); também uma máquina de fumaça cujo modelo diz “CO2” ou um dimmer genérico chamado “Fuego…” ou “Fire…”. Para mudar, use “Efeito especial”. O QLC+ não lê essa proteção: aberto no QLC+, o show dispara esses fixtures de qualquer lugar, sem armar e sem tempo máximo. As versões anteriores do DMX Total Control também não a leem: disparam esses fixtures sem armar e, ao salvar o show, tiram a proteção feita à mão, os tempos e o canal de segurança. Use esta versão em todos os computadores do show.

## Estacionar canais

Um canal **estacionado** sai sempre com um valor fixo, acima das cenas, das cuelists, dos FX, do programador, do mixer (Full, Flash…), do grand master e de Parar tudo: o hazer a 30 % a noite toda, uma cabeça quebrada apagada e parada, a luz da plateia. Esc e Blackout nunca o desestacionam.

No [menu do fixture](patch.md#selecionar) (botão direito na lista ou no plano), em **Estacionar**, que à direita diz quantos da seleção estão estacionados:

- **“Estacionar como está agora”**: todos os canais no que sai agora, então o grand master, o mixer e os limites desse momento ficam incluídos. Não é possível durante a parada de show nem durante um fade para preto (a luz deles ficaria estacionada para sempre).
- **“Estacionar apagado”**: os canais de intensidade (dimmers e cores) em 0 e os outros como estão agora; um fixture sem canais de intensidade (uma máquina de fumaça ou um hazer da biblioteca), tudo em 0. Durante a parada de show ou um fade para preto, os outros canais são estacionados no que o show envia por baixo.
- **“Também em blackout”** (um interruptor, com algum fixture estacionado na seleção): ligado, com o Blackout as intensidades estacionadas mantêm o valor. Desligado, apagam-se com o Blackout; o pan, o tilt, o gobo e os outros canais que não são luz continuam sempre estacionados.
- **“Desestacionar”** (com algum fixture estacionado na seleção) e, em **Todo o show**, **“Desestacionar tudo”**: todos os estacionados do show, não só os selecionados (como o botão do cabeçalho do Patch).

Em [Programar](programar.md#estacionar), **“Estacionar”** estaciona os valores que o programador tem nos fixtures selecionados.

Com fixtures estacionados, o cabeçalho do Patch mostra **“N estacionados · Desestacionar tudo”** (em uma janela estreita, o cadeado, o número e o botão só com o ícone). Na lista, um **cadeado** depois do nome (“Estacionado: N canais com um valor fixo na saída”); no plano 2D, do Patch e de Programar, um cadeado no canto superior direito. Em [Níveis DMX](salidas.md#níveis-dmx), um cadeado em cada canal estacionado.

- Um efeito especial protegido só pode ser estacionado **apagado**: “Estacionar como está agora” ou “Estacionar” de Programar o estacionam em 0 e dizem isso (“Um efeito especial protegido só pode ser estacionado apagado”). Estacionado, nunca dispara, mesmo armado; um disparo cortado por estacioná-lo aparece no registro como “estacionado”. Um fixture estacionado aceso que passa a ser protegido é estacionado apagado; se depois for desprotegido, continua estacionado em 0 até você desestacioná-lo. Desestacionado, só dispara depois de soltar a cena dele e pressioná-la de novo.
- Os canais estacionados seguem o fixture se você o reendereçar. Mudar o modo de um fixture remove os canais estacionados dele (“Os canais estacionados de Spot 3 foram retirados: o modo mudou”), menos em um efeito protegido, que continua estacionado apagado. Excluir um fixture remove os dele. Um pan estacionado de um fixture com pan e tilt trocados sai pelo canal do tilt, como o pan.
- A [parada de show](directo.md#parada-de-show) ganha dos canais estacionados: um fixture estacionado apagado também se ilumina, e um hazer estacionado continua estacionado. Um [fade para preto](directo.md#fade-para-preto) desce os estacionados sem “Também em blackout”.

Os canais estacionados são salvos com o show, na seção do DMX Total Control (importar o patch de outro show não os traz). Ao abrir um show com canais estacionados, eles se aplicam na hora, e um aviso diz quais (“2 fixtures estacionados: HAZER, Spot 3 (Patch)”). Abrir outro show ou um novo os remove. Ao fechar o programa eles são soltos (o palco fica em repouso), mas continuam no show. O QLC+ não estaciona canais: lá eles tocam como as cenas disserem, e a cópia para o QLC+ avisa (“2 fixtures estacionados: o QLC+ não estaciona canais”). As versões anteriores do DMX Total Control não os aplicam e, ao salvar o show, os tiram, assim como os ajustes da parada de show.

> **Não testado com equipamento real:** estacionar não foi testado com interfaces, nós nem hazers reais.

## Matrizes e fitas de LED

**“Matriz de LED”** patcheia de uma vez uma matriz ou uma fita de pixels:

1. **Nome**, **Largura (pixels)** e **Altura (pixels)** (altura 1 para uma fita).
2. **Cores**: a ordem de cada pixel (RGB, GRB, BGR, BRG, GBR, RBG ou RGBW).
3. **Cabeamento**: “Linhas”, “Linhas, em zigue-zague”, “Colunas” ou “Colunas, em zigue-zague”.
4. **Pixel 1**: o canto onde ela começa (“Canto superior esquerdo” etc.).
5. **Universo** e **Endereço**.
6. Confira o resumo e a prévia da **ordem dos pixels**, e pressione **“Patchear”**.

É criado um fixture por linha de pixels, um grupo para os efeitos de cor e o lugar deles no plano. Se uma linha precisar de mais de um universo, deixe-a mais curta ou faça o cabeamento no outro sentido.

## Importar de outro show

**“Importar”** traz para este show parte de outro (do DMX Total Control ou do QLC+):

- **“Patch de outro show…”**: os fixtures dele. Vão para os mesmos endereços se estiverem livres; se não, para os seguintes. Eles trazem a proteção de efeito especial, os tempos, o canal de segurança e o ícone do plano (o que o outro show diz nunca desprotege um CO2), mas não os canais estacionados.
- **“Cenas de outro show…”**: as cenas dele, atribuídas aos fixtures deste show que coincidem (por endereço e modelo, por nome ou por modelo). Entram em grupos com o mesmo nome, com as suas opções no grupo (tamanho, fase e velocidade ao vivo, liberação, flash, dimmer…). Um grupo novo traz também o seu crossfade e o seu master; um que já existe neste show mantém os seus.
- **“Patch e cenas de outro show…”**: as duas coisas.
- **“Patch de um programa de visualização (CSV)…”**: a tabela de fixtures que o seu programa de visualização exporta; veja [Importar um patch de um programa de visualização](patch.md#importar-um-patch-de-um-programa-de-visualização).

## Importar um patch de um programa de visualização

Se você desenha a montagem num programa de visualização, exporte a tabela de fixtures dele (CSV) e importe-a: o DMX Total Control patcheia os mesmos fixtures, nos mesmos universos e endereços, com os seus nomes, números e lugar na planta.

1. Clique em **“Importar” › “Patch de um programa de visualização (CSV)…”** e escolha o arquivo.
2. A janela **“Importar um patch”** diz quantos fixtures a tabela traz, de quantos tipos e em quais universos. Nada muda até você clicar em **“Importar”**.
3. Em **“Tipos de fixture”**, cada tipo da tabela (fixture, modo e canais) diz no que se transforma e como foi encontrado na biblioteca: **“Exato”** (o mesmo modelo e modo), **“Pelo número de canais”** (o modo com esses canais), **“Nome parecido”**, **“Lembrado”** ou **“Escolhido à mão”**. Um **“Nome parecido”** aparece em laranja para você conferir: é um modelo com um nome parecido, não o mesmo. Se não está na biblioteca, é patcheado como **“Dimmer genérico”** com os seus canais, em laranja: tudo fica nos seus endereços do mesmo jeito. Cada tipo mostra também o ícone que os seus fixtures terão no plano; se for patcheado como dimmer genérico, recebe o que o nome dele na tabela diz (um “CO2 Jet” aparece como jato de CO2; um “Flame”, como chama).
4. **“Alterar”** escolhe outro fixture e modo (**“Escolher da biblioteca…”**), um dimmer genérico ou **“Não importar”**. O que você escolhe à mão é lembrado na próxima vez que importar esse tipo.
5. Em **“Opções”**:
   - **“Colocá-los na planta do palco”**, com a **Vista**: **“De cima”** (o público embaixo da planta) ou **“De frente”** (como o público vê a montagem: os fixtures mais altos no alto). A vista de frente é proposta quando a montagem está pendurada em alturas diferentes, e a última que você usou é lembrada. A planta cresce para caber a montagem (nunca encolhe). Cada importação que os coloca centraliza de novo a montagem na planta, também depois de mudar os lados dela com “−” e “+”.
   - **Grupos**: um para cada tipo de fixture, um para cada camada (se a tabela tiver mais de uma) e os da coluna “Groups” da tabela.
6. Embaixo dos tipos ficam os problemas, antes de importar: endereços que outro fixture ou outra linha já usa (essas linhas não são importadas), linhas sem universo ou endereço, universos acima de 128, endereços que passam de 512, um segundo DMX break (não é importado), números de fixture que outro fixture já tem ou altos demais (recebem outro), um pan ou tilt invertido que não é nem sim nem não, e os universos adicionados ao show.
7. Clique em **“Importar N fixtures”**. Um aviso diz quantos são novos e quantos foram atualizados, e se ficou algum problema.

**De cada linha** são importados o fixture e o seu modo, o universo e o endereço, o nome (a coluna “Unit”) e o número de fixture: o da coluna “Channel”, senão o da outra coluna de número de fixture, senão um automático. A altura de cada cabeça móvel, se está em pé (virada ao contrário na tabela) ou pendurada, e o seu pan ou tilt invertido vão para a sua [altura e orientação](patch.md#altura-e-orientação-follow), colocando-os na planta ou não.

**Importar de novo** a tabela (depois de mudar a montagem no programa de visualização) atualiza os mesmos fixtures: ele os reconhece pelo identificador de cada linha da tabela, ou pelo mesmo nome e fixture, e muda o modo, o endereço, o nome, o número e o lugar na planta. Os que não estão mais na tabela ficam, a não ser que você marque **“Remover os N fixtures importados antes que não estão mais na tabela”** (embaixo aparecem os nomes deles). Um ícone que você escolher à mão depois é mantido, a não ser que mude o fixture com que o tipo dele é patcheado.

> **Limites:** de cada fixture só se importa o primeiro DMX break, e das rotações só se usa se a cabeça está virada ao contrário (a orientação não é importada). Vistos de cima, os fixtures pendurados um sobre o outro ficam juntos na planta: para vê-los separados, use a vista de frente.

## Excluir fixtures

Selecione-os e pressione **“Excluir”**. O programa pergunta antes: os valores deles são removidos de todas as cenas.
