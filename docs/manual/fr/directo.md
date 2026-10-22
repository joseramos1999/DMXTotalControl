# Direct

![La page Direct](../img/directo.png)

**Direct** (F1) est la page pour lancer des scènes pendant le show. Les scènes sont rangées en **groupes**, qui sont des colonnes fonctionnant en parallèle : chaque groupe a ses scènes, son mode (exclusif ou mélange), son temps de fondu enchaîné et son master.

Dans l’en-tête de la page, vous avez :

- **◀ ▶** : scène précédente ou suivante dans chaque groupe qui joue (aussi avec Ctrl+← et Ctrl+→).
- **« Pause »** / **« Reprendre »** : fige tout ce qui joue et le relance.
- **« Libérer les groupes »** : arrête les scènes de tous les groupes.
- **« Mixeur »** : passe à la vue du [mixeur](directo.md#mixeur). Un point orange indique que le mixeur modifie la sortie.
- **« Nouveau groupe »** : ajoute une colonne.

## Groupes

### Créer et organiser les groupes

1. Appuyez sur **« Nouveau groupe »** (ou sur le cadre « Ajouter un groupe » au bout des colonnes).
2. Cliquez sur le nom de la colonne pour le changer (par exemple `Wash`, `Spots`, `Strobos`, `Effets`) et appuyez sur Entrée.

Le bouton en forme d’engrenage de chaque colonne ouvre son menu : **« Changer de couleur »** (passe à la couleur suivante), **« Déplacer à gauche »**, **« Déplacer à droite »** et **« Supprimer le groupe »**. Vous pouvez aussi faire glisser le groupe par la poignée à gauche de son nom et le déposer sur un autre groupe pour prendre sa place.

La flèche **‹** à côté du nom **replie** le groupe : il devient une bande étroite avec son nom et le nombre de scènes qui jouent, pour laisser de la place aux autres. Ses scènes continuent de jouer. La flèche **›** de la bande le déplie. C’est enregistré avec le show.

### Ajouter et ordonner des scènes

1. Appuyez sur **« Ajouter des scènes »** au bas de la colonne.
2. Dans la liste, cliquez sur les scènes que vous voulez (elles sont cochées d’un ✓). Vous pouvez chercher par nom ou par dossier, et les afficher par dossiers ou par groupes.
3. Appuyez sur **« Ajouter N »**. Un double-clic sur une scène l’ajoute directement.

Pour réordonner, survolez une tuile et faites-la glisser par sa poignée (à droite de la tuile) : déposez-la sur une autre scène pour la placer devant, ou dans l’espace vide de la colonne pour la placer à la fin. Vous pouvez la déposer dans **un autre groupe** : elle s’y déplace avec ses options. Dans le menu de la tuile (clic droit), vous avez aussi **« Monter »**, **« Descendre »** et **« Retirer du groupe »**.

> **Conseil :** quand vous enregistrez depuis Programmer, vous pouvez mettre la scène directement dans un groupe avec « Ajouter au groupe ». Voir [Enregistrer](programar.md#enregistrer).

## Lancer des scènes

Cliquez sur une tuile pour lancer la scène ; cliquez à nouveau pour la libérer. La tuile qui joue prend la couleur du groupe et affiche un point qui bat.

Sur chaque tuile, vous voyez :

- Le **nom** et, en dessous, le **type** de scène. Pour une cuelist en cours, la cue où elle se trouve (« Q 3 · Couplet »). Derrière, le [rythme tapé](directo.md#rythme-dune-scène-à-pas) s’il y en a un (« 120 BPM »).
- **P7**, **P2**… : la priorité, seulement quand ce n’est pas la normale (5). En orange si elle est supérieure à 5.
- Un **éclair** : c’est une scène flash.
- Une **barre coupée en deux** en bas : une scène à pas ou une cuelist en cours ; elle alterne à chaque pas, à son rythme.

### Exclusif ou mélange

Le bouton de chaque colonne alterne entre :

- **« Exclusif »** : une seule scène joue à la fois. Quand vous en lancez une autre, il fait un fondu enchaîné vers la nouvelle.
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

Une cuelist dans un groupe est une tuile comme les autres : le clic l’allume et l’éteint. Son menu (clic droit) ajoute **« GO (cue suivante) »**, **« Retour (cue précédente) »** et **« Libérer la cuelist »**. Pour l’envoyer confortablement avec GO, mettez-la sur un [playback](playbacks.md).

### Rythme d’une scène à pas

Les scènes à pas (séquences et chenillards) et les cuelists suivent le rythme auquel vous appuyez sur leur tuile :

- Un clic la lance ou la libère, comme d’habitude.
- Pendant que la scène joue, appuyez sur sa tuile **plusieurs fois de suite** au rythme de la musique, avec moins de 2 secondes entre les appuis. Chaque appui passe au pas suivant (dans une cuelist, il donne un GO) et la scène continue seule à ce rythme : un pas à chaque battement, au lieu de ses propres temps. Le rythme est la moyenne des derniers appuis.
- Seuls comptent les appuis pendant que la scène joue : celui qui la lance commence le compte. Ainsi, changer de scène dans un groupe exclusif puis revenir ne donne de rythme à aucune.
- Après une pause de plus de 2 secondes, un clic la libère. Pour la libérer juste après avoir tapé le rythme, attendez 2 secondes.
- Le rythme tapé est joué même avec une **vitesse en direct** réglée ; si vous changez ensuite la vitesse, le rythme accélère ou ralentit avec elle. En pause, les appuis ne font pas avancer les pas.
- Le rythme est enregistré avec la scène et lui appartient : elle le suit aussi quand elle joue dans une super scène ou sur un playback. Dans ses [propriétés](directo.md#propriétés-de-la-scène) (et dans [Scènes](escenas.md#comment-joue-une-scène)), vous pouvez le saisir ou appuyer sur **« Retirer »** pour revenir à ses temps.

Cela fonctionne de la même façon avec ses boutons du [SHOW](show.md), avec ses mappages de contrôleurs et de touches en mode « Bascule » et avec la [télécommande](remoto.md).

#### GO et Stop

Les boutons **GO** et **Stop** de la barre du haut (et de la section « Rythme » du SHOW) changent l’effet d’un appui sur n’importe quelle scène, pour tous ceux qui appuient :

- **GO** allumé : appuyer sur une scène la **lance** ; sur une scène qui joue déjà, chaque appui avance d’un pas (dans une cuelist, un GO) et plusieurs appuis de suite marquent son rythme. Elle ne s’arrête jamais, même si vous restez plus de 2 secondes sans appuyer : vous marquez ainsi le tempo tranquillement.
- **Stop** allumé : appuyer sur une scène l’**arrête**. Une scène qui ne joue pas reste telle quelle.
- Les deux éteints (appuyez à nouveau sur celui qui est allumé) : comme d’habitude, un clic la lance ou la libère.

Les scènes flash continuent de fonctionner tant que vous les maintenez enfoncées, dans n’importe quel mode.

Pendant qu’elle joue, chaque tuile d’une scène à pas ou d’une cuelist porte en bas une **barre de rythme** coupée en deux : un côté s’allume puis l’autre à chaque pas (dans une cuelist, à chaque cue). Vous voyez ainsi à quel rythme va chacune. Ses boutons du SHOW, ses bandes de playback et ses boutons sur le téléphone la portent aussi.

## Masters

- **Master du groupe** : le curseur « Master » au pied de chaque colonne règle l’intensité de toutes ses scènes.
- **Grand master** : le « GM » de la barre du haut règle la lumière de toute la sortie : les dimmers et, sur les fixtures sans dimmer, leurs couleurs. Il ne touche ni au mélange de couleurs (cyan, magenta, jaune, teinte et saturation) ni aux couleurs d’un fixture ou d’une tête qui a son propre dimmer, qui baissent avec ce dimmer. Dans Options, vous choisissez s’il agit seulement sur l’intensité ou sur tous les canaux.
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
| **Rythme tapé** | Pas et cuelists : le [rythme tapé](directo.md#rythme-dune-scène-à-pas) sur sa tuile, en BPM. Saisissez-le ou appuyez sur « Retirer » pour revenir à ses temps. Agit immédiatement. |
| **Vitesse en direct** | De ×0,1 à ×10 (scènes qui bougent : pas, FX, cuelists…). |
| **Phase en direct** / **Taille en direct** | La phase (0–359°) et la taille (0–200 %) des FX. |
| **Vitesse de fondu** | Accélère ou ralentit ses fondus, de ×0,1 à ×10. |

Les curseurs en direct agissent immédiatement, pendant que vous les bougez, et changent la façon dont la scène joue dans ce groupe, pas la scène enregistrée. Les autres champs s’appliquent quand vous appuyez sur **« Appliquer »**.

Le même menu de la tuile contient **« Ouvrir dans Scènes »**, **« Modifier dans le programmeur »** (scènes statiques) et le sous-menu **« Priorité : N »** pour la changer sans ouvrir les propriétés.

## Priorités

Chaque scène a une priorité de 0 à 10 ; les nouvelles prennent celle que vous indiquez dans [Options](opciones.md#lecture) (5 par défaut).

- La scène de **priorité la plus haute commande tous les canaux qu’elle touche**, zéros compris : une scène rouge de priorité 8 sur une bleue de priorité 5 donne du rouge, pas du magenta.
- À **priorité égale**, sur les canaux d’intensité (dimmer, RGB…) elles se mélangent et la valeur la plus haute l’emporte ; sur les autres canaux, la dernière scène lancée l’emporte.
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

**Protégée des libérations par** fait que la scène ignore ces libérations :

- **« Aucun »** : elle se libère comme d’habitude.
- **« Tout (arrêt manuel seulement) »** : vous seul l’arrêtez.
- **« Son propre groupe »** : elle ignore celles de son groupe.
- **« Les autres groupes »** : elle ignore celles qui viennent d’autres groupes.

> **Conseil :** mettez une scène d’ambiance avec la protection « Tout (arrêt manuel seulement) » et lancez par-dessus des scènes avec « Toutes les scènes » : l’ambiance reste.

## Pause, libérer et arrêter

- **« Pause »** fige tout ce qui joue. **« Reprendre »** le relance.
- **« Libérer les groupes »** arrête les scènes de tous les groupes ; le carré de chaque colonne, seulement celles de ce groupe.
- **« Tout arrêter »** (Échap), dans la barre du haut, arrête tout ce qui joue, y compris hors des groupes. Il ne vide pas le programmeur.
- **« Blackout »** (Ctrl+B) met la sortie au noir sans rien arrêter. Les têtes ne bougent pas : le pan, le tilt, le zoom, le focus et les autres canaux qui ne sont pas de la lumière restent où ils sont.

## Mixeur

Le mixeur agit sur ce qui sort, par-dessus chaque scène et les modifications en direct. Appuyez sur **« Mixeur »** pour l’afficher : il y a une bande pour **« Tous les fixtures »** et une pour chaque [groupe de fixtures](patch.md#groupes-de-fixtures).

Chaque bande contient :

- Un **fader de dimmer** avec son pourcentage.
- **« Full »** : le groupe à fond. **« Blackout »** : le groupe au noir.
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
