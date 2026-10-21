# Manual do DMX Total Control

DMX Total Control é um programa livre para controlar iluminação DMX a partir de um computador com Windows. Ele foi pensado para o ao vivo: você dispara cenas por grupos, programa rápido com o programador, opera cuelists com GO nos playbacks e, se quiser, monta sua própria tela touch (o SHOW) e a usa também pelo celular.

Com o DMX Total Control você pode:

- Patchear os fixtures da sua montagem com uma biblioteca de milhares de definições, mais a Open Fixture Library, a importação de GDTF e seus próprios fixtures.
- Dar dimmer, cor, posição, gobos e qualquer outro canal aos fixtures selecionados, com paletas, FX e uma linha de comando.
- Gravar cenas estáticas, passos, FX, super cenas com linha do tempo e áudio, e cuelists com tracking.
- Dispará-las na página Ao vivo por grupos, nos Playbacks com GO ou a partir do seu SHOW.
- Enviar DMX por interfaces DMX USB, Art-Net ou sACN, e controlar tudo por MIDI, OSC, entrada DMX ou o teclado.

Os shows são salvos em arquivos `.qxw`, o mesmo formato que o QLC+ usa, então você pode abrir shows feitos com o QLC+.

## Como abrir este manual

Pressione **F12** ou o botão **“?”** da barra superior. O manual abre no lado direito da janela, na página que explica a página em que você está: se você está em Programar, ele abre em [Programar](programar.md). Os links levam a outras páginas do manual; no painel você pode voltar à página anterior ou a este “Índice”. Ele também abre em Opções › Ajuda › **“Abrir o manual”**.

## O guia e os shows de exemplo

- **O guia de primeiros passos**: um cartão no canto inferior direito que leva você pelas páginas em cinco passos (os fixtures, um visual, uma cena, Ao vivo e os playbacks). Ele aparece na primeira vez que você abre o programa; depois, em Opções › Ajuda › **“Iniciar o guia”**. As páginas podem ser usadas enquanto ele está aberto. Veja [Primeiros passos](primeros-pasos.md).
- **Os shows de exemplo**: shows prontos para abrir e testar. Ficam na seta **▾** ao lado de “Abrir”, em **“Exemplos”**:
  - **Primeros pasos** (primeiros passos): o show de prática do guia, com 4 PAR, 2 moving heads e o grupo “Mis escenas” vazio.
  - **Bolo pequeño** (um show pequeno): PAR na frente e atrás, strobos e máquina de fumaça. Cores, efeitos e gerais em três grupos, e uma página de SHOW feita com eles.
  - **Concierto con cabezas móviles** (concerto com moving heads): paletas de cor e de posição, um círculo em espelho e uma música como cuelist nos playbacks.
  - **Sala con playbacks** (casa de shows com playbacks): uma cuelist por música na primeira página de playbacks e um SHOW com esses playbacks.

Os exemplos abrem como qualquer show: se você salvar alterações, use “Salvar como…” para deixar o exemplo como estava.

## A janela do programa

A janela tem três partes: a barra superior, a barra lateral com as páginas e a página que você está usando.

### As páginas

| Página | Tecla | Para que serve |
|---|---|---|
| Ao vivo | F1 | Disparar as cenas por grupos e usar o mixer |
| Show | F2 | Sua tela de show: botões, faders, dials, grupos, relógio… |
| Playbacks | F10 | Faders com GO, pausa e flash para cuelists e cenas |
| Programar | F3 | Selecionar fixtures, dar valores a eles, FX e paletas, e gravar |
| Cenas | F4 | Criar e editar cenas, efeitos, super cenas e cuelists |
| Patch | F5 | Os fixtures da montagem, seus endereços, grupos e plano 2D |
| Fixtures | F6 | Criar suas próprias definições de fixtures |
| Saídas | F7 | Universos e interfaces DMX |
| Mapeamentos | F8 | Controladores MIDI, OSC, entrada DMX e teclado |
| Opções | F9 | Ajustes do programa, biblioteca de fixtures e atualizações |

**F11** coloca a janela em tela cheia ou a devolve ao seu tamanho. Se uma página está bloqueada, ela tem um cadeado na barra lateral e, ao abri-la, a senha é pedida, se houver (veja [Opções](opciones.md#show-e-bloqueio)).

Na parte de baixo da barra lateral ficam a versão do programa e a **CPU** que ele usa (“CPU 12 %”, de todos os núcleos do computador juntos). Acima de 70 % ela fica laranja e, acima de 90 %, vermelha: a saída pode perder o ritmo. Feche outros programas ou pare alguns efeitos.

### A barra superior

Da esquerda para a direita:

- **O show**: o nome dele. Um ponto **●** na frente avisa que há alterações não salvas; se o salvamento automático estiver ativo, também aparece a hora do último (“salvo automaticamente…”).
- **Arquivo**: “Novo show (Ctrl+N)”, “Abrir (Ctrl+O)”, a seta **▾** com os shows recentes e os “Exemplos”, “Salvar (Ctrl+S)” e “Salvar como…” (Ctrl+Shift+S).
- **Ritmo**: **“GO”** e **“Stop”** mudam o que os botões das cenas fazem (na página Ao vivo, no SHOW, nos mapeamentos e no celular). Com **GO** ligado, tocar uma cena a dispara e os toques seguintes marcam o ritmo dela, sem nunca pará-la; com **Stop** ligado, tocar uma cena a para; com os dois desligados, como sempre ([Ao vivo](directo.md#ritmo-de-uma-cena-com-passos)). Cada botão de uma cena com passos ou de uma cuelist que está rodando (na página Ao vivo, no SHOW, nos playbacks e no celular) tem dentro uma **barra dividida em duas** que acende um lado e depois o outro a cada passo: assim você vê o ritmo de cada uma. Depois, **“BeatGO”** (próxima divisão das cenas comandadas pelo BeatGO, também com a tecla G) e **“Pulso”**, que escuta a entrada de áudio do computador (escolhida em [Opções](opciones.md#áudio)): cada pico que passa a linha do medidor avança as cenas comandadas pelo Pulso. Arraste a linha do medidor para mudar o limiar. O tempo do show, o dos FX e das cenas em modo BPM, é alterado com um fader ou um dial de BPM do [SHOW](show.md) ou com os [mapeamentos](mapeos.md).
- **O cadeado**: bloqueia telas para que nada mude por engano (veja [Opções](opciones.md#show-e-bloqueio)).
- **O alvo**: o modo Mapear, para atribuir controles MIDI, OSC ou teclas (veja [Mapeamentos](mapeos.md#modo-mapear)). A luz verde dele pisca a cada mensagem que chega.
- **“N rodando”**: quantas cenas e efeitos estão rodando.
- **GM**: o grand master. Abaixo de 100 % ele fica laranja.
- **“Blackout”** (Ctrl+B): leva tudo ao preto enquanto está ligado.
- **“Parar tudo”** (Esc): para tudo o que está rodando. O programador não é limpo.
- **“?”**: abre este manual (F12).

Em uma janela estreita ou uma tela pequena, a barra fica compacta: o nome do show é cortado, “Salvar como…” vira um ícone, “N rodando” fica só no número e, nas mais estreitas, “Blackout” e “Parar tudo” ficam só com o ícone. Passe o mouse sobre um ícone para ver o que ele é. Se tudo parecer pequeno ou grande, mude o **Tamanho da interface** em [Opções](opciones.md#geral).

Ao fechar o programa, ou ao abrir ou criar outro show, se houver alterações não salvas aparece a pergunta “Salvar as alterações?” com “Cancelar”, “Não salvar” e “Salvar”. Ao fechar, o programa primeiro para todas as cenas, os playbacks e o programador, e deixa as saídas enviarem o palco em repouso: muitos aparelhos ficam com o último sinal que receberam.

Se o show que você abre usa arquivos que não estão lá (os sons, vídeos ou imagens dele; por exemplo, um show trazido de outro computador), um aviso mostra a lista. O show funciona sem eles. Com **“Procurar em uma pasta…”** você escolhe a pasta onde eles estão: são procurados pelo nome, também nas subpastas, e o show passa a usá-los (salve-o para mantê-lo assim).

> **Dica:** se o programa fechou de forma inesperada e você tinha um show não salvo, ao abri-lo de novo ele oferece “Recuperar o show não salvo?”. Pressione “Recuperar” e salve-o com um nome.

## Como este manual está organizado

Comece por **Primeiros passos** se é a primeira vez. As outras páginas explicam cada página do programa e podem ser lidas em qualquer ordem.

1. [Primeiros passos](primeros-pasos.md): uma primeira sessão completa, do patch até salvar o show.
2. [Ao vivo](directo.md): grupos, exclusivo ou mistura, crossfade, masters, liberações, flash e mixer.
3. [Show](show.md): a tela touch, suas seções, botões, faders, dials, roda de cores, grade XY, edição e bloqueio.
4. [Playbacks](playbacks.md): páginas de playbacks, GO, Pausa/Voltar, Flash, níveis, gravar em um playback e controladores APC.
5. [Programar](programar.md): seleção, grupos, plano 2D, dimmer, cor, posição, features, faders, paletas, FX, leque, cego, destacar e localizar, linha de comando, gravar e atualizar.
6. [Cenas](escenas.md): todos os tipos de cena, super cenas, cuelists com tracking, pastas, importar e exportar.
7. [Patch](patch.md): adicionar fixtures, endereços, universos, números, grupos de fixtures, plano 2D, limites, matrizes de LED e importar de outro show.
8. [Fixtures](fixtures.md): a biblioteca, o criador de fixtures, a Open Fixture Library, GDTF e seus fixtures.
9. [Saídas](salidas.md): universos, interfaces DMX USB, Art-Net e sACN, e o monitor de níveis DMX.
10. [Mapeamentos](mapeos.md): MIDI, OSC, entrada DMX e teclado, o modo Mapear e os modelos para Akai APC.
11. [Opções](opciones.md): idioma, tema, tamanho, inicialização, bloqueio, salvamento automático, prioridades, controle remoto, biblioteca, atualizações.
12. [Controle remoto](remoto.md): o SHOW no celular ou no tablet.
13. [Atalhos de teclado](atajos.md): todas as teclas e combinações.

## Palavras que você verá com frequência

- **Fixture**: cada aparelho de luz patcheado (um PAR, um moving head, uma fita de LED…).
- **Universo**: 512 canais DMX. Cada universo sai por uma interface.
- **Programador**: onde você dá valores aos fixtures selecionados. O que está no programador manda sobre tudo até você limpá-lo.
- **Cena**: o que você grava para reproduzir depois. Há vários tipos: estática, passos, FX, super cena, cuelist… (veja [Cenas](escenas.md)).
- **Grupo**: uma coluna de cenas na página Ao vivo. Em um grupo exclusivo só uma cena roda por vez.
- **Grupo de fixtures**: um conjunto de fixtures para selecioná-los de uma vez (são criados no [Patch](patch.md#grupos-de-fixtures)).
- **Cuelist**: uma lista de cues que avançam com GO. Cada cue guarda só o que muda.
- **Playback**: um fader com seus botões (GO, Pausa/Voltar, Flash) que reproduz uma cuelist ou uma cena.
- **Paleta**: um valor guardado (uma cor, uma posição…) que as cenas seguem por referência: se você muda a paleta, mudam as cenas que a usam.
- **Prioridade**: de 0 a 10. Se duas cenas controlam o mesmo canal, vence a de maior prioridade.
- **SHOW**: a tela de operação que você monta.
