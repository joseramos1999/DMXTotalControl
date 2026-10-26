# Télécommande

La **télécommande** affiche les pages du [SHOW](show.md) dans le navigateur d’un téléphone ou d’une tablette connectés au même réseau Wi-Fi que l’ordinateur. Rien à installer : elle fonctionne dans les deux sens, donc ce que vous touchez sur le téléphone change sur l’ordinateur et ce qui change sur l’ordinateur se voit sur le téléphone.

## L’activer

1. Sur l’ordinateur, ouvrez **Options** (F9) › **Télécommande**.
2. Activez **« Télécommande activée »**.
3. Si vous le souhaitez, tapez un **PIN**. Sans PIN, n’importe qui sur le réseau peut l’utiliser.
4. En dessous apparaissent les **adresses** de l’ordinateur (par exemple `http://192.168.1.20:8800/`), une pour chaque réseau auquel il est connecté. Le bouton de copie met l’adresse dans le presse-papiers.

## Connecter le téléphone ou la tablette

1. Connectez le téléphone au **même réseau Wi-Fi** que l’ordinateur.
2. Ouvrez le navigateur et tapez l’une des adresses.
3. S’il y a un PIN, tapez-le et appuyez sur **OK**. Il n’est demandé qu’une fois sur chaque appareil.

Dans Options, vous voyez combien d’appareils sont connectés.

> **Conseil :** ajoutez la page à l’écran d’accueil du téléphone : elle s’ouvre comme une application, avec l’icône du programme.

> **Conseil :** si le téléphone ne se connecte pas, vérifiez que les deux sont sur le même réseau et que le pare-feu de Windows autorise DMX Total Control à utiliser le réseau. Si un autre programme utilise le port, changez-le dans Options (l’avertissement apparaît en rouge).

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
- **Playbacks** : les bandes avec leur nom, la cue en cours et la suivante, **GO**, **Retour**, leur fader et **Flash**. Les sections qui suivent la page actuelle ont ◀ ▶ pour la changer.
- **Roue de couleur** et **grille XY**, avec leurs boutons **Libérer** et **Centre**.
- **Horloge**, **Rythme** (**GO**, **Stop** et **BeatGO** ; les boutons de scènes à pas, de cuelists ou de FX en « BPM du show » contiennent leur barre de rythme), **notes**, **en cours** et **image**.

Les **minuteurs**, le **mixeur en direct** et les **niveaux DMX** apparaissent comme « Sur l'ordinateur seulement ».

Si un playback a quelque chose d’assigné, la télécommande ajoute une page **« Playbacks »** avec les bandes de la page de playbacks actuelle.

La télécommande utilise le même [thème](opciones.md#général) que le programme et s’affiche dans la langue du téléphone si c’est l’espagnol, le portugais, le français, l’italien ou l’allemand (sinon, en anglais).

## Si la connexion est coupée

Si le téléphone perd la connexion, « Pas de connexion avec DMX Total Control. Nouvel essai… » apparaît et il se reconnecte tout seul. Ce que le doigt maintenait (un flash, un bouton) est relâché sur l’ordinateur, pour que rien ne reste allumé.
