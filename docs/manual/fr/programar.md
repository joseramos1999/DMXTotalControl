# Programmer

![La page Programmer](../img/programar.png)

**Programmer** (F3) est le programmeur : vous sélectionnez des fixtures, vous leur donnez dimmer, couleur, position, gobos ou n’importe quel autre canal, vous leur appliquez des FX et des palettes, et vous enregistrez le tout comme scène, cuelist ou cue.

Ce que vous mettez dans le programmeur **sort immédiatement et prime sur tout** (scènes, groupes et playbacks) jusqu’à ce que vous le vidiez. Le sous-titre de la page vous le rappelle : « N canaux dans le programmeur — ils priment sur les playbacks jusqu’à ce que vous les effaciez ».

La page contient, de haut en bas : l’en-tête avec ses boutons, la [ligne de commande](programar.md#ligne-de-commande), le panneau des **fixtures** à gauche et, à droite, le panneau **Rapide** et le panneau **Fonctions / Faders / Palettes / FX**.

## L’en-tête

- **« Beam on »** : dimmer à fond et shutter ouvert sur la sélection.
- **« Beam off »** : dimmer à zéro sur la sélection.
- **« Centrer »** : pan et tilt au centre.
- **« Aveugle »** : voir [Aveugle](programar.md#aveugle).
- **« Tout réinitialiser »** : libère toutes les modifications en direct (valeurs et FX). Les scènes se voient à nouveau.
- **« Réinitialiser sél. »** : ne libère que celles des fixtures sélectionnés.
- **« Snapshot »** et **« Enregistrer… »** : voir [Enregistrer](programar.md#enregistrer).

Dans une fenêtre étroite, certains boutons n’affichent que leur icône ; leur nom apparaît au survol de la souris.

## Sélectionner des fixtures

Le panneau **« Fixtures (N sél.) »** affiche une tuile par fixture avec son nom, son numéro (pour la ligne de commande), son adresse (univers.adresse) et son type.

- **Clic** sur une tuile : sélectionne seulement celle-ci.
- **Ctrl+clic** : l’ajoute à la sélection ou l’en retire. **Maj+clic** : sélectionne toutes les tuiles depuis la dernière sur laquelle vous avez cliqué jusqu’à celle-ci (dans ce sens, qui est l’ordre de la sélection) ; **Ctrl+Maj+clic** les ajoute.
- **« Tout »** et **« Aucun »**, dans l’en-tête du panneau.
- **« Inverser »** : sélectionne ceux qui ne l’étaient pas.
- **½**, **⅓**, **¼** : garde un fixture sur deux, sur trois ou sur quatre.
- **◀ ▶** : décale la sélection vers la gauche ou vers la droite.

**L’ordre compte** : l’ordre dans lequel vous sélectionnez les fixtures est celui que suivent les éventails, les FX et les Colour FX. Sur le plan 2D, chaque fixture sélectionné affiche son numéro d’ordre.

> **Conseil :** vous pouvez aussi sélectionner en tapant : `1 THRU 8` puis Entrée. Voir [la ligne de commande](programar.md#ligne-de-commande).

### Groupes de fixtures et sélections

Au-dessus des tuiles apparaissent les [groupes de fixtures](patch.md#groupes-de-fixtures) du Patch sous forme d’étiquettes rondes :

- **Clic** sur un groupe : sélectionne ses fixtures. **Ctrl+clic** ou **Maj+clic** : les ajoute.
- L’étiquette s’allume quand tous ses fixtures sont sélectionnés. Au survol, vous voyez son numéro pour la ligne de commande (« GROUP 2 dans la ligne de commande »).
- Les **sélections** d’un groupe (parties ordonnées, avec leur icône) sélectionnent leurs fixtures dans leur ordre.
- Avec beaucoup de groupes, trois rangées d’étiquettes sont visibles et défilent avec la molette : **« Tous les groupes (N) »** les ouvre (jusqu’à la moitié du panneau) et **« Moins »** les replie. Les tuiles des fixtures restent ainsi visibles.

### Plan 2D

Le bouton de grille dans l’en-tête du panneau (« Plan 2D ») remplace les tuiles par le plan de scène :

- Un **clic** sur un fixture le sélectionne ; **Ctrl+clic** ou **Maj+clic** l’ajoute ou le retire.
- **Faites glisser** sur une zone vide du plateau pour sélectionner avec un cadre. Un clic dans le vide annule la sélection.
- La **molette** zoome ; faire glisser avec le **bouton droit** ou celui du milieu déplace la vue, et un double-clic avec ces boutons la recentre.

Dans Programmer, le plan est toujours **en direct** : chaque fixture affiche sa couleur, son dimmer, son stroboscope (il clignote) et son gobo, ainsi que chaque pixel des barres et des matrices. Les fixtures se placent sur le [plan 2D du Patch](patch.md#plan-2d).

## Le panneau Rapide

### Dimmer

Le fader **Dimmer** et les boutons **100%**, **75%**, **50%**, **25%** et **0%** donnent de l’intensité à la sélection.

### Couleur

Cliquez sur l’un des 16 échantillons, ou bougez **Teinte** et **Sat.** La couleur est mélangée sur les canaux RGB (la part de blanc va au canal blanc si le fixture en a un) ou CMY de chaque fixture. Pour les roues de couleur, utilisez leurs presets dans [Fonctions](programar.md#fonctions) › Couleur.

> **Conseil :** pour un éventail de dimmer, utilisez la ligne de commande : `@ 0 THRU 100` va de 0 % sur le premier à 100 % sur le dernier. L’éventail de position est dans [Fonctions › Position](programar.md#position-grille-xy-et-éventail).

## Fonctions

**Fonctions** affiche les canaux de la sélection par onglets : **Dimmer**, **Couleur**, **Position**, **Gobo**, **Faisceau**, **Focus** et **Autres**. Les onglets sans canaux dans la sélection sont désactivés.

Chaque canal est une carte avec son nom, sa valeur en %, un curseur et ses **presets** sous forme de boutons (avec leur couleur ou l’image de leur gobo si le profil les fournit). Cliquez sur un preset pour l’appliquer. La carte est marquée en orange quand le canal est dans le programmeur ; sa **×** le libère.

Les canaux sont regroupés **par fonction**, pas par nom : « Red », « R » ou « Rouge » de fixtures différents sont le même contrôle, et de même pour dimmer, pan/tilt, CMY, roue de couleur, gobo, shutter, iris, zoom, focus, prisme… Chaque tête d’une barre garde ses canaux.

### Couleur : sélecteur et favoris

Dans l’onglet **Couleur**, le carré de gauche donne la teinte (d’un côté à l’autre) et la saturation (de haut en bas). Faites glisser la croix pour la placer, ou faites glisser ailleurs dans le carré pour un réglage fin.

En dessous, **« Favoris »** : le **+** enregistre la couleur actuelle ; un clic sur un favori l’applique ; un clic droit le retire. Les favoris servent pour tous les shows.

### Position : grille XY et éventail

L’onglet **Position** ne bouge que le **pan** et le **tilt** (avec leurs canaux fins, en 16 bits). La rotation continue du pan ou du tilt et sa vitesse se trouvent dans **Autres**.

- Faites glisser la **croix** pour la placer, ou faites glisser ailleurs dans la grille pour un mouvement lent et fin.
- La barre de gauche est le **tilt** et celle du bas le **pan**.
- Avec plusieurs fixtures, chacun est un point sur la grille.
- **« Centrer »** met pan et tilt au centre.

À droite se trouve l’**Éventail** : il ouvre la position de la sélection, dans son ordre, chaque fixture à partir de là où il était.

- **« Ouvert »** : d’un bout à l’autre (le premier d’un côté, le dernier de l’autre, celui du milieu immobile).
- **« En V »** : les extrémités s’ouvrent et le centre reste immobile.
- **« Depuis le premier »** : le premier immobile et chacun un peu plus.
- **Pan** et **Tilt** : l’ouverture de chacun (de −100 à 100 % ; double-clic : à 0). À 100 %, les extrémités bougent d’une demi-grille.
- **« Sans éventail »** : les fixtures reviennent là où ils étaient.
- Le bouton **« Éventail »** sous la grille fait qu’un glissement sur celle-ci ouvre l’éventail au lieu de bouger : en travers, le pan ; de haut en bas, le tilt.

Si vous bougez ensuite la croix, le centre bouge et l’éventail est conservé. Si la position change par un autre moyen (une palette, une autre sélection, « Tout réinitialiser »), l’éventail revient à 0 et repart de là où sont les fixtures.

### Follow : viser un point

Le bouton **« Follow »** sous la grille remplace l’éventail par un plan de scène. Cliquez ou faites glisser dessus : **toutes les lyres sélectionnées visent ce point**, chacune avec son pan et son tilt, et une ligne relie chacune au point.

- **« Hauteur du point »** : 0 m est le sol ; montez-la pour viser, par exemple, le visage de la personne qui chante (1,6 m).
- Chaque lyre calcule son pan et son tilt d’après sa place sur le plan, sa hauteur, si elle est suspendue ou posée, et la direction vers laquelle elle regarde. Tout cela se règle dans [Patch › Plan 2D › « Hauteur et orientation »](patch.md#hauteur-et-orientation-follow).
- Quand vous faites glisser, chaque lyre prend le chemin le plus court depuis là où elle visait, sans faire le grand tour.
- Les valeurs restent dans le programmeur comme n’importe quel pan et tilt : enregistrez-les dans une scène, une cue ou une palette.
- En dessous s’affichent combien de lyres visent le point, lesquelles ne l’atteignent pas (elles restent au plus près possible), lesquelles n’ont pas la course de pan et de tilt dans leur définition (540° et 270° sont utilisés) et les scanners, qui ne bougent pas (leur miroir ne suit pas cette géométrie).
- Les inversions, les limites et la permutation de pan et tilt du [Patch](patch.md#limites) sont déjà prises en compte.

Si vous bougez la croix ou l’éventail, si vous appuyez sur « Centrer », ou si vous changez la sélection, le point disparaît.

> **Non testé avec du vrai matériel :** chaque fabricant oriente ses lyres à sa façon. Si l’une vise mal, ajustez son orientation ou inversez son pan ou son tilt dans « Hauteur et orientation ».

## Faders

**Faders** affiche chaque canal de la sélection sous forme de fader vertical, avec son nom, son groupe et sa valeur (0–255). Le bouton de liste ouvre ses **presets** (« Nom (min–max) ») et la **×** le libère.

> **Conseil :** après avoir cliqué sur un fader ou un curseur, les flèches ↑ ↓ le bougent un par un et PgUp / PgDn dix par dix. Un double-clic le ramène à sa valeur de repos (normalement, zéro).

## Aveugle

Avec **« Aveugle »** actif, le programmeur modifie **sans rien envoyer aux projecteurs** : le plateau continue de montrer ce qui joue et vous préparez le look suivant. Le sous-titre prévient : « Aveugle : les projecteurs ne montrent pas ce que vous programmez ». Vous pouvez enregistrer comme d’habitude. Quand vous le désactivez, ce qui est dans le programmeur sort.

## Surbrillance et localisation

Dans la barre de la ligne de commande :

- **« Surbrillance »** : la sélection à fond en blanc ouvert, par-dessus tout, pour voir de quels fixtures il s’agit. Ce n’est pas enregistré. Appuyez à nouveau pour l’enlever.
- **« Localiser »** : met la sélection à fond, en blanc ouvert, centrée et faisceau ouvert, **dans le programmeur** (ces valeurs-là sont enregistrées).
- **◀ ▶** (PREV / NEXT) : parcourt la sélection **un par un** : un seul fixture reste sélectionné et vous voyez où vous en êtes (« 2/8 »). **« Tous »** revient à toute la sélection (ALL).

## Palettes

Une **palette** garde une valeur d’intensité, de position, de couleur ou de faisceau pour chaque fixture. Les scènes et les cues enregistrées avec une palette l’utilisent **par référence** : si vous mettez la palette à jour, elles changent toutes seules (les modifications en direct aussi).

Dans **Palettes**, il y a quatre colonnes : **Intensité**, **Position**, **Couleur** et **Faisceau**.

### Enregistrer une palette

1. Sélectionnez des fixtures et donnez-leur la valeur (par exemple, une couleur).
2. Dans la colonne **Couleur**, appuyez sur **« + Enregistrer »**.

La palette est créée avec un nom et un numéro (« Couleur 1 », COL 1). À partir de là, ce que vous enregistrez avec elle la suit.

### Utiliser une palette

Sélectionnez des fixtures et cliquez sur la palette : elle s’applique à la sélection. Une palette est marquée quand elle est appliquée à la sélection (plus nettement si elle l’est à toute la sélection). Sur un fixture qui n’a pas été enregistré dans la palette, elle utilise les valeurs d’un autre fixture du même type ou ses attributs : une couleur se mélange en RGB ou CMY, ou va à l’emplacement le plus proche de la roue de couleur.

Aussi : `COL 2`, `POS 1`, `BEAM 3` ou `INT 4` dans la ligne de commande.

### Le menu d’une palette

Clic droit sur une palette :

- **« Appliquer à la sélection »**.
- **« Charger ses fixtures dans le programmeur »**.
- **« Mettre à jour avec le programmeur »** : ajoute ou change les valeurs du programmeur dans la palette. Les scènes qui l’utilisent changent aussi.
- **« Enregistrer par-dessus »** : l’enregistre à nouveau avec seulement ce qu’il y a maintenant.
- **« Renommer… »**, **« Changer son numéro… »** et **« Supprimer »** (les scènes gardent les valeurs).

Une icône de lien sur la palette indique que des scènes l’utilisent ; l’info-bulle dit combien.

> **Conseil :** les palettes peuvent être placées dans le [SHOW](show.md) (action « Une palette »), dans la [télécommande](remoto.md) et dans un [mappage](mapeos.md), sur leurs fixtures ou sur un groupe, comme modification en direct. Alt+clic sur une palette l’envoie au SHOW.

## FX

Un **FX** fait varier un paramètre de la sélection (le dimmer, le rouge, le zoom, pan et tilt ensemble…) autour de sa valeur dans le programmeur ou, s’il n’a pas de valeur, autour de ce qu’envoient les scènes en dessous. Il joue immédiatement et s’enregistre avec la scène.

Dans **FX**, il y a trois zones : les paramètres et les FX en cours à gauche, les contrôles du FX au centre et les macros à droite.

### Lancer un FX

1. Sélectionnez des fixtures.
2. Dans **Paramètre**, choisissez ce qu’il faut faire varier. Par défaut, c’est le dernier paramètre que vous avez touché. Ceux qui ont déjà un FX dans la sélection portent la marque « FX ».
3. Appuyez sur **« Lancer le FX »**, ou cliquez sur une **macro**.
4. Réglez les contrôles : le FX change en direct.

### Les contrôles du FX

- **La figure ou le mode** (le dessin de gauche) : cliquez dessus pour en choisir un autre. Pour pan/tilt, il y a **12 figures** (cercle, huit, carré, triangle, diamant, ligne de pan, ligne de tilt, diagonale, trèfle, lissajous, ballyhoo et croix). Pour le reste, **22 modes** : la ligne pointillée est la valeur de base et l’onde passe au-dessus, en dessous ou des deux côtés ; les modes 9 à 12 sont pour les chenillards, avec un pas.
- **Amplitude** (de 0 à 250 %). Pour pan/tilt, **« Amplitude pan »** et **« Amplitude tilt »** séparément.
- **Vitesse**, en BPM. **½** et **×2** la divisent ou la doublent. **« BPM du show »** lui fait suivre le tempo du show (un fader ou un rotatif de BPM du SHOW, ou un mappage de BPM ou de Tap). **« Multiplicateur »** : de ×¼ à ×8.
- Pan/tilt seulement : **« Inverser le pan »**, **« Inverser le tilt »**, **« À l’envers »** et **« Rotation »** (0–359°).

### Timing du FX

La façon dont le FX se répartit entre les fixtures :

- **« Vague par X »** : la forme se répartit sur X fixtures (ou ensembles), chacun un peu plus tard.
- **« Pas par X »** : un fixture (ou ensemble) après l’autre, X à la fois.
- **« Regroupement »** : « Non », « Tous les X », « Blocs de X », « Diviser en X », « Miroir » ou « Par groupe » (de fixtures).
- **« Ordre »** : « Sélection », « Inversé », « Aléatoire » (avec un bouton pour un autre ordre aléatoire) ou « Par numéro ».
- **« Attente »** : temps à la base après chaque passage.
- **« Phase »**.

En dessous, un aperçu animé dessine chaque fixture tel qu’il bouge.

### FX en cours, lier et arrêter

- **« FX en cours (N) »** liste les FX du programmeur. Un clic sélectionne ses fixtures et son paramètre ; la **×** l’arrête.
- **« Lier »** : les FX liés partagent la vitesse et le timing, et s’arrêtent ensemble.
- **« Arrêter »** arrête le FX du paramètre choisi ; **« Arrêter tous les FX »**, tous.

« Tout réinitialiser » retire aussi les FX du programmeur. « Tout arrêter » de la barre du haut ne les retire pas : ce sont des modifications en direct.

### FX propres à chaque fixture

Un FX reste **un seul FX** même si vous changez une partie de ses fixtures. Si vous ne sélectionnez que certains des fixtures d’un FX et que vous changez la **forme, la taille, le sens (« À l’envers »), les inversions, la rotation ou la phase**, le changement est **propre à ces fixtures**, sur la même horloge : ils restent synchronisés avec les autres. La **vitesse et le timing** concernent toujours tout le FX.

- Le panneau prévient : « Ce FX est aussi sur d’autres fixtures… ».
- Avec tous ses fixtures sélectionnés, le changement s’applique à tout le FX.
- Si vous sélectionnez des fixtures qui n’étaient pas dans le FX, ils le rejoignent. Deux FX du même paramètre sélectionnés ensemble fusionnent en un seul, et chaque fixture garde ce qu’il faisait.
- Dans « FX en cours », vous voyez combien de fixtures ont quelque chose en propre (« N propres »).
- **« Comme les autres »** retire ce qui est propre aux fixtures sélectionnés : ils refont la même chose que le FX.

### Le miroir

Si les fixtures qui vont à l’envers (ou avec leur propre pan ou tilt inversé) sont **tous dans une moitié** du FX, chacun suit la vague de son jumeau de l’autre côté, et la vague sort donc aussi en miroir. Le panneau l’indique par « Miroir : … ».

Exemple avec 6 beams en cercle avec vague :

1. Sélectionnez les 6, choisissez **Pan / tilt**, appuyez sur **« Lancer le FX »**, figure **Cercle** et **« Vague par X »** sur « Tous ».
2. Sélectionnez seulement les 3 derniers et activez **« À l’envers »**.

Les 3 derniers tournent à l’envers et suivent la vague du 3e, du 2e et du 1er : un miroir synchronisé. Si vous mettez à l’envers un fixture sur deux (2, 4 et 6), ce n’est pas un miroir : chacun garde sa place dans la vague.

### Macros FX

À droite, les **macros FX** d’usine par catégories : **Position** (cercle, cercle en vague, huit, ballyhoo, balayage pan, cancan, pan en miroir, carré, triangle, trèfle), **Intensité** (vague de dimmer, chenillard, aléatoire, pulsation, respiration, stroboscope, du bord au centre), **Couleur** (arc-en-ciel, rouge / bleu, feu, cyan / magenta) et **Faisceau** (zoom, iris, focus). Un clic applique la macro à la sélection.

**« Enregistrer comme macro… »** enregistre les FX du programmeur comme une macro à vous, dans **« Les miennes »**, pour tous les shows. Clic droit sur une de vos macros : **« Renommer »** ou **« Supprimer »**.

## Ligne de commande

La ligne de commande fonctionne comme celle d’une console. Quand vous ouvrez Programmer, elle reçoit le clavier : tapez et appuyez sur **Entrée**. À côté, vous voyez ce qu’elle a fait, ou pourquoi elle ne comprend pas.

| Commande | Ce qu’elle fait |
|---|---|
| `1 THRU 8` | Sélectionne les fixtures 1 à 8 (aussi `1 > 8` ou `1 / 8`). `8 THRU 1`, dans l’ordre inverse. |
| `1 + 3 - 2` | Ajoute et retire. `1,3,5` équivaut à `1 + 3 + 5`. Commencer par `+ 5` ou `- 5` modifie la sélection existante. |
| `GROUP 2` | Le groupe de fixtures 2 (dans l’ordre du Patch). `GROUP 1 THRU 3` : les groupes 1 à 3. |
| `ALL` | Tous les fixtures, par ordre de numéro (après NEXT / PREV, revient à toute la sélection). |
| `@ 50` | Dimmer à 50 %. `@ 5` vaut 50 % (un seul chiffre compte en dizaines). |
| `@ FULL` / `@ OUT` | Dimmer à fond ou à zéro. `@ @` met aussi à fond. |
| `@ 0 THRU 100` | Éventail de dimmer, de 0 % sur le premier à 100 % sur le dernier. |
| `@ + 10` / `@ - 10` | Monte ou baisse le dimmer de 10 %. |
| `POS 1`, `COL 2`, `BEAM 3`, `INT 4` | Applique la palette de position, de couleur, de faisceau ou d’intensité qui porte ce numéro. |
| `LOCATE` | Localiser. |
| `HIGHLIGHT` | Surbrillance (bascule). |
| `NEXT` / `PREV` | Un par un dans la sélection. |
| `REL` | Libère les modifications de la sélection. |
| `CLEAR` | Vide le programmeur. |

Les commandes s’enchaînent : `1 THRU 4 @ FULL COL 2` sélectionne, met le dimmer à fond et applique la palette de couleur 2.

Les mots sont aussi acceptés en espagnol : HASTA, GRUPO, TODO, LLENO, TOPE, CERO, INTENSIDAD, POSICION, COLOR, HAZ, LIMPIAR, RESALTAR, LOCALIZAR, SIGUIENTE, ANTERIOR, LIBERAR, IR, ATRÁS, GRABAR, ACTUALIZAR, ELIMINAR… et en abrégé : THR, FL, GRP (ou G), CLR, LOC, HI, SIG, ANT, REL.

### Commandes de cuelists

Elles agissent sur la **cuelist sélectionnée** : celle du playback sélectionné, celle que vous avez ouverte dans Scènes ou la dernière que vous avez créée.

| Commande | Ce qu’elle fait |
|---|---|
| `GO` | La cue suivante. |
| `BACK` | La cue précédente. |
| `GO CUE 3` | Saute à la cue 3. |
| `CUE 3` | Charge la cue 3 dans le programmeur (ce qu’on voit sur scène à cette cue) pour la modifier. |
| `REC` | Enregistre la cue suivante. `REC CUE 2.5` enregistre la cue 2.5 (entre la 2 et la 3). |
| `UPD` | Met à jour la cue chargée dans le programmeur ou, s’il n’y en a pas, celle qui joue. `UPD CUE 4`, la cue 4. |
| `DELETE CUE 4` | Supprime la cue 4 en gardant l’aspect des suivantes. |
| `PB 3` | Sélectionne le playback 3 de la page actuelle (et sa cuelist). |

Si une commande s’arrête à mi-chemin (par exemple, la palette n’existe pas), ce qu’elle a déjà fait reste fait, et seule la partie à corriger reste dans la ligne.

### Historique et clavier à l’écran

- **↑ ↓** parcourent les dernières commandes.
- **Échap** vide la ligne (avec la ligne vide, Échap équivaut à « Tout arrêter »).
- Le bouton du **pavé** ouvre un clavier à l’écran pour les écrans tactiles, avec les chiffres, THRU, @, FULL, OUT, GROUP, INT, POS, COL, BEAM, NEXT, PREV, ALL, LOCATE, CLR et les touches de cuelists (CUE, le point décimal, REC, UPD, GO, BACK, DELETE, PB). Il reste ouvert pendant que vous choisissez des fixtures.

## Enregistrer

### La fenêtre « Enregistrer »

Appuyez sur **« Enregistrer… »** :

1. Dans **Enregistrer dans**, choisissez :
   - **« Nouvelle scène »** : une scène statique ou, si le programmeur contient des FX, une scène FX avec ses valeurs de base et ses FX.
   - **« Nouvelle cuelist »** : le programmeur devient la cue 1 d’une nouvelle cuelist. Avec **« La mettre sur le premier playback libre »** coché, elle apparaît aussi dans Playbacks.
   - **Une cuelist existante** : le programmeur est enregistré comme une de ses cues. La cuelist sélectionnée est choisie par défaut.
2. Tapez le **Nom** (et, pour une cue, le numéro dans **Cue** : vide, le suivant ; `2.5` va entre la 2 et la 3).
3. Pour une nouvelle scène ou cuelist : **« Ajouter au groupe »** (ou « Aucun groupe ») et **« Priorité (0-10) »**.
4. Pour une cue : **« Cette cue seulement (ce qu’elle change ne passe pas aux suivantes) »** et **« Toutes les valeurs (cue bloc) »** (aussi les valeurs égales à ce dont elle hérite, pour qu’elle reste identique quoi que fassent les cues précédentes).
5. Appuyez sur **« Enregistrer »** (ou « Enregistrer la cue N »).

Une cue ne garde que ce que le programmeur change par rapport à ce que les cues précédentes laissent sur scène. Si le numéro de cue existe déjà, le programme demande quoi faire : **« Fusionner »** (le programmeur entre dans la cue), **« Remplacer »** (la cue ne garde que ce qui vient du programmeur ; son nom et ses temps restent) ou **« Insérer après »** (une nouvelle cue entre celle-ci et la suivante).

**« Vider le programmeur après l’enregistrement »** vide le programmeur après chaque enregistrement et chaque mise à jour. L’option est mémorisée.

### Snapshot

Le menu **« Snapshot »** contient :

- **« Enregistrer les modifications dans une scène »** : met les valeurs du programmeur dans une scène statique existante.
- **« Tout ce qui sort, dans une nouvelle scène… »** : tout ce que montrent les projecteurs maintenant (scènes, modifications en direct et le mixeur, sans le grand master) devient une nouvelle scène statique, sur chaque fixture allumé. Si « Surbrillance » est active, il faut d’abord la désactiver (« Désactiver la surbrillance »).
- **« Seulement les modifications, dans une nouvelle scène… »**.

### Modifier des scènes et des cues

- **Une scène statique ou FX** : depuis Direct ou Scènes, **« Modifier dans le programmeur »** la charge dans le programmeur. Changez ce que vous voulez puis, dans [Scènes](escenas.md), appuyez sur **« Enregistrer le programmeur dans cette scène »**.
- **Une cue** : quand vous la chargez (double-clic sur la cue dans Scènes, « Charger », ou `CUE 3`), une barre « Modification de la cue 3 de « Chanson 1 » » apparaît. Changez les valeurs et appuyez sur **« Mettre à jour ▾ »** : **« Mettre à jour la cue 3 (se propage aux suivantes) »** ou **« Cette cue seulement (les suivantes gardent leur aspect) »**. La **×** arrête la modification de la cue (le programmeur garde ses valeurs).

## Vider le programmeur

- **« Tout réinitialiser »** (ou `CLEAR`) : libère tout le programmeur, valeurs et FX.
- **« Réinitialiser sél. »** (ou `REL`) : seulement les fixtures sélectionnés.
- La **×** d’une carte ou d’un fader : seulement ce canal.

Ce qui arrive aux modifications en direct quand une scène est lancée, et leur priorité face aux scènes, se décide dans [Options](opciones.md#lecture).
