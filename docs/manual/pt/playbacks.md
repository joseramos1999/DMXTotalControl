# Playbacks

![A página Playbacks](../img/playbacks.png)

**Playbacks** (F10) é uma mesa de faders. Cada **playback** reproduz uma cuelist ou qualquer cena, e tem um **GO**, um botão **Pausa/Voltar**, um botão **Flash** e um **fader** que é a intensidade dele. Os playbacks são organizados em **páginas** de 8 a 20 (10 por padrão).

É a forma mais confortável de conduzir um show com cuelists: você grava as cues a partir do programador diretamente em um playback e as dispara com GO ou com a barra de espaço.

## A tira de um playback

De cima para baixo:

- **O número** sobre a cor do playback. Ele se ilumina enquanto roda. O **×** da direita o libera. Uma cuelist de outro tipo mostra a etiqueta dela: **SUB** (submaster), **CHASE**, **TC** (timecode) ou **OVR** (override).
- **O nome**: clique nele para **selecionar** o playback (borda laranja). Um playback sem nada mostra “Vazio”.
- **A cue que roda e a próxima** (“3 · Verso”, “▸ 4 · Ponte”) e uma barra com o avanço do fade. Se a próxima é o salto de um vínculo da cue, ela leva **↺** e o número do salto: “▸ 2 ↺ 1/3” é o primeiro de três (veja [Cuelists](escenas.md#cuelists)). Um chase mostra antes a velocidade dele: “♩ 120 ▸ 4”. Uma cuelist timecode mostra antes o relógio dela: “TC 00:01:23.4 · 3 · Verso”. Uma cuelist parada mostra “Liberada”; uma cena mostra o tipo dela e se está “Ligado”, “Desligado” ou “Em pausa”.
- **Botão 1 (GO)**: a próxima cue; se o playback estiver desligado, ele o inicia.
- **Botão 2 (Pausa / Voltar)**: pausa um fade em andamento; se não houver fade, volta à cue anterior.
- **O fader** e sua porcentagem. Um fader A/B mostra “A/B” e quanto o seu fade já andou.
- **Botão 3 (Flash)**: no máximo enquanto você o pressiona (veja [O flash](playbacks.md#o-flash)).

Os botões se iluminam conforme o estado: **GO** em verde enquanto roda (pisca rápido durante um fade ou uma espera e devagar em pausa), **Pausa/Voltar** em laranja em pausa e **Flash** em laranja enquanto está pressionado e até a queda dele acabar.

### O flash

Enquanto você pressiona **Flash**, o playback vai ao máximo. Ao soltar, ele volta ao nível do fader. Se estava desligado, o flash o inicia e o libera ao soltar.

Com um **ataque** e uma **queda** nas [propriedades](playbacks.md#propriedades-de-um-playback), a mudança não é de uma vez:

- Ao pressionar, sobe ao máximo no tempo do ataque, a partir de onde está. Um playback desligado sobe a partir do zero.
- Ao soltar, volta ao nível do fader no tempo da queda. Um playback que o flash iniciou desce a zero e é liberado quando a queda acaba.
- Se você pressionar de novo durante a queda, ele sobe a partir de onde está, sem baixar antes.
- Se você mover o fader durante a queda, a queda acaba no novo nível.
- Um GO durante a queda o deixa ligado: a queda acaba no nível do fader.

Os canais que não são de intensidade (posição, roda de cores) entram na hora, como sempre.

### O fader

O fader é a intensidade do que o playback reproduz:

- Ao **subi-lo a partir do zero**, o playback inicia.
- Ao **baixá-lo até zero**, uma cena é liberada; uma cuelist fica na cue dela (isso pode ser mudado nas [propriedades](playbacks.md#propriedades-de-um-playback)).
- Um **GO com o fader abaixado** reproduz no máximo.

Em uma cuelist **submaster**, o fader manda: subi-lo a partir do zero a inicia no nível do fader, nunca no máximo, e baixá-lo até zero a libera. Liberada, GO e Voltar só escolhem a cue em que ela vai iniciar (veja [Opções da cuelist](escenas.md#opções-da-cuelist)).

Em uma cuelist **override**, o fader faz um crossfade de tudo o que ela controla, também cores, posições e gobos, entre o que está abaixo e a cue dela. No máximo, a cue dela substitui o que está abaixo (veja [Opções da cuelist](escenas.md#opções-da-cuelist)).

Em uma cuelist normal, o fader pode ser um **crossfade (A/B)** (“Fader” nas [propriedades](playbacks.md#propriedades-de-um-playback)). Então ele não é a intensidade: o playback reproduz no nível que tinha, e cada percurso do fader de uma ponta à outra é um GO cujo fade você faz à mão.

- Assim que ele sai da sua ponta, a próxima cue começa e o fade dela segue o fader: 40 % do percurso são 40 % do fade. Os atrasos, as descidas e os tempos de atributos e fixtures vão em proporção.
- Na outra ponta, a cue se completa. O percurso seguinte, no sentido contrário, reproduz a cue que vem depois.
- Se você volta à ponta de onde saiu, ele é cancelado e a cue anterior volta. Se esse percurso iniciou a cuelist, ela é liberada.
- Uma cuelist parada inicia na cue de um GO, que entra com o fader. Uma cue sem tempo faz o fade de forma linear ao longo do percurso.
- Os FX da cue entram com o seu próprio tempo.
- **GO** (o botão, Main GO ou um mapeamento) reproduz a próxima cue com o seu tempo. No meio de um percurso, termina esse fade com o seu tempo e não reproduz outra cue.
- Seguir e Esperar da próxima cue esperam o fim do percurso. A velocidade das cues e o tempo ao vivo não mudam o fade que você faz à mão.

Clique duplo no fader o leva ao máximo ou a zero. Em um fader A/B, é um percurso inteiro: a cue se completa na hora.

## Colocar algo em um playback

Há várias formas:

- **Gravar a partir do programador**: veja [Gravar em um playback](playbacks.md#gravar-em-um-playback).
- **“Adicionar”** em um playback vazio: **“Nova cuelist a partir do programador”** ou **“Colocar uma cena aqui…”**.
- **Atribuir**: pressione **“Atribuir”** no cabeçalho. À direita aparece a lista de cenas: arraste-as para os playbacks (enquanto a lista está aberta, uma cena solta em um playback ocupado o substitui) ou dê um clique duplo em uma para colocá-la no primeiro playback livre.
- **Botão direito › “Atribuir uma cena…”**: escolha a cuelist ou a cena na lista e pressione **“Colocá-la no playback N”** (ou clique duplo).

Em Programar você também pode gravar uma nova cuelist e colocá-la no primeiro playback livre (veja [Gravar](programar.md#gravar)).

As cue lists do QLC+ também podem chegar aos playbacks: veja [Cenas › Cue lists do QLC+](escenas.md#cue-lists-do-qlc).

## GO, Voltar e o playback selecionado

À direita do cabeçalho você tem os controles do **playback selecionado**:

- **GO** (o botão grande verde, ou **Espaço**): a próxima cue do playback selecionado. Mostra o nome dele e qual cue vem (“Música 1 ▸ 3 · Ponte”).
- **“Voltar”** (**Shift+Espaço**): a cue anterior.
- **“Liberar tudo”**: libera todos os playbacks e todas as cuelists que estão rodando.
- **“Velocidade das cues”**: a velocidade de todas as cuelists (veja [Velocidade das cues](playbacks.md#velocidade-das-cues)).

Espaço e Shift+Espaço funcionam nesta página sempre que você não estiver digitando em um campo de texto. Cada toque dá um só GO: manter a tecla pressionada não percorre as cues.

### O painel do playback selecionado

Embaixo aparece o playback selecionado (“Playback N · nome”), o estado e os botões dele: voltar, **GO**, pausar / continuar, liberar e **“Editar em Cenas”**.

Se for uma cuelist, aparece a lista das cues dela (CUE, NOME, DISPARO, ATRASO, FADE). A cue que roda tem uma barra verde e a próxima, uma borda verde. Em cada linha:

- **“Próximo”**: essa cue será a do próximo GO.
- **“Ir”**: salta para essa cue agora.

Um clique em uma linha só a marca: não dispara nada por engano.

## Velocidade das cues e tempo ao vivo

### Velocidade das cues

**“Velocidade das cues”**, no cabeçalho, muda a velocidade de todas as cuelists de uma vez: seus fades, atrasos, Seguir e Esperar, seu ritmo marcado e os passos de um chase. Vai de ×0,1 a ×10, com ×1 no meio.

- Age na hora, também nos fades em andamento: eles aceleram ou freiam de onde estão, sem saltos.
- O valor fica laranja quando não é ×1. Um clique duplo o volta a ×1.
- Não muda os FX nem as outras cenas.
- Não é salva: todo show abre em ×1.

Ela também está nos faders do [SHOW](show.md#faders) e nos [mapeamentos](mapeos.md#controla).

### Tempo ao vivo

Enquanto uma cue está em fade, o painel do playback selecionado mostra **“Tempo ao vivo”**: o atraso mais o fade dessa cue (por exemplo, “4,0 s”). Você pode mudá-lo enquanto ele corre:

- Digite outro tempo, ou toque em **“×½”** ou **“×2”**: o fade continua de onde está e termina no tempo novo.
- **“Agora”** termina o fade na hora.
- Se a cue seguinte tem **Seguir**, ela espera o novo final.

A mudança vale só para esta vez: a cue seguinte usa os próprios tempos. “Tempo ao vivo” fica laranja quando o tempo não é o da cue. **“Gravar na cue”** grava na cue os tempos mudados (também os dos atributos e fixtures dela). Não é possível quando a cue roda com um tempo de BACK ou com o fade de entrada de um grupo ou de um bloco, que não são os tempos dela. Isso se desfaz como qualquer outra edição da cuelist.

A mesma barra está no cabeçalho da cuelist em [Cenas](escenas.md#a-tabela-de-cues). A partir de um controlador, use os mapeamentos “Tempo ao vivo de um playback” ou “Tempo ao vivo do playback selecionado” (veja [Mapeamentos](mapeos.md#controla)).

## Gravar em um playback

1. Monte um visual em [Programar](programar.md).
2. Em Playbacks, pressione **“Gravar”**: ele pisca em vermelho e cada playback mostra o que vai acontecer ao clicar.
3. Clique em um playback:
   - **Vazio** (“Nova cuelist”): é criada uma nova cuelist com o programador como cue 1.
   - **Com uma cuelist** (“Gravar a cue N”): a próxima cue dela é gravada, só com o que muda em relação ao que ela herda.
   - **Com uma cena** (“Tem uma cena”): não é possível gravar nele.

**Esc** cancela a gravação armada, e ela também é desarmada ao sair da página.

### Atualizar uma cue

Mude os valores no programador, selecione o playback e pressione **“Atualizar ▾”**:

- **“Atualizar a cue N (as seguintes acompanham)”**: a mudança passa para as próximas cues que herdam esses canais.
- **“Só esta cue (as seguintes mantêm o visual)”**.

A cue atualizada é a que está carregada no programador ou, se não houver nenhuma, a que está rodando. O menu do botão direito de cada playback também tem **“Gravar a cue N”**, **“Atualizar a cue atual (as seguintes acompanham)”** e **“Atualizar só a cue atual”**.

> **Dica:** ao terminar de gravar, pressione “Redefinir tudo” em Programar (ou marque “Limpar o programador depois de gravar”): enquanto o programador tem valores, ele manda sobre o que você dispara.

## Páginas

- Clique no nome de uma página para vê-la. **◀ ▶** (ou **PgUp** / **PgDn**) passam para a anterior ou a próxima.
- **“+ Página”** adiciona uma página.
- **“Faders por página”**: 8, 10, 12, 16 ou 20 playbacks por página.
- Clique duplo em uma página a renomeia. Botão direito: **“Renomear…”**, **“Mover para a esquerda”**, **“Mover para a direita”** e **“Excluir página…”** (com mais de uma página). Ao excluí-la, os playbacks dela são limpos; o que eles reproduzem continua rodando e as cuelists e cenas ficam em Cenas.

## Propriedades de um playback

Botão direito › **“Propriedades…”**. As mudanças são aplicadas na hora.

- **Rótulo**: o nome que ele mostra (vazio: o da cena).
- **Cor**: uma amostra, ou o **A** para a cor do seu tipo de cena.
- **Botão 1**, **Botão 2** e **Botão 3**: o que cada um faz: “Nada”, “GO”, “Voltar”, “Pausar”, “Pausa / Voltar”, “Liberar”, “Flash”, “Flash + GO” (no máximo enquanto pressionado, e a próxima cue), “Ligar / desligar” ou “Selecionar” (para que Main GO, Gravar e Atualizar atuem sobre ele).
- **Ataque do flash** e **Queda do flash**: quanto o flash leva para subir ao máximo e para voltar ao soltar, de 0 a 10 s (0 = na hora). Valem para o botão Flash ou Flash + GO dele, os mapeamentos, o SHOW e o remoto (veja [O flash](playbacks.md#o-flash)).
- **Fader**: “Intensidade” (padrão) ou “Crossfade (A/B)”, só em uma cuelist normal (veja [O fader](playbacks.md#o-fader)). Em A/B as duas opções seguintes não aparecem: o fader sempre a inicia e nunca a libera.
- **O fader o inicia**: se, ao subir o fader, o playback inicia.
- **O fader em zero o libera**: “Padrão” (sim para uma cena e uma cuelist submaster; não para as outras cuelists: mantêm a cue), “Sim” ou “Não”.

O mesmo menu tem **“Editar em Cenas”**, **“Liberar”** e **“Limpar o playback”** (tira-o do playback; a cuelist ou a cena continua em Cenas).

## Liberar

- O **×** do número de um playback que está rodando, ou **“Liberar”** no menu dele.
- **“Liberar tudo”** no cabeçalho.

Liberar um playback também libera essa cena se ela estiver rodando em um grupo de Ao vivo.

## Controladores Akai APC

Em [Mapeamentos](mapeos.md#modelos-para-akai-apc) › “Dispositivos”, **“Mapear tudo”** atribui de uma vez um controlador inteiro aos playbacks:

- **Akai APC mini** e **APC mini mk2** → Playbacks: cada coluna é um playback 1-8 da página atual: o fader dele e, de baixo para cima, GO, Pausa/Voltar, Flash e as 5 primeiras cues dele. O botão abaixo dos pads o libera, o último fader é o grand master e os botões da direita são página anterior e próxima, GO e Voltar do selecionado, blackout, tap, BeatGO e parar tudo.
- **Akai APC40 mkII** → Playbacks: cada faixa é um playback 1-8, com as 5 primeiras cues dele nos clipes (a próxima pulsa em verde); os knobs de device são os faders dos playbacks 9-16.

Os LEDs mostram o estado de cada playback.

## Playbacks no SHOW e no remoto

- O [SHOW](show.md#playbacks-no-show) pode ter uma seção de playbacks que acompanha a página atual ou mostra uma fixa.
- No [controle remoto](remoto.md), quando algum playback tem algo, aparece uma página “Playbacks” com as tiras deles.

## Com a programação bloqueada

Se você bloquear a **“Programação”** com o cadeado da barra superior, os playbacks continuam funcionando (GO, faders, flash), mas não é possível gravar, atualizar, atribuir nem mudar as páginas ou as propriedades.
