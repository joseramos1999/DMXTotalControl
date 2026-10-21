# Patch

![La page Patch](../img/patch.png)

**Patch** (F5) contient les fixtures de votre installation : ce qu’ils sont, dans quel univers et à quelle adresse ils se trouvent, leur numéro, les groupes de fixtures et leur place sur scène.

Dans l’en-tête, vous choisissez la vue et vous avez les outils pour ajouter :

- **« Liste »** : tous les fixtures par adresse.
- **« Canaux »** : les 512 canaux de chaque univers, avec les fixtures par-dessus.
- **« Plan 2D »** : la scène vue de dessus.
- **« Importer »**, **« Matrice LED »** et **« Ajouter des fixtures »**.

## Ajouter des fixtures

1. Appuyez sur **« Ajouter des fixtures »**.
2. Choisissez le **Fabricant** (tapez dans « Rechercher un fabricant… » pour filtrer) et le **Modèle**.
3. Dans le panneau **Patch** :
   - **Nom** : celui que vous verrez dans tout le programme (par défaut, celui du modèle).
   - **Mode** : le mode DMX, avec son nombre de canaux.
   - **Quantité** : combien de fixtures identiques.
   - **Univers** et **Adresse** : l’adresse se place toute seule sur la première libre de l’univers.
   - **Écart** : canaux libres entre un fixture et le suivant.
   - Avec plus d’un fixture, **« En faire un groupe »** et le nom du **Groupe**.
4. En bas, vous voyez les adresses qu’ils vont utiliser (« Utilise les adresses 1 à 20 »). S’ils ne tiennent pas dans ce qui reste de l’univers, ils passent au suivant.
5. Appuyez sur **« Patcher le fixture »** ou **« Patcher N fixtures »**.

La fenêtre reste ouverte pour continuer à ajouter, avec l’adresse déjà sur la suivante libre. Si quelque chose ne peut pas être patché (les adresses sont occupées, par exemple), la raison apparaît en rouge en bas. Appuyez sur **« Fermer »** quand vous avez terminé.

Pour des dimmers isolés, choisissez **Generic › Generic Dimmer** : au lieu d’un mode, vous indiquez combien de **Canaux**.

> **Conseil :** si vous ne trouvez pas votre fixture, téléchargez l’[Open Fixture Library](fixtures.md#open-fixture-library), importez son GDTF ou créez-le dans la page [Fixtures](fixtures.md).

## La liste

Colonnes : case de sélection, **#** (numéro), **NOM**, **ADRESSE** (univers . adresse), **CANAUX**, **FIXTURE** (fabricant et modèle), **MODE**, **INVERSER** (pan et tilt) et le bouton d’identification.

- **« Grouper par fixture »** les regroupe par modèle, avec une case pour sélectionner tous ceux de chaque modèle. Un clic sur l’en-tête d’un modèle le replie (masque ses fixtures) ou le déplie.
- Un **double-clic** sur un fixture le modifie.

## Sélectionner

Les outils de la barre agissent sur la sélection :

- **Clic** sélectionne ; **Ctrl+clic** ajoute ou retire ; **Maj+clic** sélectionne tous les fixtures depuis le dernier sur lequel vous avez cliqué jusqu’à celui-ci (dans ce sens, qui est l’ordre de la sélection), et **Ctrl+Maj+clic** les ajoute. **« Tout »** et **« Aucun »**.
- **« Identifier »** : les fixtures sélectionnés clignotent en blanc pour les repérer sur scène. **« Arrêter l’identification »** l’arrête (cela s’arrête aussi quand vous quittez la page). Le bouton de chaque ligne n’identifie que ce fixture. La liste reste là où elle était.
- **Inverser le pan** et **inverser le tilt** (les icônes de flèches) : pour les fixtures qui en ont.
- **« Limites »**, **« Modifier »**, **« Réadresser »**, **« Renuméroter »**, **« Créer un groupe »** et **« Supprimer »** : expliqués ci-dessous.

## Adresses et univers

- **Modifier un fixture** : sélectionnez-en un et appuyez sur **« Modifier »** (ou double-clic). Vous pouvez changer **Nom**, **Mode**, **Univers**, **Adresse** et **Numéro**, puis appuyer sur **« Appliquer »**.
- **Réadresser plusieurs fixtures** : sélectionnez-les et appuyez sur **« Réadresser »**. Choisissez **Univers**, **Adresse de départ** et **Écart** : ils reçoivent des adresses consécutives, dans l’ordre de la liste.
- **Faire glisser dans la vue Canaux** : choisissez l’univers en haut et faites glisser un fixture vers une autre adresse. L’ombre est verte s’il tient et rouge si les canaux ne sont pas libres. En haut, vous voyez combien des 512 canaux sont utilisés. Un double-clic sur un fixture le modifie.

Les univers se créent et se connectent aux interfaces dans [Sorties](salidas.md).

## Numéros de fixture

Chaque fixture a un **numéro**, celui qu’utilise la [ligne de commande](programar.md#ligne-de-commande) de Programmer (`1 THRU 8 @ FULL`). Un nouveau fixture reçoit le numéro qui suit le dernier.

- Changez-en un dans **« Modifier »** › **Numéro**.
- Pour numéroter plusieurs fixtures à la suite, sélectionnez-les, appuyez sur **« Renuméroter »** et tapez le **Premier numéro** : ils sont numérotés dans l’ordre des adresses.

## Groupes de fixtures

Les groupes de fixtures servent à sélectionner d’un clic dans Patch et dans Programmer (`GROUP 2` dans la ligne de commande), et chacun a sa bande dans le [mixeur](directo.md#mixeur). Ils servent aussi dans le SHOW (roue de couleur, grille XY, palettes, dimmer et vitesse de mouvement) et dans les mappages.

1. Sélectionnez les fixtures.
2. Appuyez sur **« Créer un groupe »** et tapez son nom (par exemple, `Wash face` ou `Spots`).

Le panneau **« Groupes de fixtures »** de gauche les affiche avec leur nombre de fixtures. Un clic sur un groupe sélectionne ses fixtures ; **Ctrl+clic** ou **Maj+clic** les ajoute. Clic droit :

- **« Renommer »**.
- **« Remplacer par la sélection »** : le groupe contient désormais les fixtures sélectionnés.
- **« Enregistrer la sélection ici… »** : voir [Sélections](patch.md#sélections).
- Motifs de sélection, dans l’ordre du groupe : **« Sélectionner les impairs »**, **« Sélectionner les pairs »**, **« Sélectionner dans l’ordre inverse »**, **« Sélectionner la première moitié »**, **« Sélectionner la seconde moitié »**, **« Sélectionner des extrémités vers le milieu »** et **« Sélectionner du milieu vers les extrémités »**.
- **« Supprimer le groupe »**.

### Sélections

Une **sélection** est une partie ordonnée d’un groupe : par exemple, les spots d’un groupe de six pris des extrémités vers le milieu. L’ordre compte, car les éventails et les FX le suivent.

1. Sélectionnez les fixtures dans l’ordre voulu (ou avec un motif du menu du groupe).
2. Appuyez sur le **+** du groupe (ou « Enregistrer la sélection ici… ») et tapez un nom.

Les sélections apparaissent sous leur groupe. Un clic sélectionne leurs fixtures dans leur ordre, ici et dans Programmer. Clic droit : **« Renommer »** et **« Supprimer la sélection »**. La flèche à gauche du groupe masque ou affiche ses sélections.

## Plan 2D

Le plan montre la scène vue de dessus, avec le **PUBLIC** en bas. Il sert à sélectionner dans Programmer, à voir ce qui sort et pour les effets de couleur placés sur le plan.

Dans l’en-tête du plan :

- **« Largeur (m) »** et **« Profondeur (m) »** : la taille de la scène.
- **« Placement auto »** : répartit tous les fixtures.

Les outils (ils agissent sur la sélection, dans son ordre) :

- **« En direct »** : affiche la couleur, le dimmer, le stroboscope et le gobo de chaque fixture, ainsi que chaque pixel des barres et des matrices.
- **« Lasso »** : sélectionne en dessinant autour des fixtures au lieu d’un cadre.
- **« Sélectionner »** : clic ou cadre pour sélectionner.
- **« Déplacer »** : faites glisser les fixtures (toute la sélection bouge) ou la vue.
- **« Grille »** : « Libre », 10 cm, 25 cm, 50 cm ou 1 m : là où tombent les fixtures quand vous les déplacez.
- **« Aspect »** : l’apparence des fixtures sélectionnés : « Rond », « Carré » ou « Rectangle ».
- **« Placer »** : les place en « Ligne », « Ligne, d’avant en arrière », « Cercle », « Grille, par lignes », « Grille, en zigzag », « Grille, par colonnes » ou « Grille, en zigzag par colonnes », avec l’**Espacement (cm)** et les **Colonnes** à côté.
- **« Aligner »** : « Gauche », « Centre », « Droite », « Arrière », « Milieu », « Avant », « Répartir en largeur » et « Répartir en profondeur ».
- **15°** (clic droit : dans l’autre sens) et **90°** : font tourner la sélection.
- **« Hauteur et orientation »** : où sont accrochées les lyres sélectionnées, pour [Follow](patch.md#hauteur-et-orientation-follow).

Les lyres portent une petite flèche sur le bord : la direction vers laquelle elles regardent. Les noms des fixtures très rapprochés sont masqués pour ne pas se chevaucher : zoomez pour les lire.

La **molette** zoome, le **bouton droit** ou celui du milieu déplacent la vue et un double-clic avec eux la recentre. Les boutons du coin zooment aussi.

> **Conseil :** placez les fixtures dans l’ordre dans lequel vous voulez que les effets se déplacent : sélectionnez-les de gauche à droite et utilisez « Placer › Ligne ».

### Hauteur et orientation (Follow)

Pour que [Follow](programar.md#follow-viser-un-point) oriente correctement toutes les lyres vers un point, chacune doit savoir où elle se trouve. Placez les lyres sur le plan là où elles sont réellement, sélectionnez-les et appuyez sur **« Hauteur et orientation »**. Ce que vous changez s’applique immédiatement à toutes les lyres sélectionnées et est enregistré avec le show :

| Champ | Ce que c’est |
|---|---|
| **Hauteur** | À combien de mètres du sol de la scène se trouve l’axe de la lyre (sans réglage : 4 m suspendue, 0 m posée). |
| **Montage** | **« Suspendue »** (à une perche ou un truss, tête en bas) ou **« Posée »** (au sol). |
| **Orientation** | La direction du faisceau avec le pan au centre et le tilt penché vers l’avant. 0° est vers le public et tourne dans le sens horaire ; c’est la même rotation que celle des boutons 15° et 90°, et la flèche du plan la montre. |
| **Course du pan** / **du tilt** | Les degrés que parcourt chaque axe. « Celle de la définition » utilise ceux de la définition du fixture ; si elle ne les fournit pas, 540° et 270° sont utilisés, avec un avertissement en orange. |
| **Pan inversé** / **Tilt inversé** | Si, avec Follow, une lyre tourne ou s’incline du mauvais côté. Cela n’agit que sur Follow ; les inversions du Patch sont dans [Limites](patch.md#limites). |

« (différentes) » indique que les lyres sélectionnées n’ont pas la même valeur ; si vous la changez, toutes prennent la nouvelle.

## Limites

Sélectionnez des fixtures et appuyez sur **« Limites »** :

- **Dimmer**, **Pan** et **Tilt** : la plage autorisée, de 0 à 100 %. Par exemple, un dimmer qui ne dépasse pas 80 % ou un tilt qui ne vise pas le public.
- **« Inverser le pan »**, **« Inverser le tilt »** et **« Permuter pan et tilt »** (pour des lyres suspendues à l’envers ou tournées).
- **« Réinitialiser »** : aucune limite, aucun axe inversé ou permuté.

Avec un seul fixture, la liste **Canaux** permet de décider pour chaque canal :

- **Fondus** : s’il suit les temps de fondu. Désactivé, il saute directement à sa valeur (utile pour les roues de couleur et de gobos).
- **Dimmer** : s’il est proportionnel au grand master, aux masters de groupe et aux dimmers de scène.

Les limites agissent sur la sortie : les scènes, les effets et les modifications en direct gardent leurs valeurs, mais ce qui sort les respecte. Avec plusieurs fixtures, les valeurs affichées sont celles du premier qui a chaque contrôle, et ce que vous changez s’applique à tous ceux qui l’ont.

## Matrices et rubans LED

**« Matrice LED »** patche d’un coup une matrice ou un ruban de pixels :

1. **Nom**, **Largeur (pixels)** et **Hauteur (pixels)** (hauteur 1 pour un ruban).
2. **Couleurs** : l’ordre de chaque pixel (RGB, GRB, BGR, BRG, GBR, RBG ou RGBW).
3. **Câblage** : « Lignes », « Lignes, en zigzag », « Colonnes » ou « Colonnes, en zigzag ».
4. **Pixel 1** : le coin où il commence (« En haut à gauche », etc.).
5. **Univers** et **Adresse**.
6. Vérifiez le résumé et l’aperçu de l’**ordre des pixels**, puis appuyez sur **« Patcher »**.

Un fixture est créé par ligne de pixels, avec un groupe pour les effets de couleur et sa place sur le plan. Si une ligne a besoin de plus d’un univers, raccourcissez-la ou câblez-la dans l’autre sens.

## Importer d’un autre show

**« Importer »** amène dans ce show une partie d’un autre show (de DMX Total Control ou de QLC+) :

- **« Patch d’un autre show… »** : ses fixtures. Ils vont aux mêmes adresses si elles sont libres ; sinon, aux suivantes.
- **« Scènes d’un autre show… »** : ses scènes, assignées aux fixtures de ce show qui correspondent (par adresse et modèle, par nom ou par modèle). Elles entrent dans des groupes du même nom.
- **« Patch et scènes d’un autre show… »** : les deux.

## Supprimer des fixtures

Sélectionnez-les et appuyez sur **« Supprimer »**. Le programme demande d’abord confirmation : leurs valeurs sont retirées de toutes les scènes.
