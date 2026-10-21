# Saídas

![A página Saídas](../img/salidas.png)

**Saídas** (F7) conecta cada **universo** do show (512 canais DMX) a uma **interface**: uma interface DMX USB, um nó Art-Net ou uma saída sACN. Ela também tem o monitor de [níveis DMX](salidas.md#níveis-dmx), que mostra ao vivo o que sai.

No cabeçalho:

- **“Universos”** / **“Níveis DMX”**: as duas vistas da página.
- **“Buscar novamente”**: busca de novo as interfaces conectadas.
- **“Remover universo”** e **“Adicionar universo”**.

## Conectar um universo

1. Conecte a interface ao computador (ou à rede, para Art-Net e sACN).
2. Se ela não aparecer em **“Saídas detectadas”**, pressione **“Buscar novamente”**.
3. Na linha do universo, abra a lista suspensa (diz “Não conectado”) e escolha a saída.

Cada saída aparece com o nome do seu tipo e o da interface ou da rede: “DMX USB — …”, “ArtNet — …”, “E1.31 — …” (E1.31 é sACN). O universo conectado fica marcado em verde. Para desconectá-lo, escolha “Não conectado”.

A conexão é lembrada para a próxima vez que você abrir o programa.

> **Dica:** as interfaces DMX USB baseadas em chips FTDI precisam dos drivers delas. Se você não os tiver, o instalador do programa oferece, ao terminar, a página para baixá-los.

## Universos

- **“Adicionar universo”** adiciona um no final. Ao patchear, você escolhe em qual universo vai cada fixture.
- Clique no **nome** de um universo para alterá-lo (o nome é salvo com o show).
- **“Remover universo”** remove o **último** (sempre fica um). Se ele tiver fixtures patcheados, não é possível: mova-os para outro universo ou exclua-os antes no Patch. Se for a entrada de um controlador, o programa pergunta antes, porque esse controlador vai parar de funcionar até você ativá-lo de novo em Mapeamentos.

## Níveis DMX

**“Níveis DMX”** mostra os 512 canais de um universo tal como saem do programa, com tudo aplicado (cenas, programador, mixer, grand master, limites…):

- Escolha o **Universo** na lista suspensa.
- Cada casa tem o número do canal e o valor dele (0–255). **“Mostrar em %”** muda para porcentagem.
- As casas com borda pertencem a um fixture. Passe o mouse sobre um canal para ver de qual fixture ele é (“Nome — canal N”).

> **Dica:** se um fixture não responde, veja aqui se o canal dele se move. Se ele se move, o problema está no cabo, no endereço do aparelho ou na interface; se não, no patch ou no que você está disparando.

O mesmo monitor pode ser uma seção do [SHOW](show.md) (“Níveis DMX”).

## Entrada DMX

Uma interface com entrada DMX (ou uma mesa conectada a ela) serve para controlar o programa: ela é ativada como dispositivo em [Mapeamentos](mapeos.md#dispositivos).
