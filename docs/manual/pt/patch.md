# Patch

![A página Patch](../img/patch.png)

**Patch** (F5) tem os fixtures da sua montagem: o que são, em qual universo e endereço estão, o número deles, os grupos de fixtures e onde estão no palco.

No cabeçalho você escolhe a vista e tem as ferramentas para adicionar:

- **“Lista”**: todos os fixtures por endereço.
- **“Canais”**: os 512 canais de cada universo, com os fixtures por cima.
- **“Plano 2D”**: o palco visto de cima.
- **“Importar”**, **“Matriz de LED”** e **“Adicionar fixtures”**.

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
4. Embaixo aparece quais endereços eles vão usar (“Usa os endereços 1 a 20”). Se não couberem no que resta do universo, eles passam para o seguinte.
5. Pressione **“Patchear fixture”** ou **“Patchear N fixtures”**.

A janela fica aberta para continuar adicionando, com o endereço já no próximo livre. Se algo não puder ser patcheado (os endereços estão ocupados, por exemplo), o motivo aparece em vermelho embaixo. Pressione **“Fechar”** ao terminar.

Para dimmers avulsos, escolha **Generic › Generic Dimmer**: em vez de modo, você informa quantos **Canais**.

> **Dica:** se não encontrar seu fixture, baixe a [Open Fixture Library](fixtures.md#open-fixture-library), importe o GDTF dele ou crie-o na página [Fixtures](fixtures.md).

## A lista

Colunas: caixa de seleção, **#** (número), **NOME**, **ENDEREÇO** (universo . endereço), **CANAIS**, **FIXTURE** (fabricante e modelo), **MODO**, **INVERTER** (pan e tilt) e o botão de identificar.

- **“Agrupar por fixture”** os agrupa por modelo, com uma caixa para selecionar todos os de cada modelo. Um clique no cabeçalho de um modelo o recolhe (oculta os fixtures dele) ou o expande.
- **Clique duplo** em um fixture o edita.

## Selecionar

As ferramentas da barra trabalham sobre a seleção:

- **Clique** seleciona; **Ctrl+clique** adiciona ou tira; **Shift+clique** seleciona todos os fixtures desde o último que você clicou até esse (nesse sentido, que é a ordem da seleção), e **Ctrl+Shift+clique** os adiciona. **“Todos”** e **“Nenhum”**.
- **“Identificar”**: os fixtures selecionados piscam em branco para que você os encontre no palco. **“Parar de identificar”** interrompe isso (também para ao sair da página). O botão de cada linha identifica só esse fixture. A lista fica onde estava.
- **Inverter pan** e **inverter tilt** (os ícones de setas): para os fixtures que os têm.
- **“Limites”**, **“Editar”**, **“Reendereçar”**, **“Renumerar”**, **“Criar grupo”** e **“Excluir”**: são explicados a seguir.

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

O painel **“Grupos de fixtures”** da esquerda os mostra com o número de fixtures de cada um. Um clique em um grupo seleciona os fixtures dele; **Ctrl+clique** ou **Shift+clique** os adiciona. Botão direito:

- **“Renomear”**.
- **“Substituir pela seleção”**: o grupo passa a ter os fixtures selecionados.
- **“Salvar a seleção aqui…”**: veja [Seleções](patch.md#seleções).
- Padrões de seleção, na ordem do grupo: **“Selecionar os ímpares”**, **“Selecionar os pares”**, **“Selecionar em ordem inversa”**, **“Selecionar a primeira metade”**, **“Selecionar a segunda metade”**, **“Selecionar das pontas para o centro”** e **“Selecionar do centro para as pontas”**.
- **“Excluir grupo”**.

### Seleções

Uma **seleção** é uma parte ordenada de um grupo: por exemplo, os spots de um grupo de seis tomados das pontas para o centro. A ordem importa, porque os leques e os FX a seguem.

1. Selecione os fixtures na ordem que quiser (ou com um padrão do menu do grupo).
2. Pressione o **+** do grupo (ou “Salvar a seleção aqui…”) e digite um nome.

As seleções aparecem sob o grupo delas. Um clique seleciona os fixtures dela na sua ordem, aqui e em Programar. Botão direito: **“Renomear”** e **“Excluir a seleção”**. A seta à esquerda do grupo oculta ou mostra as seleções dele.

## Plano 2D

O plano mostra o palco visto de cima, com **PÚBLICO** embaixo. Ele serve para selecionar em Programar, para ver o que sai e para os efeitos de cor posicionados no plano.

No cabeçalho do plano:

- **“Largura (m)”** e **“Profundidade (m)”**: o tamanho do palco.
- **“Organizar automaticamente”**: distribui todos os fixtures.

As ferramentas (trabalham sobre a seleção e na ordem dela):

- **“Ao vivo”**: mostra a cor, o dimmer, o strobo e o gobo de cada fixture, e cada pixel das barras e matrizes.
- **“Laço”**: seleciona desenhando ao redor dos fixtures em vez de com um retângulo.
- **“Selecionar”**: clique ou retângulo para selecionar.
- **“Mover”**: arraste os fixtures (a seleção inteira se move) ou a vista.
- **“Grade”**: “Livre”, 10 cm, 25 cm, 50 cm ou 1 m: onde os fixtures caem ao serem movidos.
- **“Aparência”**: como os fixtures selecionados aparecem: “Redondo”, “Quadrado” ou “Retângulo”.
- **“Organizar”**: posiciona-os em “Linha”, “Linha, da frente para o fundo”, “Círculo”, “Grade, por linhas”, “Grade, em zigue-zague”, “Grade, por colunas” ou “Grade, em zigue-zague por colunas”, com o **Espaçamento (cm)** e as **Colunas** ao lado.
- **“Alinhar”**: “Esquerda”, “Centro”, “Direita”, “Fundo”, “Meio”, “Frente”, “Distribuir na largura” e “Distribuir na profundidade”.
- **15°** (botão direito: para o outro lado) e **90°**: giram a seleção.
- **“Altura e orientação”**: onde estão pendurados os moving heads selecionados, para o [Follow](patch.md#altura-e-orientação-follow).

Os moving heads têm uma pequena seta na borda: para onde eles apontam. Os nomes dos fixtures muito próximos ficam ocultos para não se sobreporem: aproxime o zoom para lê-los.

A **roda** do mouse faz zoom, o **botão direito** ou o do meio arrastam a vista e um clique duplo com eles a centraliza. Os botões do canto também fazem zoom.

> **Dica:** posicione os fixtures na ordem em que você quer que os efeitos corram: selecione-os da esquerda para a direita e use “Organizar › Linha”.

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
- **“Inverter pan”**, **“Inverter tilt”** e **“Trocar pan e tilt”** (para moving heads pendurados ao contrário ou girados).
- **“Redefinir”**: sem limites nem eixos invertidos ou trocados.

Com um só fixture, a lista **Canais** permite decidir para cada canal:

- **Fades**: se ele segue os tempos de fade. Desligado, salta direto para o valor (útil em rodas de cor e de gobos).
- **Dimmer**: se o grand master, os masters de grupo e os dimmers de cena o escalam.

Os limites atuam na saída: cenas, efeitos e edição ao vivo mantêm seus valores, mas o que sai os respeita. Com vários fixtures aparecem os valores do primeiro que tem cada controle, e o que você mudar é aplicado a todos os que o têm.

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

- **“Patch de outro show…”**: os fixtures dele. Vão para os mesmos endereços se estiverem livres; se não, para os seguintes.
- **“Cenas de outro show…”**: as cenas dele, atribuídas aos fixtures deste show que coincidem (por endereço e modelo, por nome ou por modelo). Entram em grupos com o mesmo nome.
- **“Patch e cenas de outro show…”**: as duas coisas.

## Excluir fixtures

Selecione-os e pressione **“Excluir”**. O programa pergunta antes: os valores deles são removidos de todas as cenas.
