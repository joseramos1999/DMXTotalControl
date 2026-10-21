# Sorties

![La page Sorties](../img/salidas.png)

**Sorties** (F7) connecte chaque **univers** du show (512 canaux DMX) à une **interface** : une interface DMX USB, un nœud Art-Net ou une sortie sACN. Elle contient aussi le moniteur des [niveaux DMX](salidas.md#niveaux-dmx), qui montre en direct ce qui sort.

Dans l’en-tête :

- **« Univers »** / **« Niveaux DMX »** : les deux vues de la page.
- **« Actualiser »** : recherche à nouveau les interfaces connectées.
- **« Retirer l’univers »** et **« Ajouter un univers »**.

## Connecter un univers

1. Branchez l’interface à l’ordinateur (ou au réseau, pour Art-Net et sACN).
2. Si elle n’apparaît pas dans **« Sorties détectées »**, appuyez sur **« Actualiser »**.
3. Sur la ligne de l’univers, ouvrez la liste déroulante (elle indique « Non connecté ») et choisissez la sortie.

Chaque sortie apparaît avec le nom de son type et celui de l’interface ou du réseau : « DMX USB — … », « ArtNet — … », « E1.31 — … » (E1.31 est sACN). L’univers connecté est marqué en vert. Pour le déconnecter, choisissez « Non connecté ».

La connexion est mémorisée pour la prochaine fois que vous ouvrez le programme.

> **Conseil :** les interfaces DMX USB basées sur des puces FTDI ont besoin de leurs pilotes. Si vous ne les avez pas, l’installateur du programme propose à la fin la page pour les télécharger.

## Univers

- **« Ajouter un univers »** en ajoute un à la fin. Quand vous patchez, vous choisissez dans quel univers va chaque fixture.
- Cliquez sur le **nom** d’un univers pour le changer (le nom est enregistré avec le show).
- **« Retirer l’univers »** retire le **dernier** (il en reste toujours un). S’il a des fixtures patchés, ce n’est pas possible : déplacez-les d’abord dans un autre univers ou supprimez-les dans le Patch. Si c’est l’entrée d’un contrôleur, le programme demande d’abord confirmation, car ce contrôleur ne fonctionnera plus jusqu’à ce que vous le réactiviez dans Mappages.

## Niveaux DMX

**« Niveaux DMX »** affiche les 512 canaux d’un univers tels qu’ils sortent du programme, avec tout appliqué (scènes, programmeur, mixeur, grand master, limites…) :

- Choisissez l’**Univers** dans la liste déroulante.
- Chaque case contient le numéro du canal et sa valeur (0–255). **« Afficher en % »** passe en pourcentage.
- Les cases avec un bord appartiennent à un fixture. Survolez un canal pour voir à quel fixture il appartient (« Nom — canal N »).

> **Conseil :** si un fixture ne répond pas, regardez ici si son canal bouge. S’il bouge, le problème vient du câble, de l’adresse de l’appareil ou de l’interface ; sinon, du patch ou de ce que vous lancez.

Le même moniteur peut être une section du [SHOW](show.md) (« Niveaux DMX »).

## Entrée DMX

Une interface avec entrée DMX (ou une console branchée dessus) permet de piloter le programme : elle s’active comme appareil dans [Mappages](mapeos.md#appareils).
