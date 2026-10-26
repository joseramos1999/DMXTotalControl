# Show

![La page Show](../img/show.png)

La page **Show** (F2) est votre écran d’exploitation : vous le construisez vous-même, sur mesure, avec des boutons, des faders, des rotatifs, des groupes, des playbacks, une horloge, des notes… Elle est pensée pour les écrans tactiles, pour rester en plein écran pendant le show et pour être utilisée aussi depuis le téléphone avec la [télécommande](remoto.md). Elle est enregistrée avec le show.

Un SHOW a des **pages**. Chaque page est faite de **sections** qui remplissent l’écran. Chaque section peut être divisée en deux moitiés, côte à côte ou l’une au-dessus de l’autre, autant de fois que vous le voulez, et chaque section affiche une chose : un tableau de boutons, un tableau de faders, un groupe, une horloge…

## Commencer

Avec un nouveau show, la page est vide et propose deux boutons :

- **« Nouvelle page »** : crée une page avec un tableau de boutons et passe en mode édition.
- **« Partir de mes groupes »** : crée la page « Groupes » avec tous vos groupes de Direct en haut et, en bas, un tableau de faders « Masters » (le grand master et les masters des premiers groupes), une section Rythme et une horloge. Ce bouton n’apparaît que si vous avez déjà des groupes.

## Le mode édition

Appuyez sur **« Modifier »** pour construire l’écran et sur **« Terminé »** pour revenir à son utilisation. En mode édition :

- Cliquez sur une section pour la **sélectionner** (dans un tableau, sur une de ses cellules ou sur son étiquette en haut à droite). Ses propriétés apparaissent dans le panneau **« Propriétés »** à droite.
- Quand la souris passe sur une section, et sur la section sélectionnée, une étiquette avec son type et trois boutons apparaît en haut à droite : **diviser en deux moitiés côte à côte**, **diviser l’une au-dessus de l’autre** et **retirer cette section** (dans une section avec un titre, ils sont dans sa barre de titre). Les mêmes se trouvent dans les propriétés : « Côte à côte », « Haut et bas » et « Retirer la section ».
- Les sections gardent leur hauteur : elles n’ont pas de barre en plus en édition. Avec le panneau des propriétés, elles ne se rétrécissent que jusqu’à la largeur où leurs tableaux tiennent sans défiler ; si cela ne suffit pas, la page défile sur les côtés avec la barre du bas (au plus, elle est aussi large que sans le panneau).
- Faites glisser le **séparateur** entre deux moitiés pour changer leur taille. Il s’aimante aux quarts, aux tiers et aux moitiés.
- Une section **« Vide »** affiche des boutons avec tous les types : cliquez sur l’un d’eux pour choisir ce qu’elle affiche. Si la section est petite et qu’ils ne tiennent pas, il ne reste qu’un bouton, **« Ce qu’elle affiche ▾ »**, avec la liste.

### Types de section

| Type | Ce qu’elle affiche |
|---|---|
| **Boutons** | Un tableau de boutons (colonnes × lignes). |
| **Faders** | Une rangée de faders. |
| **Un groupe** | Un groupe de Direct avec de grands boutons. |
| **Tous les groupes** | Tous les groupes de Direct, côte à côte. |
| **Playbacks** | Les playbacks de la page Playbacks, avec leur GO, leurs boutons et leur fader. |
| **Mixeur en direct** | Le [mixeur](directo.md#mixeur) de Direct. |
| **Horloge** | L’heure, avec secondes et date en option. |
| **Minuteur** | Un chronomètre ou un compte à rebours. |
| **Rythme : GO et Stop** | De grands **GO** et **Stop**, comme ceux de la barre du haut (ils changent ce que font les boutons des scènes), et **BeatGO**. Les boutons de scènes à pas, de cuelists ou de FX en « BPM du show » contiennent leur barre de rythme. |
| **Notes** | Un texte à vous, de la taille que vous voulez. |
| **En cours** | Ce qui joue, avec un bouton pour l’arrêter. |
| **Rotatifs et roues** | Un tableau de rotatifs ou de molettes sans fin. |
| **Roue de couleur** | Une roue pour donner une couleur à un groupe de fixtures. |
| **Grille XY (pan et tilt)** | Une zone pour bouger des lyres. |
| **Image** | Une image (un logo, un plan…). |
| **Niveaux DMX** | Les 512 canaux d’un univers, en direct. |
| **Vide** | Rien (en mode édition, la liste des types). |

### Propriétés de la section

Selon le type, le panneau des propriétés affiche :

- **Affiche** : le type de section.
- **Titre** : le texte de sa barre de titre. Sans titre, la section n’a pas de barre, en mode édition non plus.
- **Fond** : la couleur de fond de la section, parmi les mêmes couleurs que les boutons, ou **∅** (celle du thème). Elle apparaît comme une teinte, en édition, en jeu et sur le téléphone, et le texte posé dessus reste lisible.
- **Couleur de tous les boutons** (de tous les faders, de tous les rotatifs) : leur donne cette couleur à tous dans le tableau d’un coup. Les cellules vides restent vides.
- **Colonnes** et **Lignes** : la taille du tableau (jusqu’à 32). Les boutons, faders et rotatifs remplissent leurs cellules. Dans une section trop petite pour son tableau, les cellules ne rétrécissent pas au-delà de ce qui se lit et se touche : le tableau défile (à la molette ou en faisant glisser).
- **Remplir avec les scènes d’un groupe** (boutons) : place d’un coup un bouton pour chaque scène de ce groupe.
- **Groupe** (un groupe) : lequel elle affiche.
- **Fixtures** (roue de couleur et grille XY) : le groupe de fixtures sur lequel elles agissent, ou « Tous les fixtures avec couleur » / « Toutes les lyres ».
- **Image** (image) : le fichier, et « Image entière » ou « Remplir la section ».
- **Secondes** et **Date** (horloge).
- **Compte** (minuteur) : « Croissant (chronomètre) » ou « Décroissant (compte à rebours) », avec ses **Minutes**.
- **Texte** et **Taille** (notes).
- **Univers** (niveaux DMX).
- **Page de playbacks**, **Playbacks** et **Premier playback** (playbacks) : voir [Playbacks dans le SHOW](show.md#playbacks-dans-le-show).

## Boutons

1. Sélectionnez une section de boutons et cliquez sur une cellule vide (celle du **+**).
2. Appuyez sur **« Ajouter un bouton ici »**.
3. Choisissez son **Action** et ce dont elle a besoin (la scène, le groupe, la palette…).

Le moyen le plus rapide de remplir un tableau est de **faire glisser des scènes** depuis la liste sous les propriétés (« Faites glisser une scène sur une cellule : ») jusqu’aux cellules : chaque scène devient un bouton qui la lance.

### Actions d’un bouton

| Action | Ce qu’il fait |
|---|---|
| **Lancer une scène** | Lance la scène dans son groupe (ou seule, si elle n’est dans aucun groupe). Appuyé plusieurs fois de suite, il marque le [rythme](directo.md#rythme-dune-scène-à-pas) d’une scène à pas, d’une cuelist ou d’une scène FX avec des FX en « BPM du show ». Avec **Stop** dans la barre du haut, il l’arrête. |
| **Scène suivante d’un groupe** / **Scène précédente d’un groupe** | Dans un groupe, ou dans « Tous les groupes ». |
| **Libérer un groupe** | Un groupe, ou tous. |
| **Blackout** | Active ou retire le blackout. |
| **Tout arrêter** | Comme le bouton de la barre du haut. |
| **Tap tempo** | Marque le tempo du show (celui des FX et des scènes en mode BPM). |
| **BeatGO (division suivante)** | Comme le bouton BeatGO de la barre du haut. |
| **Une palette** | Applique la palette aux fixtures avec lesquels elle a été enregistrée (« Ses propres fixtures ») ou à un groupe de fixtures, comme une modification en direct du programmeur : elle passe par-dessus les scènes jusqu’à ce que vous appuyiez à nouveau. |
| **GO de cuelist** / **Retour de cuelist** / **Libérer une cuelist** | Sur la cuelist que vous choisissez dans **Cuelist**. |
| **Aller à une cue…** | Saute à la **Cue** que vous choisissez dans cette cuelist. |

Un bouton apporté d’une autre page avec Alt+clic affiche « Un contrôle sur une autre page » et, en dessous, ce qu’il fait (« Fait : … »).

### Options d’un bouton

- **Étiquette** : son texte. Vide (« Automatique »), elle affiche le nom de la scène, de la palette ou de l’action.
- **Comportement** (scène, blackout et palette) : **« Bascule : on / off »** ou **« Flash : tant qu’on appuie »**. Les boutons flash portent un éclair dans le coin. Sur une scène, à la place de « Bascule » s’affiche **« Appuyer : selon GO / Stop »** : il la lance ou l’arrête selon **GO** et **Stop** de la barre du haut.
- **Touche du clavier** : une touche du clavier qui l’actionne tant que vous êtes sur la page Show. La lettre s’affiche dans le coin du bouton.
- **Couleur**, **Icône** et **Image** de fond (l’image est atténuée tant que le bouton est éteint).

Un bouton allumé se remplit de sa couleur.

### Déplacer et retirer

En mode édition, faites glisser un bouton, un fader ou un rotatif vers une autre cellule de la même section pour le déplacer. Pour le retirer, sélectionnez-le et appuyez sur **Suppr** (ou « Retirer le bouton » / « Retirer le fader » dans les propriétés). **Échap** annule la sélection.

## Faders

Un fader du SHOW peut contrôler **plusieurs choses à la fois**. Dans ses propriétés, sous **Contrôle**, chaque élément est l’un de ceux-ci :

| Élément | Ce que fait le fader |
|---|---|
| **Lancer une scène (son intensité)** | En montant depuis 0, la scène démarre dans son groupe (en respectant s’il est exclusif) ; en descendant à 0, elle s’arrête ; entre les deux, c’est son intensité. |
| **Dimmer d’une scène (sans la lancer)** | Atténue la scène, quel que soit ce qui la lance (son groupe, un playback, un bouton, une super scène). Il ne la lance pas et ne l’arrête pas. Son niveau est enregistré avec le show et dure tant qu’il existe un fader ou un rotatif de dimmer sur cette scène : si vous le retirez, la scène revient à 100 %. |
| **Master d’un groupe** | Le master de ce groupe de Direct. |
| **Grand master** | Le GM de la barre du haut. |
| **Dimmer de fixtures** | Le dimmer du mixeur de tous les fixtures ou d’un groupe de fixtures. |
| **Vitesse de mouvement des lyres** | La vitesse de ce qui fait bouger les lyres (toutes ou celles d’un groupe) : leurs FX de pan/tilt, ceux du programmeur et les pas qui les déplacent. En bas ×0,1, au milieu la vitesse normale (×1), en haut ×10 ; le fader affiche le multiplicateur et un double-clic le remet à ×1. Un FX de pan/tilt et de couleur ne change que la vitesse du mouvement ; une cuelist, celle de ses FX. Le canal de vitesse des moteurs n’est pas touché. |
| **Vitesse d’une scène** | La vitesse en direct de cette scène (×0,1 à ×10, comme celle du mouvement). |
| **Lancer une scène (sa vitesse)** | Comme « Lancer une scène (son intensité) », mais le fader est sa vitesse : tout en bas la scène est arrêtée ; en le montant, elle démarre dans son groupe (en respectant s’il est exclusif) et la course est sa vitesse, ×0,1 juste au-dessus du bas, ×1 au milieu et ×10 en haut. Le texte affiche « Arrêtée » tant qu’elle ne joue pas, et le fader suit la scène si on l’arrête ou la lance d’ailleurs. La vitesse ne vaut que pour cette fois : une fois arrêtée, la scène reprend la vitesse qu’elle avait (celle du Direct ne change pas). Un double-clic la met à ×1 (et la lance si elle était arrêtée). Si un fader lance plusieurs scènes d’un même groupe exclusif, elles ne se remplacent pas : la première joue. |
| **BPM** | Le tempo du show, de 20 à 300. |

La différence entre les deux éléments de scène : **« Lancer une scène (son intensité) »** est la scène elle-même, et il la lance et l’arrête ; **« Dimmer d’une scène (sans la lancer) »** ne fait que l’atténuer, même quand elle est lancée d’ailleurs. Vous pouvez utiliser les deux en même temps : avec le dimmer à 50 % et le fader qui la lance à 80 %, la scène sort à 40 %.

Appuyez sur **« Ajouter un autre élément »** pour en ajouter un et sur l’icône de corbeille pour le retirer. Si vous faites glisser une scène sur une cellule de faders, un fader qui la lance est créé.

Avec **Plage**, **De** … **à** … (de 0 à 100 %), la course du fader ne couvre que cette partie de ce qu’il pilote, pour tous ses éléments. Par exemple, la vitesse d’une scène de 20 à 80 %, ou un dimmer de 0 à 60 % pour qu’il ne dépasse pas cette valeur. Avec « De » au-dessus de « à », ou avec **« Inverser »**, le fader fonctionne à l’envers. Un fader qui lance une scène l’arrête à l’extrémité où la valeur est la plus basse (tout en bas ; tout en haut s’il est inversé), même si « De » n’est pas 0.

Le texte du fader affiche la valeur réelle (le %, le BPM ou, s’il ne contrôle que des vitesses, le multiplicateur ×), pas la position du fader. Si un autre contrôle amène la valeur hors de la plage, le fader reste à l’extrémité. Un fader qui ne contrôle que des vitesses (du mouvement ou de scènes, une ou plusieurs) revient à ×1 avec un double-clic.

Sous chaque fader, il y a un bouton **« Flash »** : il le porte tout en haut de sa course tant que vous appuyez et, quand vous relâchez, il rend exactement ce qu’il y avait. Il s’enlève ou se remet avec l’interrupteur « Bouton flash : à fond tant qu’on appuie » de ses propriétés.

> **Conseil :** un fader avec « Master d’un groupe » de vos washes et « Dimmer de fixtures » des PAR du public vous donne un seul fader pour baisser toute la face.

## Rotatifs et roues

Les rotatifs contrôlent la même chose que les faders (le tableau ci-dessus), avec la même **Plage**. Chaque rotatif peut être :

- Un **rotatif** : il parcourt toute sa plage en trois quarts de tour. Un double-clic le ramène au début de sa course (la valeur de « De », ou celle de « à » s’il est inversé) ; s’il contrôle le BPM, à 90, ou à l’extrémité de la plage la plus proche si 90 est en dehors ; s’il ne contrôle que des vitesses, à ×1.
- Une **molette incrémentale** (« Molette incrémentale : tourne sans fin, chaque cran ajoute ou retire un pas ») : elle tourne sans fin, et chaque cran ajoute ou retire le **Pas** (de 1 à 25 %) à la valeur actuelle. Le pas est un % de la valeur, pas de la course, et la molette ne sort pas de la plage ; quand vous la tournez vers le haut avec la scène arrêtée, elle la lance sur « De ». L’arc qui l’entoure montre le niveau.

Pour les tourner, faites glisser autour du centre avec la souris ou le doigt, ou de haut en bas près du centre. Chaque cran de la molette de la souris les tourne un peu (sur une molette incrémentale, d’un cran).

## Roue de couleur et grille XY

- **Roue de couleur** : touchez la roue pour donner teinte et saturation (blanc au centre) aux fixtures de la section, ou appuyez sur l’un des échantillons du bas.
- **Grille XY (pan et tilt)** : faites glisser le point pour bouger les lyres. **« Centre »** les ramène au centre.

Les deux agissent comme une modification en direct du programmeur : elles passent par-dessus les scènes jusqu’à ce que vous appuyiez sur **« Libérer »**, et les scènes rendent alors la couleur ou la position.

## Groupes et mixeur

Les sections **Un groupe** et **Tous les groupes** affichent les colonnes de [Direct](directo.md) avec de grands boutons, leurs précédent/suivant, leur bouton de libération et leur master, sans options de modification. **Mixeur en direct** affiche les bandes du mixeur ajustées à la largeur.

Les groupes se **replient** aussi ici, avec la flèche à côté de leur nom : dans **Tous les groupes**, le groupe devient une bande étroite et les autres se partagent la largeur ; une section **Un groupe** qui occupe une moitié de la page se replie sur sa bande (ou sur son en-tête, si les moitiés sont l’une au-dessus de l’autre) et l’autre moitié s’agrandit. Leurs scènes continuent de jouer, et c’est enregistré avec la page (indépendamment du repli dans Direct). Dans un SHOW verrouillé, les groupes ne se replient pas.

## Playbacks dans le SHOW

La section **Playbacks** affiche des bandes identiques à celles de la page [Playbacks](playbacks.md) :

- **Page de playbacks** : **« Suivre la page actuelle »** (la barre de titre affiche ◀ le nom de la page ▶ pour la changer) ou une page fixe.
- **Playbacks** : combien sont affichés (jusqu’à 20) et **Premier playback** : à partir duquel.
- Trois interrupteurs pour afficher ou masquer le **deuxième bouton (Pause / Retour)**, le **troisième bouton (Flash)** et **la cue qu’il joue et la suivante**.

En mode édition, déposez une scène sur une bande pour la mettre sur ce playback.

## Utilitaires

- **Horloge** : heure et date de l’ordinateur.
- **Minuteur** : **« Démarrer »** / **« Pause »** et **« Réinitialiser »**. En compte à rebours, il devient orange dans la dernière minute et rouge en arrivant à zéro. Il continue de compter même si vous changez de page.
- **Rythme** : **GO**, **Stop** et **BeatGO**.
- **Notes** : le texte que vous écrivez dans ses propriétés (la conduite, des consignes pour l’opérateur…).
- **En cours** : ce qui joue, avec le bouton d’arrêt à côté de chaque scène.
- **Niveaux DMX** : une grille avec les 512 canaux de l’univers choisi.
- **Image** : entière ou remplissant la section.

## Pages

- En mode édition, **« + Page »** ajoute une page.
- Cliquez sur le nom d’une page pour l’afficher. Les onglets de page s’affichent s’il y en a plus d’une.
- En mode édition, un double-clic sur un onglet le renomme ; le clic droit ouvre **« Renommer… »**, **« Déplacer à gauche »**, **« Déplacer à droite »** et **« Supprimer la page… »**. À la suppression, le programme indique combien de sections et de contrôles partent avec elle : impossible d’annuler.

## Envoyer n’importe quel contrôle au SHOW

**Alt+clic** sur presque n’importe quel contrôle du programme le copie sur la page du SHOW que vous avez ouverte : une scène de Direct, le master d’un groupe, un bouton du mixeur, le grand master, le blackout, une palette de Programmer, les boutons et le fader d’un playback… Le programme l’indique par « Ajouté au SHOW, page « … » ».

Le nouveau contrôle va dans la première cellule libre d’un tableau de boutons ou de faders de cette page. S’il n’y en a aucune, une section vide en devient une (avec le titre « Tactile »), ou la dernière section est divisée.

## Plein écran et verrouillage

- **« Plein écran »** (F11) et **« Quitter le plein écran »**.
- **« Verrouiller »** : masque la barre latérale et ne laisse que le SHOW, sans modification ni navigation. Pour sortir, **maintenez « Déverrouiller » enfoncé** (une barre se remplit sous le bouton). S’il y a un mot de passe de verrouillage, il est demandé.

### Verrouillage et mode borne

- Dans Options › « SHOW et verrouillage », **« Ouvrir le SHOW en plein écran au démarrage »** ouvre directement le SHOW. Avec **« …et verrouillé »**, il démarre en plus verrouillé : seulement le SHOW, comme une borne.
- Si vous verrouillez la **« Modification du SHOW »** avec le cadenas de la barre du haut, le bouton « Modifier » disparaît : les pages s’utilisent, mais ne se modifient pas.

Voir [Options](opciones.md#show-et-verrouillage).

## Touches, mappages et télécommande

- Les **touches** des boutons ne fonctionnent que sur la page Show. C’est pourquoi, sur cette page, la touche G ne fait pas BeatGO.
- En mode Mapper, les boutons et les faders du SHOW peuvent être assignés à un contrôleur ; un contrôle MIDI peut bouger un fader du SHOW entier, avec tous ses éléments (voir [Mappages](mapeos.md)).
- Les pages du SHOW s’affichent et s’utilisent sur le téléphone ou la tablette avec la [télécommande](remoto.md).
