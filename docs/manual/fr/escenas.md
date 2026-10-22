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

## L’en-tête de l’éditeur

- **Le nom** : cliquez dessus pour le changer.
- **Le dossier** (icône de dossier) : déplacer la scène dans un dossier.
- **« Essayer »** / **« Arrêter »** : la joue maintenant, hors des groupes. Les cuelists ont leur propre GO.
- **« Ajouter au groupe… »** : l’ajoute à un groupe de Direct.
- **Convertir en pas** (racks, FX, Colour FX, Move FX, Curve FX et chenillards) : enregistre un cycle dans une nouvelle scène à pas, seulement avec les canaux auxquels la scène donne une valeur : les autres, même s’ils ont une valeur par défaut (le pan ou le zoom au centre sur beaucoup de lyres), n’entrent pas dans les pas.
- **Convertir en cuelist** (pas) : voir [Cuelists](escenas.md#cuelists).
- **Exporter**, **« Dupliquer »** et **« Supprimer »** (elle est aussi retirée de ses groupes et racks).

## Comment joue une scène

Sous l’en-tête se trouve le panneau de lecture. Il affiche ce qui a du sens pour chaque type :

- **« Montée »** et **« Descente »**.
- **« Temps de cycle »** (ou **« Temps par pas »** dans un Colour FX) et **« Vitesse »** (d’un cycle par minute à dix par seconde ; double-clic : un cycle par seconde), avec **×½** et **×2**.
- **« Fondu décalé »** (statiques et pas) : chaque fixture commence son fondu ce temps-là après le précédent, dans l’ordre de sélection.
- **« Piloté par »** : **« Durée »** (les temps normaux), **« BPM »** (les temps passent en beats et suivent le tempo du show, les fondus de chaque pas aussi ; le temps de fondu enchaîné d’un groupe du Direct et les fondus des blocs d’une super scène restent en secondes), **« BeatGO »** ou **« Pulsation »** (avance avec le bouton BeatGO ou avec les pics de l’entrée audio). Avec BeatGO ou Pulsation, **« Divisions »** (/2, ×2).
- **« Rythme tapé »** (séquences et chenillards) : le rythme tapé en appuyant plusieurs fois de suite sur son bouton ([Direct](directo.md#rythme-dune-scène-à-pas)), en BPM. Tant qu’elle en a un, ses pas suivent ce rythme et non leurs temps. **« Retirer »** revient à ses temps. Avec « BPM », un beat est un pas de ce rythme : un fondu de 0,5 b dure un demi-pas.
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

**Ctrl+clic** et **Maj+clic** sélectionnent plusieurs pas : quand vous changez un temps, il change sur tous. Les pas appliquent les palettes par valeur, pas par référence.

### FX

Une scène FX a des **valeurs de base** et une ou plusieurs **couches de FX**. Elle se crée en enregistrant depuis le programmeur avec des FX (voir [FX](programar.md#fx)).

- À gauche, la liste des FX : paramètre, fixtures et vitesse. La corbeille retire une couche.
- À droite, les contrôles de la couche choisie (les mêmes que dans Programmer) et son aperçu animé. Les changements se voient en direct pendant que la scène joue.
- **« Lier »** : les couches liées partagent la vitesse et le timing.
- Si certains fixtures font le FX à leur façon, un avertissement le signale : si vous changez ici la forme, la taille, le sens ou la phase, tous feront la même chose.
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

Ils jouent toujours de la même façon et ont leur éditeur (forme, largeur, hauteur, rotation, position, phase, répartition entre fixtures, symétrique, relatif…). Un avertissement propose **« Convertir en FX »** : ils passent dans l’éditeur de FX, aussi dans le programmeur, et les groupes, les boutons du SHOW et les mappages les gardent.

### Rack d’effets

Plusieurs scènes et effets qui jouent ensemble comme un seul, chacun sur ses fixtures.

- Dans **« Ajouter »** (à droite), cliquez sur une scène pour la mettre dans le rack.
- Dans **« Dans le rack (N) »**, la **×** la retire.

Un rack ne peut pas contenir une scène qui le joue lui-même.

### Chenillards

Les chenillards des shows QLC+ jouent normalement, mais n’ont pas d’éditeur. Vous pouvez les convertir en pas avec le bouton de conversion de l’en-tête.

## Super scènes

Une **super scène** est une timeline avec des pistes de scènes et d’audio. Elle se lance comme n’importe quelle scène (dans un groupe, dans le SHOW, avec un mappage). Son éditeur occupe toute la page ; le bouton de liste, en haut à gauche, revient à la liste des scènes.

### La barre d’outils

- **« Lecture »** / **« Arrêter »** (**Espace**) : depuis le curseur. **⏮** revient au début. À côté s’affichent le temps (et la mesure).
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

### Travailler avec les blocs

- **Faites glisser** un bloc pour le déplacer, aussi vers une autre piste. Faites glisser ses **bords** pour changer sa durée.
- Avec le bloc sélectionné, les **poignées** des coins supérieurs sont sa montée et sa descente.
- Quand vous allongez un bloc, la scène **se répète en boucle** (des repères marquent chaque tour) ou **s’étire** pour qu’un cycle dure tout le bloc. Cela se change dans le menu du bloc ou dans la barre du bas (« En boucle » / « Étirée »).
- **Clic droit** : « Répéter la scène dans le bloc » / « Étirer la scène à la taille du bloc », « Dupliquer », « Verrouiller » / « Déverrouiller », « Ouvrir la scène » et « Supprimer ».
- Un **double-clic** sur un bloc ouvre sa scène. **Suppr** supprime le bloc sélectionné.
- La barre du bas affiche le bloc sélectionné : **Début**, **Durée**, **Montée**, **Descente**.

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

## Cuelists

Une **cuelist** est une liste de cues qui avancent avec GO. Elle fonctionne en **tracking** : chaque cue ne garde **que ce qui change** et le reste vient des cues précédentes. Un GO ne change que les canaux qui changent : pas de creux, et ce qui n’est pas concerné n’est pas repris.

Les cuelists s’enregistrent depuis le programmeur, s’envoient dans [Playbacks](playbacks.md) et se modifient ici.

### Options de la cuelist

En haut de l’éditeur :

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

Dans une fenêtre étroite, une partie des options est repliée : **« Plus d’options »** les affiche.

### Le tableau des cues

| Colonne | Ce que c’est |
|---|---|
| **CUE** | Le numéro. Il accepte les décimales : la 2.5 va entre la 2 et la 3. Double-clic pour le changer. Un crayon indique que la cue est chargée dans le programmeur. |
| **NOM** | Son nom. |
| **DÉPART** | **GO** (attend un GO), **Suivre** (quand la cue précédente est terminée, plus un temps) ou **Attendre** (un temps après le GO de la cue précédente). |
| **DÉLAI** | Combien de temps la cue attend avant de commencer son fondu. |
| **FONDU** | Le temps de fondu. |
| **DESCENTE** | Le fondu des dimmers qui descendent. « = » est identique au fondu. |
| **CHANGEMENTS** | Le nombre de canaux qu’elle change, et les marques **FX**, **[R]** (valeurs libérées) et l’icône de palette si elle utilise des palettes. |
| **S** | **Ignorer** (cette cue seulement) : ce qu’elle change ne passe pas aux suivantes. |
| **B** | **Bloc** : Enregistrer et Mettre à jour gardent aussi les valeurs égales à celles dont elle hérite. |
| **NOTE** | Une note pour vous. |

Pour changer un temps, cliquez dessus et tapez les secondes, ou utilisez la **molette** de la souris sur le temps cliqué : ±0,1 s (avec Maj, 1 s). Avec **Ctrl+clic** ou **Maj+clic**, vous sélectionnez plusieurs cues et vous changez leurs temps, leur départ ou leurs marques en même temps.

La cue en cours porte une barre verte avec l’avancement du fondu ; la suivante, un bord vert. En haut s’affichent l’état (« Q 3 · Couplet ▸ 4 » ou « Libérée · GO lance la 1 ») et les boutons **Retour**, **GO** et **libérer**, les mêmes que ceux de son playback.

### Enregistrer et modifier des cues

- **« Enregistrer la cue N »** : le programmeur devient une nouvelle cue après celle sélectionnée, seulement avec ce qui change par rapport à ce dont elle hérite.
- **« Mettre à jour ▾ »** : fusionne le programmeur dans la cue sélectionnée : **« Mettre à jour la cue N (se propage aux suivantes) »** ou **« Cette cue seulement (les suivantes gardent leur aspect) »**.
- **« Charger ▾ »** (ou double-clic sur une cue) : la met dans le programmeur : **« Ce qu’on voit sur scène à la cue N »** ou **« Seulement ce que change la cue N »**. Ensuite, mettez-la à jour depuis la barre « Modification de la cue… » de Programmer.
- **« Retirer ▾ »** : avec les canaux du programmeur, **« Retirer de la cue N les canaux du programmeur (ils héritent à nouveau) »** ou **« [Libérer] les canaux du programmeur dans la cue N »** (à partir de cette cue, ils reviennent à ce qui est en dessous : d’autres scènes ou leur valeur de repos).
- **Copier** à la fin : **« Copier ce que change la cue N »** ou **« Copier tout l’aspect de la cue N (cue bloc) »**.
- **« Numéro… »** : un autre numéro pour la cue sélectionnée. Déplacer une cue change ce dont héritent les cues suivantes.
- **Supprimer** : avec **« Garder l’aspect des cues suivantes »** coché, ce que change la cue passe dans la suivante et les cues d’après restent identiques.

Dans une liste étroite (ou dans une langue aux mots longs), « Mettre à jour », « Charger » et « Retirer » n’affichent que leur icône et leur flèche **▾** ; leur nom apparaît au survol de la souris.

Le clic droit sur une cue propose **« Aller à la cue N »**, les deux façons de charger, **« Mettre à jour avec le programmeur »**, **« Numéro… »** et **« Supprimer… »**.

Les cues peuvent utiliser des **palettes par référence** et porter des **couches de FX**, qui se transmettent de cue en cue.

### La feuille de tracking

**« Feuille de tracking »** montre ce qu’il y a sur le plateau à la cue sélectionnée, fixture par fixture (« Sur scène : cue N »). Si la fenêtre est large, elle s’affiche d’elle-même à côté des cues.

- Les pastilles de couleur indiquent ce qu’est chaque valeur : **« Changements »** (cette cue la change), **« Hérités »** (elle vient d’une cue précédente, indiquée : « Q2 »), **« Bloc »** et **« Libérés »** (barrés, avec [R]).
- En haut s’affichent les palettes que la cue utilise.
- Cliquez sur des canaux pour les choisir et appuyez sur **« Libérer »** (les met en [RELEASE] à partir de cette cue) ou **« Retirer »** (ils héritent à nouveau des cues précédentes).

### Convertir des pas en cuelist

Dans une scène à pas, le bouton **« Convertir en cuelist »** de l’en-tête transforme chaque pas en une cue avec ce qui change par rapport au précédent ; les pas temporisés s’enchaînent avec **Attendre**. Ses groupes, boutons du SHOW et mappages jouent désormais la cuelist. Les pas disparaissent.

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
