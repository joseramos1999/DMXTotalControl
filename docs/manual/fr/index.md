# Manuel de DMX Total Control

DMX Total Control est un logiciel libre pour piloter l’éclairage DMX depuis un ordinateur sous Windows. Il est pensé pour le live : vous lancez des scènes par groupes, vous programmez vite avec le programmeur, vous envoyez des cuelists avec GO sur les playbacks et, si vous le souhaitez, vous construisez votre propre écran tactile (le SHOW) et vous l’utilisez aussi depuis votre téléphone.

Avec DMX Total Control, vous pouvez :

- Patcher les fixtures de votre installation avec une bibliothèque de milliers de définitions, plus l’Open Fixture Library, l’import GDTF et vos propres fixtures.
- Donner dimmer, couleur, position, gobos et n’importe quel autre canal aux fixtures sélectionnés, avec des palettes, des FX et une ligne de commande.
- Enregistrer des scènes statiques, des pas, des FX, des super scènes avec timeline et audio, et des cuelists avec tracking.
- Les lancer dans Direct par groupes, sur les Playbacks avec GO ou depuis votre SHOW.
- Sortir le DMX par des interfaces DMX USB, Art-Net ou sACN, et tout piloter depuis MIDI, OSC, une entrée DMX ou le clavier.

Les shows sont enregistrés dans des fichiers `.qxw`, le même format que celui de QLC+ : vous pouvez donc ouvrir des shows faits avec QLC+.

## Comment ouvrir ce manuel

Appuyez sur **F12** ou sur le bouton **« ? »** de la barre du haut. Le manuel s’ouvre sur le côté droit de la fenêtre, à la page qui explique la page où vous êtes : si vous êtes dans Programmer, il s’ouvre sur [Programmer](programar.md). Les liens mènent à d’autres pages du manuel ; depuis le panneau, vous pouvez revenir à la page précédente ou à ce « Sommaire ». Il s’ouvre aussi depuis Options › Aide › **« Ouvrir le manuel »**.

## Le guide et les shows d’exemple

- **Le guide de premiers pas** : une carte en bas à droite qui vous fait parcourir les pages en cinq étapes (les fixtures, un look, une scène, Direct et les playbacks). Il apparaît la première fois que vous ouvrez le programme ; ensuite, dans Options › Aide › **« Lancer le guide »**. Les pages restent utilisables pendant qu’il est ouvert. Voir [Premiers pas](primeros-pasos.md).
- **Les shows d’exemple** : des shows déjà faits, à ouvrir pour essayer. Ils sont dans la flèche **▾** à côté d’« Ouvrir », dans **« Exemples »** :
  - **Primeros pasos** : le show d’entraînement du guide, avec 4 PAR, 2 lyres et le groupe « Mis escenas » vide.
  - **Bolo pequeño** : des PAR devant et derrière, des stroboscopes et une machine à fumée. Couleurs, effets et pleins feux dans trois groupes, et une page de SHOW construite avec eux.
  - **Concierto con cabezas móviles** : des palettes de couleur et de position, un cercle en miroir et une chanson en cuelist sur les playbacks.
  - **Sala con playbacks** : une cuelist par chanson sur la première page de playbacks, et un SHOW avec ces playbacks.

Les exemples s’ouvrent comme n’importe quel show : si vous enregistrez des modifications, utilisez « Enregistrer sous… » pour laisser l’exemple tel qu’il était.

## La fenêtre du programme

La fenêtre a trois parties : la barre du haut, la barre latérale avec les pages, et la page que vous utilisez.

### Les pages

| Page | Touche | À quoi elle sert |
|---|---|---|
| Direct | F1 | Lancer les scènes par groupes et utiliser le mixeur |
| Show | F2 | Votre écran de show : boutons, faders, rotatifs, groupes, horloge… |
| Playbacks | F10 | Des faders avec GO, pause et flash pour les cuelists et les scènes |
| Programmer | F3 | Sélectionner des fixtures, leur donner des valeurs, des FX et des palettes, et enregistrer |
| Scènes | F4 | Créer et modifier des scènes, des effets, des super scènes et des cuelists |
| Patch | F5 | Les fixtures de l’installation, leurs adresses, les groupes et le plan 2D |
| Fixtures | F6 | Créer vos propres définitions de fixtures |
| Sorties | F7 | Univers et interfaces DMX |
| Mappages | F8 | Contrôleurs MIDI, OSC, entrée DMX et clavier |
| Options | F9 | Réglages du programme, bibliothèque de fixtures et mises à jour |

**F11** met la fenêtre en plein écran ou lui rend sa taille. Si une page est verrouillée, elle porte un cadenas dans la barre latérale et, quand vous l’ouvrez, le mot de passe est demandé, s’il y en a un (voir [Options](opciones.md#show-et-verrouillage)).

En bas de la barre latérale se trouvent la version du programme et le **CPU** qu’il utilise (« CPU 12 % », tous les cœurs de l’ordinateur réunis). Au-dessus de 70 %, il devient orange, et au-dessus de 90 %, rouge : la sortie peut perdre le rythme. Fermez d’autres programmes ou arrêtez quelques effets.

### La barre du haut

De gauche à droite :

- **Le show** : son nom. Un point **●** devant indique des modifications non enregistrées ; si la sauvegarde automatique est active, l’heure de la dernière s’affiche aussi (« sauvegarde auto… »).
- **Fichier** : « Nouveau show (Ctrl+N) », « Ouvrir (Ctrl+O) », la flèche **▾** avec les shows récents et les « Exemples », « Enregistrer (Ctrl+S) » et « Enregistrer sous… » (Ctrl+Maj+S).
- **Rythme** : **« GO »** et **« Stop »** changent ce que font les boutons des scènes (dans Direct, le SHOW, les mappages et le téléphone). Avec **GO** allumé, appuyer sur une scène la lance et les appuis suivants marquent son rythme, sans jamais l’arrêter ; avec **Stop** allumé, appuyer sur une scène l’arrête ; avec les deux éteints, comme d’habitude ([Direct](directo.md#rythme-dune-scène-à-pas)). Chaque bouton d’une scène à pas ou d’une cuelist en cours (dans Direct, le SHOW, les playbacks et le téléphone) contient une **barre coupée en deux** qui allume un côté puis l’autre à chaque pas : vous voyez ainsi le rythme de chacune. Ensuite, **« BeatGO »** (division suivante des scènes pilotées par BeatGO, aussi avec la touche G) et **« Pulsation »**, qui écoute l’entrée audio de l’ordinateur (choisie dans [Options](opciones.md#audio)) : chaque pic qui dépasse la ligne du vumètre fait avancer les scènes pilotées par Pulsation. Faites glisser la ligne du vumètre pour changer le seuil. Le tempo du show, celui des FX et des scènes en mode BPM, se change avec un fader ou un rotatif de BPM du [SHOW](show.md) ou avec les [mappages](mapeos.md).
- **Le cadenas** : verrouille des écrans pour que rien ne change par erreur (voir [Options](opciones.md#show-et-verrouillage)).
- **La cible** : le mode Mapper, pour assigner des contrôles MIDI, OSC ou des touches (voir [Mappages](mapeos.md#mode-mapper)). Son voyant vert clignote à chaque message reçu.
- **« N en cours »** : le nombre de scènes et d’effets en cours.
- **GM** : le grand master. En dessous de 100 %, il devient orange.
- **« Blackout »** (Ctrl+B) : met toute la sortie au noir tant qu’il est allumé.
- **« Tout arrêter »** (Échap) : arrête tout ce qui joue. Le programmeur n’est pas vidé.
- **« ? »** : ouvre ce manuel (F12).

Dans une fenêtre étroite ou sur un petit écran, la barre se compacte : le nom du show est raccourci, « Enregistrer sous… » devient une icône, « N en cours » se réduit au nombre et, dans les plus étroites, « Blackout » et « Tout arrêter » n’affichent que leur icône. Survolez une icône avec la souris pour voir ce que c’est. Si tout paraît petit ou grand, changez la **Taille de l’interface** dans [Options](opciones.md#général).

Quand vous fermez le programme, ou que vous ouvrez ou créez un autre show, s’il y a des modifications non enregistrées, la question « Enregistrer les modifications ? » s’affiche avec « Annuler », « Ne pas enregistrer » et « Enregistrer ».

Si le show que vous ouvrez utilise des fichiers manquants (ses sons, vidéos ou images ; par exemple, un show venu d’un autre ordinateur), un avis en donne la liste. Le show fonctionne sans eux. Avec **« Chercher dans un dossier… »**, vous choisissez le dossier où ils se trouvent : ils sont recherchés par leur nom, y compris dans ses sous-dossiers, et le show les utilise désormais (enregistrez-le pour le garder ainsi).

> **Conseil :** si le programme s’est fermé de façon inattendue alors que vous aviez un show non enregistré, il vous propose « Récupérer le show non enregistré ? » à la réouverture. Appuyez sur « Récupérer » et enregistrez-le sous un nom.

## Comment ce manuel est organisé

Commencez par **Premiers pas** si c’est la première fois. Les autres pages expliquent chaque page du programme et se lisent dans n’importe quel ordre.

1. [Premiers pas](primeros-pasos.md) : une première session complète, du patch à l’enregistrement du show.
2. [Direct](directo.md) : groupes, exclusif ou mélange, fondu enchaîné, masters, libérations, flash et mixeur.
3. [Show](show.md) : l’écran tactile, ses sections, boutons, faders, rotatifs, roue de couleur, grille XY, modification et verrouillage.
4. [Playbacks](playbacks.md) : pages de playbacks, GO, Pause/Retour, Flash, niveaux, enregistrer sur un playback et contrôleurs APC.
5. [Programmer](programar.md) : sélection, groupes, plan 2D, dimmer, couleur, position, fonctions, faders, palettes, FX, éventail, aveugle, surbrillance et localisation, ligne de commande, enregistrer et mettre à jour.
6. [Scènes](escenas.md) : tous les types de scène, super scènes, cuelists avec tracking, dossiers, import et export.
7. [Patch](patch.md) : ajouter des fixtures, adresses, univers, numéros, groupes de fixtures, plan 2D, limites, matrices LED et import depuis un autre show.
8. [Fixtures](fixtures.md) : la bibliothèque, le créateur de fixtures, l’Open Fixture Library, GDTF et vos fixtures.
9. [Sorties](salidas.md) : univers, interfaces DMX USB, Art-Net et sACN, et le moniteur des niveaux DMX.
10. [Mappages](mapeos.md) : MIDI, OSC, entrée DMX et clavier, le mode Mapper et les modèles pour Akai APC.
11. [Options](opciones.md) : langue, thème, taille, démarrage, verrouillage, sauvegarde automatique, priorités, télécommande, bibliothèque, mises à jour.
12. [Télécommande](remoto.md) : le SHOW sur le téléphone ou la tablette.
13. [Raccourcis clavier](atajos.md) : toutes les touches et combinaisons.

## Les mots que vous verrez souvent

- **Fixture** : chaque appareil d’éclairage patché (un PAR, une lyre, un ruban LED…).
- **Univers** : 512 canaux DMX. Chaque univers sort par une interface.
- **Programmeur** : là où vous donnez des valeurs aux fixtures sélectionnés. Ce qui est dans le programmeur prime sur tout jusqu’à ce que vous le vidiez.
- **Scène** : ce que vous enregistrez pour le rejouer ensuite. Il en existe plusieurs types : statique, pas, FX, super scène, cuelist… (voir [Scènes](escenas.md)).
- **Groupe** : une colonne de scènes dans Direct. Dans un groupe exclusif, une seule scène joue à la fois.
- **Groupe de fixtures** : un ensemble de fixtures pour les sélectionner d’un coup (ils se créent dans [Patch](patch.md#groupes-de-fixtures)).
- **Cuelist** : une liste de cues qui avancent avec GO. Chaque cue ne garde que ce qui change.
- **Playback** : un fader avec ses boutons (GO, Pause/Retour, Flash) qui joue une cuelist ou une scène.
- **Palette** : une valeur enregistrée (une couleur, une position…) que les scènes suivent par référence : si vous changez la palette, les scènes qui l’utilisent changent.
- **Priorité** : de 0 à 10. Si deux scènes contrôlent le même canal, celle qui a la priorité la plus haute l’emporte.
- **SHOW** : l’écran d’exploitation que vous construisez vous-même.
