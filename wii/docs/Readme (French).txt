Le manuel de WiiFlow

Sommaire
========

1.1) Installation de WiiFlow
1.2) Installation de WiiFlow sur une carte SD
1.3) Installation de WiiFlow sur un disque dur
1.4) Démarrer WiiFlow

2.1) Navigation dans le menu principal "Coverflow"
2.2) Navigation dans le menu de sélection du jeu
2.3) Paramètres de WiiFlow
2.4) Installation de jeux
2.5) Téléchargement des jaquettes
2.6) Mot de passe-Administration
2.7) Paramètres de jeu

3.1) Jouer une musique de fond
3.2) Installation des thèmes
3.3) Paramètres graphiques de WiiFlow (écran principal et de sélection de jeux)
3.4) Configuration du fichier ini


1.1 Installation de WiiFlow
------------------------------
Vous pouvez installer WiiFlow soit sur votre carte SD soit sur votre disque dur. Chaque méthode a
des avantages et des inconvénients : Si vous l'installez sur votre carte SD, vous aurez besoin de la
carte SD pour faire fonctionner WiiFlow. Vous devrez toujours laisser la carte SD dans votre
Wii et ne devrez pas la protéger en écriture, sinon WiiFlow ne fonctionnera pas correctement.
Il est préférable d'installer WiiFlow sur votre disque dur, mais vous aurez besoin d'une partition FAT
pour lui et d'une deuxième partition WBFS pour vos jeux Wii. Si vous avez seulement une partition
WBFS, vous aurez besoin de repartitionner votre disque dur. Cela va supprimer tout vos jeux Wii
installés. Alors je vais vous expliquer les deux méthodes maintenant :

1.2 Installation de WiiFlow sur une carte SD
-----------------------------------------------
Il suffit de copier le contenu de l'archive à la racine de votre carte SD. Vous devriez
obtenir le chemin d'accès suivant :
SD:/wiiflow/language.ini
WiiFlow sauvegardera tous les paramètres et les fichiers dans le répertoire SD:/wiiflow/.
Exception : si vous connectez un disque dur avec des partitions FAT et WBFS, WiiFlow
sauvegardera ses fichiers sur la partition FAT. Pour éviter cela, vous devez ouvrir le
fichier SD:/wiiflow/wiiflow.ini et ajoutez la mention "data_on_usb = no" dans la catégorie
[GENERAL]. WiiFlow ignorera la partition FAT. Si wiiflow.ini n'est pas là, copier celui
qui a été créé sur le disque dur.

1.3 Installation de WiiFlow sur un disque dur
------------------------------------------------
Il est nécessaire de diviser votre disque dur en au moins deux partitions pour installer
WiiFlow sur l'une d'elle. L'une est la partition WBFS qui contient tout les jeux Wii
et l'autre est une partition FAT16 ou FAT32 et contient les données de WiiFlow.
(Si vous avez qu'une partition WBFS sur votre disque dur, vous devez le reformater
en deux parties. La première en FAT et la seconde en WBFS. Vous allez perdre 
tout les jeux déjà installés.)
Si vous avez obtenu cela, il vous suffit de copier le contenu de l'archive sur votre
disque dur externe. Vous devez obtenir le chemin d'accès suivant:
USB:/wiiflow/language.ini
WiiFlow sauvegardera tous les paramètres et les fichiers dans le répertoire USB:/wiiflow/.
Exception : Si vous mettez une carte SD avec l'entrée "data_on_usb = no" dans wiiflow.ini
dans votre Wii, WiiFlow utilisera le dossier de la SD et non celui de votre disque dur.

1.4 Démarrer WiiFlow
-----------------------
Vous pouvez lancer WiiFlow en exécutant le boot.dol de l'archive. WiiFlow se lance
de partout. Il ne se soucie pas du répertoire courant. Il saura travailler dans le
répertoire que vous avez installé (SD:/wiiflow ou USB:/wiiflow). Mais je vais vous
expliquer quelques options pour démarrer WiiFlow :
1.4.1) Démarrer WiiFlow via l'Homebrew Channel
Vous devez enregistrer le fichier "boot.dol" dans le répertoire SD:/apps/wiiflow/ pour que
l'Homebrew Channel puisse le trouver. Les fichiers meta.xml et icon.png appartiennent aussi à 
ce répertoire, mais ils ne sont pas nécessaires pour l'exécution.
1.4.2) Démarrer WiiFlow grâce à une chaîne WiiFlow
Si vous avez une chaîne WiiFlow (pas un forwarder) le boot.dol est déjà à l'intérieur
et il sera lancé. Il n'est pas nécessaire de l'enregistrer n'importe où ailleurs.
Vous pouvez installer une de ces chaînes, avec l'aide d'un Wad-Manager.
1.4.3) Démarrer WiiFlow grâce à une chaîne Forwarder
Un forwarder est une chaîne qui exécute un fichier spécifique dans un répertoire spécifique.
Donc, vous avez besoin de renommer le boot.dol et de le placer dans le dossier cherché par
le forwarder pour qu'il puisse le trouver. Demandez à l'auteur du forwarder le dossier recherché.
1.4.4) Démarrer WiiFlow  grâce à Preloader
Renommez boot.dol en WiiFlow.dol et enregistrez-le à la racine de votre carte SD. Maintenant
démarrez Preloader et sélectionnez "Load/Install File" et puis sélectionnez "WiiFlow.dol".
Après un court laps de temps le fichier est installé et vous pouvez démarrer automatiquement
WiiFlow au démarrage de votre Wii.



2,1 à 2,7 n'est pas traduit (désolé)
---------------------------------------
!!! J'ai sauté la traduction de ces points parce que WiiFlow devrait être !!!
!!! auto-explicatif. Je vais la faire plus tard pour éviter une sortie retardée. !!!



3.1 Jouer une musique de fond
--------------------------------
Si vous voulez écouter de la musique dans WiiFlow, vous pouvez copier un ou plusieurs fichiers
audio dans /wiiflow/music/. WiiFlow joue en ordre aléatoire l'un de ces fichiers à chaque 
fois que vous le démarrez. Quand la musique arrive à sa fin, il va commencer à nouveau. 
Si vous voulez que WiiFlow joue un autre fichier, vous devez faire un redémarrage en maintenant 
le bouton B et en appuyant sur Accueil. WiiFlow ne joue que les fichiers OGG et n'est pas 
compatible avec le format MP3, mais il existe des convertisseurs gratuits.

3.2) Installation des thèmes
-------------------------------
Vous pouvez changer l'apparence de WiiFlow en installant un thème supplémentaire. Vous pouvez
soit télécharger un thème ou en créer un vous-même. Pour installer un thème il suffit de le copier
dans le répertoire /wiiflow/themes/. Un thème se compose d'un fichier ini et d'un répertoire,
qui contient les images et les polices. Vous n'avez qu'à vérifier si les deux ont le même nom
(par exemple pour le fichier "/wiiflow/themes/pear.ini" appartient au répertoire
"/wiiflow/themes/pear/" et à tout les fichiers à l'intérieur de celui-ci).
Après avoir copié le fichier ini et son répertoire, vous pouvez choisir ce thème dans les paramètres.

3.3) Paramètres graphiques de WiiFlow (écrans principal et de sélection de jeux)
---------------------------------------------------------------------------------
Cette fonction a été faite pour aider les concepteurs de thème même si tout cela
peut paraître un peu compliqué. Vous pouvez personnaliser WiiFlow en cliquant sur 
"Ajuster Coverflow" dans les paramètres en page 2/6. Vous pouvez déplacer les 
jaquettes, la caméra et beaucoup plus avec cela. Vous pouvez même configurer 
l'oscillation des jaquettes. Je ne vais pas expliquer les diverses fonctions ici. 
Il suffit de le tenter par vous même. Sur la partie supérieure de l'écran, vous trouverez 
quelques boutons. Sur le bord gauche, ils sont réservés pour changer la vue actuelle. 
Au milieu, vous pouvez basculer entre les différentes fonctions et sur le bord droit 
ce sont les boutons pour enregistrer ou pour quitter sans enregistrer vos modifications. 
Les paramètres sont eux-mêmes sur le fond de l'écran. Vous pouvez copier tout en maintenant 
enfoncé le bouton B et en appuyant sur 1. Maintenant, vous pouvez passer à un autre point 
de vue, maintenez le bouton B à nouveau et appuyez sur 2 pour "coller". Vos modifications 
seront enregistrées à l'intérieur du fichier ini qui appartient au thème. Pendant que vous 
êtes sur le réglage d'un paramètre, appuyez sur B pour accélérer (donc appuyez sur B + A au lieu
d'appuyer que sur A lorsque vous cliquez sur le bouton).

3.4 Configuration du fichier ini
-----------------------------------
WiiFlow va créer le fichier /wiiflow/wiiflow.ini à sa première mise en route. Il enregistrera 
tous les paramètres à l'intérieur. Vous pouvez ouvrir et modifier ce fichier avec votre PC, 
mais l'éditeur Windows par défaut "notepad" ne peut pas l'afficher correctement. Il faut donc 
utiliser Wordpad sous Windows (et éviter tout formatage texte) ou un meilleur éditeur de texte 
comme Notepad++. Je vais seulement expliquer les réglages que vous ne pouvez pas trouver 
à l'intérieur de WiiFlow. Tous les réglages doivent être à l'intérieur de la catégorie [GENERAL]:

data_on_usb = no: WiiFlow ignorera la partition FAT sur votre disque dur et
utilisera la carte SD à la place (comme expliqué en cours d'installation).
compress_cache = no: Il s'agit d'une compression zlib inutile des couvertures cache.
Ignorez simplement ce paramètre et laissez sur "no".
cover_buffer = 120: Le nombre maximum de jaquettes stockées en mémoire. 120 est une bonne
valeur pour une résolution de 512x512 pixels ou 1024x1024 pixels par jaquette.
keep_png = oui: WiiFlow travaille seulement avec les jaquettes qui sont dans le dossier de cache.
Ici, vous pouvez forcer WiiFlow à supprimer le fichier png, après qu'un fichier de cache ait été créé.
max_fsaa = 5: Maximum anticrénelage (ici: 5x). Cela serait la limite dans chaque thèmes
et une valeur supérieur à cinq est rarement assez rapide pour WiiFlow. Donc l'antiliasing
appliqué est le minimum entre cette valeur et celle que l'on trouve dans la description du thème.
url_flat_covers =... : Ici vous pouvez entrer les URL pour les faces de jaquettes.
url_full_covers =... : Idem que ci-dessus, mais pour les jaquettes complètes (avant et arrière).

Vous pouvez entrer des URL de plus ici en les séparant par le caractère "|". WiiFlow utilise les
variables que vous voulez utiliser pour spécifier la recherche de jaquettes. Utilisez soit {gameid4}
ou {gameid6} pour indiquer l'ID du jeu (avec 4 ou 6 lettres). Utilisez {loc} pour indiquer le code pays 
(comme il est utilisé sur wiitdb) et {région} pour la région de jeu (comme il est utilisé sur wiiboxart).
