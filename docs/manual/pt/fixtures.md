# Fixtures

![A página Fixtures](../img/fixtures.png)

Para patchear um aparelho, o programa precisa da **definição** dele: quais canais ele tem, o que cada um faz e em quais modos ele trabalha. Esta página explica de onde vêm as definições e como criar as suas na página **Fixtures** (F6).

## A biblioteca de fixtures

Ao adicionar fixtures no [Patch](patch.md#adicionar-fixtures) você escolhe entre todas estas definições:

- **A biblioteca incluída**: milhares de fixtures de muitos fabricantes, no formato `.qxf`.
- **Os genéricos**: 16 fixtures básicos no fabricante **Generic** (PAR LED RGB, RGBA, RGBW, RGBWA, RGBWA+UV e branco, barra de LED de 8 segmentos, moving heads spot, beam e wash, scanner, strobo, blinder, laser, máquina de fumaça e hazer), além do **Generic Dimmer**.
- **A Open Fixture Library**, se você baixá-la.
- **Os fixtures GDTF** que você importar.
- **Seus fixtures**: os que você cria nesta página ou importa.

Em [Opções › Biblioteca de fixtures](opciones.md#biblioteca-de-fixtures) aparece quantas definições estão disponíveis e está tudo o que é preciso para ampliá-la.

> **Dica:** para que Programar, os FX e as paletas reconheçam a função de cada canal (dimmer, vermelho, pan, gobo…), a definição tem que trazer o grupo ou o preset do canal. As da biblioteca trazem; nas suas, escolha bem o preset de cada canal.

## Criar um fixture

A página **Fixtures** é o criador de fixtures. À esquerda, **“Meus fixtures”** lista os que você criou; à direita, o editor.

1. Pressione **“Novo fixture”**, ou **“Partir da biblioteca…”** para começar a partir de uma cópia de um que já existe (ele recebe o mesmo nome com “(cópia)”).
2. Em **Fixture**, digite o **Fabricante** (por exemplo `Generic` ou a marca), o **Modelo** (`PAR 18x10W RGBWA`) e escolha o **Tipo**.
3. Adicione os **canais** (veja abaixo).
4. Prepare os **modos**.
5. Pressione **“Salvar fixture”**.

O fixture fica disponível na hora no Patch, dentro do seu fabricante. No topo aparece o estado dele: “Alterações não salvas”, “Salvo nos seus fixtures” ou “Ainda não salvo”. Se você abrir outro fixture com alterações não salvas, o programa pergunta se quer descartá-las (“Continuar editando” / “Descartar alterações”).

### Canais

Em **Canais**, o botão **“Adicionar”** oferece os canais mais comuns (dimmer, vermelho, verde, azul, branco, âmbar, UV, pan e pan fino, tilt e tilt fino, strobo, roda de cores, roda de gobos, velocidade de pan/tilt) e **“Canal personalizado”**.

Em cada canal:

- O **nome** e o **preset** dele (a lista suspensa): o preset diz o que o canal faz. Mudar de preset muda também o nome, a não ser que você tenha dado um nome próprio a ele.
- A lixeira o exclui.

Um canal **“Personalizado”** tem ainda:

- **Grupo**: a função geral dele (intensidade, cor, gobo, feixe…).
- **Cor**: em um canal de intensidade, qual cor ele é (vermelho, verde…) ou “Nenhum” para um dimmer.
- **Fino**: é o canal fino (16 bits) de outro.
- **“Faixas (N)”**: o que cada faixa de valores DMX faz, por exemplo 0-7 Fechado, 8-255 Strobo. Pressione **“Adicionar faixa”** e digite a **Descrição** dela. As faixas não podem se sobrepor. Elas são os presets que depois você verá como botões em Programar.

Os canais novos são adicionados no final do modo que estiver aberto.

### Modos

Em **Modos**, **“Adicionar”** cria um modo com todos os canais. Cada aba é um modo:

- **Nome** do modo e a lixeira para excluí-lo (sempre fica um).
- A lista é a **ordem DMX** do modo: marque os canais que ele usa e mova-os com **▲ ▼** até a posição deles. O número da esquerda é o canal DMX dele dentro do modo.

### Excluir um fixture seu

Abra o fixture em “Meus fixtures” e pressione **“Excluir”**. Ele some da biblioteca na próxima inicialização.

Seus fixtures são salvos como arquivos `.qxf` na pasta de fixtures do usuário (em Opções › “Pasta de fixtures do usuário” › “Abrir pasta”).

## Open Fixture Library

A Open Fixture Library é uma biblioteca aberta com milhares de fixtures adicionais (licença MIT).

1. Abra **Opções** (F9).
2. Em **Biblioteca de fixtures › Open Fixture Library**, pressione **“Baixar”**. É preciso internet.
3. Espere a barra de progresso terminar (“Cancelar” a interrompe).

Ao terminar, o programa diz quantos fixtures foram baixados e quantos são novos. Depois, no mesmo lugar aparece quantos ela tem e quando foi baixada.

Para atualizá-la, pressione **“Atualizar”** (o mesmo botão). Se em Opções estiver ativado “Procurar atualizações ao iniciar”, o programa verifica uma vez por dia no GitHub se a biblioteca tem fixtures novos ou melhorados desde o seu download, avisa você e destaca o botão; “Procurar atualizações” verifica na hora. A atualização:

- adiciona os fixtures novos (eles são listados abaixo);
- altera os que melhoraram, inclusive os que já estão patcheados (se o novo modo deles não couber onde estão, eles ficam com a versão anterior até você patcheá-los de novo);
- mantém os que a biblioteca não tem mais, caso um show os use;
- não mexe nos seus fixtures nem nos da biblioteca incluída: se tiverem o mesmo fabricante e modelo, fica o seu (ou o incluído).

Um download que não pode ser lido, ou que é interrompido, não muda nada.

## Importar GDTF

GDTF é o formato aberto que os fabricantes publicam no GDTF-Share (com uma conta gratuita).

1. Baixe os `.gdtf` dos seus aparelhos (ou o `.zip` do download múltiplo do GDTF-Share). O botão **“GDTF-Share”** de Opções abre o site.
2. Em **Opções › Biblioteca de fixtures › Fixtures GDTF**, pressione **“Importar GDTF…”** e escolha os arquivos, as pastas ou o `.zip`.
3. Eles são convertidos em segundo plano, com uma barra de progresso. **“Parar”** interrompe a importação.

Ao terminar, o programa indica quantos foram convertidos e, se algum não pôde ser lido, por quê. Se o mesmo fixture já estiver em outra biblioteca, o do GDTF é adicionado como “(GDTF)” e você pode usar os dois. Importá-lo de novo o atualiza.

De cada modo é usado o primeiro *DMX break*; os canais virtuais (sem DMX) são omitidos.

## Importar arquivos de fixtures

**Opções › Importar arquivos de fixtures › “Importar…”** aceita definições `.qxf` (o formato do QLC+), arquivos `.gdtf` ou um `.zip` que os contenha. Eles são adicionados à biblioteca e o programa indica quantos são novos.

### Importar meus fixtures do QLC+

Se você criou fixtures no QLC+ neste computador, **Opções › “Importar meus fixtures do QLC+” › “Importar”** os copia para os seus fixtures.

## Formatos que não são importados

As bibliotecas de outros programas em formatos fechados ou criptografados não podem ser importadas. A maioria dos fixtures está na Open Fixture Library ou no GDTF-Share; os que faltarem, crie-os nesta página.
