Le manuel de WiiFlow

Sommaire
========

1.1) Installation de WiiFlow
1.2) Installation de WiiFlow sur une carte SD
1.3) Installation de WiiFlow sur un disque dur
1.4) D�marrer WiiFlow

2.1) Navigation dans le menu principal "Coverflow"
2.2) Navigation dans le menu de s�lection du jeu
2.3) Param�tres de WiiFlow
2.4) Installation de jeux
2.5) T�l�chargement des jaquettes
2.6) Mot de passe-Administration
2.7) Param�tres de jeu

3.1) Jouer une musique de fond
3.2) Installation des th�mes
3.3) Param�tres graphiques de WiiFlow (�cran principal et de s�lection de jeux)
3.4) Configuration du fichier ini


1.1 Installation de WiiFlow
------------------------------
Vous pouvez installer WiiFlow soit sur votre carte SD soit sur votre disque dur. Chaque m�thode a
des avantages et des inconv�nients : Si vous l'installez sur votre carte SD, vous aurez besoin de la
carte SD pour faire fonctionner WiiFlow. Vous devrez toujours laisser la carte SD dans votre
Wii et ne devrez pas la prot�ger en �criture, sinon WiiFlow ne fonctionnera pas correctement.
Il est pr�f�rable d'installer WiiFlow sur votre disque dur, mais vous aurez besoin d'une partition FAT
pour lui et d'une deuxi�me partition WBFS pour vos jeux Wii. Si vous avez seulement une partition
WBFS, vous aurez besoin de repartitionner votre disque dur. Cela va supprimer tout vos jeux Wii
install�s. Alors je vais vous expliquer les deux m�thodes maintenant :

1.2 Installation de WiiFlow sur une carte SD
-----------------------------------------------
Il suffit de copier le contenu de l'archive � la racine de votre carte SD. Vous devriez
obtenir le chemin d'acc�s suivant :
SD:/wiiflow/language.ini
WiiFlow sauvegardera tous les param�tres et les fichiers dans le r�pertoire SD:/wiiflow/.
Exception : si vous connectez un disque dur avec des partitions FAT et WBFS, WiiFlow
sauvegardera ses fichiers sur la partition FAT. Pour �viter cela, vous devez ouvrir le
fichier SD:/wiiflow/wiiflow.ini et ajoutez la mention "data_on_usb = no" dans la cat�gorie
[GENERAL]. WiiFlow ignorera la partition FAT. Si wiiflow.ini n'est pas l�, copier celui
qui a �t� cr�� sur le disque dur.

1.3 Installation de WiiFlow sur un disque dur
------------------------------------------------
Il est n�cessaire de diviser votre disque dur en au moins deux partitions pour installer
WiiFlow sur l'une d'elle. L'une est la partition WBFS qui contient tout les jeux Wii
et l'autre est une partition FAT16 ou FAT32 et contient les donn�es de WiiFlow.
(Si vous avez qu'une partition WBFS sur votre disque dur, vous devez le reformater
en deux parties. La premi�re en FAT et la seconde en WBFS. Vous allez perdre 
tout les jeux d�j� install�s.)
Si vous avez obtenu cela, il vous suffit de copier le contenu de l'archive sur votre
disque dur externe. Vous devez obtenir le chemin d'acc�s suivant:
USB:/wiiflow/language.ini
WiiFlow sauvegardera tous les param�tres et les fichiers dans le r�pertoire USB:/wiiflow/.
Exception : Si vous mettez une carte SD avec l'entr�e "data_on_usb = no" dans wiiflow.ini
dans votre Wii, WiiFlow utilisera le dossier de la SD et non celui de votre disque dur.

1.4 D�marrer WiiFlow
-----------------------
Vous pouvez lancer WiiFlow en ex�cutant le boot.dol de l'archive. WiiFlow se lance
de partout. Il ne se soucie pas du r�pertoire courant. Il saura travailler dans le
r�pertoire que vous avez install� (SD:/wiiflow ou USB:/wiiflow). Mais je vais vous
expliquer quelques options pour d�marrer WiiFlow :
1.4.1) D�marrer WiiFlow via l'Homebrew Channel
Vous devez enregistrer le fichier "boot.dol" dans le r�pertoire SD:/apps/wiiflow/ pour que
l'Homebrew Channel puisse le trouver. Les fichiers meta.xml et icon.png appartiennent aussi � 
ce r�pertoire, mais ils ne sont pas n�cessaires pour l'ex�cution.
1.4.2) D�marrer WiiFlow gr�ce � une cha�ne WiiFlow
Si vous avez une cha�ne WiiFlow (pas un forwarder) le boot.dol est d�j� � l'int�rieur
et il sera lanc�. Il n'est pas n�cessaire de l'enregistrer n'importe o� ailleurs.
Vous pouvez installer une de ces cha�nes, avec l'aide d'un Wad-Manager.
1.4.3) D�marrer WiiFlow gr�ce � une cha�ne Forwarder
Un forwarder est une cha�ne qui ex�cute un fichier sp�cifique dans un r�pertoire sp�cifique.
Donc, vous avez besoin de renommer le boot.dol et de le placer dans le dossier cherch� par
le forwarder pour qu'il puisse le trouver. Demandez � l'auteur du forwarder le dossier recherch�.
1.4.4) D�marrer WiiFlow  gr�ce � Preloader
Renommez boot.dol en WiiFlow.dol et enregistrez-le � la racine de votre carte SD. Maintenant
d�marrez Preloader et s�lectionnez "Load/Install File" et puis s�lectionnez "WiiFlow.dol".
Apr�s un court laps de temps le fichier est install� et vous pouvez d�marrer automatiquement
WiiFlow au d�marrage de votre Wii.



2,1 � 2,7 n'est pas traduit (d�sol�)
---------------------------------------
!!! J'ai saut� la traduction de ces points parce que WiiFlow devrait �tre !!!
!!! auto-explicatif. Je vais la faire plus tard pour �viter une sortie retard�e. !!!



3.1 Jouer une musique de fond
--------------------------------
Si vous voulez �couter de la musique dans WiiFlow, vous pouvez copier un ou plusieurs fichiers
audio dans /wiiflow/music/. WiiFlow joue en ordre al�atoire l'un de ces fichiers � chaque 
fois que vous le d�marrez. Quand la musique arrive � sa fin, il va commencer � nouveau. 
Si vous voulez que WiiFlow joue un autre fichier, vous devez faire un red�marrage en maintenant 
le bouton B et en appuyant sur Accueil. WiiFlow ne joue que les fichiers OGG et n'est pas 
compatible avec le format MP3, mais il existe des convertisseurs gratuits.

3.2) Installation des th�mes
-------------------------------
Vous pouvez changer l'apparence de WiiFlow en installant un th�me suppl�mentaire. Vous pouvez
soit t�l�charger un th�me ou en cr�er un vous-m�me. Pour installer un th�me il suffit de le copier
dans le r�pertoire /wiiflow/themes/. Un th�me se compose d'un fichier ini et d'un r�pertoire,
qui contient les images et les polices. Vous n'avez qu'� v�rifier si les deux ont le m�me nom
(par exemple pour le fichier "/wiiflow/themes/pear.ini" appartient au r�pertoire
"/wiiflow/themes/pear/" et � tout les fichiers � l'int�rieur de celui-ci).
Apr�s avoir copi� le fichier ini et son r�pertoire, vous pouvez choisir ce th�me dans les param�tres.

3.3) Param�tres graphiques de WiiFlow (�crans principal et de s�lection de jeux)
---------------------------------------------------------------------------------
Cette fonction a �t� faite pour aider les concepteurs de th�me m�me si tout cela
peut para�tre un peu compliqu�. Vous pouvez personnaliser WiiFlow en cliquant sur 
"Ajuster Coverflow" dans les param�tres en page 2/6. Vous pouvez d�placer les 
jaquettes, la cam�ra et beaucoup plus avec cela. Vous pouvez m�me configurer 
l'oscillation des jaquettes. Je ne vais pas expliquer les diverses fonctions ici. 
Il suffit de le tenter par vous m�me. Sur la partie sup�rieure de l'�cran, vous trouverez 
quelques boutons. Sur le bord gauche, ils sont r�serv�s pour changer la vue actuelle. 
Au milieu, vous pouvez basculer entre les diff�rentes fonctions et sur le bord droit 
ce sont les boutons pour enregistrer ou pour quitter sans enregistrer vos modifications. 
Les param�tres sont eux-m�mes sur le fond de l'�cran. Vous pouvez copier tout en maintenant 
enfonc� le bouton B et en appuyant sur 1. Maintenant, vous pouvez passer � un autre point 
de vue, maintenez le bouton B � nouveau et appuyez sur 2 pour "coller". Vos modifications 
seront enregistr�es � l'int�rieur du fichier ini qui appartient au th�me. Pendant que vous 
�tes sur le r�glage d'un param�tre, appuyez sur B pour acc�l�rer (donc appuyez sur B + A au lieu
d'appuyer que sur A lorsque vous cliquez sur le bouton).

3.4 Configuration du fichier ini
-----------------------------------
WiiFlow va cr�er le fichier /wiiflow/wiiflow.ini � sa premi�re mise en route. Il enregistrera 
tous les param�tres � l'int�rieur. Vous pouvez ouvrir et modifier ce fichier avec votre PC, 
mais l'�diteur Windows par d�faut "notepad" ne peut pas l'afficher correctement. Il faut donc 
utiliser Wordpad sous Windows (et �viter tout formatage texte) ou un meilleur �diteur de texte 
comme Notepad++. Je vais seulement expliquer les r�glages que vous ne pouvez pas trouver 
� l'int�rieur de WiiFlow. Tous les r�glages doivent �tre � l'int�rieur de la cat�gorie [GENERAL]:

data_on_usb = no: WiiFlow ignorera la partition FAT sur votre disque dur et
utilisera la carte SD � la place (comme expliqu� en cours d'installation).
compress_cache = no: Il s'agit d'une compression zlib inutile des couvertures cache.
Ignorez simplement ce param�tre et laissez sur "no".
cover_buffer = 120: Le nombre maximum de jaquettes stock�es en m�moire. 120 est une bonne
valeur pour une r�solution de 512x512 pixels ou 1024x1024 pixels par jaquette.
keep_png = oui: WiiFlow travaille seulement avec les jaquettes qui sont dans le dossier de cache.
Ici, vous pouvez forcer WiiFlow � supprimer le fichier png, apr�s qu'un fichier de cache ait �t� cr��.
max_fsaa = 5: Maximum anticr�nelage (ici: 5x). Cela serait la limite dans chaque th�mes
et une valeur sup�rieur � cinq est rarement assez rapide pour WiiFlow. Donc l'antiliasing
appliqu� est le minimum entre cette valeur et celle que l'on trouve dans la description du th�me.
url_flat_covers =... : Ici vous pouvez entrer les URL pour les faces de jaquettes.
url_full_covers =... : Idem que ci-dessus, mais pour les jaquettes compl�tes (avant et arri�re).

Vous pouvez entrer des URL de plus ici en les s�parant par le caract�re "|". WiiFlow utilise les
variables que vous voulez utiliser pour sp�cifier la recherche de jaquettes. Utilisez soit {gameid4}
ou {gameid6} pour indiquer l'ID du jeu (avec 4 ou 6 lettres). Utilisez {loc} pour indiquer le code pays 
(comme il est utilis� sur wiitdb) et {r�gion} pour la r�gion de jeu (comme il est utilis� sur wiiboxart).
