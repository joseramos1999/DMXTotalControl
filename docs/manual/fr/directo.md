# Direct

![La page Direct](../img/directo.png)

**Direct** (F1) est la page pour lancer des scènes pendant le show. Les scènes sont rangées en **groupes**, qui sont des colonnes fonctionnant en parallèle : chaque groupe a ses scènes, son mode (exclusif ou mélange), son temps de fondu enchaîné et son master.

Dans l’en-tête de la page, vous avez :

- **◀ ▶** : scène précédente ou suivante dans chaque groupe qui joue (aussi avec Ctrl+← et Ctrl+→).
- **« Pause »** / **« Reprendre »** : fige tout ce qui joue et le relance.
- **« Libérer les groupes »** : arrête les scènes de tous les groupes.
- **« Replier les groupes »** / **« Déplier les groupes »** : replie toutes les colonnes d’un coup, pour les voir toutes ; quand elles sont toutes repliées, les déplie.
- **« Mixeur »** : passe à la vue du [mixeur](directo.md#mixeur). Un point orange indique que le mixeur modifie la sortie.
- **« Nouveau groupe »** : ajoute une colonne.

## Groupes

### Créer et organiser les groupes

1. Appuyez sur **« Nouveau groupe »** (ou sur le cadre « Ajouter un groupe » au bout des colonnes).
2. Cliquez sur le nom de la colonne pour le changer (par exemple `Wash`, `Spots`, `Strobos`, `Effets`) et appuyez sur Entrée.

Le bouton en forme d’engrenage de chaque colonne ouvre son menu : **« Changer de couleur »** (passe à la couleur suivante), **« Déplacer à gauche »**, **« Déplacer à droite »** et **« Supprimer le groupe »**. Vous pouvez aussi faire glisser le groupe par la poignée à gauche de son nom et le déposer sur un autre groupe pour prendre sa place.

La flèche **‹** à côté du nom **replie** le groupe : il devient une bande étroite avec son nom et le nombre de scènes qui jouent, pour laisser de la place aux autres. Ses scènes continuent de jouer. La flèche **›** de la bande le déplie. C’est enregistré avec le show. Pour les replier ou les déplier tous d’un coup, utilisez **« Replier les groupes »** / **« Déplier les groupes »** dans l’en-tête.

### Ajouter et ordonner des scènes

1. Appuyez sur **« Ajouter des scènes »** au bas de la colonne.
2. Dans la liste, cliquez sur les scènes que vous voulez (elles sont cochées d’un ✓). Vous pouvez chercher par nom ou par dossier, et les afficher par dossiers ou par groupes.
3. Appuyez sur **« Ajouter N »**. Un double-clic sur une scène l’ajoute directement.

Pour réordonner, survolez une tuile et faites-la glisser par sa poignée (à droite de la tuile) : déposez-la sur une autre scène pour la placer devant, ou dans l’espace vide de la colonne pour la placer à la fin. Vous pouvez la déposer dans **un autre groupe** : elle s’y déplace avec ses options. Dans le menu de la tuile (clic droit), vous avez aussi **« Monter »**, **« Descendre »** et **« Retirer du groupe »**.

> **Conseil :** quand vous enregistrez depuis Programmer, vous pouvez mettre la scène directement dans un groupe avec « Ajouter au groupe ». Voir [Enregistrer](programar.md#enregistrer).

## Lancer des scènes

Cliquez sur une tuile pour lancer la scène (avec **GO**, le mode dans lequel démarre le programme). Pour la libérer, appuyez sur le carré de l’en-tête du groupe (« Libérer ce groupe ») ou passez sur **Stop** et appuyez dessus. La tuile qui joue prend la couleur du groupe et affiche un point qui bat.

Sur chaque tuile, vous voyez :

- Le **nom** et, en dessous, le **type** de scène. Pour une cuelist en cours, la cue où elle se trouve (« Q 3 · Couplet »). Derrière, le [rythme tapé](directo.md#rythme-dune-scène-à-pas) s’il y en a un (« 120 BPM »).
- **P7**, **P2**… : la priorité, seulement quand ce n’est pas la normale (5). En orange si elle est supérieure à 5.
- Un **éclair** : c’est une scène flash.
- Une **barre coupée en deux** en bas : une scène à pas, une cuelist ou un FX en « BPM du show » en cours ; elle alterne à chaque pas (dans un FX, à chaque battement), à son rythme.

### Exclusif ou mélange

Le bouton de chaque colonne alterne entre :

- **« Exclusif »** : une seule scène joue à la fois. Quand vous en lancez une autre, il fait un fondu enchaîné vers la nouvelle. Les [faders du SHOW](show.md#faders) qui lancent des scènes sont l’exception : ils ajoutent la leur, vous pouvez donc en monter plusieurs du même groupe à la fois.
- **« Mélange »** : les scènes s’additionnent ; plusieurs peuvent jouer en même temps.

### Temps de fondu enchaîné

La liste déroulante à côté de « Exclusif »/« Mélange » est le temps de fondu du groupe :

- **« Par scène »** : chaque scène utilise ses propres fondus d’entrée et de sortie.
- **« Cut »** : sans fondu.
- **0,5 s à 10 s** : ce temps pour toutes les scènes du groupe, à la place des leurs.

### Scène précédente et suivante

Les boutons **◀ ▶** de chaque colonne lancent la scène précédente ou suivante du groupe, dans l’ordre de la colonne. Ceux de l’en-tête de la page font la même chose dans tous les groupes qui jouent.

### Scènes flash

Une scène **flash** ne joue que tant que vous maintenez sa tuile enfoncée. Elle s’active dans ses [propriétés](directo.md#propriétés-de-la-scène) avec « Flash (joue tant qu’on appuie) ».

### Cuelists dans un groupe

Une cuelist dans un groupe est une tuile comme les autres : le clic la lance (avec **Stop**, l’arrête). Son menu (clic droit) ajoute **« GO (cue suivante) »**, **« Retour (cue précédente) »** et **« Libérer la cuelist »**. Pour l’envoyer confortablement avec GO, mettez-la sur un [playback](playbacks.md).

### Rythme d’une scène à pas

Les scènes à pas (séquences et chenillards), les cuelists et les scènes FX avec des FX en « BPM du show » suivent le rythme auquel vous appuyez sur leur tuile :

- Un clic la lance (avec **GO**, le mode dans lequel démarre le programme ; voir plus bas).
- Pendant que la scène joue, appuyez sur sa tuile **plusieurs fois de suite** au rythme de la musique, avec moins de 2 secondes entre les appuis. Chaque appui passe au pas suivant (dans une cuelist, il donne un GO) et la scène continue seule à ce rythme : un pas à chaque battement, au lieu de ses propres temps. Le rythme est la moyenne des derniers appuis. Un appui qui arrive un peu en retard, juste après le pas de ce battement (moins d’un demi-battement), est ce même pas : il n’en donne pas d’autre, il remet seulement le rythme en place.
- Seuls comptent les appuis pendant que la scène joue : celui qui la lance commence le compte. Ainsi, changer de scène dans un groupe exclusif puis revenir ne donne de rythme à aucune.
- Après une pause de plus de 2 secondes, un clic donne un pas sans changer son rythme, et la scène continue de jouer. Pour l’arrêter, passez sur **Stop** et appuyez dessus.
- Le rythme tapé est joué même avec une **vitesse en direct** réglée ; si vous changez ensuite la vitesse, le rythme accélère ou ralentit avec elle. En pause, les appuis ne font pas avancer les pas.
- Le rythme est enregistré avec la scène et lui appartient : elle le suit aussi quand elle joue dans une super scène ou sur un playback. Dans ses [propriétés](directo.md#propriétés-de-la-scène) (et dans [Scènes](escenas.md#comment-joue-une-scène)), vous pouvez le saisir ou appuyer sur **« Retirer »** pour revenir à ses temps.
- Dans une **scène FX**, le rythme est celui de ses FX en **« BPM du show »** : le premier appui d’une série relance leur cycle (comme au lancement), et chacun des suivants les amène au battement suivant, un cycle plus loin fois leur multiplicateur (un chenillard passe au fixture suivant ; à ×¼, ils avancent d’un quart de cycle). Plusieurs de suite leur donnent un cycle à chaque battement (fois leur multiplicateur), au lieu du BPM du show. Les FX à vitesse propre ne changent pas. Sans rythme tapé, ou après « Retirer », ils suivent le BPM du show. Si vous désactivez « BPM du show » sur tous ses FX, la scène perd son rythme tapé.

Cela fonctionne de la même façon avec ses boutons du [SHOW](show.md), avec ses mappages de contrôleurs et de touches en mode « Bascule » et avec la [télécommande](remoto.md).

#### GO et Stop

Les boutons **GO** et **Stop** de la barre du haut (et de la section « Rythme » du SHOW) décident de l’effet d’un appui sur n’importe quelle scène, pour tous ceux qui appuient. L’un des deux est toujours allumé : le programme démarre toujours en **GO**, et appuyer sur l’autre change de mode :

- **GO** allumé : appuyer sur une scène la **lance** ; sur une scène qui joue déjà, chaque appui avance d’un pas (dans une cuelist, un GO ; dans une scène FX, le premier relance le cycle de ses FX en « BPM du show » et les suivants les amènent au battement suivant) et plusieurs appuis de suite marquent son rythme. Elle ne s’arrête jamais, même si vous restez plus de 2 secondes sans appuyer : vous marquez ainsi le tempo tranquillement.
- **Stop** allumé : appuyer sur une scène l’**arrête**. Une scène qui ne joue pas reste telle quelle.
- Pour libérer les scènes d’un groupe sans changer de mode, appuyez sur le carré de son en-tête (« Libérer ce groupe ») ; **« Tout arrêter »** (Échap) arrête tout.

Les scènes flash continuent de fonctionner tant que vous les maintenez enfoncées, dans n’importe quel mode.

Pendant qu’elle joue, chaque tuile d’une scène à pas, d’une cuelist ou d’une scène FX avec des FX en « BPM du show » porte en bas une **barre de rythme** coupée en deux : un côté s’allume puis l’autre à chaque pas (dans une cuelist, à chaque cue ; dans une scène FX, à chaque battement, un cycle de ces FX à ×1). Vous voyez ainsi à quel rythme va chacune. Ses boutons du SHOW, ses bandes de playback et ses boutons sur le téléphone la portent aussi.

## Masters

- **Master du groupe** : le curseur « Master » au pied de chaque colonne règle l’intensité de toutes ses scènes.
- **Grand master** : le « GM » de la barre du haut règle la lumière de toute la sortie : les dimmers et, sur les fixtures sans dimmer, leurs couleurs. Il ne touche ni au mélange de couleurs (cyan, magenta, jaune, teinte et saturation) ni aux couleurs d’un fixture ou d’une tête qui a son propre dimmer, qui baissent avec ce dimmer. Dans Options, vous choisissez s’il agit seulement sur l’intensité ou sur tous les canaux. Il ne baisse pas les fixtures **protégés du grand master** (dans les [limites du fixture](patch.md#limites) ou son menu) : la lumière de salle, un serveur média, un hazer. S’il y en a, une **ancre** à côté du « GM » le rappelle (si la barre a la place) et, au survol, indique combien (« Le grand master ne baisse pas 3 fixtures qui en sont protégés »).
- **Dimmer en direct** : chaque scène a en plus son propre dimmer, dans ses propriétés, par-dessus le master du groupe.

Les masters et le grand master n’agissent pas sur les canaux dont l’option « Dimmer » est désactivée dans les [limites du fixture](patch.md#limites). Si c’est le dimmer d’un fixture, il ne compte plus comme son propre dimmer : le grand master baisse alors ses couleurs.

## Propriétés de la scène

Faites un clic droit sur une tuile et choisissez **« Propriétés… »**.

| Champ | Ce qu’il fait |
|---|---|
| **Nom** | Le nom de la scène. |
| **Montée** / **Descente** | Ses temps, en secondes. Si le groupe a un temps de fondu enchaîné réglé, c’est celui-ci qui est utilisé. |
| **Priorité (0-10)** | Quelle scène l’emporte quand deux touchent le même canal (voir [Priorités](directo.md#priorités)). |
| **Au démarrage, libère** | Ce qu’elle arrête en démarrant : [modes de libération](directo.md#modes-de-libération-et-protection). |
| **Protégée des libérations par** | Les libérations qu’elle ignore. |
| **Flash (joue tant qu’on appuie)** | En fait une scène flash. |
| **Dimmer en direct** | Son intensité, par-dessus le master du groupe. |
| **Rythme tapé** | Pas, cuelists et scènes FX avec des FX en « BPM du show » : le [rythme tapé](directo.md#rythme-dune-scène-à-pas) sur sa tuile, en BPM. Saisissez-le ou appuyez sur « Retirer » pour revenir à ses temps (dans une scène FX, au BPM du show). Agit immédiatement. |
| **Vitesse en direct** | De ×0,1 à ×10 (scènes qui bougent : pas, FX, cuelists…). |
| **Phase en direct** / **Taille en direct** | La phase (0–359°) et la taille (0–200 %) des FX. |
| **Vitesse de fondu** | Accélère ou ralentit ses fondus, de ×0,1 à ×10. |

Les curseurs en direct agissent immédiatement, pendant que vous les bougez, et changent la façon dont la scène joue dans ce groupe, pas la scène enregistrée. Les autres champs s’appliquent quand vous appuyez sur **« Appliquer »**.

Le même menu de la tuile contient **« Ouvrir dans Scènes »**, **« Modifier dans le programmeur »** (scènes statiques) et le sous-menu **« Priorité : N »** pour la changer sans ouvrir les propriétés.

## Priorités

Chaque scène a une priorité de 0 à 10 ; les nouvelles prennent celle que vous indiquez dans [Options](opciones.md#lecture) (5 par défaut).

- La scène de **priorité la plus haute commande tous les canaux qu’elle touche**, zéros compris : une scène rouge de priorité 8 sur une bleue de priorité 5 donne du rouge, pas du magenta.
- À **priorité égale**, sur les canaux d’intensité (dimmer, RGB…) elles se mélangent et la valeur la plus haute l’emporte ; sur les autres canaux, le **dernier changement** l’emporte : la dernière scène lancée ou le dernier GO d’une cuelist qui a changé ce canal. Ce qu’une cuelist ne fait que garder des cues précédentes, elle ne le reprend pas.
- Si vous changez la priorité d’une scène qui joue, elle s’applique immédiatement.
- Quand vous libérez une scène de priorité plus haute avec un fondu, elle fait un fondu vers ce qu’il y a en dessous.
- **Le programmeur prime sur tout**, intensités comprises. Dans Options, vous pouvez baisser la priorité des modifications en direct.

## Modes de libération et protection

**Au démarrage, libère** décide quelles scènes s’arrêtent quand celle-ci démarre :

- **« Par défaut du groupe »** : dans un groupe exclusif, les autres scènes de son groupe ; dans un groupe en mélange, aucune.
- **« Aucun »** : n’arrête rien.
- **« Son groupe »** : les autres scènes de son groupe.
- **« Toutes les scènes »** : toutes les autres.
- **« Les autres groupes »** : les scènes des autres groupes.

Quand un [fader du SHOW](show.md#faders) la lance, elle ne libère rien, quel que soit son mode : elle s’ajoute aux autres. Avec son bouton, elle libère comme d’habitude. Et tant que ce fader est monté, rien ne l’arrête, sauf le descendre tout en bas.

**Protégée des libérations par** fait que la scène ignore ces libérations :

- **« Aucun »** : elle se libère comme d’habitude.
- **« Tout (arrêt manuel seulement) »** : vous seul l’arrêtez.
- **« Son propre groupe »** : elle ignore celles de son groupe.
- **« Les autres groupes »** : elle ignore celles qui viennent d’autres groupes.

> **Conseil :** mettez une scène d’ambiance avec la protection « Tout (arrêt manuel seulement) » et lancez par-dessus des scènes avec « Toutes les scènes » : l’ambiance reste.

## Pause, libérer et arrêter

- **« Pause »** fige tout ce qui joue. **« Reprendre »** le relance.
- **« Libérer les groupes »** arrête les scènes de tous les groupes ; le carré de chaque colonne, seulement celles de ce groupe. Pendant 10 s, [« Ramener ce qui jouait »](directo.md#ramener-ce-qui-jouait) relance ce qu’a arrêté « Libérer les groupes » (son bouton, ou un pad ou un bouton du SHOW dans « Tous les groupes ») ; ce qui est libéré dans un seul groupe (son carré, ou un pad ou un bouton du SHOW de ce groupe) ou par une action d’une cue ne peut pas être ramené.
- **« Tout arrêter »** (Échap), dans la barre du haut, arrête tout ce qui joue, y compris hors des groupes. Il ne vide pas le programmeur. Ni « Tout arrêter » ni « Libérer les groupes » n’arrêtent les scènes qu’un [fader du SHOW](show.md#faders) monté retient : seulement le descendre tout en bas. Il désarme aussi les [effets spéciaux](directo.md#effets-spéciaux-protégés), et pendant 10 s vous pouvez [ramener ce qui jouait](directo.md#ramener-ce-qui-jouait). Avec [« Tout arrêter en fondu »](directo.md#tout-arrêter-en-fondu), les fonctions baissent avant de s’arrêter.
- **« Blackout »** (Ctrl+B) met la sortie au noir sans rien arrêter. Les têtes ne bougent pas : le pan, le tilt, le zoom, le focus et les autres canaux qui ne sont pas de la lumière restent où ils sont. Il désarme aussi les effets spéciaux. Pour baisser la lumière peu à peu et finir en Blackout, utilisez un [« Fondu au noir »](directo.md#fondu-au-noir).

## Sécurité pendant le show

Pour qu’une erreur ou une panne de l’ordinateur ne gâche pas le show : le [Mode show](directo.md#mode-show), les [effets spéciaux protégés](directo.md#effets-spéciaux-protégés), les [canaux parqués](directo.md#canaux-parqués), l’[arrêt du show](directo.md#arrêt-du-show), [ramener ce qui jouait](directo.md#ramener-ce-qui-jouait), [tout arrêter en fondu](directo.md#tout-arrêter-en-fondu) et le [fondu au noir](directo.md#fondu-au-noir). Leurs réglages sont dans [Options › Sécurité pendant le show](opciones.md#sécurité-pendant-le-show).

> **Important :** l’ordinateur ne doit jamais être la seule sécurité d’une machine à CO2, à flammes, à étincelles froides ou à confettis. Réglez les machines et les nœuds DMX pour qu’ils s’éteignent s’ils perdent le signal DMX (et non pour qu’ils gardent la dernière valeur), installez un arrêt d’urgence ou un bouton homme mort matériel là où la machine en a besoin, et suivez les règles du lieu et les instructions du fabricant. Si le programme se bloque ou si l’ordinateur s’éteint, aucun logiciel ne peut couper un déclenchement : seul le matériel le peut.

### Mode show

Le **bouclier** de la barre du haut, juste après le cadenas, active ou désactive le **Mode show** d’un clic. Activé, il est orange, et la deuxième ligne du titre commence par « Mode show · ». Verrouiller le SHOW (aussi au démarrage avec « …et verrouillé ») l’active tout seul ; le déverrouiller ne le désactive pas. Avec le SHOW verrouillé, le bouclier s’affiche, mais on ne peut pas cliquer dessus. C’est un réglage de l’ordinateur, pas du show.

Avec le Mode show activé, ces actions demandent de maintenir un bouton enfoncé 1,5 s (« Maintenir pour fermer », « Maintenir pour ouvrir » ou « Maintenir pour continuer »), même sans modifications non enregistrées :

- fermer le programme (la × de la fenêtre, Alt+F4, la barre des tâches) et installer une mise à jour ;
- **Nouveau** et **Ouvrir** (aussi Ctrl+N et Ctrl+O), les shows récents, les exemples et le show d’entraînement du guide ;
- ouvrir un show depuis l’Explorateur de fichiers de Windows.

Relâcher trop tôt, **« Annuler »** ou Échap annulent ; Échap dans cette fenêtre n’arrête jamais le show. Maintenir Espace ou Entrée fonctionne aussi (la répétition de la touche ne compte pas). Ensuite, s’il y a des modifications non enregistrées, « Enregistrer les modifications ? » arrive comme toujours. Quand Windows s’arrête ou que vous fermez la session avec le Mode show activé, la question peut retenir l’arrêt : fermez d’abord le programme.

En Mode show, si le portable passe sur batterie, une barre rouge « Sur batterie : branchez le chargeur » s’affiche sur toutes les pages (aussi verrouillé et en plein écran) jusqu’au retour du chargeur ; à 20 % ou moins, elle ajoute « Batterie à N % ». La batterie est vérifiée toutes les 30 s et à chaque message d’alimentation de Windows. Sans le Mode show, un message le dit une fois (et une autre quand elle devient faible), et [Options](opciones.md#sécurité-pendant-le-show) l’affiche.

Si le mode de gestion de l’alimentation de Windows a la suspension sélective USB activée, au démarrage une barre bleue avertit que l’interface DMX peut s’arrêter, avec **« Comment la désactiver »** (elle ouvre [Suspension sélective USB](opciones.md#suspension-sélective-usb)) et une × qui la masque jusqu’au prochain démarrage. Le programme ne fait que lire ces réglages de Windows : il ne les change jamais.

### Effets spéciaux protégés

Un **effet spécial protégé** est un fixture qui ne peut pas se déclencher par erreur. Les machines à CO2, à flammes, à étincelles froides et à confettis le sont toutes seules, et n’importe quel fixture peut l’être à la main (voir [Patch › Effets spéciaux protégés](patch.md#effets-spéciaux-protégés)). Ils portent un bouclier dans la liste et sur le plan du Patch.

- **Désarmés** (le programme démarre toujours ainsi), tous leurs canaux sortent à 0, quoi qu’il joue.
- **Armés**, ils ne sortent que depuis des **scènes** (flash aussi), des **cuelists**, des **chasers**, des **séquences**, des **super scènes** et les valeurs de base des **scènes FX**. Vous les déclenchez donc avec un bouton du SHOW en flash (il déclenche tant que vous le maintenez), une tuile de Direct, un pad de l’APC, un playback ou le [téléphone](remoto.md#sécurité-sur-le-téléphone).
- **Jamais**, même armés, ne les déclenchent : le programmeur, Identifier, la surbrillance, le mixeur (Full, Flash, couleur, strobo), les FX et leurs macros, les Move FX, les Colour FX et les mappings du plan, les Curve FX, les scripts ni une entrée DMX transmise à la sortie. Le grand master ne les règle pas.
- Les [limites](patch.md#limites) ne comptent que pendant le déclenchement : au repos, c’est toujours 0 qui sort. N’inversez pas les canaux d’une machine à effets : un canal de déclenchement inversé ne se déclenche qu’avec une valeur qui, inversée, l’allume.
- Leurs canaux de pan et de tilt (ceux d’une lyre à flammes) ne sortent qu’armés et depuis ces fonctions, sans durée maximale ; désarmés, à 0. Tous les autres, sauf celui de sécurité, se déclenchent et la durée maximale s’applique à eux (voir [Patch › Effets spéciaux protégés](patch.md#effets-spéciaux-protégés)).

#### Armer les effets

Le bouton **« Armer les effets »** (avec une flamme) est dans la barre du haut, à côté de « Blackout », seulement quand le show a des effets protégés, aussi avec le SHOW verrouillé. Un clic les arme et un autre les désarme. Armés, le bouton devient rouge avec le temps qui reste avant qu’ils se désarment seuls (« ARMÉS · 9:59 »), et son bord pulse pendant qu’un effet se déclenche. Dans une fenêtre plus étroite, il affiche « Armer » ou seulement la flamme : en dessous de 1450 px de large, « Armer les effets », « Blackout » et « Tout arrêter » n’affichent que leur icône, et en dessous d’environ 1200 px le nombre en cours disparaît et le grand master se raccourcit.

Armés, une grande barre rouge **« EFFETS ARMÉS »** s’affiche sur toutes les pages (aussi verrouillé et en plein écran) avec ce qui se déclenche (« Déclenchement : CO2 1 ») ou ce qui attend d’être relâché, et un bouton **« Désarmer »**.

La flèche **▾** à côté indique s’ils sont armés et dans combien de temps ils se désarment seuls, **« Se déclenchent maintenant »**, **« Attendent d’être relâchés »** (relâchez sa scène, son bouton ou son fader, à 0, et il pourra se déclencher), **« Derniers déclenchements »** (nom · heure · secondes · comment il a fini : relâché, coupé à son temps maximal, désarmé…), **« Ouvrir le journal »** et **« Réglages de chaque effet : Patch »** (ces deux-là, pas avec le SHOW verrouillé).

Ils ne s’arment pas si aucun fixture n’est protégé, avec le Blackout activé (« Le Blackout est activé : désactivez-le pour armer les effets spéciaux ») ni avec l’arrêt du show en place. On les arme aussi avec un [bouton du SHOW](show.md#actions-dun-bouton), un [mappage](mapeos.md#pilote) ou le [téléphone](remoto.md#sécurité-sur-le-téléphone).

#### Durée maximale et attente minimale

- Un déclenchement dure tant que sa scène, son bouton ou son fader le donnent, au plus sa **durée maximale** (par défaut : CO2 3 s, flammes 2 s, étincelles 5 s, confettis 2 s). Ensuite il est coupé, même si vous continuez d’appuyer, et il ne revient pas tant que vous ne relâchez pas.
- Entre un déclenchement et le suivant, il y a une **attente minimale** (1 s par défaut) depuis le relâchement. Un appui pendant l’attente ne déclenche pas plus tard : relâchez et appuyez de nouveau.
- Armer pendant qu’une scène ou un fader donnent déjà une valeur à l’effet ne le déclenche pas : il attend que vous relâchiez. Désarmer pendant qu’un effet se déclenche le coupe, même si vous réarmez aussitôt (il ne revient pas tant que vous ne relâchez pas) ; une commande qui ne fait qu’armer ne fait rien s’ils sont déjà armés.
- Le **canal de sécurité**, si la machine en a un, ne sort dans ses valeurs que pendant que l’effet est armé et se déclenche ; sinon, 0. À lui seul, il ne déclenche jamais la machine.
- Si le programme cesse d’envoyer des trames (il est très occupé), chaque univers coupe quand même un déclenchement à son temps, avec au plus 40 ms de retard.

#### Quand ils se désarment

Ils se désarment seuls :

- avec **Tout arrêter** (Échap, son bouton, un pad, un bouton du SHOW ou le téléphone) et avec le **Blackout** (Ctrl+B, son bouton, un bouton du SHOW, un pad ou le téléphone ; pas le « Blackout » d’une bande du mixeur) ;
- avec l’**arrêt du show**, un **fondu au noir** (à son début) et **ramener ce qui jouait** ;
- quand vous ouvrez ou commencez **un autre show**, et quand vous **fermez** le programme ;
- après quelques **minutes sans déclenchement** (10 par défaut : « Désarmer les effets inutilisés après », dans [Options](opciones.md#sécurité-pendant-le-show)), jamais pendant qu’un effet se déclenche ;
- quand le dernier effet protégé est supprimé ou déprotégé, et **à la main**.

Un message le dit (« Effets spéciaux désarmés : Tout arrêter »). Ils ne sont jamais enregistrés armés. Un fader du SHOW qui retient une scène avec un CO2 continue de la retenir après Échap, mais le CO2 reste à 0 jusqu’à ce que vous descendiez le fader tout en bas.

#### Le journal des déclenchements

Chaque session écrit un fichier dans `%LOCALAPPDATA%\DMXTotalControl\DMX Total Control\effects-log` (« Ouvrir le journal » ouvre le dossier) : chaque déclenchement avec son heure, le numéro et le nom du fixture, sa durée et comment il a fini (relâché, coupé à son temps maximal, désarmé, arrêt du show, parqué, retiré), et quand les effets ont été armés et désarmés, et pourquoi.

> **Non testé avec du matériel réel :** rien de tout cela n’a été testé avec des machines à CO2, à flammes, à étincelles ou à confettis, avec leurs canaux de sécurité, avec des nœuds Art-Net ou sACN ni avec ce qu’elles font quand elles perdent le signal DMX. Essayez-le avec votre matériel, sans public et avec la machine non chargée, avant de l’utiliser pendant un show.

### Canaux parqués

Un canal **parqué** sort toujours à une valeur fixe, au-dessus des scènes, du programmeur, du mixeur, du grand master et de Tout arrêter : le hazer à 30 % toute la nuit, une lyre en panne éteinte et immobile. Échap et Blackout ne le déparquent jamais ; avec le Blackout, ses intensités s’éteignent, sauf avec « Aussi en blackout ». On parque dans le [Patch](patch.md#parquer-des-canaux) et dans [Programmer](programar.md#parquer). À l’ouverture d’un show avec des canaux parqués, ils s’appliquent tout de suite, et un message dit lesquels (« 2 fixtures parqués : HAZER, Spot 3 (Patch) »).

### Arrêt du show

L’**arrêt du show** est une lumière de sécurité pour arrêter le show et éclairer le public : par-dessus tout, il met du blanc ouvert sur les lumières, éteint les stroboscopes et les lasers, immobilise les lyres et désarme les effets spéciaux.

Pour **le mettre ou le lever**, appuyez **deux fois sur la touche Pause** (en moins de 2 s ; pas dans un show qui la mappe, voir [Mappages](mapeos.md#clavier)), ou **maintenez 1 s** un bouton du SHOW « Arrêt du show (maintenir 1 s) », un pad ou une touche [mappés](mapeos.md#pilote), ou la main de la barre du [téléphone](remoto.md#sécurité-sur-le-téléphone). Un appui plus court ne fait rien, et un seul appui sur la touche Pause ne fait qu’avertir (« Arrêt du show : appuyez de nouveau sur Pause pour le mettre »). Pendant que vous maintenez, une barre orange « Arrêt du show : continuez à maintenir… » se remplit. Une fois en place, une grande barre rouge **« ARRÊT DU SHOW »** s’affiche sur toutes les pages (aussi verrouillé et en plein écran) avec **« Maintenir pour lever »** (maintenez-le une seconde). Si le programme perd le clavier pendant que vous maintenez une touche mappée (une autre fenêtre, un message de Windows), le maintien est relâché et rien ne change.

Tant qu’il est en place :

- les effets spéciaux sont désarmés (« …désarmés : arrêt du show ») et ne peuvent pas être armés (« L’arrêt du show est en place : levez-le pour armer les effets spéciaux ») ;
- les lumières (lyres, scanners, PAR et washes, barres LED, blinders et dimmers) des groupes choisis passent en **blanc ouvert** au niveau choisi : le dimmer au niveau, le rouge, le vert, le bleu et le blanc à fond (sans dimmer, les couleurs au niveau), CMY, ambre et UV à 0, le shutter ouvert, la roue de couleurs sur blanc ou ouverte et les programmes intégrés sur « sans fonction » ;
- les stroboscopes s’éteignent (leurs intensités à 0, ou tous leurs canaux s’ils n’en ont pas) et les lasers passent à 0 sur tous leurs canaux (un laser DMX s’éteint à 0 : vérifiez le vôtre) ;
- tous les shutters s’ouvrent (sans strobo ; un canal de strobo « lent à rapide » passe à 0) et le strobo du mixeur s’arrête ;
- les lyres restent où elles sont (pan et tilt, les fins aussi) ;
- la fumée, les hazers, les ventilateurs, les bulles, la neige, les autres effets, les gobos, les prismes et le zoom restent tels quels, et les effets protégés à 0.

Il l’emporte sur le programmeur, le grand master, le Blackout (le reste reste au noir) et les canaux parqués : un fixture parqué éteint s’allume aussi, et un hazer parqué reste parqué. Les [limites](patch.md#limites) comptent toujours (un dimmer inversé, un maximum). Les fixtures qu’il éclaire et le niveau se choisissent dans [Options › Arrêt du show](opciones.md#arrêt-du-show).

- **Tout arrêter** (Échap) pendant l’arrêt du show arrête les fonctions en dessous, et l’arrêt du show reste.
- Quand vous le **levez**, le show est comme ses fonctions le jouent maintenant (elles ont continué en dessous) et le strobo du mixeur revient. Les effets restent désarmés : armez-les à la main ; un CO2 retenu doit d’abord être relâché.
- **Nouveau**, **Ouvrir** et la fermeture du programme le lèvent. Il n’est jamais enregistré en place : après une fermeture inattendue ou un redémarrage, remettez-le.
- Le journal des effets de la session note « Arrêt du show en place » et « Arrêt du show levé ».

> **Non testé avec du matériel réel :** le blanc ouvert et les valeurs de shutter des vrais fixtures, qu’un laser soit vraiment éteint à 0, la touche Pause des vrais claviers, les LED de l’APC et le téléphone sur un vrai Wi-Fi.

### Ramener ce qui jouait

Après **Tout arrêter** (Échap, son bouton, un pad, un bouton du SHOW ou le téléphone) ou **« Libérer les groupes »** (tous les groupes, pas un seul), une barre bleue sous la barre du haut dit ce qui a été arrêté (« Arrêté : 4 scènes et 1 cuelist · Ramener ce qui jouait (Ctrl+Z) » ; « Libéré : … » après « Libérer les groupes »), avec le bouton **« Ramener »**, une bande qui se vide en 10 secondes et une × qui la masque jusqu’au prochain Tout arrêter. Elle s’affiche aussi avec le SHOW verrouillé. **Ctrl+Z** fait la même chose dans Direct et dans le SHOW (pas pendant que vous le modifiez ni pendant que vous tapez), et la barre n’indique « (Ctrl+Z) » que là où il fonctionne. « Ramener » ignore un appui dans sa première demi-seconde (un deuxième appui de panique sur Tout arrêter).

Reviennent, avec un fondu de 0,5 s :

- la scène que jouait chaque groupe ;
- les scènes lancées depuis des boutons du SHOW, depuis Scènes ou depuis des playbacks ;
- les cuelists, sur la cue où elles étaient, au niveau de leur playback ; une cuelist en pause revient en pause.

Les FX et les pas recommencent leur cycle, les fonctions en beats utilisent leur propre fondu et les cuelists « Timecode » suivent leur timecode. Ne reviennent pas : les flashs maintenus (dans Direct, le SHOW, les pads ou les playbacks), les scènes qu’un fader du SHOW retient (elles ne se sont jamais arrêtées), le programmeur, le mixeur, le grand master, l’armement des effets, les canaux parqués ni l’arrêt du show.

Cela fonctionne une fois, et un message dit ce qui est revenu (« Ramené : 4 scènes et 1 cuelist »). Un autre Tout arrêter dans les 10 s s’ajoute au premier (pas le second Échap pendant le fondu de « Tout arrêter en fondu ») ; « Libérer les groupes » ne garde que les groupes, et un Tout arrêter ensuite s’y ajoute. Les fonctions supprimées entre-temps sont sautées, et un show nouveau ou ouvert l’efface.

Ramener **n’arme jamais** les effets spéciaux : s’ils étaient armés (après « Libérer les groupes », qui ne désarme pas, ou quand vous les avez armés de nouveau après Tout arrêter), il les désarme d’abord (« Effets spéciaux désarmés : Ramener ce qui jouait »). Ce qui revient avec un CO2 attend que vous relâchiez et reste à 0, même après avoir armé de nouveau.

### Tout arrêter en fondu

Avec **« Tout arrêter en fondu »** dans [Options](opciones.md#sécurité-pendant-le-show) (de 0 à 5 s ; 0, tout de suite, comme toujours), Tout arrêter baisse les fonctions pendant cette durée. Les scènes que retiennent les faders du SHOW et les canaux parqués restent ; les effets protégés s’éteignent tout de suite (ils sont désarmés). Pendant le fondu, le bouton « Tout arrêter » porte une bande et dit « Fondu en cours : Échap de nouveau arrête tout de suite » : **Échap de nouveau** arrête tout tout de suite, même ce que vous avez lancé pendant le fondu. Ensuite, vous pouvez ramener ce que le premier Échap a arrêté ; ce que vous avez lancé pendant le fondu ne revient pas.

### Fondu au noir

Un **« Fondu au noir »** (un [bouton du SHOW](show.md#actions-dun-bouton) ou un [mappage](mapeos.md#pilote), avec sa durée : 3 s par défaut) baisse les intensités en sortie pendant cette durée, puis active le Blackout, par exemple à la fin d’un set. En dessous, la lumière est de nouveau à fond : quand vous enlevez le Blackout, elle revient.

- Il agit au-dessus du grand master (aussi sur les fixtures qui en sont protégés) et sur les canaux parqués sans « Aussi en blackout ».
- Il ne touche pas les canaux parqués avec « Aussi en blackout », le pan, le tilt ni les autres canaux qui ne sont pas de la lumière, ni les effets protégés.
- À son début, il désarme les effets spéciaux.
- Un autre appui, ou le Blackout, le terminent tout de suite. Avec le Blackout déjà activé, il ne fait rien.
- Pendant le fondu, le bouton « Blackout » porte une bande et dit « Fondu au noir : le Blackout arrive à la fin (Blackout maintenant : tout de suite) ».

> **Non testé avec du matériel réel :** le Mode show (la batterie, l’ordinateur éveillé, la suspension USB), Ramener, Tout arrêter en fondu et le Fondu au noir n’ont pas été testés avec de vrais portables, interfaces DMX, LED de l’APC ou machines à effets.

## Mixeur

Le mixeur agit sur ce qui sort, par-dessus chaque scène et les modifications en direct. Appuyez sur **« Mixeur »** pour l’afficher : il y a une bande pour **« Tous les fixtures »** et une pour chaque [groupe de fixtures](patch.md#groupes-de-fixtures).

Chaque bande contient :

- Un **fader de dimmer** avec son pourcentage.
- **« Full »** : le groupe à fond. **« Blackout »** : le groupe au noir, sauf les effets spéciaux protégés (aucun bouton du mixeur ne les éteint ni ne les désarme).
- **« Teinte »** et **« Saturation »** : teintent la couleur. **« Réinitialiser la couleur »** retire la teinte.
- **« Stroboscope »** et sa vitesse, de 0,5 à 20 Hz.
- **« Flash »** : à fond tant qu’on appuie. **« Flash excl. »** : ce groupe à fond et le reste au noir tant qu’on appuie.
- **« Full excl. »** : ce groupe à fond et le reste au noir.

Ctrl+clic sur un bouton de flash le garde allumé jusqu’au clic suivant. **« Réinitialiser le mixeur »** remet tout à son état normal. Le bord d’une bande devient orange avec un flash ou un full, et rouge avec un blackout.

> **Conseil :** le mixeur peut aussi être une section du [SHOW](show.md), et ses faders et ses boutons peuvent être [mappés](mapeos.md) sur un contrôleur MIDI.

## Contrôles dans le SHOW et sur les contrôleurs

Les tuiles de scène, les masters des groupes, les boutons ◀ ▶ et de libération de chaque groupe, ceux de l’en-tête (◀ ▶, « Pause » et « Libérer les groupes ») et les contrôles du mixeur :

- Avec **Alt+clic**, ils sont copiés sur la page du SHOW ouverte (voir [Show](show.md#envoyer-nimporte-quel-contrôle-au-show)).
- En mode Mapper, ils peuvent être assignés à un contrôleur (voir [Mappages](mapeos.md#mode-mapper)).

Tant qu’un mappage utilise « La scène sélectionnée », ou en mode Mapper, la tuile de la scène sélectionnée (la dernière sur laquelle vous avez appuyé) porte une fine barre sur son bord inférieur (voir [Mappages](mapeos.md#la-scène-sélectionnée)).
