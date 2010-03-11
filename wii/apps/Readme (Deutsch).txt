

    WiiFlow  Handbuch


  Inhalt
 ========

1.1) Installation von WiiFlow
1.2) Installation von WiiFlow auf SD-Karte
1.3) Installation von WiiFlow auf Festplatte
1.4) Starten von WiiFlow

2.1) Bedienung im Hauptbildschirm "Coverflow"
2.2) Bedienung im Spiele-Bildschirm
2.3) WiiFlow Einstellungen
2.4) Spiele installieren
2.5) H�llen herunterladen
2.6) Kennwortschutz verwalten
2.7) Spiele-Einstellungen

3.1) Hintergrundmusik einrichten
3.2) Themenpakete verwalten
3.3) WiiFlow anpassen (Haupt- und Spiele-Bildschirm)
3.4) ini-Datei von WiiFlow anpassen und einstellen



  1.1 Installation von WiiFlow
 ------------------------------
WiiFlow kann entweder auf SD-Karte oder auf Festplatte installiert werden. 
Beides hat Vor- und Nachteile: Bei der Installation auf SD-Karte wird immer 
eine SD-Karte zum Betrieb ben�tigt. Diese sollte immer eingelegt bleiben und 
darf nicht schreibgesch�tzt sein. Wenn sie nicht eingelegt ist, funktioniert 
WiiFlow eventuell nicht richtig, zeigt keine H�llen an und spielt die Musik 
nicht ab. Eventuell ist ein Start ohne die SD-Karte gar nicht m�glich.
F�r die Festplatteninstallation wird eine Festplatte mit einer FAT32- und einer 
WBFS-Partition ben�tigt. Eine eventuell schon vorhandene reine WBFS-Festplatte 
m�sste gegebenenfalls neu partitioniert werden, was ein Verlust aller bereits 
installierten Spiele bedeuten w�rde. Ich erkl�re nun beide Installationen.

  1.2 Installation von WiiFlow auf SD-Karte
 -------------------------------------------
Kopieren Sie den Inhalt des Paketes einfach in das Hauptverzeichnis Ihrer 
SD-Karte. Auf Ihrer SD-Karte sollte sich dann folgender Pfad befinden:
SD:\wiiflow\language.ini
WiiFlow wird alle Einstellungen in das Verzeichnis SD:\wiiflow\ speichern.
Ausnahme: Wenn Sie eine Festplatte mit FAT- und WBFS-Partition haben, wird 
WiiFlow seine Daten auf die FAT-Partition der Festplatte speichern. Um dieses 
zu verhindern, kann in der Datei SD:\wiiflow\wiiflow.ini unter der Rubrik 
[GENERAL] der Eintrag "data_on_usb=no" erstellt werden. WiiFlow ignoriert dann 
die FAT-Partition. Falls die wiiflow.ini nicht vorhanden ist, wird sie von 
WiiFlow beim Erststart erstellt.

  1.3 Installation von WiiFlow auf Festplatte
 ---------------------------------------------
Damit WiiFlow von Festplatte aus arbeiten kann, ist es notwendig, die 
Festplatte in mindestens zwei Partitionen aufzuteilen. Eine der beiden 
Partitionen ist die WBFS-Partition und enth�lt alle Wii-Spiele. Die andere muss 
eine FAT16- oder FAT32-Partition sein und enth�lt die Daten f�r WiiFlow.
 (Wenn Sie bereits eine Festplatte haben, die nur eine WBFS-Partition besitzt,
  m�ssen Sie diese neu partitionieren. Dabei gehen alle Spiele verloren).
Kopieren Sie den Inhalt des Paketes in das Hauptverzeichnis Ihrer FAT32-
Partition auf der Festplatte. Dort sollte sich dann folgender Pfad befinden:
USB:\wiiflow\language.ini
WiiFlow wird alle Einstellungen in das Verzeichnis USB:\wiiflow\ speichern.
Ausnahme: Wenn Sie eine SD-Karte mit dem Eintrag "data_on_usb=no" in der Datei 
SD:\wiiflow\wiiflow.ini einlegen, werden die Daten auf die SD-Karte gespeichert.

  1.4 Starten von WiiFlow
 -------------------------
Um WiiFlow zu starten, f�hren sie einfach die beigef�gte Datei "boot.dol" aus.
Diese Datei kann von �berall aus gestartet werden. WiiFlow arbeitet alleine in 
dem Verzeichnis, in dem es installiert wurde (SD:\wiiflow  oder  USB:\wiiflow).
Dennoch erkl�re ich hier kurz ein paar M�glichkeiten WiiFlow zu starten:
1.4.1) Starten von WiiFlow �ber den Homebrew-Kanal
Speichern Sie die Datei "boot.dol" in dem Verzeichnis SD:\apps\wiiflow\boot.dol 
damit der Homebrew-Kanal WiiFlow finden kann. Die Dateien meta.xml und icon.png 
geh�ren auch in dieses Verzeichnis, sind aber nicht zum starten notwendig.
1.4.2) Starten von WiiFlow �ber einen WiiFlow-Kanal
Wenn Sie einen WiiFlow-Kanal haben (keinen Forwarder-Kanal), ist die "boot.dol" 
bereits in dem Kanal integriert und wird von dort auch gestartet.
Der Kanal wird normalerweise �ber einen WAD-Manager installiert.
1.4.3) Starten von WiiFlow �ber einen Forwarder-Kanal
Ein Forwarder ist ein Kanal, der eine bestimmte Datei von einer bestimmten 
Stelle aus startet. Die Datei "boot.dol" muss also eventuell umbenannt und an 
diese Stelle gebracht werden. N�heres erfahren Sie vom Autor des Forwarders.
1.4.4) Starten von WiiFlow �ber Preloader
Benennen Sie die Datei "boot.dol" in "WiiFlow.dol" um und speichern Sie diese 
in das Hauptverzeichnis Ihrer SD-Karte. Starten Sie anschlie�end den Preloader 
und w�hlen Sie dort "Load/Install File" und dann die "WiiFlow.dol" aus. Nach 
kurzer Zeit ist die Datei installiert und in den "Settings" kann man nun den 
Preloader so einstellen, dass nach dem Einschalten automatisch WiiFlow startet.



  2.1 Bedienung im Hauptbildschirm
 ----------------------------------
Nach dem Starten von WiiFlow erscheint direkt der Hauptbildschirm. (Dieser wird 
auch als Coverflow bezeichnet.) Um ein Spiel hervorzuheben, dr�ckt man entweder 
die Richtungstasten der Wii Fernbedienung oder zeigt mit ihr auf den linken 
oder rechten Rand des Bildschirms. Dort kann man dann auf die Pfeile klicken, 
um weiterzubl�ttern. Wenn man schneller bl�ttern m�chte, kann durch dr�cken der 
Plus- und Minus-Tasten bzw. durch gedr�ckt halten von B und den Plus- und 
Minus-Tasten entweder Seitenweise oder zum n�chsten Buchstaben bl�ttern. 
Diese beiden Funktionen k�nnen in den Einstellungen getauscht werden.
Um ein Spiel auszuw�hlen, zeigt man einfach auf die entsprechende H�lle und 
dr�ckt auf A. Wenn der Zeiger sich au�erhalb des Bildschirms befindet, wird 
das Spiel ausgew�hlt, dessen H�lle hervorgehoben ist. Man kann auch ein Spiel 
sofort starten, indem man B gedr�ckt h�lt, mit der Fernbedienung auf die 
entsprechende H�lle zeigt und dann A dr�ckt.
Wenn man den Zeiger an den unteren Rand bewegt, erscheinen dort vier Symbole. 
Links ist das erste f�r Allgemeine Informationen und das zweite f�r die WiiFlow-
Einstellungen. Das in der Mitte ist f�r die Favoriten und das rechte ist zum 
beenden von WiiFlow. Die Reihenfolge kann allerdings je nach Thema variieren.
Zum Beenden klickt man entweder das Symbol an oder dr�ckt auf die Home-Taste 
der Wii-Fernbedienung.  H�lt man B gedr�ckt und dr�ckt auf die Home-Taste, 
startet WiiFlow neu. Mit den Tasten 1 und 2 kann man zwischen unterschiedlich 
angeordneten Bildschirmen hin- und herschalten.

  2.2 Bedienung im Spiele-Bildschirm
 -----------------------------------
Wenn ein Spiel ausgew�hlt wurde, wird die ausgew�hlte H�lle gro� dargestellt. 
Die H�lle kann gedreht werden, indem man darauf zeigt und A dr�ckt. Durch 
dr�cken von B oder auf den Knopf Zur�ck kommt man zur�ck zum Hauptbildschirm. 
Das Spiel wird gestartet, indem man entweder auf Start klickt oder auf A wenn 
sich der Zeiger au�erhalb des Bildschirms befindet. Auch hier kann mit den 
Richtungstasten Links und Rechts zum n�chsten Spiel gewechselt werden. Mit den 
Plus- und Minus- Tasten ist die vertikale Bewegung m�glich. Mit den 
Richtungstasten Oben und Unten kann man die Kn�pfe ausw�hlen.
�ber die vier Schaltfl�chen kann man Spiel-spezifische Einstellungen vornehmen 
(n�heres unter Punkt 2.7), es zu den Favoriten hinzuf�gen oder davon entfernen, 
es vor unerlaubtem Starten sch�tzen sowie den Schutz aufheben und es dauerhaft 
von der Festplatte l�schen. Diese vier Funktionen k�nnen blockiert werden indem 
man einen Kennwortschutz einstellt (dies wird unter Punkt 2.6 erkl�rt).

  2.3 WiiFlow Einstellungen
 ---------------------------
Hier kann man die Einstellungen anpassen, die WiiFlow generell nutzen soll. 
Einige Einstellungen k�nnen sp�ter auch pro Spiel vorgenommen werden. Die 
Einstellungen sind �ber mehrere Seiten verteilt. Wechseln kann man mit dem 
Knopf in der unteren linken Ecke oder mit den Plus- und Minus-Tasten. Zur�ck 
zum Hauptbildschirm kommt man mit dem Zur�ck-Knopf in der unteren rechten Ecke.
Hier nun eine kurze Beschreibung der einzelnen Funktionen:

H�llen und Namen: Hier k�nnen die H�llen f�r die Spiele heruntergeladen werden.
  Au�erdem kann man hier eine Titel-Datenbank herunterladen, um die Spiele mit 
  den richtigen Namen angezeigt zu bekommen (mehr unter 2.5).
Passwortschutz: WiiFlow verf�gt �ber eine Passwort-Funktion. Wenn diese aktiv 
  ist, wird verhindert, das gesch�tzte Spiele angezeigt, etwas an den 
  Einstellungen ver�ndert oder Spiele gel�scht werden (mehr unter 2.6).
3D-H�llen: Die H�llen k�nnen auch nur als Bild angezeigt werden.
Vibration: Aktiviert oder deaktiviert die Vibration in der Wii-Fernbedienung.
Thema: Hier kann das Thema ausgew�hlt werden, das angezeigt werden soll. Man 
  kann mehrere Themen installieren und hier eines aussuchen (mehr unter 3.2).
Sprache: Urspr�nglich ist hier die englische Sprache eingestellt. F�r Deutsch 
  sollte man diesen Wert auf GERMAN umstellen.
Coverflow einstellen: Hier kann die Position der H�llen und deren Verhalten 
  eingestellt werden. Diese Unterfunktion ist sehr komplex und ausschlie�lich 
  in Englisch vorhanden. Es in dieser Anleitung zu erkl�ren w�rde sie sprengen.
Spiel installieren: Dieses Unterprogramm von WiiFlow kopiert die Wii-Spiele von 
  DVD-ROM oder DVD-R auf die angeschlossene USB-Festplatte (mehr unter 2.4).
Zur�ck zum Wii-Men�: Hier stellt man das Verhalten des Home-Knopfes im Haupt-
  bildschirm und von der Wii-Fernbedienung ein. Wenn diese Funktion aktiv ist, 
  kehrt WiiFlow immer zur�ck zum Wii-Men�. Ansonsten geht WiiFlow zur�ck zu dem 
  Programm, von dem aus es gestartet wurde (z.B. den Homebrew-Kanal).
Favoriten-Modus merken: Durch aktivieren dieser Funktion zeigt WiiFlow beim 
  starten nur die Favoriten an, falls sie beim Beenden auch aktiv waren. Da man 
  nur eine Handvoll Spiele auf einmal spielt, kann dies sehr n�tzlich sein.
Standard Such-Modus: Hier wird das Verhalten der Plus- und Minus-Tasten im 
  Hauptbildschirm eingestellt. WiiFlow springt dann entweder zum n�chsten 
  Buchstaben des Alphabets oder bl�ttert eine Seite weiter.
Lautst�rke: Hier wird eingestellt wie laut die entsprechenden T�ne sein sollen: 
 -Hintergrundmusik: Die Musik, die unter \wiiflow\music\ gespeichert wurde.
 -WiiFlow-Tasten:   Die Ger�usche, die von den Kn�pfen her kommen.
 -H�llen-Bewegung:  Ger�usche von H�llen beim Ausw�hlen. (Themen-bedingt)
 -Spiele-Melodie:   Die Musik des ausgew�hlten Spieles, welche normalerweise 
                    vom Disc-Kanal abgespielt wird.
  Ist eine Lautst�rke auf 0 eingestellt, wird sie gar nicht gespielt.
Standard Video-Modus: Hier kann ein Spiel gezwungen werden, einen anderen 
  Video-Modus (z.B. NTSC, PAL-50 oder PAL-60) zu verwenden. Diese Einstellung 
  kann auch f�r einzelne Spiele separat eingestellt werden (mehr unter 2.7).
Standard Spiel-Sprache: Hier kann man ein Spiel dazu zwingen, eine andere 
  Sprache zu verwenden. Auch diese Einstellung kann f�r einzelne Spiele separat 
  eingestellt werden (mehr unter 2.7).
"Error 002" korrigieren: Einige neuere Spiele erfordern das Aktivieren dieser 
  Option, da sonst beim Starten der Fehler Nr. 002 auftritt. �ltere Spiele 
  st�rt diese Option nicht. Dennoch kann auch diese separat eingestellt werden.
Ocarina: Aktiviert oder deaktiviert das integrierte Schummelmodul von WiiFlow.
TV-Breite/H�he einstellen: Hier kann man die Aufl�sung des Fernsehers anpassen.
Horizontaler/Vertikaler Versatz: Verschiebt das Bild auf dem Fernseher.

  2.4 Spiele installieren
 -------------------------
Um neue Spiele hinzuzuf�gen, gehen Sie in die Einstellungen und w�hlen Sie den 
Men�punkt "Spiele installieren" auf Seite 2 aus. Sie werden nun aufgefordert, 
ein Spiel einzulegen. Nachdem Sie dies getan haben, best�tigen Sie es durch 
klicken auf Start oder brechen Sie mit dem Zur�ck-Knopf ab. W�hrend WiiFlow das 
Laufwerk initialisiert, haben Sie keine Kontrolle �ber den Cursor. Anschlie�end 
sehen Sie einen Statusbalken und eine Prozentzahl, die Ihnen den Fortschritt 
der Installation anzeigt. Wenn das Spiel fertig installiert wurde, k�nnen Sie 
weitere Spiele hinzuf�gen. Nachdem alle Spiele installiert wurden, sollten Sie 
die H�llen f�r die neu hinzugef�gten Spiele herunterladen.

  2.5 H�llen herunterladen
 -------------------------
In den Einstellungen kann auf der ersten Seite H�llen und Namen herunterladen. 
Dies kann entweder f�r alle installierten Spiele erfolgen oder nur f�r die neu 
hinzugekommenen. Wenn man die H�llen oder Namen manuell angepasst hat, sollte 
man immer nur die fehlenden Daten installieren. Man kann aber auch nach neuen 
Versionen suchen, indem man alle Daten frisch herunterl�d. Dies kann aber bei 
den H�llen je nach Anzahl der installierten Spiele einige Zeit dauern. WiiFlow 
bezieht die H�llen aus dem Internet. Eine Internetverbindung muss unbedingt 
bestehen und WiiFlow muss darauf zugreifen k�nnen. Ansonsten kann es sein, das 
WiiFlow abst�rzt. Die Wii muss in diesem Falle dann neu gestartet werden.

  2.6 Kennwortschutz verwalten
 ------------------------------
Auf der ersten Seite der Einstellungen k�nnen Sie einen Passwortschutz 
einstellen, um das �ndern der Einstellungen, ein versehentliches L�schen eines 
Spieles oder das Anzeigen von ausgew�hlten Spielen zu verhindern. Diese 
Funktion ist auch als "Child-Mode" bekannt und haupts�chlich f�r Kinder oder 
Benutzer gedacht, die sich nicht mit dem Programm auskennen.
Klicken Sie zuerst auf "Code setzen" und suchen Sie sich einen 4-stelligen Code 
aus. WiiFlow ist jetzt gesch�tzt, aber die Sperre sieht man erst nach einem 
Neustart. Wenn Sie ab jetzt �nderungen vornehmen m�chten, m�ssen Sie zuerst den 
4-stelligen Code eingeben um WiiFlow zu entsperren. Danach k�nnen Sie bis zum 
n�chsten Neustart beliebig viele �nderungen vornehmen.
Um den Code zu �ndern, entsperren Sie zuerst WiiFlow wie oben beschrieben und 
geben Sie entweder einen neuen Code ein oder klicken Sie in dem Eingabefenster 
auf L�schen um den Code zu entfernen und die Sperre dauerhaft auszuschalten.

  2.7 Spiele-Einstellungen
 --------------------------
Hier kann man die Einstellungen anpassen, die WiiFlow generell nutzen soll. 
Einige Einstellungen k�nnen sp�ter auch pro Spiel vorgenommen werden. Die 
Einstellungen sind �ber mehrere Seiten verteilt. Wechseln kann man mit dem 
Knopf in der unteren linken Ecke oder mit den Plus- und Minus-Tasten. Zur�ck 
zum Hauptbildschirm kommt man mit dem Zur�ck-Knopf in der unteren rechten Ecke.
Hier nun eine kurze Beschreibung der einzelnen Funktionen:

Video-Modus: Hier kann das ausgew�hlte Spiel gezwungen werden, einen anderen 
  Video-Modus (z.B. NTSC, PAL-50 oder PAL-60) zu verwenden.
Sprache: Das Spiel wird dazu gezwungen, eine andere Sprache zu verwenden.
Ocarina: Aktiviert oder deaktiviert das Schummelmodul f�r dieses Spiel.
Startdatei: Normalerweise wird die main.dol gestartet. Allerdings laden einige 
  Spiele eine andere Startdatei von DVD. Um das Spiel dennoch erfolgreich 
  starten zu k�nnen kann hier eine alternative Startdatei ausgew�hlt werden.
L�nder-Strings patchen: Dies �ndert den L�ndercode in der Spieledatei. Wer eine 
  japanische Wii hat, oder japanische Spiele starten m�chte und Probleme hat, 
  kann diese Einstellung mal versuchen. Die Sprache wird dabei nicht umgestellt.
"Error 002" korrigieren: Einige neuere Spiele erfordern das Aktivieren dieser 
  Option, da sonst beim Starten der Fehler Nr. 002 auftritt.
Vipatch: Dies wird ben�tigt, um den Videomodus innerhalb der Startdatei zu 
  korrigieren. Wenn man Probleme mit der Darstellung von ausl�ndischen Spielen 
  hat und eine Ver�nderung des Video-Modus nichts bringt, kann mit dieser 
  Einstellung vielleicht diese Probleme beheben.
cIOS-Version: Hier kann man ausw�hlen, welches cIOS man f�r dieses Spiel 
  benutzen m�chte. Wenn man aber ein anderes cIOS w�hlt, als das aktuelle, 
  dauert der Start des Spieles ein wenig l�nger.
H�llen aktualisieren: Durch diesen Men�punkt wird die H�lle f�r das ausgew�hlte 
  Spiel erneut versucht herunterzuladen und zu aktualisieren.
IOS-Wechsel blockieren: Einige neuere Spiele erfordern das Aktivieren dieser 
  Option, da diese ein anderes IOS laden m�chten. Es wird aber ein cIOS 
  gebraucht, um auf die Festplatte zugreifen zu k�nnen.
Video-Modi patchen: Dies durchsucht die Startdatei nach Stellen, an denen der 
  Video-Modus ge�ndert wird und korrigiert sie entsprechend der Einstellungen.
  Daf�r muss unter Video-Modus unbedingt PAL-50, PAL-60 oder NTSC eingestellt 
  werden. Bei einigen Spielen funktioniert diese Einstellung nicht.
 -"keinen" ver�ndert nichts an der Startdatei (Kein Video-Modus-Patch).
 -"normal" ersetzt Video-Modi mit der gleichen Aufl�sung, die erwartet wird 
           (gleiche Anzahl an Linien  und  derselbe Modus)
 -"mehr" ersetzt alle Video-Modi, die denselben Modus haben. Die Anzahl der 
         Linien wird ignoriert (Progressive oder Interlaced).
 -"alle" ersetzt alle Video-Modi ohne eine �berpr�fung. Bei dieser Einstellung 
         k�nnen allerdings Fehler w�hrend des Spielablaufs auftreten.



  3.1 Hintergrundmusik einrichten
 ---------------------------------
Wenn man Hintergrundmusik in WiiFlow haben m�chte, kann man ein oder mehrere 
Musikst�cke im Verzeichnis \wiiflow\music\ abspeichern. WiiFlow w�hlt beim 
Start zuf�llig eine Datei aus und spielt diese dann ab. Wenn die Datei zu Ende 
ist, beginnt sie wieder von vorne. WiiFlow spielt erst beim n�chsten Start eine 
andere Datei ab. Ein Neustart kann mit B + Home erzwungen werden.
Die Musik-Dateien m�ssen im OGG-Format gespeichert werden. WiiFlow spielt keine 
mp3-Dateien ab. Es gibt aber kostenlose Konvertierprogramme von mp3 zu ogg.

  3.2 Themenpakete verwalten
 ----------------------------
WiiFlow kann optisch ver�ndert werden, indem man zus�tzliche Themen installiert.
Diese Themen kann man entweder selbst erstellen oder als Paket heruntergeladen. 
Themen werden einfach in das Verzeichnis \wiiflow\themes\ kopiert. Sie bestehen 
aus einer ini-Datei und einem gleichnamigen Verzeichnis, welches die Grafiken 
und Schriftarten enth�lt. Hier ist darauf zu achten, das beide Dateien wirklich 
den gleichen Namen haben (z.B. geh�rt zur Datei \wiiflow\themes\Birne.ini das 
Verzeichnis \wiiflow\themes\Birne\ und dort drin befinden sich die Grafiken).
Wenn das Themenpaket installiert ist, kann es in den Einstellungen ausgew�hlt 
werden. Bitte speichern Sie in das themes-Verzeichnis nur Themenpakete ab.

  3.3 WiiFlow anpassen  (Haupt- und Spiele-Bildschirm)
 ----------------------
In den Optionen kann man auf Seite 2 WiiFlow optisch anpassen. Der Men�punkt 
Coverflow einstellen erlaubt es, die H�llen oder die Kamera zu verschieben. 
Au�erdem kann man dort auch einstellen, wie stark die H�llen rotieren sollen.
Die einzelnen Funktionen sind eigentlich selbsterkl�rend und werden hier nicht 
n�her beschrieben. An der oberen linken Ecke des Bildschirms findet man Kn�pfe, 
mit denen man zwischen den verschiedenen Ansichten wechseln kann. In der Mitte 
kann man zwischen den einzelnen Funktionen umschalten. Oben rechts sind die 
Kn�pfe zum Speichern oder Abbrechen und an der unteren Kante findet man die 
einzelnen Funktionen an sich.
Man kann die Einstellungen kopieren, indem man B gedr�ckt h�lt und 1 dr�ckt. 
Wenn man dann auf eine andere Ansicht wechselt, kann man die kopierten 
Einstellungen durch B und 2 an dieser Stelle einf�gen. Gespeichert werden diese 
�nderungen in der entsprechenden ini-Datei des ausgew�hlten Themas.

  3.4 ini-Datei von WiiFlow anpassen
 ------------------------------------
Sp�testens nach dem Erststart erstellt WiiFlow eine Datei, in welcher alle 
Einstellungen von WiiFlow gespeichert werden. Diese kann am PC ge�ffnet und 
bearbeitet werden. Allerdings zeigt der Windows Standard-Editor Notepad diese 
Datei nicht korrekt an. Man kann sie aber ohne Probleme im Wordpad �ffnen, 
sollte dann allerdings keinerlei Formatierung verwenden. Besser ist aber auf 
jeden Fall, sich einen vern�nftigen Texteditor zuzulegen (z.B. notepad++).
Da die meisten Einstellungen in WiiFlow gemacht werden k�nnen, erkl�re ich hier 
nur die Einstellungen, die ausschlie�lich in der ini-Datei Verwendung finden. 
Alle hier erw�hnten Einstellungen gelten f�r die Rubrik [GENERAL]:

data_on_usb=no : WiiFlow ignoriert eine eventuell vorhandene FAT-Partition auf 
  der externen USB-Festplatte (Wurde unter Installation bereits erkl�rt).
compress_cache=no : Nutzlose zlib-Komrimierung des Cache-Speichers. Ignoriert 
  diese Einstellung einfach und l�scht diese Zeile falls sie vorhanden ist.
cover_buffer=120 : Maximale Anzahl an H�llen, die im Arbeitsspeicher hinterlegt 
  bleiben sollen. 120 ist ein guter Wert f�r 512x512 oder 1024x1024 Pixel/H�lle.
keep_png=yes : WiiFlow arbeitet nur mit den H�llen im Cache-Speicher. Hier kann 
  man einstellen, das die png-Bilder von der SD-Karte gel�scht werden, sobald 
  sie in den Cache-Speicher kopiert wurden.
max_fsaa=5 : Maximales Antialiasing (Weichzeichnen). Dies sollte in jedem Thema 
  begrenzt werden und bei einem Wert h�her als f�nf, ist WiiFlow selten noch 
  fl�ssig darzustellen.
url_flat_covers=... : Hier kann die URL f�r die H�llen eingetragen werden.
url_full_covers=... : Dasselbe wie oben, lediglich f�r komplette H�llen (also 
  H�llen, die Vorder- und R�ckseite haben).

Man kann in hier auch mehrere URL's angeben, indem man sie mit dem Zeichen "|" 
voneinander trennt. WiiFlow bietet auch Platzhalter an. Um die Spiele-ID in die 
URL einzuf�gen, geben Sie entweder {gameid4} oder {gameid6} ein (je nachdem ob 
Sie die ID 4- oder 6-stellig ben�tigen). Der Platzhalter {loc} steht f�r den 
geltenden L�ndercode (wie es bei "wiitdb" verwendet wird) und {region} steht 
f�r die geltende Region (wie es bei "wiiboxart" verwendet wird).


=--------------------------------------------------- (geschrieben von Domi78) -=

