# Scènes

![La page Scènes](../img/escenas.png)

**Scènes** (F4) est la bibliothèque de tout ce que vous avez enregistré : scènes statiques, pas, FX, effets de couleur, racks, super scènes et cuelists. À gauche se trouve la liste ; à droite, l’éditeur de la scène choisie, différent selon son type.

Presque tout se crée depuis le programmeur : les nouveaux effets prennent les fixtures sélectionnés dans [Programmer](programar.md), et les pas et les cues prennent ses valeurs.

## La liste des scènes

- **« Rechercher… »** : filtre par nom (ou par nom de dossier).
- **« Dossiers »** / **« Groupes »** : afficher les scènes par [dossiers](escenas.md#dossiers) ou par les groupes de Direct où elles se trouvent (« Hors groupe » pour les autres).
- **Tout replier** / **Tout déplier** : le bouton de droite.
- **Filtre par type** : « Tout », « Statique », « Pas », « Cuelists », « FX », « Colour FX », « Racks » et « Super scènes ».
- **Ctrl+clic** et **Maj+clic** sélectionnent plusieurs scènes pour les déplacer dans un dossier (« Vers un dossier… »).

Chaque scène porte une bande de la couleur de son type. Cliquez sur une scène pour l’ouvrir dans l’éditeur.

## Créer une scène

Appuyez sur **« Nouvelle »** :

| Option | Ce qu’elle crée |
|---|---|
| **Statique (depuis le programmeur)** | Une scène statique avec les valeurs du programmeur. |
| **Pas (programmeur = premier pas)** | Une scène à pas. |
| **Cuelist (programmeur = cue 1)** | Une cuelist. |
| **FX (dans le programmeur)** | Vous emmène dans Programmer › FX : les FX se font là-bas et s’enregistrent avec « Enregistrer… ». |
| **Colour FX (fixtures sélectionnés)** | Un effet de couleur sur les fixtures sélectionnés, dans leur ordre. |
| **Mapping sur le plan 2D (fixtures sélectionnés)** | Un effet de couleur placé là où sont les fixtures sur le plan 2D. |
| **Rack d’effets** | Un rack vide. |
| **Super scène (timeline avec audio)** | Une super scène vide. |
| **Importer depuis un fichier…** | Des scènes et des effets exportés d’un autre show (voir [Importer et exporter](escenas.md#importer-et-exporter)). |
| **Cue lists de QLC+…** | Des cuelists à partir des chenillards, des séquences et des listes de cues d’un show QLC+ (voir [Cue lists de QLC+](escenas.md#cue-lists-de-qlc)). |

## L’en-tête de l’éditeur

- **Le nom** : cliquez dessus pour le changer.
- **Le dossier** (icône de dossier) : déplacer la scène dans un dossier.
- **« Essayer »** / **« Arrêter »** : la joue maintenant, hors des groupes. Les cuelists ont leur propre GO.
- **« Ajouter au groupe… »** : l’ajoute à un groupe de Direct.
- **Convertir en pas** (racks, FX, Colour FX, Move FX, Curve FX et chenillards) : enregistre un cycle dans une nouvelle scène à pas, seulement avec les canaux auxquels la scène donne une valeur : les autres, même s’ils ont une valeur par défaut (le pan ou le zoom au centre sur beaucoup de lyres), n’entrent pas dans les pas. Si un FX a des délais, il enregistre le cycle une fois que tous ses fixtures ont démarré.
- **Convertir en cuelist** (pas et chenillards) : voir [Cuelists](escenas.md#cuelists).
- **« QLC+ »** (cuelists) : enregistre une copie du show pour QLC+ ; voir [Cuelists et QLC+](escenas.md#cuelists-et-qlc).
- **Exporter**, **« Dupliquer »** et **« Supprimer »** (elle est aussi retirée de ses groupes et racks).

## Comment joue une scène

Sous l’en-tête se trouve le panneau de lecture. Il affiche ce qui a du sens pour chaque type :

- **« Montée »** et **« Descente »**.
- **« Temps de cycle »** (ou **« Temps par pas »** dans un Colour FX) et **« Vitesse »** (d’un cycle par minute à dix par seconde ; double-clic : un cycle par seconde), avec **×½** et **×2**.
- **« Fondu décalé »** (statiques et pas) : chaque fixture commence son fondu ce temps-là après le précédent, dans l’ordre de sélection.
- **« Piloté par »** : **« Durée »** (les temps normaux), **« BPM »** (les temps passent en beats et suivent le tempo du show, les fondus de chaque pas aussi ; le temps de fondu enchaîné d’un groupe du Direct et les fondus des blocs d’une super scène restent en secondes), **« BeatGO »** ou **« Pulsation »** (avance avec le bouton BeatGO ou avec les pics de l’entrée audio). Avec BeatGO ou Pulsation, **« Divisions »** (/2, ×2).
- **« Rythme tapé »** (séquences, chenillards et scènes FX avec des FX en « BPM du show ») : le rythme tapé en appuyant plusieurs fois de suite sur son bouton ([Direct](directo.md#rythme-dune-scène-à-pas)), en BPM. Tant qu’elle en a un, ses pas suivent ce rythme et non leurs temps ; dans une scène FX, ses FX en « BPM du show » font un cycle à chaque battement au lieu de suivre le BPM du show. **« Retirer »** revient à ses temps (ou au BPM du show). Avec « BPM », un beat est un pas de ce rythme : un fondu de 0,5 b dure un demi-pas.
- **« Début »** : **« En cours »** ou **« En pause, appuyez à nouveau pour jouer »**.
- **« Joue »** : **« Toujours »** ou **« N fois »**, **« puis »** **« La libérer »** ou **« Garder la dernière image »**, **« et jouer »** « Rien », « La scène suivante du groupe », « La scène précédente du groupe » ou n’importe quelle autre scène.

## Types de scène

### Statique

Elle garde des valeurs fixes. Elle affiche combien de canaux et de fixtures elle contient, et les palettes qu’elle utilise (« Utilise ces palettes : quand elles changent, la scène change aussi »).

Pour la modifier :

1. Appuyez sur **« Modifier dans le programmeur »** : elle est chargée dans Programmer.
2. Changez ce que vous voulez.
3. Revenez dans Scènes et appuyez sur **« Enregistrer le programmeur dans cette scène »**.

Si vous enregistrez des FX dans une scène statique, elle devient une scène FX.

### Pas

Une succession de looks, chacun avec sa **montée**, son **maintien** et sa **descente**.

- **« Ajouter un pas »** : le programmeur devient un nouveau pas après celui sélectionné.
- **« Enregistrer dans le pas »** : enregistre le programmeur dans le pas sélectionné.
- **« Charger le pas »** (ou double-clic sur un pas) : le met dans le programmeur pour le voir et le modifier.
- **Dupliquer**, **▲ ▼** pour réordonner et **supprimer** le pas.
- Le bouton **∞** du maintien fait attendre un GO au pas (« Jusqu’au GO »).

**Ctrl+clic** et **Maj+clic** sélectionnent plusieurs pas : quand vous changez un temps, il change sur tous. Les pas utilisent les palettes **par référence**, comme les scènes et les cues : si vous mettez à jour une palette, les pas enregistrés avec elle changent (celui qui joue, aussitôt). L’icône de palette d’un pas indique lesquelles il utilise.

### FX

Une scène FX a des **valeurs de base** et une ou plusieurs **couches de FX**. Elle se crée en enregistrant depuis le programmeur avec des FX (voir [FX](programar.md#fx)).

- À gauche, la liste des FX : paramètre, fixtures et vitesse (en « BPM du show », le rythme tapé de la scène si elle en a un). La corbeille retire une couche.
- À droite, les contrôles de la couche choisie (les mêmes que dans Programmer) et son aperçu animé. Les changements se voient en direct pendant que la scène joue.
- **« Lier »** : les couches liées partagent la vitesse et le timing.
- Si certains fixtures font le FX à leur façon, un avertissement le signale : si vous changez ici la forme, la taille, le sens ou la phase, tous feront la même chose.
- **« Par fixture »** : le décalage et le délai de chaque fixture, en direct aussi. Ici, « Répartir… » et « Sans décalage ni délai » portent sur tous (voir [Décalage et délai par fixture](programar.md#décalage-et-délai-par-fixture)).
- Pour ajouter des couches ou changer les valeurs de base : **« Modifier dans le programmeur »** puis **« Enregistrer le programmeur dans cette scène »**.

### Colour FX et mappings

Un **Colour FX** est un effet de pixels (plus de 60 : dégradés, feu, météores, stroboscopes, ondes…) qui parcourt les fixtures dans l’ordre de sélection.

- **« Effet »** : lequel.
- **« Agit sur »** : « Couleur (RGB) », « Blanc », « Ambre », « UV », « Dimmer (valeur) » ou « Shutter ».
- **« Couleurs »** : jusqu’à 5, selon l’effet. Cliquez sur une couleur pour la changer (échantillons ou teinte), ou « Aucune couleur ».
- **« Texte »** : dans les effets de texte, le texte qui parcourt les fixtures.
- **« Image »** et **« Animation »** : dans les effets d’image, le fichier et « Fixe », « Défilement latéral », « Défilement vertical » ou « Animée (GIF) ».
- **« Disposition »** : **« En rangée »** (dans l’ordre de sélection) ou **« Sur le plan 2D »** (là où sont les fixtures).
- **« Paramètres »** : ceux propres à chaque effet.
- **« Utiliser la sélection du programmeur »** : remplace ses fixtures par ceux qui sont sélectionnés.

En haut, vous voyez la couleur réelle de chaque tête pendant que l’effet joue (appuyez sur « Essayer » pour la voir).

Un **mapping sur le plan 2D** est un Colour FX qui naît déjà avec la disposition du plan : l’effet, le texte ou l’image se placent sur le plateau comme le sont les fixtures.

### Move FX et Curve FX

Ce sont des effets de l’ancien module : les **Move FX** bougent pan/tilt selon une forme et les **Curve FX** donnent une onde (sinus, triangle, rampes, carré, aléatoire, stroboscope…) à des canaux. Vous les trouverez dans d’anciens shows ou dans des shows QLC+ ; ils ne se créent plus depuis « Nouvelle ».

Ils jouent toujours de la même façon et ont leur éditeur (forme, largeur, hauteur, rotation, position, phase, répartition entre fixtures, symétrique, relatif…). Un avertissement propose **« Convertir en FX »** : ils passent dans l’éditeur de FX, aussi dans le programmeur, et les groupes, les boutons du SHOW et les mappages les gardent. La phase d’un Move FX devient la « Phase » du FX, et le décalage de départ de chaque fixture, son « Décalage » ([Décalage et délai par fixture](programar.md#décalage-et-délai-par-fixture)).

### Rack d’effets

Plusieurs scènes et effets qui jouent ensemble comme un seul, chacun sur ses fixtures.

- Dans **« Ajouter »** (à droite), cliquez sur une scène pour la mettre dans le rack.
- Dans **« Dans le rack (N) »**, la **×** la retire.

Un rack ne peut pas contenir une scène qui le joue lui-même.

### Chenillards

Les chenillards des shows QLC+ jouent normalement, mais n’ont pas d’éditeur : convertissez-les en pas (enregistre un cycle) ou en cuelist avec les boutons de l’en-tête.

## Super scènes

Une **super scène** est une timeline avec des pistes de scènes et d’audio. Elle se lance comme n’importe quelle scène (dans un groupe, dans le SHOW, avec un mappage). Son éditeur occupe toute la page ; le bouton de liste, en haut à gauche, revient à la liste des scènes.

### La barre d’outils

- **« Lecture »** / **« Arrêter »** (**Espace**) : depuis le curseur. **⏮** revient au début. À côté s’affichent le temps (et la mesure).
- Les flèches **Annuler** et **Rétablir** (**Ctrl+Z** / **Ctrl+Y**). Voir [Annuler](escenas.md#annuler).
- **« Coller »** (**Ctrl+V**) : les blocs copiés, au curseur et à partir de la piste sélectionnée. Voir [Copier et coller](escenas.md#copier-et-coller).
- **« Temps »** / **« Mesures »** : la grille. En mesures, le **BPM**, la signature (**4/4**, **3/4**, **2/4**) et **« Mesure 1 ici »** pour que la première mesure commence au curseur et tombe juste avec la musique.
- **« Magnétisme »** : les blocs et les points s’alignent sur la grille.
- **« Fin ici »** : la super scène se termine au curseur (la **×** retire le marqueur : elle se termine avec son dernier bloc). **« Boucle »** : elle recommence.
- **« + Piste »** et **« + Audio »** (un fichier audio sur la piste sélectionnée, au curseur).
- **« Valeurs statiques »** : **« Prendre les valeurs du programmeur »** (elles jouent tant que la super scène tourne) ou **« Effacer les valeurs statiques »**.
- **Zoom** − et +.

### Placer des scènes sur la timeline

1. Appuyez sur **« + Piste »** s’il n’y en a aucune.
2. Faites glisser une scène de la liste de gauche sur une piste. Ou sélectionnez une piste, placez le curseur et faites un **double-clic** sur la scène : elle se place au curseur.

Les blocs peuvent se chevaucher.

Une cuelist **« Timecode »** avec son horloge propre dans un bloc suit la timeline : le 00:00 de son horloge est le début du bloc. Placez-la sur une autre piste, à côté du bloc audio de la chanson et au même point : chaque cue joue à son moment de la chanson. Si vous lisez depuis un autre point, la cuelist va droit à la cue de cette heure. Étirée, son horloge va plus vite ou plus lentement pour que la cuelist dure le bloc.

### Travailler avec les blocs

- **Faites glisser** un bloc pour le déplacer, aussi vers une autre piste. Faites glisser ses **bords** pour changer sa durée.
- Avec un seul bloc sélectionné, les **poignées** des coins supérieurs sont sa montée et sa descente.
- Quand vous allongez un bloc, la scène **se répète en boucle** (des repères marquent chaque tour) ou **s’étire** pour qu’un cycle dure tout le bloc. Cela se change dans le menu du bloc ou dans la barre du bas (« En boucle » / « Étirée »).
- **Clic droit** : « Répéter la scène dans le bloc » / « Étirer la scène à la taille du bloc », « Copier », « Couper », « Dupliquer », « Verrouiller » / « Déverrouiller », « Ouvrir la scène » et « Supprimer ». Sur un bloc de la sélection, le menu agit sur toute la sélection ; « Ouvrir la scène » ouvre celle du bloc cliqué.
- Un **double-clic** sur un bloc ouvre sa scène. **Suppr** (ou **Retour arrière**) supprime les blocs sélectionnés ; ceux des pistes verrouillées restent. **Ctrl+D** les duplique juste après eux : chaque copie sur sa piste.
- La barre du bas affiche le bloc sélectionné : **Début**, **Durée**, **Montée**, **Descente**. Avec plusieurs blocs, elle indique leur nombre. Avec un ou plusieurs blocs, elle propose **Copier**, **Couper**, **Dupliquer** et **Supprimer**.

### Sélectionner plusieurs blocs

- **Clic** sur un bloc : lui seul. Sa piste devient la piste sélectionnée.
- **Ctrl+clic** ajoute un bloc à la sélection ou l’en retire.
- **Maj+clic** sélectionne du dernier bloc choisi jusqu’à celui-ci : les blocs des pistes entre les deux, du début le plus tôt à la fin la plus tardive.
- **Faire glisser sur une zone vide** des pistes dessine un rectangle qui sélectionne les blocs qu’il touche. Avec **Ctrl** ou **Maj**, ils s’ajoutent à la sélection.
- **Ctrl+A** sélectionne tous les blocs des pistes visibles. Ceux d’une piste que vous masquez sortent de la sélection.
- Un clic sur une zone vide désélectionne tout (et déplace le curseur). Avec Ctrl ou Maj, la sélection reste.

Quand vous **faites glisser** un bloc sélectionné, toute la sélection se déplace :

- Tous se déplacent d’autant dans le temps : le bloc que vous faites glisser s’aligne sur la grille et les autres gardent leurs écarts. Le premier s’arrête à 0.
- Ils montent ou descendent du même nombre de pistes. Les pistes où ils vont s’allument.
- Les blocs verrouillés et ceux des pistes verrouillées restent en place.
- Si l’un d’eux tombait hors des pistes ou sur une piste verrouillée, ils se déplacent seulement dans le temps.
- Les bords, la montée et la descente ne changent que le bloc sous la souris.

Cliquer sur un bloc de la sélection et le relâcher sans le déplacer ne laisse que lui sélectionné. Chaque action sur la sélection est une seule étape d’[Annuler](escenas.md#annuler) (« Déplacer 3 blocs »).

### Copier et coller

- **Ctrl+C** copie les blocs sélectionnés. **Ctrl+X** les coupe : il les copie et les enlève. Ceux des pistes verrouillées sont copiés, mais restent.
- **Ctrl+V** ou **« Coller »** les colle au curseur, à partir de la piste sélectionnée (sans piste sélectionnée, à partir de la première).
- **Clic droit** sur une zone vide d’une piste : **« Coller ici »** colle à cet endroit et sur cette piste. **« Tout sélectionner »** fait la même chose que **Ctrl+A**.
- Le premier bloc tombe sur le curseur et les autres gardent leurs écarts dans le temps. Celui de la piste la plus haute va sur la piste sélectionnée et les autres gardent leurs écarts en pistes.
- S’il manque des pistes, elles sont ajoutées à la fin. Une piste masquée qui reçoit des blocs s’affiche.
- Les blocs collés gardent leur durée, leur montée et leur descente, la boucle ou l’étirement et leur couleur. Ils arrivent déverrouillés, comme avec Dupliquer.
- Ce que vous collez reste sélectionné et le curseur passe à sa fin : appuyez de nouveau sur **Ctrl+V** et la copie suivante se place juste après.
- Vous pouvez coller dans une autre super scène du même show : ouvrez-la et collez. Les scènes ne sont pas copiées : le bloc collé joue la même scène.
- Un bloc audio collé dans la même super scène joue le même audio. Dans une autre super scène, il a son propre audio, lu dans le même fichier : le supprimer là-bas n’emporte jamais l’original. Si vous avez coupé le bloc, son audio est relu dans le fichier.
- Rien n’est collé sur une piste verrouillée, ni une scène qui joue cette super scène (par exemple, un rack qui la contient). Un message le signale.
- Ce que vous copiez est gardé tant que le show est ouvert. Ouvrir un autre show ou en commencer un nouveau le vide. Une scène que vous supprimez de la liste en sort.
- Coller et couper sont une seule étape d’[Annuler](escenas.md#annuler). Ce qui est collé pendant que la super scène joue s’entend à partir de la prochaine « Lecture ».

### Audio

Les blocs audio (wav, flac, ogg, aiff, et mp3 si le système le prend en charge) affichent leur **forme d’onde**, calculée en arrière-plan. Il faut une piste non verrouillée pour en ajouter un.

### Pistes et automation

Dans l’en-tête de chaque piste :

- **M** : muet (la piste ne joue pas).
- **Cadenas** : verrouiller (ses blocs et ses points ne bougent pas).
- **D** : la ligne d’automation du **dimmer** (sur une piste audio, le volume).
- **φ** : la ligne d’automation de la **phase** de ses effets.
- **⋯** : « Renommer » (aussi avec un double-clic sur le nom), « Monter », « Descendre », « Effacer l’automation de cette ligne » et « Supprimer la piste ».
- La flèche de droite masque ou affiche la piste.

Sur une ligne d’automation : un **clic** ajoute un point, le **faire glisser** le déplace, un **double-clic** l’efface et un **clic droit** alterne entre droite et courbe douce. Sans points, le dimmer est à fond et la phase à 0.

### Se déplacer dans la timeline

- Un clic ou un glissement sur la **règle** déplace le curseur ; un **double-clic** sur la règle lance la lecture à partir de là. Un clic sur une piste vide déplace aussi le curseur.
- **Ctrl+molette** : zoom. **Maj+molette** : se déplacer dans le temps. La molette seule fait défiler les pistes.

### Annuler

- **Ctrl+Z** annule la dernière modification. **Ctrl+Y** (ou **Ctrl+Maj+Z**) la rétablit. Les flèches de la barre d’outils font de même ; au survol, elles indiquent quelle modification elles annulent ou rétablissent.
- Chaque super scène a son propre historique, jusqu’à 100 étapes. Il dure tant que le show est ouvert : l’enregistrement ne l’efface pas, ouvrir un autre show ou en commencer un nouveau, si.
- Comptent les modifications des pistes, des blocs, de l’automation, de la grille, du BPM, de la fin, de la boucle et des valeurs statiques. Plusieurs modifications du même champ de la barre du bas (ou du BPM) en moins d’une seconde forment une seule étape.
- Ne comptent pas : masquer ou afficher des pistes, le zoom, le curseur, « Magnétisme » et la sélection.
- Pendant que vous tapez dans un champ, Ctrl+Z annule ce que vous avez tapé dans le champ.
- Cela fonctionne pendant la lecture de la super scène. Un bloc retiré s’arrête ; un bloc qui revient joue à partir du prochain « Lecture ».
- Un bloc audio revient avec son audio, relu depuis son fichier.
- Les blocs d’une scène supprimée de la liste ne reviennent pas.

## Cuelists

Une **cuelist** est une liste de cues qui avancent avec GO. Elle fonctionne en **tracking** : chaque cue ne garde **que ce qui change** et le reste vient des cues précédentes. Un GO ne change que les canaux qui changent : pas de creux, et ce qui n’est pas concerné n’est pas repris.

Avec d’autres cuelists et scènes de même priorité, chaque canal qui n’est pas une intensité (position, gobo, roue de couleur…) vient du dernier qui l’a changé : le GO qui le change le prend, et ce que la cuelist ne fait que garder des cues précédentes n’est pas repris. Quand il est libéré, il fait un fondu vers celui qui l’avait avant. Voir [Priorités](directo.md#priorités).

Les cuelists s’enregistrent depuis le programmeur, s’envoient dans [Playbacks](playbacks.md) et se modifient ici.

### Options de la cuelist

En haut de l’éditeur :

- **« Type »** : comment la cuelist joue. Une ligne à côté des boutons dit ce que fait le type choisi. Dans Playbacks et dans cet éditeur, une cuelist d’un autre type porte son étiquette : **SUB**, **CHASE**, **TC** ou **OVR**.
  - **« Normal »** (par défaut) : chaque GO lance la cue suivante.
  - **« Chase »** : les cues s’enchaînent seules, en boucle, à une vitesse (voir plus bas).
  - **« Timecode »** : chaque cue joue à son heure sur une horloge (voir plus bas).
  - **« Submaster »** : le fader de son playback décide (voir plus bas).
  - **« Override »** : son fader fait un fondu de tout ce qu’elle tient entre ce qui est dessous et sa cue (voir plus bas).
- **« Tracking »** : activé, chaque cue ne garde que ce qui change. Désactivé, chaque cue est indépendante.
- **« Après la dernière cue »** : **« Boucle »** (revient à la première), **« Libérer »** ou **« Rester »**.
- **« Temps de libération »** : le fondu à la libération de la cuelist.
- **« Réinitialiser en libérant »** : après une libération, GO repart de la première cue.
- **« Libérée par un autre GO »** : une chanson qui se termine quand la suivante commence.
- **« Fondu des nouvelles cues »** : le fondu de la première cue enregistrée ; les suivantes prennent les temps de la précédente.
- **« Retour »** : les temps de BACK : **« Temps de la cue »** ou **« Temps fixe »**.
- **« Priorité »**.
- **« GO sur le temps »** : « Non », « BeatGO » ou « Pulsation » donnent un GO à chaque division.
- **« Rythme tapé »** : le rythme tapé en appuyant plusieurs fois de suite sur son bouton, en BPM : un GO à chaque battement. Tant qu’elle en a un, ses déclenchements Suivre et Attendre ne comptent pas. **« Retirer »** revient à ses temps.
- **« Déplacement au noir »** : quand une cue est terminée, les fixtures éteints que la cue suivante allume prennent déjà sa position, sa couleur et son faisceau. Au GO, ils ne font que monter en intensité. **« Après »** est l’attente après la fin de la cue (0 s par défaut). **« Durée »** est le temps que prennent ces mouvements (2 s par défaut).
  - Un fixture qu’un autre playback ou la cue elle-même éclaire ne bouge pas.
  - Sur un fixture sans dimmer, son RGB est sa lumière : seuls ses autres attributs sont préparés.
  - Les canaux d’un FX ne sont pas préparés : le FX démarre à son GO.
  - Si vous choisissez une autre cue pour le prochain GO (« Suivant » dans [Playbacks](playbacks.md)), c’est celle-là qui est préparée.
  - Ne s’applique pas à une cuelist « Chase » ni « Override ».

Dans une fenêtre étroite, une partie des options est repliée : **« Plus d’options »** les affiche.

Une cuelist **« Submaster »** obéit au fader de son [playback](playbacks.md#le-fader) :

- Quand vous montez le fader depuis zéro, elle démarre sur sa cue suivante, au niveau du fader et jamais à fond.
- Quand vous le descendez à zéro, elle est libérée avec son temps de libération. Cela se change dans les [propriétés](playbacks.md#propriétés-dun-playback) du playback.
- Pendant qu’elle joue, GO et Retour changent de cue au niveau du fader.
- Libérée, GO et Retour choisissent seulement la cue sur laquelle elle démarrera (Pause/Retour et « Aller » aussi). Le playback l’affiche comme la suivante (« ▸ 3 »). Après la dernière cue, GO revient à la première ; avec « Rester », elle reste sur la dernière.
- Si vous la libérez avec le fader en haut, descendez le fader à zéro et remontez-le pour la lancer.
- Le flash la met à fond tant que vous appuyez.
- Dans un groupe de Direct et sur un bouton du SHOW, elle joue comme une cuelist normale. De même quand elle n’est sur aucun playback ou quand son fader ne la lance pas (« Le fader le lance » désactivé).

Une cuelist **« Chase »** parcourt ses cues toute seule, en boucle :

- **« Vitesse »** : un pas à chaque battement, en BPM (de 10 à 600 ; 120 par défaut). Avec **« BPM du show »**, elle suit le tempo du show. **×¼**, **×½**, **×1**, **×2** et **×4** la multiplient.
- **« Fondu »** : la part du pas que dure le changement, en %. Avec 0 (par défaut), c’est une coupure. Avec **« Temps de la cue »**, chaque cue prend son propre délai et son propre fondu.
- **« Sens »** : **« En avant »**, **« À l’envers »**, **« Aller-retour »** (fait demi-tour à la première et à la dernière cue) ou **« Aléatoire »** (ne joue jamais un pas deux fois de suite).
- GO passe tout de suite au pas suivant, et les suivants comptent à partir de là. Retour revient d’un pas dans l’autre sens. Pause la fige là où elle est.
- Un **« Rythme tapé »** l’emporte sur sa vitesse ; **« Retirer »** y revient. Avec BeatGO ou Pulsation, elle n’avance qu’à chaque division.
- La [vitesse des cues](playbacks.md#vitesse-des-cues) et la vitesse en direct l’accélèrent ou la ralentissent aussi.
- Le tracking reste valable : un pas peut ne changer que la couleur et garder le reste.
- Elle repart toujours au début : elle n’a ni colonne **DÉPART** ni **« Après la dernière cue »**, et les liens des cues ne comptent pas. Avec un fondu en %, les temps des cues sont grisés : ils ne servent pas.
- Dans une super scène, un bloc dure ses pas multipliés par la durée d’un pas. En « Aller-retour », l’aller et le retour.

Dans une cuelist **« Override »**, le fader de son [playback](playbacks.md#le-fader) la mélange avec ce qui est dessous :

- Le fader fait un fondu de tout ce qu’elle tient : intensités, couleurs, positions et gobos. À mi-course, les lyres sont à mi-chemin. À zéro, tout est comme si elle n’était pas là.
- À fond, sa cue remplace ce qui est dessous, sur les intensités aussi : un bleu sur un rouge donne du bleu, pas du magenta.
- Les cues, les GO et les fondus marchent comme dans une cuelist normale. Le fondu d’une cue et le fader se multiplient.
- Quand vous la libérez, tout revient en fondu vers ce qui est dessous, avec son temps de libération. Avec le fader à zéro, elle reste chargée, comme toute cuelist.
- Quand vous choisissez « Override », une cuelist à la priorité par défaut (5) passe à 10. Le programmeur reste au-dessus.
- Une cuelist ou une scène de la même priorité lancée après elle se mélange par-dessus comme toujours : sur les intensités, la valeur la plus haute l’emporte.
- Un canal avec **Fondus** désactivé dans les [limites du fixture](patch.md#limites) saute à sa valeur dès que le fader quitte zéro.
- Ses FX jouent comme dans une cuelist normale : le fader ne leur fait pas ce fondu.
- Si la cuelist joue déjà, le passage à « Override » (ou depuis « Override ») compte à partir de son prochain démarrage.
- Usages typiques : un blinder ou la lumière de salle, une position fixe pour un solo, une lumière de secours sur un fader.

Une cuelist **« Timecode »** lance chaque cue à son heure sur une horloge :

- Chaque cue a son **HEURE** (heures:minutes:secondes.centièmes). La colonne **HEURE** remplace **DÉPART**. Tapez l’heure (« 00:01:23.40 », « 1:23.4 » ou « 83.4 ») ou utilisez la molette. Si vous l’effacez, la cue n’a plus d’heure et l’horloge ne la lance pas.
- **« Horloge »** : **« Horloge propre »** ou **« Timecode MIDI (MTC) »** (voir plus bas). Avec l’horloge propre, GO lance la cuelist et démarre l’horloge à 00:00, ou à l’heure de la cue où elle se trouve (celle choisie avec « Aller à la cue » ou, avec « Réinitialiser en libérant » désactivé, la suivante après une libération).
- C’est la dernière cue de la liste dont l’heure est arrivée qui joue. Avant l’heure de la première cue, la cuelist ne donne rien.
- Quand elle passe à la cue suivante, la cue fait son fondu. Un saut (en arrière ou de plusieurs cues) va droit à l’aspect de cette heure, sans fondu.
- GO passe à la cue suivante avec son fondu, et l’horloge saute à son heure. Retour va à la cue précédente et à son heure, sans fondu (avec un temps fixe dans **« Retour »**, dans ce temps). « Aller à la cue » met l’horloge à son heure. Avec une cue sans heure, l’horloge continue comme avant.
- Pause arrête l’horloge. La [vitesse des cues](playbacks.md#vitesse-des-cues) et la vitesse en direct l’accélèrent ou la ralentissent.
- Ses Suivre, Attendre et liens ne comptent pas. « Après la dernière cue » vaut pour le GO sur la dernière cue.
- L’en-tête et son playback montrent l’horloge : **« TC 00:01:23.4 »**.
- **« Mettre à l’horloge »** (l’horloge de la barre d’outils) met la cue sélectionnée à l’heure de l’horloge pendant que la cuelist joue.
- Une cue enregistrée pendant que l’horloge tourne prend son heure. Avec la cuelist libérée ou en pause, elle prend l’heure de la cue précédente plus 5 s.
- Un triangle signale une cue dont l’heure est avant celle de la cue d’avant. Des deux, seule la dernière joue.
- Quand vous choisissez « Timecode », les cues sans heure en reçoivent une : 00:00 pour la première et 5 s de plus que la précédente pour les autres.
- Dans une [super scène](escenas.md#super-scènes), l’horloge est la timeline.

Avec **« Timecode MIDI (MTC) »**, la cuelist suit le timecode qu’un DAW, un lecteur vidéo ou un générateur de timecode envoie à une entrée MIDI :

- Activez l’entrée de l’appareil MIDI dans [Mappages › Appareils](mapeos.md#appareils). Aucun mappage n’est nécessaire.
- **« Décalage »** : le timecode où se trouve le 00:00 de la cuelist, en heures:minutes:secondes:images (par exemple « 01:00:00:00 »). Avant cette heure, la cuelist ne donne rien.
- À côté s’affiche le timecode qui arrive, **« MTC 01:00:12:05 · 25 fps »**, ou **« Aucun timecode MIDI »**.
- Fréquences d’images : 24, 25, 29,97 drop-frame et 30.
- Quand vous la lancez, la cuelist attend le timecode et va droit à la cue de cette heure. Si c’est la première cue, la cue fait son fondu.
- Si le timecode s’arrête ou se coupe, la cuelist reste sur sa cue. S’il revient d’une autre heure (un retour en arrière ou un saut), la cuelist va droit à la cue de cette heure, sans fondu.
- GO, Retour et « Aller à la cue » ne font rien : c’est le timecode qui mène. GO sur la cuelist libérée la lance.
- La vitesse des cues ne fait pas avancer l’horloge ; elle ne change que les fondus.
- Dans une super scène aussi, l’horloge est le timecode.

### Le tableau des cues

| Colonne | Ce que c’est |
|---|---|
| **CUE** | Le numéro. Il accepte les décimales : la 2.5 va entre la 2 et la 3. Double-clic pour le changer. Un crayon indique que la cue est chargée dans le programmeur. |
| **NOM** | Son nom. |
| **DÉPART** | **GO** (attend un GO), **Suivre** (quand la cue précédente est terminée, plus un temps) ou **Attendre** (un temps après le GO de la cue précédente). |
| **HEURE** | Seulement dans une cuelist « Timecode », à la place de DÉPART : l’heure de la cue sur l’horloge. Un triangle signale si elle est avant celle de la cue d’avant. |
| **DÉLAI** | Combien de temps la cue attend avant de commencer son fondu. |
| **FONDU** | Le temps de fondu. |
| **DESCENTE** | Le fondu des dimmers qui descendent. « = » est identique au fondu. |
| **CHANGEMENTS** | Le nombre de canaux qu’elle change, et les marques **FX**, **[R]** (valeurs libérées), l’icône de palette si elle utilise des palettes, une horloge si elle a ses propres temps ou une courbe, un maillon si elle a un lien et un éclair si elle a des actions (survolez-les pour les voir). |
| **S** | **Ignorer** (cette cue seulement) : ce qu’elle change ne passe pas aux suivantes. |
| **B** | **Bloc** : Enregistrer et Mettre à jour gardent aussi les valeurs égales à celles dont elle hérite. |
| **MIB** | Seulement avec « Déplacement au noir » : la cue est préparée pendant que ses fixtures sont éteints. Décochée, ses mouvements se voient à son GO. |
| **NOTE** | Une note pour vous. |

Pour changer un temps, cliquez dessus et tapez les secondes, ou utilisez la **molette** de la souris sur le temps cliqué : ±0,1 s (avec Maj, 1 s). Avec **Ctrl+clic** ou **Maj+clic**, vous sélectionnez plusieurs cues et vous changez leurs temps, leur départ ou leurs marques en même temps.

La cue en cours porte une barre verte avec l’avancement du fondu ; la suivante, un bord vert. En haut s’affichent l’état (« Q 3 · Couplet ▸ 4 », avec « ↺ 1/3 » si la suivante est le saut d’un lien, ou « Libérée · GO lance la 1 ») et les boutons **Retour**, **GO** et **libérer**, les mêmes que ceux de son playback.

Pendant le fondu d’une cue, l’en-tête affiche aussi **« Temps en direct »** : le temps de cette cue, que vous pouvez changer pendant qu’il s’écoule (en tapant un autre temps, avec **« ×½ »**, **« ×2 »** ou **« Maintenant »**), et le bouton qui l’enregistre dans la cue. Cela fonctionne comme dans [Playbacks](playbacks.md#temps-en-direct).

### Enregistrer et modifier des cues

- **« Enregistrer la cue N »** : le programmeur devient une nouvelle cue après celle sélectionnée, seulement avec ce qui change par rapport à ce dont elle hérite.
- **« Mettre à jour ▾ »** : fusionne le programmeur dans la cue sélectionnée : **« Mettre à jour la cue N (se propage aux suivantes) »** ou **« Cette cue seulement (les suivantes gardent leur aspect) »**.
- **« Charger ▾ »** (ou double-clic sur une cue) : la met dans le programmeur : **« Ce qu’on voit sur scène à la cue N »** ou **« Seulement ce que change la cue N »**. Ensuite, mettez-la à jour depuis la barre « Modification de la cue… » de Programmer.
- **« Retirer ▾ »** : avec les canaux du programmeur, **« Retirer de la cue N les canaux du programmeur (ils héritent à nouveau) »** ou **« [Libérer] les canaux du programmeur dans la cue N »** (à partir de cette cue, ils reviennent à ce qui est en dessous : d’autres scènes ou leur valeur de repos).
- **Copier** à la fin : **« Copier ce que change la cue N »** ou **« Copier tout l’aspect de la cue N (cue bloc) »**.
- **« Numéro… »** : un autre numéro pour la cue sélectionnée. Déplacer une cue change ce dont héritent les cues suivantes.
- **Supprimer** : avec **« Garder l’aspect des cues suivantes »** coché, ce que change la cue passe dans la suivante et les cues d’après restent identiques.

Dans une liste étroite (ou dans une langue aux mots longs), « Mettre à jour », « Charger » et « Retirer » n’affichent que leur icône et leur flèche **▾** ; leur nom apparaît au survol de la souris.

Le clic droit sur une cue propose **« Aller à la cue N »**, les deux façons de charger, **« Mettre à jour avec le programmeur »**, **« Temps… »**, **« Lien et actions… »**, **« Numéro… »** et **« Supprimer… »**.

Les cues peuvent utiliser des **palettes par référence** et porter des **couches de FX**, qui se transmettent de cue en cue.

### La feuille de tracking

**« Détails de la cue »** a trois onglets : **« Tracking »**, **« Temps »** et **« Lien et actions »**. Si la fenêtre est large, le panneau s’affiche de lui-même à côté des cues.

**« Tracking »** montre ce qu’il y a sur le plateau à la cue sélectionnée, fixture par fixture (« Sur scène : cue N »).

- Les pastilles de couleur indiquent ce qu’est chaque valeur : **« Changements »** (cette cue la change), **« Hérités »** (elle vient d’une cue précédente, indiquée : « Q2 »), **« Bloc »** et **« Libérés »** (barrés, avec [R]).
- En haut s’affichent les palettes que la cue utilise.
- Cliquez sur des canaux pour les choisir et appuyez sur **« Libérer »** (les met en [RELEASE] à partir de cette cue) ou **« Retirer »** (ils héritent à nouveau des cues précédentes).

### Temps de la cue

Dans une cue, chaque attribut et chaque fixture peuvent avoir leur propre délai et leur propre fondu. Sélectionnez une cue (ou plusieurs) et ouvrez **« Détails de la cue »** › **« Temps »**, ou clic droit › **« Temps… »**. Avec plusieurs cues sélectionnées, les changements valent pour toutes.

- **« Temps par attribut »** : **Intensité**, **Couleur**, **Position** et **Faisceau**, chacun avec DÉLAI et FONDU. Vide ou « = » est le temps de la cue. Le stroboscope va avec Intensité ; gobo, prisme, zoom, focus et iris vont avec Faisceau.
- **« Temps par fixture »** : **« + Fixtures sélectionnés »** ajoute les fixtures sélectionnés dans Programmer, avec les temps de la cue. Changez ensuite leurs temps. **×** en retire un et **« Retirer »** les retire tous.
- Le temps d’un fixture l’emporte sur celui de son attribut, et celui d’un attribut l’emporte sur celui de la cue.
- Les dimmers qui descendent prennent le fondu de leur fixture, s’il en a un. Sinon, la DESCENTE de la cue ; avec « = », le fondu d’Intensité.
- Une cue dure autant que son temps le plus long : Suivre, la barre verte et la durée dans une super scène en tiennent compte. Le temps d’un attribut que la cue ne change pas compte aussi.
- Un seul temps pour tout, comme avant : le fondu d’entrée d’un groupe ou d’un bloc de super scène sur la première cue, et un temps fixe de **« Retour »**. Avec **« Temps de la cue »**, BACK prend tous les temps de la cue vers laquelle il revient.
- Les couches de FX entrent avec le fondu de la cue.
- Une nouvelle cue prend les temps d’attribut de la cue précédente, mais pas ceux de ses fixtures. Copier une cue copie tous ses temps.

Exemple : la cue 2 change la couleur et la position avec un fondu de 0, et Position a un délai de 1 s et un fondu de 3 s. Au GO, la couleur change aussitôt ; une seconde plus tard, les lyres bougent pendant 3 s.

**Courbes.** **« Courbe des fondus »** dit comment avancent les fondus de la cue :

- **Linéaire** (par défaut) : la même vitesse tout du long.
- **Douce** : lente au début et à la fin.
- **Départ lent** : elle bouge à peine au début et accélère à la fin.
- **Départ rapide** : elle avance presque d’un coup, puis se pose.

La courbe vaut pour tout ce que la cue fait en fondu : dimmers qui montent et qui descendent, couleurs, mouvements et canaux qu’elle libère. Chaque attribut a aussi sa **COURBE** ; « = » est celle de la cue. Le temps de libération de la cuelist, un temps fixe de **« Retour »**, le fondu d’entrée d’un groupe et les couches de FX sont toujours linéaires. L’horloge de la colonne CHANGEMENTS apparaît aussi si la courbe de la cue n’est pas linéaire.

**Éventail de temps.** Dans **« Temps par fixture »**, **« Éventail »** répartit un délai ou un fondu sur plusieurs fixtures :

- Choisissez **Délai** ou **Fondu**, et les temps **de** et **à**.
- Choisissez la forme : **L’un après l’autre**, **Depuis le centre** ou **Depuis les extrémités**.
- Choisissez les fixtures : **« Sélectionnés dans Programmer »**, dans l’ordre où vous les avez sélectionnés, ou **« De cette cue, par numéro »**.
- **« Appliquer »** donne à chaque fixture son temps dans les cues sélectionnées. Son autre temps ne change pas.

Avec 5 fixtures et un délai de 0 à 2 s, **L’un après l’autre** donne 0 s, 0,5 s, 1 s, 1,5 s et 2 s. **Depuis le centre** donne 2 s, 1 s, 0 s, 1 s et 2 s. **Depuis les extrémités** donne 0 s, 1 s, 2 s, 1 s et 0 s. Si « de » est plus grand que « à », l’ordre s’inverse. L’éventail écrit des temps fixes, pas une règle : vous pouvez ensuite changer chaque ligne.

### Liens et actions

Sélectionnez une cue et ouvrez **« Détails de la cue »** › **« Lien et actions »**, ou clic droit › **« Lien et actions… »**. Avec plusieurs cues sélectionnées, vous modifiez la dernière choisie.

**Lien.** **« Après cette cue »** indique la cue jouée après elle : **« La cue suivante »** (le cas normal) ou une autre cue de la liste. **« Nombre de fois »** indique combien de fois le lien saute avant de continuer ; 0 veut dire toujours. Un nouveau lien commence à 1.

- Avec cue 5 → cue 2, 3 fois, les GO jouent 1 2 3 4 5, 2 3 4 5, 2 3 4 5, 2 3 4 5 et 6 : les cues 2 à 5 jouent 4 fois. En dessous, une phrase l’explique : « Les cues 2 à 5 jouent 4 fois, puis la 6. »
- Avec 0 (toujours), les cues se répètent jusqu’à ce que vous alliez à une autre cue.
- Un lien vers l’avant saute des cues ; un lien vers la cue elle-même la répète.
- Le départ de la cue visée s’applique : si la cue 2 a Suivre, la boucle avance toute seule.
- Cette cue est la suivante partout : le bord vert du tableau, l’en-tête (« ▸ 2 ↺ 1/3 » : le premier saut sur trois ; seulement « ↺ » si c’est toujours), les playbacks, le SHOW, le téléphone et les LED du contrôleur.
- BACK et « Aller à la cue » ne comptent pas de saut. Libérer la cuelist, ou la relancer, remet le compte à zéro.
- Une fois tous les sauts faits, la liste continue après la cue. La prochaine fois qu’elle y arrive, le compte repart de zéro.
- Sur la dernière cue, le lien passe avant **« Après la dernière cue »**.
- Dans une cuelist « Chase » ou « Timecode », les liens ne comptent pas : l’onglet le dit. Le lien est gardé pour quand la liste redevient « Normal ».
- Si vous supprimez la cue visée, le lien disparaît et un message le dit.

**Actions au départ.** **« + Ajouter une action ▾ »** ajoute ce que fait la cue quand elle démarre :

- **« GO d’une cuelist »**, **« Aller à une cue… »** d’une autre cuelist et **« Libérer une cuelist »**.
- **« Lancer une scène »** et **« Arrêter une scène »**, dans son groupe si elle en a un (comme les boutons du SHOW). Elles ne changent pas la scène sélectionnée.
- **« Libérer un groupe »** (ou tous) et **« Libérer tous les playbacks »**.
- **« Régler le BPM »**, de 20 à 300.

Chaque ligne se modifie sur place ; la **×** la retire. Les actions se font dans l’ordre quand la cue démarre par GO, Suivre, Attendre ou « Aller à la cue », y compris la première cue au lancement de la cuelist. Elles ne se font pas avec BACK, quand une modification réapplique la cue, ni quand une super scène démarre la cuelist en cours de route.

- La cuelist elle-même n’apparaît pas : c’est le rôle du lien.
- Une action sur une scène ou une cuelist supprimée affiche « (supprimée) » et ne fait rien.
- Si deux cuelists se lancent l’une l’autre, la boucle s’arrête : une cuelist lancée par une action ne lance pas, avec ses actions, une cuelist plus haut dans cette chaîne. De plus, la même cue ne refait pas ses actions avant 200 ms.
- Copier une cue ne copie ni son lien ni ses actions.

### Annuler

Les flèches **Annuler** et **Rétablir**, après la corbeille, annulent et rétablissent les modifications de la cuelist, une à une. **Ctrl+Z** et **Ctrl+Y** (ou **Ctrl+Maj+Z**) aussi. Au survol de la souris, elles disent ce qu’elles annulent : « Annuler : Enregistrer la cue 3 ».

- Tout ce qui change les cues ou les options de la cuelist peut être annulé : enregistrer (aussi depuis Programmer, un playback ou la ligne de commande), mettre à jour, retirer, libérer, copier, supprimer, le numéro, les temps, le départ, le nom, la note, les marques S et B, le lien, les actions et les options du haut, sauf « GO sur le temps » et « Rythme tapé ».
- Chaque cuelist garde ses 50 dernières étapes.
- Plusieurs crans de molette (ou valeurs saisies) sur le même temps, à moins de 2 secondes d’intervalle, forment une seule étape. Si vous revenez à la valeur de départ, l’étape disparaît.
- Après une annulation, la première cue qui change est sélectionnée. Si la cuelist joue, le plateau change aussitôt.
- Dans un champ avec du texte saisi, Ctrl+Z annule d’abord ce texte.
- Créer la cuelist et convertir des pas en cuelist ne s’annulent pas. Annuler garde les valeurs actuelles des palettes et ne fait pas revenir les canaux des fixtures supprimés.
- Une priorité ou un temps de libération changés ailleurs (Direct, Playbacks) restent tels quels.
- L’historique n’est pas enregistré avec le show. Nouveau et Ouvrir le vident, et supprimer la cuelist aussi.

### Convertir des pas ou un chenillard en cuelist

Dans une scène à pas, le bouton **« Convertir en cuelist »** de l’en-tête transforme chaque pas en une cue avec ce qui change par rapport au précédent ; les pas temporisés s’enchaînent avec **Attendre**. Ses groupes, boutons du SHOW et mappages jouent désormais la cuelist. Les pas disparaissent. Les cues continuent d’utiliser les palettes des pas.

Dans un chenillard, chaque cue porte la scène de son pas (les Move FX deviennent des FX) et libère ce que le pas précédent avait et celui-ci non ; les pas qui ne sont ni des scènes ni des effets donnent une cue vide avec une note. Ses scènes restent.

Un ping-pong fait l’aller et le retour ; un ordre aléatoire joue dans l’ordre. Les temps en beats passent en secondes au BPM du moment. Les pas d’une copie pour QLC+ reviennent avec le numéro, le nom et la note de chaque cue.

### Cuelists et QLC+

QLC+ ne lit pas les cuelists : si vous ouvrez ce show dans QLC+, elles n’y sont pas. Pour les emmener dans QLC+, appuyez sur **« QLC+ »** dans l’en-tête d’une cuelist. **« Enregistrer une copie pour QLC+ »** écrit un autre `.qxw` dans lequel chaque cuelist cochée est une liste de pas de QLC+ :

- **Sequence** (par défaut) : un seul élément dans QLC+.
- **Chaser** : une scène par cue, dans un dossier au nom de la cuelist.

Avec **« Une liste de cues pour chacune dans la Console virtuelle »** cochée, la copie s’ouvre sur la Console virtuelle de QLC+, avec un cadre « Cuelists » et une liste de cues par cuelist : « Suivant » est le GO, « Précédent » le BACK et « Stop » la libère. Votre show ne change pas.

La case **« Avec leurs touches et boutons du contrôleur »** donne à chaque liste de cues les touches et les boutons du contrôleur de sa cuelist : GO, Retour, Pause/Retour et Libérer deviennent « Suivant », « Précédent », « Lecture » et « Stop ». Comptent les mappages de la cuelist et ceux du playback qui la porte sur la page actuelle. Chaque bouton de QLC+ prend une touche et une entrée. Les contrôles OSC et ceux d’un appareil non activé ne passent pas ; la fenêtre le signale.

Chaque pas contient tout ce qu’on voit sur scène à cette cue, et sa note donne le numéro et le nom (« Q2.5 · Couplet »). Le fondu de la cue devient le fondu d’entrée du pas, et la descente, son fondu de sortie. GO laisse le pas attendre « Suivant » ; Attendre et Suivre deviennent un temps. Dans une cuelist « Chase », les pas s’enchaînent seuls, en boucle et dans son sens : chacun dure un pas de sa vitesse, avec son fondu. Dans une cuelist « Timecode », chaque pas dure jusqu’à l’heure de la cue suivante. Les pas gardent l’ID de la cuelist : les super scènes et les racks qui la jouent jouent les pas.

Sous chaque cuelist, la fenêtre indique ce qui ne passe pas :

- « Chase » avec « BPM du show » ou avec un rythme tapé : les pas gardent un BPM fixe, selon son multiplicateur : celui du show à ce moment avec « BPM du show », sinon celui de la cuelist. Ils ne suivent pas le rythme tapé.
- « Timecode » : les pas défilent dès le premier GO ; ils ne suivent ni le timecode MIDI ni la timeline d’une super scène. Le premier pas joue dès le GO, et non à l’heure de sa cue. Sur une cue sans heure, ils s’arrêtent jusqu’à un GO.
- « Submaster » : les pas démarrent avec un GO, pas avec le fader.
- « Override » : les pas se mélangent à ce qui est dessous comme les autres.
- Les FX : seulement leurs valeurs de base.
- Le délai, aussi celui d’un attribut ou d’un fixture : le pas commence aussitôt.
- Une descente différente du fondu : seulement approximative.
- Les temps par attribut et par fixture : seulement le fondu de la cue.
- Les courbes des fondus : seulement linéaires.
- Un Attendre plus court que le fondu, ou en « Chase » et « Timecode » un pas plus court que son fondu : le pas suivant attend la fin du fondu.
- Les liens : les pas jouent dans l’ordre. Si la dernière cue a toujours un lien vers la première, les pas recommencent.
- Les actions des cues : elles ne s’exécutent pas.
- En Sequence, les valeurs libérées vont à leur valeur de repos. En Chaser, elles sont laissées à QLC+.
- Avec « Rester », GO sur la dernière cue arrête les pas.
- GO sur le temps et rythme tapé : les pas ne suivent pas le rythme.
- « Déplacement au noir » : les fixtures bougent pendant qu’ils montent.
- « Libérée par un autre GO » : ces listes vont dans un cadre « Cuelists (une à la fois) », où en lancer une arrête les autres.

Une cuelist qui a déjà sa liste de cues dans la Console virtuelle (un show de QLC+ dont vous avez converti les pas en cuelist) n’en reçoit pas d’autre : l’ancienne la relance. Essayez la copie dans QLC+ avant de l’utiliser en direct : QLC+ fait ses fondus à sa façon.

### Cue lists de QLC+

**« Nouvelle » › « Cue lists de QLC+… »** convertit en cuelists les chenillards, les séquences et les listes de cues de la Console virtuelle d’un show QLC+ : du show ouvert ou d’un autre (« D’un autre show… »). Cochez celles que vous voulez convertir. Les listes de cues de la Console virtuelle viennent d’abord, cochées ; puis les chenillards et les séquences qu’aucune liste n’utilise. Celles qui ont déjà une cuelist à leur nom ne sont pas cochées.

Chaque pas devient une cue avec ce qu’il change ; les pas temporisés s’enchaînent avec Attendre et ceux qui attendent sont des GO. Une liste de cues de la Console virtuelle donne son nom à la cuelist.

- **« Remplacer les originaux »** (seulement dans le show ouvert) : la cuelist prend leur place et leur ID. Leurs groupes, boutons du SHOW et mappages passent à la cuelist, mais QLC+ ne les voit plus. Sans cette case, les originaux restent et le show joue toujours pareil dans QLC+.
- **« Les mettre sur les playbacks libres de cette page »**. Si elles ne tiennent pas toutes, le message dit combien restent dehors.
- **« Importer leurs touches et boutons du contrôleur comme mappages »** : « Suivant », « Précédent », « Lecture » et « Stop » de chaque liste de cues deviennent GO, Retour, Pause/Retour et Libérer de sa cuelist. Ne passent pas les contrôles OSC, les touches avec Ctrl, Alt ou Maj, les entrées d’un univers sans contrôleur dans ce show ni les contrôles qui ont déjà un mappage. Le message dit combien de mappages sont créés et quels contrôles restent dehors. Un bouton d’une Akai APC arrive avec ses LED, comme quand vous l’apprenez.

D’un autre show, seules les cuelists entrent, sur les fixtures de ce show qui correspondent aux siens (d’abord la même adresse et le même modèle). Le message dit combien de fixtures manquent.

Ne passent pas : l’ordre aléatoire (elles jouent dans l’ordre), les beats (ils passent en secondes au BPM actuel), le fader latéral des listes de cues et les pas qui ne sont ni des scènes ni des effets. Les listes de cues de l’ancien format de QLC+ ne peuvent pas être converties : ouvrez-les et enregistrez-les d’abord dans QLC+.

À l’ouverture d’un show QLC+ avec des listes de cues et sans cuelists, le message « … ouvert » rappelle où les convertir.

## Dossiers

Les dossiers rangent la liste des scènes (aussi dans le SHOW, dans les super scènes, dans les racks et quand vous ajoutez des scènes à un groupe).

- **Créer** : le bouton de dossier avec **+** à côté du champ de recherche, ou **« Nouveau dossier… »** dans le menu d’une scène.
- **Ranger des scènes** : faites-les glisser sur le dossier, ou clic droit › **« Déplacer vers le dossier »**. Cela fonctionne avec plusieurs scènes sélectionnées.
- **Sortir** : clic droit › **« Retirer de son dossier »**.
- **Renommer ou supprimer** : le bouton **⋯** du dossier (ou clic droit) : **« Renommer le dossier »** et **« Supprimer le dossier (ses scènes restent) »**.
- Cliquez sur l’en-tête d’un dossier pour le replier ou le déplier.

Les dossiers sont enregistrés avec le show.

## Importer et exporter

- **Exporter** : le bouton d’export de l’en-tête enregistre la scène dans un fichier `.dtcfx` (« Effets DMX Total Control ») pour la charger dans d’autres shows. Cela fonctionne aussi avec les racks.
- **Importer** : **« Nouvelle » › « Importer depuis un fichier… »**. Les fixtures sont associés à ceux de ce show ; si l’un de ceux du fichier n’y est pas, sa partie est ignorée et un avertissement s’affiche.
- **Les scènes de tout un autre show** : dans [Patch › Importer](patch.md#importer-dun-autre-show), « Scènes d’un autre show… ».
- **Pour QLC+** : « QLC+ » dans l’en-tête d’une cuelist ([Cuelists et QLC+](escenas.md#cuelists-et-qlc)).
- **Depuis QLC+** : « Nouvelle » › « Cue lists de QLC+… » ([Cue lists de QLC+](escenas.md#cue-lists-de-qlc)).
