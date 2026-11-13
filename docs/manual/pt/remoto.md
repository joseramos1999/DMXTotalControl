# Controle remoto

O **controle remoto** mostra as páginas do [SHOW](show.md) no navegador de um celular ou de um tablet conectados à mesma rede Wi-Fi que o computador. Não é preciso instalar nada: funciona nos dois sentidos, então o que você toca no celular muda no computador e o que muda no computador aparece no celular.

## Ativá-lo

1. No computador, abra **Opções** (F9) › **Controle remoto**.
2. Ative **“Controle remoto ativado”**.
3. Se quiser, digite um **PIN**. Sem PIN, qualquer pessoa na rede pode usá-lo.
4. Embaixo aparecem os **endereços** do computador (por exemplo `http://192.168.1.20:8800/`), um para cada rede a que ele está conectado, com o nome da rede na frente. O Wi-Fi vem primeiro. O botão de copiar coloca o endereço na área de transferência. Se o computador tiver **nome na rede**, o primeiro é o do nome, por exemplo `http://dmxtotalcontrol.local:8800/` (mais abaixo).

## Conectar o celular ou o tablet

1. Conecte o celular à **mesma rede Wi-Fi** que o computador.
2. Abra o navegador e digite um dos endereços, ou escaneie o código QR (mais abaixo).
3. Se houver PIN, digite-o e pressione **OK**. Ele é pedido uma só vez em cada dispositivo.

Em Opções aparece quantos dispositivos estão conectados.

> **Dica:** adicione a página à tela inicial do celular: ela abre como um aplicativo, com o ícone do programa.

> **Dica:** se o celular não conecta, verifique se os dois estão na mesma rede e se o firewall do Windows permite que o DMX Total Control use a rede. Se outro programa usa a porta, altere-a em Opções (o aviso aparece em vermelho).

> **Dica:** na primeira vez que você ativa o controle remoto, o Windows pergunta se o DMX Total Control pode usar a rede. Permita nas redes **privadas**. Se o Windows tiver o Wi-Fi como rede **pública**, o celular não chega ao computador: mude-a para privada nas configurações de rede do Windows.

> **Dica:** se o Wi-Fi do local não deixa os dispositivos se verem, ative no computador o **hotspot móvel** do Windows e conecte o celular a ele. O remoto fica sempre em `http://192.168.137.1:8800/`.

## Escanear o código QR

Com o controle remoto ativado, embaixo dos endereços aparece um **código QR**. Com ele, o remoto abre sem digitar nada:

1. Conecte o celular ou o tablet à **mesma rede Wi-Fi** que o computador.
2. Aponte a câmera para o código.
3. Toque no link que aparece.

- O código leva o endereço que tem a borda colorida: no início, o primeiro endereço numérico, que funciona em todos os celulares. Para usar outro, clique na linha dele ou no botão de código QR dele.
- Se houver PIN, o código também o leva: o celular entra sem pedi-lo. O PIN não fica na barra de endereços do navegador.
- Por isso, com PIN, qualquer pessoa que tirar uma foto do código pode entrar.
- **“Maior”** mostra o código em tamanho grande, para escaneá-lo a alguns metros.
- No tema **Noite** o código fica mais suave, para não ofuscar.

## Encontrá-lo pelo nome

O roteador pode dar outro endereço ao computador de um dia para o outro. O nome não muda: com o controle remoto ativado, o celular abre `http://dmxtotalcontrol.local:8800/` mesmo que o endereço seja outro. Assim, o ícone da tela inicial continua funcionando.

Se você abrir o remoto com o código QR ou com um endereço numérico, o celular verifica em segundo plano se chega ao computador pelo nome. Se chegar, ele passa sozinho para `http://dmxtotalcontrol.local:8800/`, na mesma página e sem pedir o PIN. Adicione o ícone à tela inicial depois dessa mudança: a barra de endereços já mostra o nome.

- Em Opções, o endereço **“Pelo nome”** vem primeiro na lista.
- O nome é alterado em **Nome na rede**. Aceita letras, números e hifens; as maiúsculas viram minúsculas e os espaços viram hifens. Vazio, não há nome.
- Funciona no iPhone e no iPad, e em celulares com Android 12 ou posterior. Nos mais antigos, use um endereço numérico ou o código QR.
- Não funciona se o celular estiver com uma VPN ativada ou usando os dados móveis.
- Se o celular não chegar pelo nome, fica no endereço numérico sem avisar.
- Se depois o nome parar de responder, o celular volta ao endereço numérico, desde que esse endereço responda.
- Se outro computador da rede já usa o nome (por exemplo, outro DMX Total Control), este fica com o seguinte, `dmxtotalcontrol-2`, e Opções o indica embaixo do nome.
- Se a porta de rede 5353 não estiver disponível, ou se outros computadores já usam o nome e do `-2` ao `-9`, aparece um aviso em vermelho. Os endereços numéricos continuam funcionando.

## O que aparece

No topo fica a barra do remoto:

- O **nome do show** e as **abas** das páginas do SHOW. O celular lembra a última página que você abriu.
- Um **ponto** que indica se há conexão com o computador.
- **BO**: blackout.
- O botão de **tela cheia**.

Embaixo, a página com suas seções, como no computador:

- **Botões**: são pressionados com o dedo, também os de flash (enquanto o dedo está em cima). Eles se iluminam quando estão ativos.
- **Faders**: são arrastados; os que têm botão **Flash** o têm embaixo.
- **Dials e rodas**: movem-se arrastando o dedo para cima e para baixo. Os faders e os dials seguem a mesma [faixa](show.md#faders) que no computador e mostram o valor real.
- **Grupos**: as cenas deles como botões e o master.
- **Playbacks**: as tiras com o nome, a cue que roda e a próxima, **GO**, **Voltar**, o fader e **Flash**. Um fader A/B mostra “A/B” e quanto o seu fade já andou, com a borda tracejada. As seções que acompanham a página atual têm ◀ ▶ para mudá-la.
- **Roda de cores** e **grade XY**, com os botões **Liberar** e **Centro**.
- **Relógio**, **Ritmo** (**GO**, **Stop** e **BeatGO**; os botões de cenas com passos, cuelists ou FX em “BPM do show” têm dentro a barra do ritmo), **notas**, **rodando agora** e **imagem**.
- **Temporizador**: é o mesmo do computador, com **Iniciar** / **Pausar** e **Zerar**. Se você o inicia no celular, ele conta também no computador. Se você o pausa no computador, ele para em todos os celulares. Na contagem regressiva fica laranja no último minuto e vermelho ao chegar a zero.
- **Níveis DMX**: os 512 canais do universo escolhido, como no computador: quanto maior o valor, mais a célula brilha. O celular os recebe cerca de 8 vezes por segundo, só enquanto essa página está na tela.
- **Mixer ao vivo**: as mesmas tiras do [mixer](directo.md#mixer) do computador, uma ao lado da outra: “Todos os fixtures” e uma para cada grupo de fixtures. Se não couberem, deslize-as para o lado. Cada tira tem o dimmer, **Full**, **Blackout**, **Matiz**, **Saturação**, **Redefinir cor**, **Strobo** com sua velocidade, **Flash**, **Flash excl.** e **Full excl.**
  - **Flash** e **Flash excl.** funcionam enquanto o dedo está em cima. Eles são soltos se o celular perde a conexão, é bloqueado ou você muda de aplicativo.
  - Ao soltá-los, eles se apagam, mesmo que estivessem fixos no computador (Ctrl+clique).
  - A borda da tira fica laranja com um flash ou um full, e vermelha com blackout.
  - O que você muda no celular muda no computador, e vice-versa.

Se algum playback tiver algo atribuído, o remoto adiciona uma página **“Playbacks”** com as tiras da página de playbacks atual.

O remoto usa o mesmo [tema](opciones.md#geral) que o programa e aparece no idioma do celular se for espanhol, português, francês, italiano ou alemão (se não, em inglês).

## Se a conexão cair

Se o celular perde a conexão, aparece “Sem conexão com o DMX Total Control. Tentando de novo…” e ele se reconecta sozinho. O que o dedo estava pressionando (um flash, um botão) é solto no computador, para que nada fique ligado.
