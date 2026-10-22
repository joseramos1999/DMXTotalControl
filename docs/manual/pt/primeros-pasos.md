# Primeiros passos

![O guia de primeiros passos na página Patch](../img/primeros-pasos.png)

## O guia

Na primeira vez que você abre o programa aparece o **guia de primeiros passos**: um cartão no canto inferior direito com cinco passos (seus fixtures, um visual, uma cena, Ao vivo e os playbacks). **“Próximo”** e **“Voltar”** mudam de passo e levam você à página de cada um; as páginas podem ser usadas enquanto o cartão está aberto. Você o fecha com o **✕** e o abre de novo em Opções › Ajuda › **“Iniciar o guia”**.

No primeiro passo, **“Abrir o show de prática”** abre o exemplo “Primeros pasos” (se você tiver alterações não salvas, antes ele pergunta se quer salvá-las). Ele já tem quatro PAR LED RGBW no grupo de fixtures `PARs`, dois moving heads em `Móviles` e o grupo “Mis escenas” vazio na página Ao vivo, pronto para as suas cenas.

## A sessão completa

Nesta primeira sessão você vai fazer o caminho completo: conectar a saída, patchear alguns fixtures, montar um visual no programador, gravá-lo como cena em um grupo, dispará-lo na página Ao vivo, gravar uma cuelist em um playback e salvar o show. Isso leva uns vinte minutos.

Usaremos fixtures da biblioteca genérica: quatro PAR LED RGBW e dois moving heads. Se você tiver outros aparelhos, procure o fabricante e o modelo deles no lugar dos genéricos.

> **Dica:** você pode seguir este guia sem interface DMX conectada. O que sai aparece em Saídas › “Níveis DMX” e no plano 2D de Programar.

> **Dica:** com o show de prática aberto você já tem os fixtures e o grupo: pule para o passo 4 e use o grupo de fixtures `Móviles` onde aqui aparece `Cabeças`.

## 1. Conecte a saída

1. Conecte sua interface DMX USB ao computador (ou verifique se o computador está na mesma rede que o seu nó Art-Net ou sACN).
2. Abra **Saídas** (F7).
3. Na linha do universo 1, abra a lista suspensa que diz “Não conectado” e escolha sua interface.
4. Se ela não aparecer, pressione **“Buscar novamente”**.

O universo conectado fica marcado em verde. Você encontra mais detalhes em [Saídas](salidas.md).

## 2. Patcheie alguns fixtures

1. Abra **Patch** (F5) e pressione **“Adicionar fixtures”**.
2. Em **Fabricante**, escolha **Generic** (você pode digitar em “Buscar fabricante…”).
3. Em **Modelo**, escolha **LED PAR RGBW**.
4. No painel **Patch** à direita:
   - **Modo**: “5ch DRGBW”.
   - **Quantidade**: 4.
   - **Universo**: 1. O **Endereço** é definido sozinho no primeiro livre.
   - Deixe marcado **“Criar um grupo com eles”** e digite em **Grupo** o nome `PARs`.
5. Pressione **“Patchear 4 fixtures”**. Embaixo você verá quais endereços eles usam.
6. Sem fechar a janela, escolha agora **Moving Head Spot**, modo “11ch Standard”, quantidade 2, grupo `Cabeças`, e pressione **“Patchear 2 fixtures”**.
7. Pressione **“Fechar”**.

> **Dica:** configure em cada aparelho o mesmo endereço DMX que a lista do Patch mostra. Para encontrar um aparelho no palco, selecione-o e pressione **“Identificar”**: ele pisca em branco.

## 3. Crie um grupo na página Ao vivo

As cenas são disparadas na página Ao vivo, organizadas em grupos. Crie um antes de gravar:

1. Abra **Ao vivo** (F1) e pressione **“Novo grupo”**.
2. Clique no nome da coluna (“Grupo 1”), apague-o, digite `Mis escenas` e pressione Enter.
3. Na lista suspensa de tempo de crossfade da coluna (diz “Por cena”), escolha **2 s**.

O grupo é **“Exclusivo”**: só uma cena roda por vez e, ao pressionar outra, ele faz um crossfade.

## 4. Monte um visual no programador

1. Abra **Programar** (F3).
2. No painel de fixtures, clique na etiqueta do grupo **PARs**: os quatro são selecionados.
3. No painel **Rápido**, pressione **100%** ao lado do fader de **Dimmer**.
4. Clique na amostra de cor **vermelha**.

Os PAR acendem em vermelho assim que você toca: o que você define no programador sai na hora, por cima de tudo.

> **Dica:** o mesmo com a linha de comando: digite `GROUP 1 @ FULL` e pressione Enter. `GROUP 1` é o primeiro grupo de fixtures. Veja [a linha de comando](programar.md#linha-de-comando).

## 5. Grave a cena no grupo

1. Pressione **“Gravar…”** (no canto superior direito).
2. Em **Gravar em**, deixe **“Nova cena”**.
3. Em **Nome**, digite `Vermelho`.
4. Em **Adicionar ao grupo**, escolha **Mis escenas**.
5. Pressione **“Gravar”**.

Grave um segundo visual: com os PAR selecionados e o dimmer em 100 %, clique na amostra **azul**, pressione **“Gravar…”**, nome `Azul`, grupo **Mis escenas** e **“Gravar”**.

Antes de ir para a página Ao vivo, pressione **“Redefinir tudo”** para esvaziar o programador. Enquanto o programador tem valores, eles mandam sobre as cenas e você não veria o que dispara.

> **Dica:** na janela de gravar, marque **“Limpar o programador depois de gravar”** e o programador será esvaziado sozinho cada vez que você gravar. A opção fica memorizada.

## 6. Dispare as cenas na página Ao vivo

1. Abra **Ao vivo** (F1).
2. Pressione **Vermelho**: o cartão se ilumina com a cor do grupo e os PAR entram em vermelho.
3. Pressione **Azul**: o grupo faz um crossfade de 2 segundos de vermelho para azul.
4. Mova o **Master** da coluna para baixar a intensidade de todo o grupo.
5. Pressione **Azul** de novo para liberá-la, ou pressione o quadrado do cabeçalho da coluna (“Liberar este grupo”).

Mais em [Ao vivo](directo.md).

## 7. Grave uma cuelist em um playback

As cuelists avançam cue a cue com GO. A forma mais rápida de criar uma é gravar diretamente em um playback.

1. Em **Programar**, clique na etiqueta **Cabeças**.
2. Pressione **“Beam on”** (dimmer no máximo e shutter aberto) e **“Centralizar”** (pan e tilt no centro).
3. Abra **Playbacks** (F10) e pressione **“Gravar”**: o botão pisca em vermelho.
4. Clique no playback **1**, que está vazio: é criada uma nova cuelist com a cue 1.
5. Volte para **Programar**, abra **Features**, aba **Posição**, e arraste a cruz da grade XY para outro lugar.
6. Em **Playbacks**, pressione **“Gravar”** e clique de novo no playback 1: a cue 2 é gravada, só com o que mudou (a posição).
7. Volte para **Programar** e pressione **“Redefinir tudo”**.
8. Em **Playbacks**, clique no nome do playback 1 para selecioná-lo e pressione **Espaço** (ou o grande botão **GO**): a cue 1 roda. Pressione de novo: passa para a cue 2.

O fader do playback é a intensidade dele: se você o sobe a partir do zero, a cuelist inicia. Mais em [Playbacks](playbacks.md).

## 8. Salve o show

1. Pressione **Ctrl+S** (ou o disquete da barra superior).
2. Como é a primeira vez, escolha a pasta e o nome: ele é salvo como arquivo `.qxw`.

A partir daí, Ctrl+S salva no mesmo arquivo e mantém como `.bak` a versão que havia antes do primeiro salvamento de cada sessão. Com o salvamento automático ativo (veja [Opções](opciones.md#salvamento-automático)), o programa salva a cada poucos minutos uma cópia de recuperação à parte, sem mexer no seu arquivo: se ele fechar de forma inesperada, oferece recuperar as alterações.

## E agora…

- Monte sua tela de operação no [Show](show.md).
- Aprenda a usar [paletas](programar.md#paletas) e [FX](programar.md#fx) no programador.
- Edite as cues, os tempos e o tracking delas em [Cenas](escenas.md#cuelists).
- Conecte um controlador MIDI em [Mapeamentos](mapeos.md).
