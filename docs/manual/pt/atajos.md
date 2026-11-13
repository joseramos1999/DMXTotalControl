# Atalhos de teclado

Todas as teclas e combinações do programa, por página. As teclas que você atribuir em [Mapeamentos](mapeos.md#teclado) e as teclas dos botões do [SHOW](show.md#opções-de-um-botão) somam-se a estas. Uma tecla atribuída em Mapeamentos tem preferência sobre o atalho do programa que use a mesma tecla.

## Em qualquer página

| Tecla | O que faz |
|---|---|
| **F1** | Ao vivo |
| **F2** | Show |
| **F3** | Programar |
| **F4** | Cenas |
| **F5** | Patch |
| **F6** | Fixtures |
| **F7** | Saídas |
| **F8** | Mapeamentos |
| **F9** | Opções |
| **F10** | Playbacks |
| **F11** | Tela cheia (e voltar à janela) |
| **F12** | Abre este manual na página em que você está (também o botão **“?”** da barra superior) |
| **Ctrl+N** | Novo show |
| **Ctrl+O** | Abrir um show |
| **Ctrl+S** | Salvar o show (na primeira vez, pede um nome) |
| **Ctrl+Shift+S** | Salvar o show com outro nome (“Salvar como…”) |
| **Ctrl+B** | Blackout (ligar e desligar) |
| **Esc** | Parar tudo. No modo Mapear, cancela o controle pendente ou sai do modo; com “Gravar” armado em Playbacks, desarma-o |
| **Ctrl+→** | Próxima cena em cada grupo que está rodando |
| **Ctrl+←** | Cena anterior em cada grupo que está rodando |
| **G** | BeatGO (exceto na página Show) |

Se uma página está bloqueada, a tecla dela pede a senha (se houver) e desbloqueia. Com o SHOW bloqueado (modo quiosque) as teclas de página não funcionam, e com a Configuração bloqueada também não funcionam Ctrl+N, Ctrl+O nem Ctrl+Shift+S.

T, G e as teclas que você atribuir em Mapeamentos não atuam enquanto você digita em um campo de texto, como a linha de comando de Programar.

## Playbacks

| Tecla | O que faz |
|---|---|
| **Espaço** | GO do playback selecionado (um GO por toque) |
| **Shift+Espaço** | Voltar no playback selecionado |
| **PgUp** | Página de playbacks anterior |
| **PgDn** | Próxima página de playbacks |
| **Esc** | Desarma “Gravar” |

Funcionam na página Playbacks sempre que você não estiver digitando em um campo de texto.

## Programar

| Tecla | O que faz |
|---|---|
| Digitar | O que você digita na página vai para a linha de comando |
| **Enter** | Executa o comando |
| **↑** / **↓** | Os últimos comandos |
| **Esc** | Esvazia a linha de comando (com a linha vazia, Parar tudo) |

Veja [a linha de comando](programar.md#linha-de-comando).

## Show

| Tecla | O que faz |
|---|---|
| A tecla de um botão | Pressiona esse botão (enquanto você a mantém, se for flash) |
| **Del** / **Backspace** | No modo de edição, remove o controle da célula selecionada |
| **Esc** | No modo de edição, tira a seleção |

## Super cenas

| Tecla ou gesto | O que faz |
|---|---|
| **Espaço** | Reproduzir ou parar a partir do cursor |
| **Del** / **Backspace** | Exclui os blocos selecionados |
| **Ctrl+A** | Seleciona todos os blocos das faixas visíveis |
| **Ctrl+D** | Duplica os blocos selecionados logo depois deles |
| **Ctrl+C** | Copia os blocos selecionados |
| **Ctrl+X** | Recorta os blocos selecionados (os de faixas bloqueadas só são copiados) |
| **Ctrl+V** | Cola os blocos copiados no cursor, a partir da faixa selecionada |
| **Ctrl+Z** | Desfaz a última alteração (não enquanto você digita em um campo) |
| **Ctrl+Y** / **Ctrl+Shift+Z** | Refaz o que foi desfeito |
| **Ctrl+clique** em um bloco | Adiciona-o à seleção ou o tira |
| **Shift+clique** em um bloco | Seleciona do último bloco escolhido até este |
| Arrastar em uma área vazia das faixas | Desenha um retângulo que seleciona os blocos que ele toca (com **Ctrl** ou **Shift**, eles se somam) |
| Botão direito em uma área vazia de uma faixa | “Colar aqui” e “Selecionar tudo” |
| **Ctrl+roda** | Zoom na linha do tempo |
| **Shift+roda** | Deslocar-se no tempo |
| Clique duplo na régua | Reproduzir a partir dali |

As teclas dos blocos não agem enquanto você digita em um campo. Consulte [Selecionar vários blocos](escenas.md#selecionar-vários-blocos) e [Copiar e colar](escenas.md#copiar-e-colar).

## Faders e controles deslizantes

Depois de clicar em um fader ou um controle deslizante:

| Tecla ou gesto | O que faz |
|---|---|
| **↑** / **→** | Sobe um passo |
| **↓** / **←** | Desce um passo |
| **PgUp** / **PgDn** | Sobe ou desce dez passos |
| Clique duplo | Devolve-o ao valor de repouso (normalmente, zero) |

## Cuelists e passos (Cenas)

| Tecla ou gesto | O que faz |
|---|---|
| **Ctrl+clique** / **Shift+clique** | Seleciona várias cues ou passos para alterar os tempos deles juntos |
| Clique duplo em uma cue ou um passo | Carrega-o no programador |
| Clique duplo no número de uma cue | Altera o número dela |
| Roda do mouse sobre um tempo clicado | ±0,1 s (com **Shift**, ±1 s) |
| **Ctrl+Z** | Desfaz a última edição da cuelist aberta (em um campo com texto digitado, primeiro esse texto) |
| **Ctrl+Y** / **Ctrl+Shift+Z** | Refaz o que foi desfeito |
| **Esc** em um campo | Deixa o valor como estava |

## Mouse e modificadores

| Gesto | Onde | O que faz |
|---|---|---|
| **Alt+clique** | Quase qualquer controle | Leva-o para a página do SHOW aberta |
| **Shift+clique** | Modo Mapear | Usa o controle em vez de mapeá-lo |
| **Ctrl+clique** | Botões Flash do mixer | Deixam o flash ligado até o próximo clique |
| **Ctrl+clique** / **Shift+clique** | Fixtures, grupos e plano 2D (Programar e Patch), lista de cenas, blocos de uma super cena | Adiciona ou tira da seleção |
| Roda do mouse | Plano 2D | Zoom |
| Arrastar com o botão direito ou o do meio | Plano 2D | Move a vista (clique duplo: centraliza-a) |
| Clique com o botão direito | Botão 15° do plano 2D | Gira para o outro lado |
| Clique com o botão direito | Botão do SHOW (ao vivo) | Para o que ele dispara (a cena, o grupo, o playback ou a paleta dele), também com GO |
| Manter pressionado “Desbloquear” | Show bloqueado | Desbloqueia |
