# Télécommande

La **télécommande** affiche les pages du [SHOW](show.md) dans le navigateur d’un téléphone ou d’une tablette connectés au même réseau Wi-Fi que l’ordinateur. Rien à installer : elle fonctionne dans les deux sens, donc ce que vous touchez sur le téléphone change sur l’ordinateur et ce qui change sur l’ordinateur se voit sur le téléphone.

## L’activer

1. Sur l’ordinateur, ouvrez **Options** (F9) › **Télécommande**.
2. Activez **« Télécommande activée »**.
3. Si vous le souhaitez, tapez un **PIN**. Sans PIN, n’importe qui sur le réseau peut l’utiliser.
4. En dessous apparaissent les **adresses** de l’ordinateur (par exemple `http://192.168.1.20:8800/`), une pour chaque réseau auquel il est connecté, précédée du nom du réseau. Le Wi-Fi vient en premier. Le bouton de copie met l’adresse dans le presse-papiers. Si l’ordinateur a un **nom sur le réseau**, la première adresse est celle du nom, par exemple `http://dmxtotalcontrol.local:8800/` (plus bas).

## Connecter le téléphone ou la tablette

1. Connectez le téléphone au **même réseau Wi-Fi** que l’ordinateur.
2. Ouvrez le navigateur et tapez l’une des adresses, ou scannez le code QR (plus bas).
3. S’il y a un PIN, tapez-le et appuyez sur **OK**. Il n’est demandé qu’une fois sur chaque appareil.

Dans Options, vous voyez combien d’appareils sont connectés.

> **Conseil :** ajoutez la page à l’écran d’accueil du téléphone : elle s’ouvre comme une application, avec l’icône du programme.

> **Conseil :** si le téléphone ne se connecte pas, vérifiez que les deux sont sur le même réseau et que le pare-feu de Windows autorise DMX Total Control à utiliser le réseau. Si un autre programme utilise le port, changez-le dans Options (l’avertissement apparaît en rouge).

> **Conseil :** la première fois que vous activez la télécommande, Windows demande si DMX Total Control peut utiliser le réseau. Autorisez-le sur les réseaux **privés**. Si Windows considère le Wi-Fi comme un réseau **public**, le téléphone n’atteint pas l’ordinateur : passez-le en privé dans les paramètres réseau de Windows.

> **Conseil :** si le Wi-Fi de la salle empêche les appareils de se voir, activez sur l’ordinateur le **point d’accès sans fil mobile** de Windows et connectez-y le téléphone. La télécommande est alors toujours à `http://192.168.137.1:8800/`.

## Scanner le code QR

Quand la télécommande est activée, un **code QR** apparaît sous les adresses. Il ouvre la télécommande sans rien taper :

1. Connectez le téléphone ou la tablette au **même réseau Wi-Fi** que l’ordinateur.
2. Pointez son appareil photo vers le code.
3. Touchez le lien qui apparaît.

- Le code contient l’adresse qui a la bordure de couleur : au début, la première adresse numérique, qui fonctionne sur tous les téléphones. Pour en utiliser une autre, cliquez sur sa ligne ou sur son bouton de code QR.
- S’il y a un PIN, le code le contient aussi : le téléphone entre sans le demander. Le PIN ne reste pas dans la barre d’adresse du navigateur.
- C’est pourquoi, avec un PIN, quiconque photographie le code peut entrer.
- **« Plus grand »** affiche le code en grand, pour le scanner à quelques mètres.
- Avec le thème **Nuit**, le code est atténué, pour ne pas éblouir.

## Le trouver par son nom

Le routeur peut donner une autre adresse à l’ordinateur d’un jour à l’autre. Le nom, lui, ne change pas : quand la télécommande est activée, le téléphone ouvre `http://dmxtotalcontrol.local:8800/` même si l’adresse a changé. Ainsi, l’icône de l’écran d’accueil continue de fonctionner.

Si vous ouvrez la télécommande avec le code QR ou une adresse numérique, le téléphone vérifie en arrière-plan s’il atteint l’ordinateur par son nom. Si c’est le cas, il passe tout seul à `http://dmxtotalcontrol.local:8800/`, sur la même page et sans demander le PIN. Ajoutez l’icône à l’écran d’accueil après ce changement : la barre d’adresse affiche alors le nom.

- Dans Options, l’adresse **« Par son nom »** est la première de la liste.
- Le nom se change dans **Nom sur le réseau**. Il accepte les lettres, les chiffres et les tirets ; les majuscules deviennent des minuscules et les espaces des tirets. Vide, il n’y a pas de nom.
- Cela fonctionne sur iPhone et iPad, et sur les téléphones avec Android 12 ou plus récent. Sur les plus anciens, utilisez une adresse numérique ou le code QR.
- Cela ne fonctionne pas si le téléphone a un VPN activé ou utilise les données mobiles.
- Si le téléphone n’atteint pas l’ordinateur par son nom, il reste sur l’adresse numérique, sans aucun avertissement.
- Si le nom cesse ensuite de répondre, le téléphone revient à l’adresse numérique, à condition que cette adresse réponde.
- Si un autre ordinateur du réseau utilise déjà le nom (par exemple, un autre DMX Total Control), celui-ci prend le suivant, `dmxtotalcontrol-2`, et Options l’indique sous le nom.
- Si le port réseau 5353 n’est pas disponible, ou si d’autres ordinateurs utilisent déjà le nom et `-2` à `-9`, un avertissement apparaît en rouge. Les adresses numériques fonctionnent toujours.

## Ce qui s’affiche

En haut se trouve la barre de la télécommande :

- Le **nom du show** et les **onglets** des pages du SHOW. Le téléphone se souvient de la dernière page ouverte.
- Un **point** qui indique si la connexion avec l’ordinateur est établie.
- **BO** : blackout.
- Le bouton de **plein écran**.

En dessous, la page avec ses sections, comme sur l’ordinateur :

- **Boutons** : ils s’actionnent avec le doigt, y compris les flash (tant que le doigt est dessus). Ils s’allument quand ils sont actifs.
- **Faders** : ils se font glisser ; ceux qui ont un bouton **Flash** l’ont en dessous.
- **Rotatifs et roues** : ils se tournent en faisant glisser le doigt de haut en bas. Les faders et les rotatifs suivent la même [plage](show.md#faders) que sur l’ordinateur et affichent la valeur réelle.
- **Groupes** : leurs scènes en boutons et leur master.
- **Playbacks** : les bandes avec leur nom, la cue en cours et la suivante, **GO**, **Retour**, leur fader et **Flash**. Un fader A/B affiche « A/B » et où en est son fondu, avec une bordure en pointillés. Les sections qui suivent la page actuelle ont ◀ ▶ pour la changer.
- **Roue de couleur** et **grille XY**, avec leurs boutons **Libérer** et **Centre**.
- **Horloge**, **Rythme** (**GO**, **Stop** et **BeatGO** ; les boutons de scènes à pas, de cuelists ou de FX en « BPM du show » contiennent leur barre de rythme), **notes**, **en cours** et **image**.
- **Minuteur** : c’est le même que sur l’ordinateur, avec **Démarrer** / **Pause** et **Réinitialiser**. Si vous le démarrez sur le téléphone, il compte aussi sur l’ordinateur. Si vous le mettez en pause sur l’ordinateur, il s’arrête sur tous les téléphones. En compte à rebours, il devient orange dans la dernière minute et rouge en arrivant à zéro.
- **Niveaux DMX** : les 512 canaux de l’univers choisi, comme sur l’ordinateur : plus la valeur est haute, plus la case est lumineuse. Le téléphone les reçoit environ 8 fois par seconde, seulement tant que cette page est à l’écran.
- **Mixeur en direct** : les mêmes bandes que le [mixeur](directo.md#mixeur) de l’ordinateur, côte à côte : « Tous les fixtures » et une pour chaque groupe de fixtures. Si elles ne tiennent pas, faites-les défiler sur le côté. Chaque bande contient le dimmer, **Full**, **Blackout**, **Teinte**, **Saturation**, **Réinitialiser la couleur**, **Stroboscope** avec sa vitesse, **Flash**, **Flash excl.** et **Full excl.**
  - **Flash** et **Flash excl.** agissent tant que le doigt est dessus. Ils sont relâchés si le téléphone perd la connexion, se verrouille ou si vous changez d’application.
  - En les relâchant, ils s’éteignent, même s’ils étaient bloqués allumés sur l’ordinateur (Ctrl+clic).
  - Le bord d’une bande devient orange avec un flash ou un full, et rouge avec un blackout.
  - Ce que vous changez sur le téléphone change sur l’ordinateur, et inversement.

Si un playback a quelque chose d’assigné, la télécommande ajoute une page **« Playbacks »** avec les bandes de la page de playbacks actuelle.

La télécommande utilise le même [thème](opciones.md#général) que le programme et s’affiche dans la langue du téléphone si c’est l’espagnol, le portugais, le français, l’italien ou l’allemand (sinon, en anglais).

## Si la connexion est coupée

Si le téléphone perd la connexion, « Pas de connexion avec DMX Total Control. Nouvel essai… » apparaît et il se reconnecte tout seul. Ce que le doigt maintenait (un flash, un bouton) est relâché sur l’ordinateur, pour que rien ne reste allumé.
