# Patch

![La page Patch](../img/patch.png)

**Patch** (F5) contient les fixtures de votre installation : ce qu’ils sont, dans quel univers et à quelle adresse ils se trouvent, leur numéro, les groupes de fixtures et leur place sur scène.

Dans l’en-tête, vous choisissez la vue et vous avez les outils pour ajouter :

- **« Liste »** : tous les fixtures par adresse.
- **« Canaux »** : les 512 canaux de chaque univers, avec les fixtures par-dessus.
- **« Plan 2D »** : la scène vue de dessus.
- **« Importer »**, **« Matrice LED »** et **« Ajouter des fixtures »**.
- Avec des fixtures parqués, **« N parqués · Tout déparquer »** : combien il y en a et le bouton pour tous les déparquer (voir [Parquer des canaux](patch.md#parquer-des-canaux)).

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
4. En bas, vous voyez les adresses qu’ils vont utiliser (« Utilise les adresses 1 à 20 »). S’ils ne tiennent pas dans ce qui reste de l’univers, ils passent au suivant. Si le mode a d’autres *DMX breaks* (un fixture GDTF avec deux entrées DMX), un avis vous demande de patcher aussi son mode « (DMX break 2) », à sa propre adresse.
5. Appuyez sur **« Patcher le fixture »** ou **« Patcher N fixtures »**.

La fenêtre reste ouverte pour continuer à ajouter, avec l’adresse déjà sur la suivante libre. Si quelque chose ne peut pas être patché (les adresses sont occupées, par exemple), la raison apparaît en rouge en bas. Appuyez sur **« Fermer »** quand vous avez terminé.

Pour des dimmers isolés, choisissez **Generic › Generic Dimmer** : au lieu d’un mode, vous indiquez combien de **Canaux**.

**« Rechercher sur GDTF-Share… »**, en bas, ouvre [GDTF-Share](fixtures.md#gdtf-share) : téléchargez le fixture et appuyez sur **« Le patcher »** pour qu’il soit choisi ici.

> **Conseil :** si vous ne trouvez pas votre fixture, cherchez-le sur GDTF-Share, téléchargez l’[Open Fixture Library](fixtures.md#open-fixture-library), importez son GDTF ou créez-le dans la page [Fixtures](fixtures.md).

## La liste

Colonnes : case de sélection, **#** (numéro), **NOM**, **ADRESSE** (univers . adresse), **CANAUX**, **FIXTURE** (fabricant et modèle), **MODE**, **INVERSER** (pan et tilt) et le bouton d’identification. Après le nom, un **bouclier** marque un [effet spécial protégé](patch.md#effets-spéciaux-protégés) et un **cadenas**, un fixture avec des canaux [parqués](patch.md#parquer-des-canaux).

- **« Grouper par fixture »** les regroupe par modèle, avec une case pour sélectionner tous ceux de chaque modèle. Un clic sur l’en-tête d’un modèle le replie (masque ses fixtures) ou le déplie.
- Un **double-clic** sur un fixture le modifie.

## Sélectionner

Les outils de la barre agissent sur la sélection :

- **Clic** sélectionne ; **Ctrl+clic** ajoute ou retire ; **Maj+clic** sélectionne tous les fixtures depuis le dernier sur lequel vous avez cliqué jusqu’à celui-ci (dans ce sens, qui est l’ordre de la sélection), et **Ctrl+Maj+clic** les ajoute. **« Tout »** et **« Aucun »**. La case d’une ligne ajoute ou retire son fixture (comme Ctrl+clic) ; avec Maj, elle ajoute tous ceux depuis le dernier sur lequel vous avez cliqué.
- **« Identifier »** : les fixtures sélectionnés clignotent en blanc pour les repérer sur scène. **« Arrêter l’identification »** l’arrête (cela s’arrête aussi quand vous quittez la page). Le bouton de chaque ligne n’identifie que ce fixture. La liste reste là où elle était. Les effets spéciaux protégés ne sont jamais identifiés (faire clignoter un CO2 le déclencherait) : un message dit combien sont restés en dehors.
- **Inverser le pan** et **inverser le tilt** (les icônes de flèches) : pour les fixtures qui en ont.
- **« Limites »**, le bouton de la **flamme** (**« Effet spécial »**), **« Modifier »**, **« Réadresser »**, **« Renuméroter »**, **« Créer un groupe »** et **« Supprimer »** : expliqués ci-dessous.

Un **clic droit** sur une ligne de la liste, ou sur un fixture du plan 2D sans bouger la souris, ouvre le menu du fixture : **« Effet spécial… »**, **« Identifier »**, **« Limites… »**, **« Protéger du grand master »**, **« Modifier… »**, **« Parquer tel quel »**, **« Parquer éteint »**, **« Aussi en blackout »**, **« Déparquer »** et **« Tout déparquer »**. Si le fixture n’était pas sélectionné, il le sélectionne d’abord.

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

Le panneau **« Groupes de fixtures »** de gauche les affiche avec leur nombre de fixtures. Un clic sur un groupe sélectionne ses fixtures (dans la vue « Plan 2D », il l’isole : voir [Isoler un groupe sur le plan](patch.md#isoler-un-groupe-sur-le-plan)) ; **Ctrl+clic** ou **Maj+clic** les ajoute. Clic droit :

- **« Renommer »**.
- **« Monter »** et **« Descendre »** : le déplacent dans la liste.
- **« Remplacer par la sélection »** : le groupe contient désormais les fixtures sélectionnés.
- **« Enregistrer la sélection ici… »** : voir [Sélections](patch.md#sélections).
- Motifs de sélection, dans l’ordre du groupe : **« Sélectionner les impairs »**, **« Sélectionner les pairs »**, **« Sélectionner dans l’ordre inverse »**, **« Sélectionner la première moitié »**, **« Sélectionner la seconde moitié »**, **« Sélectionner des extrémités vers le milieu »** et **« Sélectionner du milieu vers les extrémités »**.
- **« Supprimer le groupe »**.

**L’ordre des groupes**, c’est vous qui le choisissez : faites glisser un groupe par sa poignée (à gauche de son nom) vers un autre endroit de la liste, ou utilisez « Monter » et « Descendre ». **« A–Z »**, dans l’en-tête du panneau, les trie par ordre alphabétique (les nombres comme des nombres : « Truss 2 » avant « Truss 10 »). Les nouveaux groupes vont à la fin. L’ordre est enregistré avec le show, et c’est le même dans Programmer et dans les numéros de `GROUP` de la ligne de commande.

### Sélections

Une **sélection** est une partie ordonnée d’un groupe : par exemple, les spots d’un groupe de six pris des extrémités vers le milieu. L’ordre compte, car les éventails et les FX le suivent.

1. Sélectionnez les fixtures dans l’ordre voulu (ou avec un motif du menu du groupe).
2. Appuyez sur le **+** du groupe (ou « Enregistrer la sélection ici… ») et tapez un nom.

Les sélections apparaissent sous leur groupe. Un clic sélectionne leurs fixtures dans leur ordre, ici et dans Programmer. Clic droit : **« Renommer »** et **« Supprimer la sélection »**. La flèche à gauche du groupe masque ou affiche ses sélections.

## Plan 2D

Le plan montre la scène vue de dessus, avec le **PUBLIC** en bas. Il sert à sélectionner dans Programmer, à voir ce qui sort et pour les effets de couleur placés sur le plan.

Dans l’en-tête du plan :

- **« Largeur (m) »** et **« Profondeur (m) »** : la taille de la scène, jusqu’à 200 m. Ils l’agrandissent ou la rétrécissent à droite et à l’avant ; un fixture laissé hors d’une scène plus petite reste où il est.
- **« Placement auto »** : répartit tous les fixtures.

Hors de la scène, au milieu de chaque côté, il y a un **« − »** et un **« + »** : ils retirent ou ajoutent 1 m de ce côté (à gauche, à droite, au fond ou à l’avant, côté public) ; maintenez-les enfoncés pour plusieurs mètres. Les fixtures restent où ils sont sur la scène : à gauche et au fond, ils se déplacent avec ce côté, tout comme le point que vise [Follow](programar.md#follow-viser-un-point), et la nouvelle bande apparaît de ce côté. Tant que la souris est dessus, la vue du plan ne change pas et les boutons ne bougent pas, vous pouvez donc cliquer plusieurs fois de suite ; quand vous l’éloignez, le plan se réajuste (si vous aviez zoomé ou déplacé la vue, elle reste telle quelle). Le **« − »** d’un côté ne fonctionne que si aucun fixture n’est à 1 m ou moins de ce côté : déplacez-le d’abord (survolez le bouton pour savoir pourquoi).

Les outils (ils agissent sur la sélection, dans son ordre) :

- **« En direct »** : affiche la couleur, le dimmer, le stroboscope et le gobo de chaque fixture, ainsi que chaque pixel des barres et des matrices.
- **« Lasso »** : sélectionne en dessinant autour des fixtures au lieu d’un cadre.
- **« Sélectionner »** : clic ou cadre pour sélectionner.
- **« Déplacer »** : faites glisser les fixtures (toute la sélection bouge) ou la vue.
- **« Grille »** : « Libre », 10 cm, 25 cm, 50 cm ou 1 m : là où tombent les fixtures quand vous les déplacez.
- **« Aspect »** : l’apparence des fixtures sélectionnés : « Rond », « Carré » ou « Rectangle », et leur **« Icône »** (voir [Icônes, couleurs et légende](patch.md#icônes-couleurs-et-légende)).
- **« Placer »** : les place en « Ligne », « Ligne, d’avant en arrière », « Cercle », « Grille, par lignes », « Grille, en zigzag », « Grille, par colonnes » ou « Grille, en zigzag par colonnes », avec l’**Espacement (cm)** et les **Colonnes** à côté.
- **« Aligner »** : « Gauche », « Centre », « Droite », « Arrière », « Milieu », « Avant », « Répartir en largeur » et « Répartir en profondeur ».
- **15°** (clic droit : dans l’autre sens) et **90°** : font tourner la sélection.
- **« Hauteur et orientation »** : où sont accrochées les lyres sélectionnées, pour [Follow](patch.md#hauteur-et-orientation-follow).

Les lyres portent une petite flèche sur le bord (une encoche, quand le plan les montre petites) : la direction vers laquelle elles regardent. Les noms des fixtures très rapprochés sont masqués pour ne pas se chevaucher : zoomez pour les lire. Un **bouclier** en haut à gauche marque un effet spécial protégé (pâle ; rouge avec les effets armés ; avec un anneau qui pulse pendant qu’il se déclenche) et un **cadenas** en haut à droite, un fixture parqué.

La **molette** zoome, le **bouton droit** ou celui du milieu déplacent la vue et un double-clic avec eux la recentre. Les boutons du coin zooment aussi. Un clic droit sans bouger la souris ouvre le [menu du fixture](patch.md#sélectionner).

> **Conseil :** placez les fixtures dans l’ordre dans lequel vous voulez que les effets se déplacent : sélectionnez-les de gauche à droite et utilisez « Placer › Ligne ».

### Icônes, couleurs et légende

Chaque fixture porte sur le plan l’icône de ce qu’il est : lyre, scanner, wash ou PAR, barre LED, stroboscope, blinder, dimmer ou traditionnel, laser, fumée, hazer, jet de CO2, flamme, étincelles froides, confettis, bulles, neige, ventilateur, effet ou autre. C’est son type de fixture qui le dit, sauf si le nom de son modèle en dit plus : un « Effect » appelé « CO2 Jet » est un jet de CO2 et un dimmer appelé « Blinder » est un blinder, mais une lyre appelée « Beam » ou « Wash » reste une lyre. Un dimmer générique, qui peut tenir la place de n’importe quel fixture, prend l’icône de son propre nom (« CO2 1 », « FLAME 3 », « BLINDER1 »). Les barres LED et les matrices montrent leurs cellules ; les autres fixtures à plusieurs cellules (un blinder à plusieurs lampes, un stroboscope à plusieurs segments) ont l’icône à côté de leurs cellules.

Pour la choisir à la main, sélectionnez les fixtures et utilisez **« Aspect » › « Icône »** ; **« Automatique »** revient à la sienne. L’icône choisie est enregistrée avec le show.

Chaque type de fixture du show a aussi sa couleur, sur l’icône et sur le bord, la même chaque fois que vous ouvrez le show : deux types avec la même icône se distinguent par leur couleur. Une icône choisie à la main sur un fixture de la bibliothèque ne change pas sa couleur. En direct, les fixtures allumés montrent leur lumière et ceux qui sont éteints, leur icône.

La **légende**, en haut à gauche du plan, liste les types de fixture avec leur icône, leur couleur et leur nombre (les dimmers génériques, d’après ce qu’ils remplacent). Un clic sur un type sélectionne tous ses fixtures, aussi sur le plan de Programmer ; **Ctrl+clic** ou **Maj+clic** les ajoute à la sélection. Un clic sur son titre la replie ou la déplie, et c’est mémorisé. La scène se place à côté ou en dessous de la légende, pour qu’elle ne cache aucun fixture.

### Isoler un groupe sur le plan

Dans la vue « Plan 2D », un clic sur un groupe du panneau « Groupes de fixtures » l’**isole** : les autres fixtures sont verrouillés et estompés sur le plan, et rien ne les sélectionne ni ne les déplace : ni un clic, ni le cadre, ni le lasso, ni la légende, ni « Déplacer » (un clic sur l’un d’eux ne fait rien, et le cadre peut commencer dessus). Il est ainsi facile de sélectionner une partie d’un groupe sur un plan chargé et d’en faire des groupes plus petits. De la sélection ne restent que les fixtures du groupe qui étaient déjà sélectionnés. Le groupe isolé a une bordure épaisse et un œil, et à côté de la légende du plan une étiquette indique quel groupe vous voyez et combien de fixtures il a.

- **« Tout »** sélectionne tous les fixtures du groupe, comme **Ctrl+clic** sur le groupe. **Ctrl+clic** sur un autre groupe n’ajoute que ceux de ses fixtures qui sont aussi dans le groupe isolé.
- Dans la légende, un clic sur un type ne sélectionne que ses fixtures du groupe ; les types qui n’en ont aucun sont estompés.
- Un clic sur un autre groupe isole celui-ci à la place ; un motif de sélection de son menu isole d’abord ce groupe. Un clic sur une sélection sélectionne toujours tous ses fixtures : si certains sont verrouillés, il isole d’abord le groupe de la sélection ou, si elle contient des fixtures d’autres groupes, met fin à l’isolement.
- Après **« Créer un groupe »**, le groupe reste isolé : vous pouvez continuer à le diviser en d’autres groupes.
- Pour terminer : un autre clic sur le groupe, la **×** de l’étiquette ou **Échap**. Cela se termine aussi en passant à « Liste » ou « Canaux », en ouvrant un autre show ou en supprimant le groupe. Ce n’est pas enregistré avec le show.

Dans les vues « Liste » et « Canaux », un clic sur un groupe sélectionne toujours ses fixtures, comme avant.

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
- **« Protéger du grand master »** : le grand master ne baisse aucun de ses canaux, qu’il agisse seulement sur l’intensité ou sur tous : la lumière de salle, un serveur média, un hazer. Les masters de groupe, les dimmers de scène, le Blackout et le [fondu au noir](directo.md#fondu-au-noir) le baissent toujours. Avec plusieurs fixtures dont seuls certains sont protégés, il apparaît désactivé avec « (seulement certains) », et l’activer les protège tous. Il est aussi dans le [menu du fixture](patch.md#sélectionner), coché quand tous les fixtures sélectionnés sont protégés. Dans la liste, une **ancre** après le nom (« Protégé du grand master : il ne baisse pas avec lui ») ; sur le plan 2D, du Patch et de Programmer, une ancre en bas à gauche (sur une barre, à sa droite). « Réinitialiser » n’enlève pas la protection. Elle est enregistrée avec le show, dans chaque fixture, et [importer le patch d’un autre show](patch.md#importer-dun-autre-show) la reprend. QLC+ ne la lit pas : là-bas, le grand master baisse le fixture.

Avec un seul fixture, la liste **Canaux** permet de décider pour chaque canal :

- **Fondus** : s’il suit les temps de fondu. Désactivé, il saute directement à sa valeur (utile pour les roues de couleur et de gobos).
- **Dimmer** : s’il est proportionnel au grand master, aux masters de groupe et aux dimmers de scène. Le grand master, en outre, ne règle que la lumière (voir [Masters](directo.md#masters)). Si le fixture est protégé du grand master, seuls les masters de groupe et les dimmers de scène le règlent.

Les limites agissent sur la sortie : les scènes, les effets et les modifications en direct gardent leurs valeurs, mais ce qui sort les respecte. Avec plusieurs fixtures, les valeurs affichées sont celles du premier qui a chaque contrôle, et ce que vous changez s’applique à tous ceux qui l’ont.

## Effets spéciaux protégés

Un **effet spécial protégé** ne se déclenche qu’avec les effets armés, depuis des scènes, des cuelists et des chasers, et chaque déclenchement dure au plus sa durée maximale. Comment les armer et ce qui les déclenche est expliqué dans [Direct](directo.md#effets-spéciaux-protégés).

**Lesquels le sont.** Les machines à **CO2**, à **flammes**, à **étincelles froides** et à **confettis** se protègent toutes seules ; celles à **bulles** et à **neige**, non (protégez-les à la main si vous voulez). La classe vient de son type de fixture, des mots de sa définition, du nom d’un dimmer générique (« CO2 1 », « FLAME 3 », « FEU 2 », « PYRO 1 ») ou de l’icône choisie sur le plan. Les règles automatiques ne font que protéger, elles n’enlèvent jamais la protection : changer le nom ou l’icône ne déprotège jamais un fixture ; un fixture qui devient l’une de ces classes est protégé, et un message le dit (« CO2 4 est maintenant un effet spécial protégé ») ; un CO2 que vous avez déprotégé à la main reste déprotégé même si vous l’appelez « CO2 9 ».

> **Limites :** « C02 » écrit avec un zéro n’est pas reconnu, et « FOCO2 » n’est pas un CO2. Une machine non reconnue n’est pas protégée : vérifiez le bouclier dans la liste et protégez-la à la main.

**« Effet spécial »** (le bouton de la flamme, à côté de « Limites », ou « Effet spécial… » dans le menu du fixture) ouvre ses réglages pour la sélection :

- **« Effet spécial protégé »** : le désactiver demande d’abord (« Retirer la protection ? »). Sans elle, le fixture sort comme n’importe quel autre canal : sans armer, sans temps maximal et aussi depuis le programmeur, Identifier, le mixeur et les FX.
- **« Durée maximale de déclenchement »** : de 0,1 à 30 s (par défaut : CO2 3 s, flammes 2 s, étincelles 5 s, confettis 2 s, les autres 3 s).
- **« Attente minimale entre deux déclenchements »** : de 0 à 60 s (1 s par défaut), depuis le relâchement.
- **« Canal de sécurité »** : **« Aucun »** ou le canal du fixture dont la machine a besoin pour se déclencher, avec ses **« Valeurs de / à »** (de 0 à 255 ; « de » ne peut pas dépasser « à »). Ce canal ne sort dans ces valeurs que pendant que l’effet est armé et se déclenche (« de », si la scène ne lui donne pas de valeur) ; sinon, 0. On ne peut le choisir que quand les fixtures sélectionnés ont les mêmes canaux, et un fixture d’un seul canal ne peut pas en avoir (« … a besoin d’un autre canal pour se déclencher »). Si le fixture change de mode ou de définition et qu’un autre canal se trouve à sa place, le canal de sécurité est retiré, et un message le dit (« Le canal de sécurité de … a été retiré »).
- **« Selon sa classe (…) »** : revient à la protection et aux temps de sa classe, sans canal de sécurité.

**Quels canaux se déclenchent.** Tous sauf le pan, le tilt et celui de sécurité, quel que soit leur type (intensité, effet, hauteur…) : la durée maximale s’applique à eux. Le pan et le tilt ne sortent qu’avec les effets armés et depuis des scènes, sans limite de temps, et à 0 désarmés : une scène qui ne fait qu’orienter la machine ne la déclenche pas. Une machine qui n’a que pan et tilt se déclenche sur tous ses canaux sauf celui de sécurité. Un fixture à cheval sur deux univers (d’un show de QLC+) ne se déclenche jamais : la fenêtre l’indique ; déplacez-le dans un seul univers.

Dans la liste, un **bouclier** après le nom ; sur le plan, un bouclier en haut à gauche : pâle normalement, rouge avec les effets armés et avec un anneau qui pulse pendant qu’il se déclenche. **Identifier** ne les allume jamais. [Niveaux DMX](salidas.md#niveaux-dmx) et le plan en direct montrent ce qui sort vraiment : un CO2 désarmé affiche 0 même si sa scène joue.

La protection est enregistrée avec le show, dans sa section de DMX Total Control, et [importer le patch d’un autre show](patch.md#importer-dun-autre-show) l’apporte. À l’ouverture d’un show plus ancien ou de QLC+, un message dit une fois quels fixtures sont maintenant des effets protégés par leur classe (« 2 fixtures de ce show sont maintenant des effets spéciaux protégés : … ») ; aussi une machine à fumée dont le modèle dit « CO2 » ou un dimmer générique appelé « Fuego… » ou « Fire… ». Pour le changer, utilisez « Effet spécial ». QLC+ ne lit pas cette protection : ouvert dans QLC+, le show déclenche ces fixtures depuis n’importe où, sans armer et sans durée maximale. Les versions antérieures de DMX Total Control ne la lisent pas non plus : elles déclenchent ces fixtures sans armer et, en enregistrant le show, elles retirent la protection mise à la main, les durées et le canal de sécurité. Utilisez cette version sur tous les ordinateurs du show.

## Parquer des canaux

Un canal **parqué** sort toujours à une valeur fixe, au-dessus des scènes, des cuelists, des FX, du programmeur, du mixeur (Full, Flash…), du grand master et de Tout arrêter : le hazer à 30 % toute la nuit, une lyre en panne éteinte et immobile, la lumière de salle. Échap et Blackout ne le déparquent jamais.

Dans le [menu du fixture](patch.md#sélectionner) (clic droit dans la liste ou sur le plan) :

- **« Parquer tel quel »** : tous ses canaux à ce qui sort maintenant, donc le grand master, le mixeur et les limites de ce moment sont inclus. Impossible pendant l’arrêt du show ou un fondu au noir (leur lumière resterait parquée pour de bon).
- **« Parquer éteint »** : les canaux d’intensité (dimmers et couleurs) à 0 et les autres tels qu’ils sont maintenant ; un fixture sans canaux d’intensité (une machine à fumée ou un hazer de la bibliothèque), tout à 0. Pendant l’arrêt du show ou un fondu au noir, les autres canaux sont parqués à ce que le show envoie en dessous.
- **« Aussi en blackout »** : avec le Blackout, ses intensités parquées gardent leur valeur. Sans cela, elles s’éteignent avec le Blackout ; le pan, le tilt, le gobo et les autres canaux qui ne sont pas de la lumière restent toujours parqués.
- **« Déparquer »** et **« Tout déparquer »**.

Dans [Programmer](programar.md#parquer), **« Parquer »** parque les valeurs que le programmeur a sur les fixtures sélectionnés.

Avec des fixtures parqués, l’en-tête du Patch affiche **« N parqués · Tout déparquer »** (dans une fenêtre étroite, le cadenas, le nombre et le bouton avec seulement son icône). Dans la liste, un **cadenas** après le nom (« Parqué : N canaux avec une valeur fixe en sortie ») ; sur le plan 2D, du Patch et de Programmer, un cadenas en haut à droite. Dans [Niveaux DMX](salidas.md#niveaux-dmx), un cadenas sur chaque canal parqué.

- Un effet spécial protégé ne peut être parqué qu’**éteint** : « Parquer tel quel » ou « Parquer » de Programmer le parquent à 0 et le disent (« Un effet spécial protégé ne peut être parqué qu’éteint »). Parqué, il ne se déclenche jamais, même armé ; un déclenchement coupé en le parquant apparaît dans le journal comme « parqué ». Un fixture parqué allumé qui devient protégé est parqué éteint ; s’il est déprotégé ensuite, il reste parqué à 0 jusqu’à ce que vous le déparquiez. Déparqué, il ne se déclenche qu’après avoir relâché sa scène et l’avoir enfoncée de nouveau.
- Les canaux parqués suivent le fixture si vous le réadressez. Changer le mode d’un fixture retire ses canaux parqués (« Les canaux parqués de Spot 3 ont été retirés : son mode a changé »), sauf pour un effet protégé, qui reste parqué éteint. Supprimer un fixture retire les siens. Un pan parqué d’un fixture avec pan et tilt intervertis sort par le canal du tilt, comme le pan.
- L’[arrêt du show](directo.md#arrêt-du-show) l’emporte sur les canaux parqués : un fixture parqué éteint s’allume aussi, et un hazer parqué reste parqué. Un [fondu au noir](directo.md#fondu-au-noir) baisse les canaux parqués sans « Aussi en blackout ».

Les canaux parqués sont enregistrés avec le show, dans sa section de DMX Total Control (importer le patch d’un autre show ne les apporte pas). À l’ouverture d’un show avec des canaux parqués, ils s’appliquent tout de suite, et un message dit lesquels (« 2 fixtures parqués : HAZER, Spot 3 (Patch) »). Ouvrir un autre show ou un nouveau les retire. À la fermeture du programme, ils sont relâchés (la scène revient au repos), mais ils restent dans le show. QLC+ ne parque pas les canaux : là, ils jouent comme le disent leurs scènes, et la copie pour QLC+ le signale (« 2 fixtures parqués : QLC+ ne parque pas les canaux »). Les versions antérieures de DMX Total Control ne les appliquent pas et, en enregistrant le show, elles les retirent, comme les réglages de l’arrêt du show.

> **Non testé avec du matériel réel :** le parcage n’a pas été testé avec de vraies interfaces, de vrais nœuds ou de vrais hazers.

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

- **« Patch d’un autre show… »** : ses fixtures. Ils vont aux mêmes adresses si elles sont libres ; sinon, aux suivantes. Ils apportent leur protection d’effet spécial, leurs temps, leur canal de sécurité et leur icône du plan (ce que dit l’autre show ne déprotège jamais un CO2), mais pas leurs canaux parqués.
- **« Scènes d’un autre show… »** : ses scènes, assignées aux fixtures de ce show qui correspondent (par adresse et modèle, par nom ou par modèle). Elles entrent dans des groupes du même nom.
- **« Patch et scènes d’un autre show… »** : les deux.
- **« Patch d’un logiciel de visualisation (CSV)… »** : la table de fixtures qu’exporte votre logiciel de visualisation ; voir [Importer un patch d’un logiciel de visualisation](patch.md#importer-un-patch-dun-logiciel-de-visualisation).

## Importer un patch d’un logiciel de visualisation

Si vous concevez l’installation dans un logiciel de visualisation, exportez sa table de fixtures (CSV) et importez-la : DMX Total Control patche les mêmes fixtures, dans les mêmes univers et aux mêmes adresses, avec leurs noms, leurs numéros et leur place sur le plan.

1. Appuyez sur **« Importer » › « Patch d’un logiciel de visualisation (CSV)… »** et choisissez le fichier.
2. La fenêtre **« Importer un patch »** indique combien de fixtures contient la table, de combien de types et dans quels univers. Rien ne change tant que vous n’appuyez pas sur **« Importer »**.
3. Dans **« Types de fixture »**, chaque type de la table (fixture, mode et canaux) indique ce qu’il devient et comment il a été trouvé dans la bibliothèque : **« Exact »** (le même modèle et le même mode), **« Par nombre de canaux »** (le mode avec ces canaux), **« Nom proche »**, **« Mémorisé »** ou **« Choisi à la main »**. Un **« Nom proche »** s’affiche en orange pour que vous le vérifiiez : c’est un modèle au nom proche, pas le même. S’il n’est pas dans la bibliothèque, il est patché comme **« Dimmer générique »** avec ses canaux, en orange : tout reste à ses adresses malgré tout. Chaque type montre aussi l’icône qu’auront ses fixtures sur le plan ; s’il est patché comme dimmer générique, il prend celle que dit son nom dans la table (un « CO2 Jet » apparaît comme jet de CO2 ; un « Flame », comme flamme).
4. **« Changer »** choisit un autre fixture et un autre mode (**« Choisir dans la bibliothèque… »**), un dimmer générique ou **« Ne pas importer »**. Ce que vous choisissez à la main est mémorisé pour la prochaine fois que vous importez ce type.
5. Dans **« Options »** :
   - **« Les placer sur le plan de scène »**, avec la **Vue** : **« Vue de dessus »** (le public en bas du plan) ou **« Vue de face »** (comme le public voit l’installation : les fixtures les plus hauts en haut). La vue de face est proposée quand l’installation est accrochée à des hauteurs différentes, et la dernière que vous avez utilisée est mémorisée. Le plan s’agrandit pour contenir l’installation (il ne rétrécit jamais). Chaque import qui les place recentre l’installation sur le plan, même après avoir modifié ses côtés avec « − » et « + ».
   - **Groupes** : un par type de fixture, un par calque (si la table en a plus d’un) et ceux de la colonne « Groups » de la table.
6. Sous les types se trouvent les problèmes, avant d’importer : adresses déjà utilisées par un autre fixture ou une autre ligne (ces lignes ne sont pas importées), lignes sans univers ou sans adresse, univers au-delà de 128, adresses au-delà de 512, un second DMX break (non importé), numéros de fixture déjà pris par un autre fixture ou trop élevés (ils en reçoivent un autre), un pan ou tilt inversé qui n’est ni oui ni non, et les univers ajoutés au show.
7. Appuyez sur **« Importer N fixtures »**. Un message indique combien sont nouveaux et combien ont été mis à jour, et s’il reste un problème.

**De chaque ligne** sont importés le fixture et son mode, l’univers et l’adresse, le nom (la colonne « Unit ») et le numéro de fixture : celui de la colonne « Channel », sinon celui de l’autre colonne de numéro de fixture, sinon un numéro automatique. La hauteur de chaque lyre, si elle est posée (retournée dans la table) ou suspendue, et son pan ou tilt inversé vont à sa [hauteur et orientation](patch.md#hauteur-et-orientation-follow), que vous les placiez sur le plan ou non.

**Importer à nouveau** la table (après avoir changé l’installation dans le logiciel de visualisation) met à jour les mêmes fixtures : ils sont reconnus par l’identifiant de chaque ligne de la table, ou par le même nom et le même fixture, et leur mode, leur adresse, leur nom, leur numéro et leur place sur le plan changent. Ceux qui ne sont plus dans la table restent, sauf si vous cochez **« Supprimer les N fixtures importés avant qui ne sont plus dans la table »** (leurs noms s’affichent en dessous). Une icône que vous choisissez à la main ensuite est conservée, sauf si le fixture avec lequel son type est patché change.

> **Limites :** seul le premier DMX break de chaque fixture est importé, et des rotations on ne garde que le fait qu’une lyre soit retournée (l’orientation n’est pas importée). Vus de dessus, les fixtures accrochés l’un au-dessus de l’autre se retrouvent ensemble sur le plan : pour les voir séparés, utilisez la vue de face.

## Supprimer des fixtures

Sélectionnez-les et appuyez sur **« Supprimer »**. Le programme demande d’abord confirmation : leurs valeurs sont retirées de toutes les scènes.
