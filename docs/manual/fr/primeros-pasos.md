# Premiers pas

![Le guide de premiers pas sur la page Patch](../img/primeros-pasos.png)

## Le guide

La première fois que vous ouvrez le programme, le **guide de premiers pas** apparaît : une carte en bas à droite avec cinq étapes (vos fixtures, un look, une scène, Direct et les playbacks). **« Suivant »** et **« Retour »** changent d’étape et vous emmènent à la page de chacune ; les pages restent utilisables pendant que la carte est ouverte. Vous la fermez avec la **✕** et vous la rouvrez dans Options › Aide › **« Lancer le guide »**.

À la première étape, **« Ouvrir le show d’entraînement »** ouvre l’exemple « Primeros pasos » (si vous avez des modifications non enregistrées, il vous demande d’abord s’il faut les enregistrer). Il contient déjà quatre PAR LED RGBW dans le groupe de fixtures `PARs`, deux lyres dans `Móviles` et le groupe « Mis escenas » vide dans Direct, prêt pour vos scènes.

## La session complète

Dans cette première session, vous allez faire tout le parcours : connecter la sortie, patcher quelques fixtures, construire un look dans le programmeur, l’enregistrer comme scène dans un groupe, le lancer dans Direct, enregistrer une cuelist sur un playback et enregistrer le show. Comptez une vingtaine de minutes.

Nous utiliserons des fixtures de la bibliothèque générique : quatre PAR LED RGBW et deux lyres. Si vous avez d’autres appareils, cherchez leur fabricant et leur modèle à la place des génériques.

> **Conseil :** vous pouvez suivre ce guide sans interface DMX connectée. Ce qui sort se voit dans Sorties › « Niveaux DMX » et sur le plan 2D de Programmer.

> **Conseil :** avec le show d’entraînement ouvert, vous avez déjà les fixtures et le groupe : passez à l’étape 4 et utilisez le groupe de fixtures `Móviles` là où ce guide indique `Lyres`.

## 1. Connectez la sortie

1. Branchez votre interface DMX USB à l’ordinateur (ou vérifiez que l’ordinateur est sur le même réseau que votre nœud Art-Net ou sACN).
2. Ouvrez **Sorties** (F7).
3. Sur la ligne de l’univers 1, ouvrez la liste déroulante qui indique « Non connecté » et choisissez votre interface.
4. Si elle n’apparaît pas, appuyez sur **« Actualiser »**.

L’univers connecté est marqué en vert. Vous trouverez plus de détails dans [Sorties](salidas.md).

## 2. Patchez quelques fixtures

1. Ouvrez **Patch** (F5) et appuyez sur **« Ajouter des fixtures »**.
2. Dans **Fabricant**, choisissez **Generic** (vous pouvez taper dans « Rechercher un fabricant… »).
3. Dans **Modèle**, choisissez **LED PAR RGBW**.
4. Dans le panneau **Patch** à droite :
   - **Mode** : « 5ch DRGBW ».
   - **Quantité** : 4.
   - **Univers** : 1. L’**Adresse** se place toute seule sur la première libre.
   - Laissez cochée **« En faire un groupe »** et tapez dans **Groupe** le nom `PARs`.
5. Appuyez sur **« Patcher 4 fixtures »**. En bas, vous voyez les adresses qu’ils utilisent.
6. Sans fermer la fenêtre, choisissez maintenant **Moving Head Spot**, mode « 11ch Standard », quantité 2, groupe `Lyres`, et appuyez sur **« Patcher 2 fixtures »**.
7. Appuyez sur **« Fermer »**.

> **Conseil :** réglez sur chaque appareil la même adresse DMX que celle de la liste du Patch. Pour trouver un appareil sur scène, sélectionnez-le et appuyez sur **« Identifier »** : il clignote en blanc.

## 3. Créez un groupe dans Direct

Les scènes se lancent dans Direct, organisées en groupes. Créez-en un avant d’enregistrer :

1. Ouvrez **Direct** (F1) et appuyez sur **« Nouveau groupe »**.
2. Cliquez sur le nom de la colonne (« Groupe 1 »), effacez-le, tapez `Mis escenas` et appuyez sur Entrée.
3. Dans la liste déroulante du temps de fondu enchaîné de la colonne (elle indique « Par scène »), choisissez **2 s**.

Le groupe est **« Exclusif »** : une seule scène joue à la fois et, quand vous en lancez une autre, il fait un fondu enchaîné.

## 4. Construisez un look dans le programmeur

1. Ouvrez **Programmer** (F3).
2. Dans le panneau des fixtures, cliquez sur l’étiquette du groupe **PARs** : les quatre sont sélectionnés.
3. Dans le panneau **Rapide**, appuyez sur **100%** à côté du fader **Dimmer**.
4. Cliquez sur l’échantillon de couleur **rouge**.

Les PAR s’allument en rouge dès que vous y touchez : ce que vous mettez dans le programmeur sort immédiatement, par-dessus tout.

> **Conseil :** même chose avec la ligne de commande : tapez `GROUP 1 @ FULL` et appuyez sur Entrée. `GROUP 1` est le premier groupe de fixtures. Voir [la ligne de commande](programar.md#ligne-de-commande).

## 5. Enregistrez la scène dans le groupe

1. Appuyez sur **« Enregistrer… »** (en haut à droite).
2. Dans **Enregistrer dans**, laissez **« Nouvelle scène »**.
3. Dans **Nom**, tapez `Rouge`.
4. Dans **Ajouter au groupe**, choisissez **Mis escenas**.
5. Appuyez sur **« Enregistrer »**.

Enregistrez un deuxième look : avec les PAR sélectionnés et le dimmer à 100 %, cliquez sur l’échantillon **bleu**, appuyez sur **« Enregistrer… »**, nom `Bleu`, groupe **Mis escenas** et **« Enregistrer »**.

Avant d’aller dans Direct, appuyez sur **« Tout réinitialiser »** pour vider le programmeur. Tant que le programmeur contient des valeurs, elles priment sur les scènes et vous ne verriez pas ce que vous lancez.

> **Conseil :** dans la fenêtre d’enregistrement, cochez **« Vider le programmeur après l’enregistrement »** et le programmeur se videra tout seul à chaque enregistrement. L’option est mémorisée.

## 6. Lancez les scènes dans Direct

1. Ouvrez **Direct** (F1).
2. Appuyez sur **Rouge** : la tuile s’allume avec la couleur du groupe et les PAR passent au rouge.
3. Appuyez sur **Bleu** : le groupe fait un fondu enchaîné de 2 secondes du rouge au bleu.
4. Bougez le **Master** de la colonne pour baisser l’intensité de tout le groupe.
5. Appuyez à nouveau sur **Bleu** pour la libérer, ou appuyez sur le carré de l’en-tête de la colonne (« Libérer ce groupe »).

Plus de détails dans [Direct](directo.md).

## 7. Enregistrez une cuelist sur un playback

Les cuelists avancent cue par cue avec GO. Le moyen le plus rapide d’en créer une est d’enregistrer directement sur un playback.

1. Dans **Programmer**, cliquez sur l’étiquette **Lyres**.
2. Appuyez sur **« Beam on »** (dimmer à fond et shutter ouvert) et **« Centrer »** (pan et tilt au centre).
3. Ouvrez **Playbacks** (F10) et appuyez sur **« Enregistrer »** : le bouton clignote en rouge.
4. Cliquez sur le playback **1**, qui est vide : une nouvelle cuelist est créée avec la cue 1.
5. Revenez dans **Programmer**, ouvrez **Fonctions**, onglet **Position**, et faites glisser la croix de la grille XY ailleurs.
6. Dans **Playbacks**, appuyez sur **« Enregistrer »** et cliquez à nouveau sur le playback 1 : la cue 2 est enregistrée, avec seulement ce qui a changé (la position).
7. Revenez dans **Programmer** et appuyez sur **« Tout réinitialiser »**.
8. Dans **Playbacks**, cliquez sur le nom du playback 1 pour le sélectionner et appuyez sur **Espace** (ou sur le grand bouton **GO**) : la cue 1 joue. Appuyez encore : elle passe à la cue 2.

Le fader du playback est son intensité : si vous le montez depuis zéro, la cuelist démarre. Plus de détails dans [Playbacks](playbacks.md).

## 8. Enregistrez le show

1. Appuyez sur **Ctrl+S** (ou sur la disquette de la barre du haut).
2. Comme c’est la première fois, choisissez un dossier et un nom : il est enregistré dans un fichier `.qxw`.

À partir de là, Ctrl+S enregistre dans le même fichier. Avec la sauvegarde automatique active (voir [Options](opciones.md#sauvegarde-automatique)), le programme enregistre tout seul toutes les quelques minutes et garde la version précédente en `.bak`.

## Et maintenant…

- Construisez votre écran d’exploitation dans le [Show](show.md).
- Apprenez à utiliser les [palettes](programar.md#palettes) et les [FX](programar.md#fx) dans le programmeur.
- Modifiez les cues, leurs temps et leur tracking dans [Scènes](escenas.md#cuelists).
- Branchez un contrôleur MIDI dans [Mappages](mapeos.md).
