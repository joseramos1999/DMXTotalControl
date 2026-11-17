# Raccourcis clavier

Toutes les touches et combinaisons du programme, par page. Les touches que vous assignez vous-même dans [Mappages](mapeos.md#clavier) et les touches des boutons du [SHOW](show.md#options-dun-bouton) s’ajoutent à celles-ci. Une touche assignée dans Mappages a la priorité sur le raccourci du programme qui utilise la même touche.

## Sur n’importe quelle page

| Touche | Ce qu’elle fait |
|---|---|
| **F1** | Direct |
| **F2** | Show |
| **F3** | Programmer |
| **F4** | Scènes |
| **F5** | Patch |
| **F6** | Fixtures |
| **F7** | Sorties |
| **F8** | Mappages |
| **F9** | Options |
| **F10** | Playbacks |
| **F11** | Plein écran (et retour à la fenêtre) |
| **F12** | Ouvre ce manuel sur la page où vous êtes (aussi le bouton **« ? »** de la barre du haut) |
| **Ctrl+N** | Nouveau show |
| **Ctrl+O** | Ouvrir un show |
| **Ctrl+S** | Enregistrer le show (la première fois, demande un nom) |
| **Ctrl+Maj+S** | Enregistrer le show sous un autre nom (« Enregistrer sous… ») |
| **Ctrl+B** | Blackout (allumer et éteindre) |
| **Pause** deux fois (en 2 s) | Met ou lève l’[arrêt du show](directo.md#arrêt-du-show), sur n’importe quelle page, avec n’importe quel modificateur et même pendant que vous tapez. Un seul appui ne fait qu’avertir (« Arrêt du show : appuyez de nouveau sur Pause pour le mettre ») ; la maintenir ne fait rien de plus. Elle ne peut pas être assignée dans Mappages ; un show qui la mappait déjà la garde |
| **Échap** | Tout arrêter. En mode Mapper, annule le contrôle en attente ou quitte le mode ; avec « Enregistrer » armé dans Playbacks, le désarme ; avec un groupe isolé sur le plan de Patch, termine l’isolement (avec un menu, une liste déroulante ou une fenêtre de Patch ouverts, comme « Ajouter des fixtures », « Modifier » ou « Limites », il les ferme seulement ; un avertissement comme « Enregistrer les modifications ? » ne se ferme pas avec Échap). Dans une fenêtre où vous tapez un nom ou un numéro (« Créer un groupe », « Renommer », « Renuméroter »…), la ferme sans rien appliquer : appuyez de nouveau sur Échap pour tout arrêter |
| **Échap** deux fois | Pendant le fondu de « Tout arrêter en fondu », arrête tout tout de suite |
| **Ctrl+Z** | Dans Direct et dans le SHOW (pas en mode édition ni pendant que vous tapez), pendant 10 s après « Tout arrêter » ou « Libérer les groupes » : [ramener ce qui jouait](directo.md#ramener-ce-qui-jouait) |
| **Ctrl+→** | Scène suivante dans chaque groupe qui joue |
| **Ctrl+←** | Scène précédente dans chaque groupe qui joue |
| **G** | BeatGO (sauf sur la page Show) |

Si une page est verrouillée, sa touche demande le mot de passe (s’il y en a un) et déverrouille. Avec le SHOW verrouillé (mode borne), les touches de page ne fonctionnent pas, et avec la Configuration verrouillée, Ctrl+N, Ctrl+O et Ctrl+Maj+S non plus.

T, G et les touches que vous assignez dans Mappages n’agissent pas pendant que vous tapez dans un champ de texte, comme la ligne de commande de Programmer. La touche Pause (l’arrêt du show), elle, fonctionne pendant que vous tapez.

## Mode show

Avec le [Mode show](directo.md#mode-show) activé, quand vous fermez le programme ou ouvrez ou commencez un autre show :

| Touche | Ce qu’elle fait |
|---|---|
| Maintenir **Espace** ou **Entrée** 1,5 s | Comme maintenir le bouton : confirme (la répétition de la touche ne compte pas) |
| **Échap** | Annule ; dans cette fenêtre, n’arrête jamais le show |

## Playbacks

| Touche | Ce qu’elle fait |
|---|---|
| **Espace** | GO du playback sélectionné (un GO par appui) |
| **Maj+Espace** | Retour sur le playback sélectionné |
| **PgUp** | Page de playbacks précédente |
| **PgDn** | Page de playbacks suivante |
| **Échap** | Désarme « Enregistrer » |

Elles fonctionnent sur la page Playbacks tant que vous n’êtes pas en train de taper dans un champ de texte.

## Programmer

| Touche | Ce qu’elle fait |
|---|---|
| Taper | Ce que vous tapez sur la page va dans la ligne de commande |
| **Entrée** | Exécute la commande |
| **↑** / **↓** | Les dernières commandes |
| **Échap** | Vide la ligne de commande (avec la ligne vide, Tout arrêter) |

Voir [la ligne de commande](programar.md#ligne-de-commande).

## Show

| Touche | Ce qu’elle fait |
|---|---|
| La touche d’un bouton | Actionne ce bouton (tant que vous la maintenez, s’il est flash) |
| **Suppr** / **Retour arrière** | En mode édition, retire le contrôle de la cellule sélectionnée |
| **Échap** | En mode édition, annule la sélection |

## Super scènes

| Touche ou geste | Ce qu’il fait |
|---|---|
| **Espace** | Lecture ou arrêt depuis le curseur |
| **Suppr** / **Retour arrière** | Supprime les blocs sélectionnés |
| **Ctrl+A** | Sélectionne tous les blocs des pistes visibles |
| **Ctrl+D** | Duplique les blocs sélectionnés juste après eux |
| **Ctrl+C** | Copie les blocs sélectionnés |
| **Ctrl+X** | Coupe les blocs sélectionnés (ceux des pistes verrouillées sont seulement copiés) |
| **Ctrl+V** | Colle les blocs copiés au curseur, à partir de la piste sélectionnée |
| **Ctrl+Z** | Annule la dernière modification (pas pendant la saisie dans un champ) |
| **Ctrl+Y** / **Ctrl+Maj+Z** | Rétablit ce qui a été annulé |
| **Ctrl+clic** sur un bloc | L’ajoute à la sélection ou l’en retire |
| **Maj+clic** sur un bloc | Sélectionne du dernier bloc choisi jusqu’à celui-ci |
| Faire glisser sur une zone vide des pistes | Dessine un rectangle qui sélectionne les blocs qu’il touche (avec **Ctrl** ou **Maj**, ils s’ajoutent) |
| Clic droit sur une zone vide d’une piste | « Coller ici » et « Tout sélectionner » |
| **Ctrl+molette** | Zoom sur la timeline |
| **Maj+molette** | Se déplacer dans le temps |
| Double-clic sur la règle | Lecture à partir de là |

Les touches des blocs n’agissent pas pendant la saisie dans un champ. Voir [Sélectionner plusieurs blocs](escenas.md#sélectionner-plusieurs-blocs) et [Copier et coller](escenas.md#copier-et-coller).

## Faders et curseurs

Après avoir cliqué sur un fader ou un curseur :

| Touche ou geste | Ce qu’il fait |
|---|---|
| **↑** / **→** | Monte d’un pas |
| **↓** / **←** | Descend d’un pas |
| **PgUp** / **PgDn** | Monte ou descend de dix pas |
| Double-clic | Le ramène à sa valeur de repos (normalement, zéro) |

## Cuelists et pas (Scènes)

| Touche ou geste | Ce qu’il fait |
|---|---|
| **Ctrl+clic** / **Maj+clic** | Sélectionne plusieurs cues ou pas pour changer leurs temps en même temps |
| Double-clic sur une cue ou un pas | Le charge dans le programmeur |
| Double-clic sur le numéro d’une cue | Change son numéro |
| Molette sur un temps cliqué | ±0,1 s (avec **Maj**, ±1 s) |
| **Ctrl+Z** | Annule la dernière modification de la cuelist ouverte (dans un champ avec du texte saisi, d’abord ce texte) |
| **Ctrl+Y** / **Ctrl+Maj+Z** | Rétablit ce qui a été annulé |
| **Échap** dans un champ | Laisse la valeur telle qu’elle était |

## Souris et modificateurs

| Geste | Où | Ce qu’il fait |
|---|---|---|
| **Alt+clic** | Presque n’importe quel contrôle | L’envoie sur la page du SHOW ouverte |
| **Maj+clic** | Mode Mapper | Utilise le contrôle au lieu de le mapper |
| **Ctrl+clic** | Boutons Flash du mixeur | Gardent le flash allumé jusqu’au clic suivant |
| **Ctrl+clic** / **Maj+clic** | Fixtures, groupes et plan 2D (Programmer et Patch), liste des scènes, blocs d’une super scène | Ajoute à la sélection ou en retire |
| Molette | Plan 2D | Zoom |
| Glisser avec le bouton droit ou celui du milieu | Plan 2D | Déplace la vue (double-clic : la recentre) |
| Clic droit | Bouton 15° du plan 2D | Tourne dans l’autre sens |
| Clic droit | Bouton du SHOW (en direct) | Arrête ce qu’il lance (sa scène, son groupe, son playback ou sa palette), même avec GO ; celui d’« Armer les effets » les désarme |
| Maintenir « Déverrouiller » enfoncé | Show verrouillé | Déverrouille |
| Maintenir 1 s | Bouton « Arrêt du show » du SHOW, un pad ou une touche mappés, la main du téléphone, « Maintenir pour lever » | Met ou lève l’arrêt du show |
