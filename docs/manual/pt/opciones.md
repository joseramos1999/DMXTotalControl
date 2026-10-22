# Opções

![A página Opções](../img/opciones.png)

**Opções** (F9) reúne os ajustes do programa. À esquerda ficam os ajustes gerais, a inicialização, o bloqueio, o salvamento automático, a reprodução e o áudio; à direita, o controle remoto, a biblioteca de fixtures, as atualizações, a ajuda e “Sobre”.

Os ajustes desta página são do computador, não do show: valem para todos os shows que você abrir.

## Geral

- **Idioma**: “Idioma do sistema”, Español, English, Português, Français, Italiano ou Deutsch. É aplicado na próxima vez que o programa iniciar.
- **Tema**: **Escuro**, **Claro**, **Alto contraste** ou **Noite (atenuado)**. Muda na hora. Claro, para salas iluminadas; Alto contraste, para ler de longe; Noite, muito escuro e em vermelho, para uma cabine no escuro sem ofuscar você. O [controle remoto](remoto.md) segue o mesmo tema.
- **Tamanho da interface**: de 80 % a 150 %, menor para notebooks pequenos e maior para telas touch. É aplicado na próxima vez que o programa iniciar.
- **Abrir o último show ao iniciar**.

> **Dica:** o tema Claro funciona melhor em ensaios de dia ou ao ar livre; Alto contraste, quando há muita luz em volta da tela.

## SHOW e bloqueio

- **Abrir o SHOW em tela cheia ao iniciar**: para instalações e quiosques. O programa inicia na página Show em tela cheia.
- **…e bloqueado**: além disso, inicia bloqueado: só o SHOW, sem navegação. Para sair, mantenha pressionado o botão do cadeado (com a senha, se houver). Veja [Show](show.md#tela-cheia-e-bloqueio).
- **Bloquear telas**: **“Bloquear…”** / **“Desbloquear…”**. É o mesmo que o cadeado da barra superior.

### Bloquear telas

O bloqueio evita que algo mude por engano durante o show. A página Ao vivo e o SHOW continuam funcionando, e os playbacks continuam rodando.

1. Pressione o **cadeado** da barra superior (ou “Bloquear…” aqui).
2. Escolha o que bloquear:
   - **Configuração**: Patch, Fixtures, Saídas, Mapeamentos e Opções. Também não é possível abrir nem criar outro show.
   - **Programação**: Programar e Cenas. Em Playbacks não é possível gravar, atualizar nem atribuir.
   - **Edição do SHOW**: as páginas do SHOW são usadas, não alteradas.
3. Se quiser, digite uma **senha** (é opcional).
4. Pressione **“Bloquear”**.

As páginas bloqueadas têm um cadeado na barra lateral. Ao abrir uma, a senha é pedida (se houver) e tudo é desbloqueado. Para desbloquear, pressione o cadeado da barra superior. **“Remover a senha”** fica na mesma janela, ao bloquear.

## Salvamento automático

- **Salvar uma cópia de recuperação**, **A cada (minutos)**: de 1 a 60.

O salvamento automático não mexe no arquivo do show: a cada poucos minutos, se houver alterações não salvas, ele salva à parte uma cópia de recuperação, e o ponto **●** continua lá. Só **Salvar** grava o arquivo; a versão que havia antes do primeiro salvamento de cada sessão é mantida como `.bak`. A cópia é apagada quando você salva, quando fecha (salvando ou não) e quando abre outro show. Se o programa fechar de forma inesperada, ao abri-lo de novo ele oferece a cópia (“Recuperar o show não salvo?”) com o nome do show e a hora da cópia: ao recuperá-la, abre-se esse show com as alterações, ainda não salvas; salve-o para mantê-las. Enquanto você decide, o último show já está aberto como estava salvo (com “Abrir o último show ao iniciar”): “Descartar” o deixa assim. Na barra superior aparece a hora da última cópia (“cópia de recuperação…”); se ela falhar (um disco cheio), aparece um aviso em vermelho.

## Reprodução

- **Prioridade padrão das novas cenas**: de 0 a 10. Se duas cenas controlam o mesmo canal, vence a de maior prioridade (veja [Prioridades](directo.md#prioridades)).
- **Prioridade das edições ao vivo**: o que você define em Programar vence as cenas desta prioridade ou menor. Com 10, todas as cenas.
- **Quando uma cena começa, as edições ao vivo**:
  - **“Devolvem os canais à cena”**: os canais que a cena toca deixam de estar no programador.
  - **“Ficam sempre”**.
  - **“São todas redefinidas”**: o programador é esvaziado.
- **O grand master atua sobre**: **“Canais de intensidade”** (a luz: os dimmers e as cores dos fixtures sem dimmer) ou **“Todos os canais”**.

## Áudio

- **Entrada de áudio**: o microfone ou a entrada de linha que o **Pulso** e as matrizes de áudio escutam. **“Padrão do sistema”** usa a que o sistema operacional tiver. O Pulso muda de entrada na hora; uma matriz de áudio que já esteja rodando, na próxima vez que iniciar. Uma entrada escolhida que agora não está conectada aparece como “(não conectada)”, e enquanto isso é usada a padrão.

## Shows recentes

A lista dos últimos shows abertos (também na seta **▾** da barra superior). **“Limpar lista”** a apaga.

## Controle remoto

Para usar as páginas do SHOW em um celular ou um tablet na mesma rede Wi-Fi, sem instalar nada:

- **Controle remoto ativado**.
- **Porta**: altere-a se outro programa a usar (por padrão, 8800). Se ela estiver ocupada, aparece um aviso em vermelho.
- **PIN**: é pedido uma vez em cada dispositivo. Vazio, qualquer pessoa na rede pode usá-lo.
- Os **endereços** que devem ser abertos no navegador do celular, com um botão para copiá-los.
- Quantos dispositivos estão conectados.

Veja [Controle remoto](remoto.md).

## Biblioteca de fixtures

Indica quantas definições de fixtures estão disponíveis (a biblioteca incluída, os genéricos e os seus) e permite ampliá-la:

- **Open Fixture Library**: **“Baixar”** (precisa de internet). Já baixada, aparece quantos fixtures ela tem e quando foi baixada, e o botão passa a ser **“Atualizar”**: traz os fixtures novos e os melhorados, e mantém os seus fixtures e os que a biblioteca não tiver mais. Se houver novidades, um aviso informa e o botão fica destacado (veja [Fixtures](fixtures.md#open-fixture-library)).
- **Importar arquivos de fixtures**: **“Importar…”** definições `.qxf` (o formato do QLC+), arquivos `.gdtf` ou um `.zip` com eles.
- **Fixtures GDTF**: **“GDTF-Share”** abre o site; **“Importar GDTF…”** importa arquivos, pastas ou o `.zip` do download múltiplo. Indica quantos fixtures GDTF há na biblioteca.
- **Importar meus fixtures do QLC+**: **“Importar”** copia os fixtures que você criou no QLC+.
- **Pasta de fixtures do usuário**: **“Abrir pasta”**.

Tudo isso é explicado em [Fixtures](fixtures.md).

## Atualizações

- **Procurar atualizações ao iniciar**: uma vez por dia, no GitHub: o programa e, se você baixou a Open Fixture Library, se ela tem fixtures novos (um aviso informa). Nada é baixado nem instalado sem perguntar a você. Sem internet, não incomoda.
- **Incluir versões de teste**: as novidades antes, menos testadas.

Abaixo aparece o estado (“Você tem a versão mais recente”, “A versão … já está disponível”) e os botões:

- **“Procurar atualizações”**: verifica agora (também a Open Fixture Library).
- **“Novidades”**: o que há de novo nessa versão.
- **“Baixar e instalar”**: baixa o instalador e verifica o hash SHA-256 dele antes de salvá-lo. Se uma versão não publica o hash, ela só pode ser baixada pela página dela (“Abrir no GitHub”).
- **“Instalar agora”**: o programa fecha (antes pergunta se você quer salvar o show), a nova versão é instalada e ele abre de novo.
- **“Ao fechar”** / **“Não ao fechar”**: instalá-la quando você fechar o programa.
- **“Abrir no GitHub”**: a página da versão.

Quando há uma nova versão, aparece também um aviso acima das páginas com os mesmos botões. **“Mais tarde”** tira o aviso para essa versão; ela continua disponível aqui.

## Ajuda

- **Manual**: **“Abrir o manual”** abre este manual no índice. F12 ou o botão **“?”** da barra superior o abrem na página em que você está. Ele aparece no idioma do programa (espanhol, inglês, português, francês, italiano ou alemão); as capturas de tela são as da versão em espanhol.
- **Guia de primeiros passos**: **“Iniciar o guia”** abre o guia de novo (veja [Primeiros passos](primeros-pasos.md#o-guia)).
- **Shows de exemplo**: ficam na seta **▾** ao lado de “Abrir” da barra superior, em “Exemplos” (veja [o índice](index.md#o-guia-e-os-shows-de-exemplo)).

## Sobre

A versão do programa, a licença (software livre, Apache 2.0) e os créditos.
