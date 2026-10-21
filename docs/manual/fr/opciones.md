# Options

![La page Options](../img/opciones.png)

**Options** (F9) réunit les réglages du programme. À gauche se trouvent les réglages généraux, le démarrage, le verrouillage, la sauvegarde automatique, la lecture et l’audio ; à droite, la télécommande, la bibliothèque de fixtures, les mises à jour, l’aide et « À propos ».

Les réglages de cette page appartiennent à l’ordinateur, pas au show : ils valent pour tous les shows que vous ouvrez.

## Général

- **Langue** : « Langue du système », Español, English, Português, Français, Italiano ou Deutsch. S’applique au prochain démarrage du programme.
- **Thème** : **Sombre**, **Clair**, **Contraste élevé** ou **Nuit (atténué)**. Il change immédiatement. Clair, pour les salles éclairées ; Contraste élevé, pour lire de loin ; Nuit, très sombre et en rouge, pour une régie dans le noir sans vous éblouir. La [télécommande](remoto.md) suit le même thème.
- **Taille de l’interface** : de 80 % à 150 %, plus petite pour les petits portables et plus grande pour les écrans tactiles. S’applique au prochain démarrage du programme.
- **Ouvrir le dernier show au démarrage**.

> **Conseil :** le thème Clair convient mieux aux répétitions de jour ou en plein air ; Contraste élevé, quand il y a beaucoup de lumière autour de l’écran.

## SHOW et verrouillage

- **Ouvrir le SHOW en plein écran au démarrage** : pour les installations et les bornes. Le programme démarre sur la page Show en plein écran.
- **…et verrouillé** : il démarre en plus verrouillé : seulement le SHOW, sans navigation. Pour sortir, maintenez le bouton du cadenas enfoncé (avec le mot de passe, s’il y en a un). Voir [Show](show.md#plein-écran-et-verrouillage).
- **Verrouiller les écrans** : **« Verrouiller… »** / **« Déverrouiller… »**. C’est la même chose que le cadenas de la barre du haut.

### Verrouiller les écrans

Le verrouillage évite que quelque chose change par erreur pendant le show. Direct et le SHOW continuent de fonctionner, et les playbacks continuent de jouer.

1. Appuyez sur le **cadenas** de la barre du haut (ou sur « Verrouiller… » ici).
2. Choisissez ce qu’il faut verrouiller :
   - **Configuration** : Patch, Fixtures, Sorties, Mappages et Options. Il n’est pas non plus possible d’ouvrir ou de créer un autre show.
   - **Programmation** : Programmer et Scènes. Dans Playbacks, il n’est pas possible d’enregistrer, de mettre à jour ni d’assigner.
   - **Modification du SHOW** : les pages du SHOW s’utilisent, elles ne se modifient pas.
3. Si vous le souhaitez, tapez un **mot de passe** (il est facultatif).
4. Appuyez sur **« Verrouiller »**.

Les pages verrouillées portent un cadenas dans la barre latérale. Quand vous en ouvrez une, le mot de passe est demandé (s’il y en a un) et tout est déverrouillé. Pour déverrouiller, appuyez sur le cadenas de la barre du haut. **« Supprimer le mot de passe »** se trouve dans la même fenêtre, au moment de verrouiller.

## Sauvegarde automatique

- **Enregistrer automatiquement**, **Toutes les (minutes)** : de 1 à 60.

Les shows qui ont un nom sont écrasés et la version précédente est gardée en `.bak`. Les shows sans titre sont gardés à part et proposés à la réouverture du programme après une fermeture inattendue (« Récupérer le show non enregistré ? »). La barre du haut affiche l’heure de la dernière sauvegarde automatique ; si elle échoue (un disque plein, un fichier en lecture seule), un avertissement apparaît en rouge.

## Lecture

- **Priorité par défaut des nouvelles scènes** : de 0 à 10. Si deux scènes contrôlent le même canal, la priorité la plus haute l’emporte (voir [Priorités](directo.md#priorités)).
- **Priorité des modifications en direct** : ce que vous réglez dans Programmer l’emporte sur les scènes de cette priorité et en dessous. Avec 10, sur toutes les scènes.
- **Quand une scène démarre, les modifications en direct** :
  - **« Rendent ses canaux à la scène »** : les canaux que touche la scène ne sont plus dans le programmeur.
  - **« Restent toujours »**.
  - **« Sont toutes réinitialisées »** : le programmeur est vidé.
- **Le grand master agit sur** : **« Canaux d’intensité »** ou **« Tous les canaux »**.

## Audio

- **Entrée audio** : le micro ou l’entrée ligne qu’écoutent **Pulsation** et les matrices audio. **« Par défaut du système »** utilise celle du système d’exploitation. Pulsation change d’entrée immédiatement ; une matrice audio déjà en cours, à son prochain démarrage. Une entrée choisie qui n’est pas connectée pour l’instant apparaît comme « (non connectée) », et en attendant, celle par défaut est utilisée.

## Shows récents

La liste des derniers shows ouverts (aussi dans la flèche **▾** de la barre du haut). **« Vider la liste »** l’efface.

## Télécommande

Pour utiliser les pages du SHOW depuis un téléphone ou une tablette sur le même réseau Wi-Fi, sans rien installer :

- **Télécommande activée**.
- **Port** : changez-le si un autre programme l’utilise (par défaut, 8800). S’il est occupé, un avertissement apparaît en rouge.
- **PIN** : demandé une fois sur chaque appareil. Vide, tout le monde sur le réseau peut l’utiliser.
- Les **adresses** à ouvrir dans le navigateur du téléphone, avec un bouton pour les copier.
- Le nombre d’appareils connectés.

Voir [Télécommande](remoto.md).

## Bibliothèque de fixtures

Indique combien de définitions de fixtures sont disponibles (la bibliothèque intégrée, les génériques et les vôtres) et permet de l’enrichir :

- **Open Fixture Library** : **« Télécharger »** (nécessite internet). Une fois téléchargée, vous voyez combien de fixtures elle contient et quand elle a été téléchargée, et le bouton devient **« Mettre à jour »** : il apporte les fixtures nouveaux et améliorés, et conserve vos fixtures et ceux que la bibliothèque n’a plus. S’il y a des nouveautés, le programme le signale et le bouton est mis en évidence (voir [Fixtures](fixtures.md#open-fixture-library)).
- **Importer des fichiers de fixtures** : **« Importer… »** des définitions `.qxf` (le format de QLC+), des fichiers `.gdtf` ou un `.zip` qui les contient.
- **Fixtures GDTF** : **« GDTF-Share »** ouvre son site ; **« Importer GDTF… »** importe des fichiers, des dossiers ou le `.zip` de son téléchargement groupé. Indique combien de fixtures GDTF se trouvent dans la bibliothèque.
- **Importer mes fixtures QLC+** : **« Importer »** copie les fixtures que vous avez créés dans QLC+.
- **Dossier des fixtures utilisateur** : **« Ouvrir le dossier »**.

Tout cela est expliqué dans [Fixtures](fixtures.md).

## Mises à jour

- **Rechercher les mises à jour au démarrage** : une fois par jour, sur GitHub : le programme et, si vous avez téléchargé l’Open Fixture Library, si elle a de nouveaux fixtures (un avis le signale). Rien n’est téléchargé ni installé sans vous le demander. Sans internet, cela ne gêne pas.
- **Inclure les versions de test** : les nouveautés plus tôt, moins testées.

En dessous s’affichent l’état (« Vous avez la dernière version », « La version … est disponible ») et les boutons :

- **« Rechercher les mises à jour »** : vérifie maintenant (l’Open Fixture Library aussi).
- **« Nouveautés »** : ce qui est nouveau dans cette version.
- **« Télécharger et installer »** : télécharge l’installateur et vérifie son empreinte SHA-256 avant de l’enregistrer. Si une version ne publie pas son empreinte, elle ne peut être téléchargée que depuis sa page (« Voir sur GitHub »).
- **« Installer maintenant »** : le programme se ferme (il propose d’abord d’enregistrer le show), la nouvelle version s’installe et le programme se rouvre.
- **« À la fermeture »** / **« Pas à la fermeture »** : l’installer quand vous fermez le programme.
- **« Voir sur GitHub »** : la page de la version.

Quand une nouvelle version existe, un avis apparaît aussi au-dessus des pages avec les mêmes boutons. **« Plus tard »** masque l’avis pour cette version ; elle reste disponible ici.

## Aide

- **Manuel** : **« Ouvrir le manuel »** ouvre ce manuel sur son sommaire. F12 ou le bouton **« ? »** de la barre du haut l’ouvrent sur la page où vous êtes. Il s’affiche dans la langue du programme (espagnol, anglais, portugais, français, italien ou allemand) ; les captures d’écran sont celles de la version espagnole.
- **Guide de premiers pas** : **« Lancer le guide »** rouvre le guide (voir [Premiers pas](primeros-pasos.md#le-guide)).
- **Shows d’exemple** : ils sont dans la flèche **▾** à côté d’« Ouvrir » dans la barre du haut, dans « Exemples » (voir [le sommaire](index.md#le-guide-et-les-shows-dexemple)).

## À propos

La version du programme, sa licence (logiciel libre, Apache 2.0) et les crédits.
