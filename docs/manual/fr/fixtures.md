# Fixtures

![La page Fixtures](../img/fixtures.png)

Pour patcher un appareil, le programme a besoin de sa **définition** : les canaux qu’il a, ce que fait chacun et les modes dans lesquels il fonctionne. Cette page explique d’où viennent les définitions et comment créer les vôtres dans la page **Fixtures** (F6).

## La bibliothèque de fixtures

Quand vous ajoutez des fixtures dans le [Patch](patch.md#ajouter-des-fixtures), vous choisissez parmi toutes ces définitions :

- **La bibliothèque intégrée** : des milliers de fixtures de nombreux fabricants, au format `.qxf`.
- **Les génériques** : 16 fixtures de base sous le fabricant **Generic** (PAR LED RGB, RGBA, RGBW, RGBWA, RGBWA+UV et blanc, barre LED de 8 segments, lyres spot, beam et wash, scanner, stroboscope, blinder, laser, machine à fumée et hazer), plus le **Generic Dimmer**.
- **L’Open Fixture Library**, si vous la téléchargez.
- **Les fixtures GDTF** que vous téléchargez depuis GDTF-Share ou importez.
- **Vos fixtures** : ceux que vous créez dans cette page ou que vous importez.

Dans [Options › Bibliothèque de fixtures](opciones.md#bibliothèque-de-fixtures), vous voyez combien de définitions sont disponibles et vous trouvez tout ce qu’il faut pour l’enrichir.

> **Conseil :** pour que Programmer, les FX et les palettes reconnaissent la fonction de chaque canal (dimmer, rouge, pan, gobo…), la définition doit fournir le groupe ou le preset du canal. Celles de la bibliothèque le fournissent ; dans les vôtres, choisissez bien le preset de chaque canal.

## Créer un fixture

La page **Fixtures** est le créateur de fixtures. À gauche, **« Mes fixtures »** liste ceux que vous avez créés ; à droite, l’éditeur.

1. Appuyez sur **« Nouveau fixture »**, ou sur **« Partir de la bibliothèque… »** pour partir d’une copie d’un fixture existant (il porte le même nom avec « (copie) »).
2. Dans **Fixture**, tapez le **Fabricant** (par exemple `Generic` ou la marque), le **Modèle** (`PAR 18x10W RGBWA`) et choisissez le **Type**.
3. Ajoutez les **canaux** (voir ci-dessous).
4. Préparez les **modes**.
5. Appuyez sur **« Enregistrer le fixture »**.

Le fixture est immédiatement disponible dans le Patch, sous son fabricant. En haut s’affiche son état : « Modifications non enregistrées », « Enregistré dans vos fixtures » ou « Pas encore enregistré ». Si vous ouvrez un autre fixture alors qu’il y a des modifications non enregistrées, le programme demande s’il faut les abandonner (« Continuer à modifier » / « Abandonner les modifications »).

### Canaux

Dans **Canaux**, le bouton **« Ajouter »** propose les canaux les plus courants (dimmer, rouge, vert, bleu, blanc, ambre, UV, pan et pan fin, tilt et tilt fin, stroboscope, roue de couleur, roue de gobos, vitesse de pan/tilt) et **« Canal personnalisé »**.

Pour chaque canal :

- Son **nom** et son **preset** (la liste déroulante) : le preset dit ce que fait le canal. Changer de preset change aussi le nom, sauf si vous lui en avez donné un vous-même.
- La corbeille le supprime.

Un canal **« Personnalisé »** a en plus :

- **Groupe** : sa fonction générale (intensité, couleur, gobo, faisceau…).
- **Couleur** : pour un canal d’intensité, la couleur qu’il représente (rouge, vert…) ou « Aucun » pour un dimmer.
- **Fin** : c’est le canal fin (16 bits) d’un autre canal.
- **« Plages (N) »** : ce que fait chaque plage de valeurs DMX, par exemple 0-7 Fermé, 8-255 Stroboscope. Appuyez sur **« Ajouter une plage »** et tapez sa **Description**. Les plages ne peuvent pas se chevaucher. Ce sont les presets que vous verrez ensuite en boutons dans Programmer.

Les nouveaux canaux s’ajoutent à la fin du mode ouvert.

### Modes

Dans **Modes**, **« Ajouter »** crée un mode avec tous les canaux. Chaque onglet est un mode :

- **Nom** du mode et la corbeille pour le supprimer (il en reste toujours un).
- La liste est l’**ordre DMX** du mode : cochez les canaux qu’il utilise et placez-les avec **▲ ▼** à leur position. Le numéro de gauche est son canal DMX dans le mode.

### Supprimer un de vos fixtures

Ouvrez le fixture dans « Mes fixtures » et appuyez sur **« Supprimer »**. Il disparaît de la bibliothèque au prochain démarrage.

Vos fixtures sont enregistrés en fichiers `.qxf` dans le dossier des fixtures utilisateur (dans Options › « Dossier des fixtures utilisateur » › « Ouvrir le dossier »).

## Open Fixture Library

L’Open Fixture Library est une bibliothèque ouverte avec des milliers de fixtures supplémentaires (licence MIT).

1. Ouvrez **Options** (F9).
2. Dans **Bibliothèque de fixtures › Open Fixture Library**, appuyez sur **« Télécharger »**. Il faut internet.
3. Attendez la fin de la barre de progression (« Annuler » l’arrête).

À la fin, le programme indique combien de fixtures ont été téléchargés et combien sont nouveaux. Ensuite, au même endroit, vous voyez combien elle en contient et quand elle a été téléchargée.

Pour la mettre à jour, appuyez sur **« Mettre à jour »** (le même bouton). Si « Rechercher les mises à jour au démarrage » est activé dans Options, le programme vérifie une fois par jour sur GitHub si la bibliothèque a de nouveaux fixtures ou des fixtures améliorés depuis votre téléchargement, vous prévient et met le bouton en évidence ; « Rechercher les mises à jour » vérifie sur-le-champ. La mise à jour :

- ajoute les nouveaux fixtures (ils sont listés en dessous) ;
- modifie ceux qui ont été améliorés, y compris ceux que vous avez patchés (si leur nouveau mode ne tient pas là où ils sont, ils gardent la version précédente jusqu’à ce que vous les patchiez à nouveau) ;
- conserve ceux que la bibliothèque n’a plus, au cas où un show les utilise ;
- ne touche ni à vos fixtures ni à ceux de la bibliothèque intégrée : s’ils ont le même fabricant et le même modèle, c’est le vôtre (ou celui de la bibliothèque intégrée) qui reste.

Un téléchargement illisible, ou interrompu, ne change rien.

## GDTF-Share

GDTF-Share est le site où les fabricants publient leurs fixtures en GDTF. Le programme l’utilise avec votre propre compte gratuit.

Il s’ouvre avec **« GDTF-Share… »** dans **Options › Bibliothèque de fixtures › Fixtures GDTF**, ou avec **« Rechercher sur GDTF-Share… »** dans **Patch › Ajouter des fixtures**. En bas, **« gdtf-share.com »** ouvre son site.

### Le compte

Appuyez sur **« Se connecter »** et tapez votre nom d’utilisateur et votre mot de passe gdtf-share.com. Ils ne sont envoyés qu’à gdtf-share.com. Si vous n’avez pas de compte, **« Créer un compte gratuit »** ouvre son site.

- Sans **« Se souvenir sur cet ordinateur »**, le mot de passe reste seulement en mémoire, jusqu’à la fermeture du programme : il est demandé une fois par session.
- Avec cette option, il est enregistré chiffré pour votre utilisateur Windows : toute personne utilisant ce compte Windows peut se connecter avec.
- Le nom d’utilisateur est toujours retenu.

Une fois connecté, le bouton affiche votre nom d’utilisateur. Son menu propose **« Se déconnecter »** (le mot de passe enregistré reste pour la prochaine fois) et, si vous l’avez enregistré, **« Oublier le mot de passe enregistré »**, qui l’efface de l’ordinateur. Après une déconnexion avec le mot de passe enregistré, le bouton **« Se connecter »** ouvre ce menu : **« Se connecter »** et **« Oublier le mot de passe enregistré »**.

### Rechercher

La liste de GDTF-Share est enregistrée sur l’ordinateur et fonctionne sans internet. Sa date est en haut (« Liste du… »). Elle est téléchargée à nouveau quand elle a plus de 20 heures, ou avec **« Actualiser la liste »** (les flèches). Si GDTF-Share envoie une liste vide, la précédente reste.

- Tapez une partie du fabricant ou du nom, dans n’importe quel ordre.
- Les filtres : **« Tous »**, **« Des fabricants »** (publiés par le fabricant lui-même), **« Dans ma bibliothèque »** et **« Mises à jour »** (ceux de votre bibliothèque qui ont une révision plus récente).
- À gauche, les fabricants de ce que vous trouvez et le nombre de fixtures de chacun.
- Chaque fixture indique qui l’a publié (**Fabricant** ou **Utilisateur**), sa note, et s’il est **Dans la bibliothèque**, s’il a une **Révision plus récente** ou si vous l’avez importé **Depuis un fichier**.
- 400 au plus sont affichés : tapez davantage pour affiner la liste.

### Télécharger et importer

Choisissez un fixture : ses détails sont à droite (qui l’a publié, la révision, la version GDTF, la taille et les canaux de chaque mode).

1. Pour une révision plus ancienne, choisissez-la dans **Révisions**.
2. Appuyez sur **« Télécharger et importer »**. Il est téléchargé et converti comme avec « Importer GDTF », avec une barre de progression. **« Annuler »** l’arrête. Si vous en téléchargez plusieurs à la fois, ceux déjà arrivés sont quand même importés. Le `.gdtf` d’origine est conservé (voir « Convertir à nouveau », plus bas).
3. Depuis Ajouter des fixtures, **« Le patcher »** choisit le fixture à patcher (aussi un fixture qui était déjà dans votre bibliothèque).

Si le fixture est déjà dans votre bibliothèque, le bouton est **« Mettre à jour vers cette révision »** ou **« Télécharger à nouveau »**. Si vous avez une autre publication du même fixture, un avis le signale : la nouvelle la remplace.

### Révisions plus récentes

Un fixture de votre bibliothèque a une révision plus récente quand GDTF-Share a une révision postérieure du même fixture. Sont comptés ceux que vous avez téléchargés depuis GDTF-Share, et ceux importés depuis un fichier dont la dernière révision a le même texte qu’une seule révision de GDTF-Share.

- Dans GDTF-Share, **« Mises à jour »** les liste et **« Tout mettre à jour »** télécharge la révision la plus récente de chacun. Les fixtures patchés avec eux la reçoivent.
- Dans **Options › Bibliothèque de fixtures**, la ligne **« Révisions plus récentes sur GDTF-Share »** indique combien il y en a, et **« Mettre à jour »** ouvre GDTF-Share sur « Mises à jour ».
- Avec le mot de passe enregistré et **« Rechercher les mises à jour au démarrage »** activé ([Options](opciones.md#mises-à-jour)), le programme se connecte tout seul une fois par jour au démarrage, télécharge la liste et signale si les fixtures que vous avez téléchargés depuis GDTF-Share ont des révisions plus récentes. Chaque révision est signalée une fois. S’il n’y arrive pas, il ne dit rien.

### Convertir à nouveau

Le programme conserve le `.gdtf` de chaque fixture que vous téléchargez depuis GDTF-Share. **« Convertir à nouveau »**, dans **Options › Bibliothèque de fixtures › Fixtures téléchargés depuis GDTF-Share**, les convertit de nouveau avec la version du programme que vous avez, sans les télécharger : ils reçoivent ainsi les améliorations d’une nouvelle version. Les fixtures patchés avec eux reçoivent le changement. La ligne apparaît quand il y a des fixtures téléchargés depuis GDTF-Share.

## Importer GDTF

GDTF est le format ouvert que les fabricants publient sur GDTF-Share (avec un compte gratuit).

1. Téléchargez les `.gdtf` de vos appareils depuis gdtf-share.com (ou le `.zip` de son téléchargement groupé). Pour les rechercher et les télécharger depuis le programme, voir GDTF-Share, plus haut.
2. Dans **Options › Bibliothèque de fixtures › Fixtures GDTF**, appuyez sur **« Importer GDTF… »** et choisissez les fichiers, les dossiers ou le `.zip`.
3. Ils sont convertis en arrière-plan, avec une barre de progression. **« Arrêter »** interrompt l’import.

À la fin, le programme indique combien ont été convertis et, si certains n’ont pas pu être lus, pourquoi. Si le même fixture est déjà dans une autre bibliothèque, celui de GDTF est ajouté avec « (GDTF) » et vous pouvez utiliser les deux. L’importer à nouveau le met à jour.

Un mode avec plusieurs *DMX breaks* (deux entrées DMX, ou une partie sur un autre univers) devient plusieurs modes. Le premier break garde le nom du mode. Chacun des autres est un autre mode, par exemple « Standard (DMX break 2) », avec seulement ses canaux, à partir de l’adresse 1. Patchez-le comme un autre fixture, à sa propre adresse : **« Ajouter des fixtures »** vous le rappelle quand vous choisissez le mode.

Les canaux virtuels (sans DMX) sont ignorés. Ils ne sont pas nécessaires : un fixture RGB sans dimmer s’allume par ses couleurs, et le grand master les baisse.

Certains canaux changent de fonction selon la valeur d’un autre canal (par exemple, la rotation du prisme ne tourne qu’avec le prisme sur « rotation »). Les fonctions qui ne font rien y sont ignorées et, là où il y en a plusieurs, chaque plage porte leurs noms ensemble (par exemple « 0 deg / Stop »).

Pour voir ces changements sur un fixture importé avant, importez de nouveau son `.gdtf`. Pour ceux téléchargés depuis GDTF-Share, appuyez sur « Convertir à nouveau ».

## Importer des fichiers de fixtures

**Options › Importer des fichiers de fixtures › « Importer… »** accepte des définitions `.qxf` (le format de QLC+), des fichiers `.gdtf` ou un `.zip` qui les contient. Ils sont ajoutés à la bibliothèque et le programme indique combien sont nouveaux.

### Importer mes fixtures QLC+

Si vous avez créé des fixtures dans QLC+ sur cet ordinateur, **Options › « Importer mes fixtures QLC+ » › « Importer »** les copie dans vos fixtures.

## Formats qui ne s’importent pas

Les bibliothèques d’autres programmes dans des formats fermés ou chiffrés ne peuvent pas être importées. La plupart des fixtures existent dans l’Open Fixture Library ou sur GDTF-Share, que vous pouvez consulter depuis le programme ; ceux qui manquent, créez-les dans cette page.
