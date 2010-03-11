

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
2.5) Hüllen herunterladen
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
eine SD-Karte zum Betrieb benötigt. Diese sollte immer eingelegt bleiben und 
darf nicht schreibgeschützt sein. Wenn sie nicht eingelegt ist, funktioniert 
WiiFlow eventuell nicht richtig, zeigt keine Hüllen an und spielt die Musik 
nicht ab. Eventuell ist ein Start ohne die SD-Karte gar nicht möglich.
Für die Festplatteninstallation wird eine Festplatte mit einer FAT32- und einer 
WBFS-Partition benötigt. Eine eventuell schon vorhandene reine WBFS-Festplatte 
müsste gegebenenfalls neu partitioniert werden, was ein Verlust aller bereits 
installierten Spiele bedeuten würde. Ich erkläre nun beide Installationen.

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
Partitionen ist die WBFS-Partition und enthält alle Wii-Spiele. Die andere muss 
eine FAT16- oder FAT32-Partition sein und enthält die Daten für WiiFlow.
 (Wenn Sie bereits eine Festplatte haben, die nur eine WBFS-Partition besitzt,
  müssen Sie diese neu partitionieren. Dabei gehen alle Spiele verloren).
Kopieren Sie den Inhalt des Paketes in das Hauptverzeichnis Ihrer FAT32-
Partition auf der Festplatte. Dort sollte sich dann folgender Pfad befinden:
USB:\wiiflow\language.ini
WiiFlow wird alle Einstellungen in das Verzeichnis USB:\wiiflow\ speichern.
Ausnahme: Wenn Sie eine SD-Karte mit dem Eintrag "data_on_usb=no" in der Datei 
SD:\wiiflow\wiiflow.ini einlegen, werden die Daten auf die SD-Karte gespeichert.

  1.4 Starten von WiiFlow
 -------------------------
Um WiiFlow zu starten, führen sie einfach die beigefügte Datei "boot.dol" aus.
Diese Datei kann von überall aus gestartet werden. WiiFlow arbeitet alleine in 
dem Verzeichnis, in dem es installiert wurde (SD:\wiiflow  oder  USB:\wiiflow).
Dennoch erkläre ich hier kurz ein paar Möglichkeiten WiiFlow zu starten:
1.4.1) Starten von WiiFlow über den Homebrew-Kanal
Speichern Sie die Datei "boot.dol" in dem Verzeichnis SD:\apps\wiiflow\boot.dol 
damit der Homebrew-Kanal WiiFlow finden kann. Die Dateien meta.xml und icon.png 
gehören auch in dieses Verzeichnis, sind aber nicht zum starten notwendig.
1.4.2) Starten von WiiFlow über einen WiiFlow-Kanal
Wenn Sie einen WiiFlow-Kanal haben (keinen Forwarder-Kanal), ist die "boot.dol" 
bereits in dem Kanal integriert und wird von dort auch gestartet.
Der Kanal wird normalerweise über einen WAD-Manager installiert.
1.4.3) Starten von WiiFlow über einen Forwarder-Kanal
Ein Forwarder ist ein Kanal, der eine bestimmte Datei von einer bestimmten 
Stelle aus startet. Die Datei "boot.dol" muss also eventuell umbenannt und an 
diese Stelle gebracht werden. Näheres erfahren Sie vom Autor des Forwarders.
1.4.4) Starten von WiiFlow über Preloader
Benennen Sie die Datei "boot.dol" in "WiiFlow.dol" um und speichern Sie diese 
in das Hauptverzeichnis Ihrer SD-Karte. Starten Sie anschließend den Preloader 
und wählen Sie dort "Load/Install File" und dann die "WiiFlow.dol" aus. Nach 
kurzer Zeit ist die Datei installiert und in den "Settings" kann man nun den 
Preloader so einstellen, dass nach dem Einschalten automatisch WiiFlow startet.



  2.1 Bedienung im Hauptbildschirm
 ----------------------------------
Nach dem Starten von WiiFlow erscheint direkt der Hauptbildschirm. (Dieser wird 
auch als Coverflow bezeichnet.) Um ein Spiel hervorzuheben, drückt man entweder 
die Richtungstasten der Wii Fernbedienung oder zeigt mit ihr auf den linken 
oder rechten Rand des Bildschirms. Dort kann man dann auf die Pfeile klicken, 
um weiterzublättern. Wenn man schneller blättern möchte, kann durch drücken der 
Plus- und Minus-Tasten bzw. durch gedrückt halten von B und den Plus- und 
Minus-Tasten entweder Seitenweise oder zum nächsten Buchstaben blättern. 
Diese beiden Funktionen können in den Einstellungen getauscht werden.
Um ein Spiel auszuwählen, zeigt man einfach auf die entsprechende Hülle und 
drückt auf A. Wenn der Zeiger sich außerhalb des Bildschirms befindet, wird 
das Spiel ausgewählt, dessen Hülle hervorgehoben ist. Man kann auch ein Spiel 
sofort starten, indem man B gedrückt hält, mit der Fernbedienung auf die 
entsprechende Hülle zeigt und dann A drückt.
Wenn man den Zeiger an den unteren Rand bewegt, erscheinen dort vier Symbole. 
Links ist das erste für Allgemeine Informationen und das zweite für die WiiFlow-
Einstellungen. Das in der Mitte ist für die Favoriten und das rechte ist zum 
beenden von WiiFlow. Die Reihenfolge kann allerdings je nach Thema variieren.
Zum Beenden klickt man entweder das Symbol an oder drückt auf die Home-Taste 
der Wii-Fernbedienung.  Hält man B gedrückt und drückt auf die Home-Taste, 
startet WiiFlow neu. Mit den Tasten 1 und 2 kann man zwischen unterschiedlich 
angeordneten Bildschirmen hin- und herschalten.

  2.2 Bedienung im Spiele-Bildschirm
 -----------------------------------
Wenn ein Spiel ausgewählt wurde, wird die ausgewählte Hülle groß dargestellt. 
Die Hülle kann gedreht werden, indem man darauf zeigt und A drückt. Durch 
drücken von B oder auf den Knopf Zurück kommt man zurück zum Hauptbildschirm. 
Das Spiel wird gestartet, indem man entweder auf Start klickt oder auf A wenn 
sich der Zeiger außerhalb des Bildschirms befindet. Auch hier kann mit den 
Richtungstasten Links und Rechts zum nächsten Spiel gewechselt werden. Mit den 
Plus- und Minus- Tasten ist die vertikale Bewegung möglich. Mit den 
Richtungstasten Oben und Unten kann man die Knöpfe auswählen.
Über die vier Schaltflächen kann man Spiel-spezifische Einstellungen vornehmen 
(näheres unter Punkt 2.7), es zu den Favoriten hinzufügen oder davon entfernen, 
es vor unerlaubtem Starten schützen sowie den Schutz aufheben und es dauerhaft 
von der Festplatte löschen. Diese vier Funktionen können blockiert werden indem 
man einen Kennwortschutz einstellt (dies wird unter Punkt 2.6 erklärt).

  2.3 WiiFlow Einstellungen
 ---------------------------
Hier kann man die Einstellungen anpassen, die WiiFlow generell nutzen soll. 
Einige Einstellungen können später auch pro Spiel vorgenommen werden. Die 
Einstellungen sind über mehrere Seiten verteilt. Wechseln kann man mit dem 
Knopf in der unteren linken Ecke oder mit den Plus- und Minus-Tasten. Zurück 
zum Hauptbildschirm kommt man mit dem Zurück-Knopf in der unteren rechten Ecke.
Hier nun eine kurze Beschreibung der einzelnen Funktionen:

Hüllen und Namen: Hier können die Hüllen für die Spiele heruntergeladen werden.
  Außerdem kann man hier eine Titel-Datenbank herunterladen, um die Spiele mit 
  den richtigen Namen angezeigt zu bekommen (mehr unter 2.5).
Passwortschutz: WiiFlow verfügt über eine Passwort-Funktion. Wenn diese aktiv 
  ist, wird verhindert, das geschützte Spiele angezeigt, etwas an den 
  Einstellungen verändert oder Spiele gelöscht werden (mehr unter 2.6).
3D-Hüllen: Die Hüllen können auch nur als Bild angezeigt werden.
Vibration: Aktiviert oder deaktiviert die Vibration in der Wii-Fernbedienung.
Thema: Hier kann das Thema ausgewählt werden, das angezeigt werden soll. Man 
  kann mehrere Themen installieren und hier eines aussuchen (mehr unter 3.2).
Sprache: Ursprünglich ist hier die englische Sprache eingestellt. Für Deutsch 
  sollte man diesen Wert auf GERMAN umstellen.
Coverflow einstellen: Hier kann die Position der Hüllen und deren Verhalten 
  eingestellt werden. Diese Unterfunktion ist sehr komplex und ausschließlich 
  in Englisch vorhanden. Es in dieser Anleitung zu erklären würde sie sprengen.
Spiel installieren: Dieses Unterprogramm von WiiFlow kopiert die Wii-Spiele von 
  DVD-ROM oder DVD-R auf die angeschlossene USB-Festplatte (mehr unter 2.4).
Zurück zum Wii-Menü: Hier stellt man das Verhalten des Home-Knopfes im Haupt-
  bildschirm und von der Wii-Fernbedienung ein. Wenn diese Funktion aktiv ist, 
  kehrt WiiFlow immer zurück zum Wii-Menü. Ansonsten geht WiiFlow zurück zu dem 
  Programm, von dem aus es gestartet wurde (z.B. den Homebrew-Kanal).
Favoriten-Modus merken: Durch aktivieren dieser Funktion zeigt WiiFlow beim 
  starten nur die Favoriten an, falls sie beim Beenden auch aktiv waren. Da man 
  nur eine Handvoll Spiele auf einmal spielt, kann dies sehr nützlich sein.
Standard Such-Modus: Hier wird das Verhalten der Plus- und Minus-Tasten im 
  Hauptbildschirm eingestellt. WiiFlow springt dann entweder zum nächsten 
  Buchstaben des Alphabets oder blättert eine Seite weiter.
Lautstärke: Hier wird eingestellt wie laut die entsprechenden Töne sein sollen: 
 -Hintergrundmusik: Die Musik, die unter \wiiflow\music\ gespeichert wurde.
 -WiiFlow-Tasten:   Die Geräusche, die von den Knöpfen her kommen.
 -Hüllen-Bewegung:  Geräusche von Hüllen beim Auswählen. (Themen-bedingt)
 -Spiele-Melodie:   Die Musik des ausgewählten Spieles, welche normalerweise 
                    vom Disc-Kanal abgespielt wird.
  Ist eine Lautstärke auf 0 eingestellt, wird sie gar nicht gespielt.
Standard Video-Modus: Hier kann ein Spiel gezwungen werden, einen anderen 
  Video-Modus (z.B. NTSC, PAL-50 oder PAL-60) zu verwenden. Diese Einstellung 
  kann auch für einzelne Spiele separat eingestellt werden (mehr unter 2.7).
Standard Spiel-Sprache: Hier kann man ein Spiel dazu zwingen, eine andere 
  Sprache zu verwenden. Auch diese Einstellung kann für einzelne Spiele separat 
  eingestellt werden (mehr unter 2.7).
"Error 002" korrigieren: Einige neuere Spiele erfordern das Aktivieren dieser 
  Option, da sonst beim Starten der Fehler Nr. 002 auftritt. Ältere Spiele 
  stört diese Option nicht. Dennoch kann auch diese separat eingestellt werden.
Ocarina: Aktiviert oder deaktiviert das integrierte Schummelmodul von WiiFlow.
TV-Breite/Höhe einstellen: Hier kann man die Auflösung des Fernsehers anpassen.
Horizontaler/Vertikaler Versatz: Verschiebt das Bild auf dem Fernseher.

  2.4 Spiele installieren
 -------------------------
Um neue Spiele hinzuzufügen, gehen Sie in die Einstellungen und wählen Sie den 
Menüpunkt "Spiele installieren" auf Seite 2 aus. Sie werden nun aufgefordert, 
ein Spiel einzulegen. Nachdem Sie dies getan haben, bestätigen Sie es durch 
klicken auf Start oder brechen Sie mit dem Zurück-Knopf ab. Während WiiFlow das 
Laufwerk initialisiert, haben Sie keine Kontrolle über den Cursor. Anschließend 
sehen Sie einen Statusbalken und eine Prozentzahl, die Ihnen den Fortschritt 
der Installation anzeigt. Wenn das Spiel fertig installiert wurde, können Sie 
weitere Spiele hinzufügen. Nachdem alle Spiele installiert wurden, sollten Sie 
die Hüllen für die neu hinzugefügten Spiele herunterladen.

  2.5 Hüllen herunterladen
 -------------------------
In den Einstellungen kann auf der ersten Seite Hüllen und Namen herunterladen. 
Dies kann entweder für alle installierten Spiele erfolgen oder nur für die neu 
hinzugekommenen. Wenn man die Hüllen oder Namen manuell angepasst hat, sollte 
man immer nur die fehlenden Daten installieren. Man kann aber auch nach neuen 
Versionen suchen, indem man alle Daten frisch herunterläd. Dies kann aber bei 
den Hüllen je nach Anzahl der installierten Spiele einige Zeit dauern. WiiFlow 
bezieht die Hüllen aus dem Internet. Eine Internetverbindung muss unbedingt 
bestehen und WiiFlow muss darauf zugreifen können. Ansonsten kann es sein, das 
WiiFlow abstürzt. Die Wii muss in diesem Falle dann neu gestartet werden.

  2.6 Kennwortschutz verwalten
 ------------------------------
Auf der ersten Seite der Einstellungen können Sie einen Passwortschutz 
einstellen, um das Ändern der Einstellungen, ein versehentliches Löschen eines 
Spieles oder das Anzeigen von ausgewählten Spielen zu verhindern. Diese 
Funktion ist auch als "Child-Mode" bekannt und hauptsächlich für Kinder oder 
Benutzer gedacht, die sich nicht mit dem Programm auskennen.
Klicken Sie zuerst auf "Code setzen" und suchen Sie sich einen 4-stelligen Code 
aus. WiiFlow ist jetzt geschützt, aber die Sperre sieht man erst nach einem 
Neustart. Wenn Sie ab jetzt Änderungen vornehmen möchten, müssen Sie zuerst den 
4-stelligen Code eingeben um WiiFlow zu entsperren. Danach können Sie bis zum 
nächsten Neustart beliebig viele Änderungen vornehmen.
Um den Code zu ändern, entsperren Sie zuerst WiiFlow wie oben beschrieben und 
geben Sie entweder einen neuen Code ein oder klicken Sie in dem Eingabefenster 
auf Löschen um den Code zu entfernen und die Sperre dauerhaft auszuschalten.

  2.7 Spiele-Einstellungen
 --------------------------
Hier kann man die Einstellungen anpassen, die WiiFlow generell nutzen soll. 
Einige Einstellungen können später auch pro Spiel vorgenommen werden. Die 
Einstellungen sind über mehrere Seiten verteilt. Wechseln kann man mit dem 
Knopf in der unteren linken Ecke oder mit den Plus- und Minus-Tasten. Zurück 
zum Hauptbildschirm kommt man mit dem Zurück-Knopf in der unteren rechten Ecke.
Hier nun eine kurze Beschreibung der einzelnen Funktionen:

Video-Modus: Hier kann das ausgewählte Spiel gezwungen werden, einen anderen 
  Video-Modus (z.B. NTSC, PAL-50 oder PAL-60) zu verwenden.
Sprache: Das Spiel wird dazu gezwungen, eine andere Sprache zu verwenden.
Ocarina: Aktiviert oder deaktiviert das Schummelmodul für dieses Spiel.
Startdatei: Normalerweise wird die main.dol gestartet. Allerdings laden einige 
  Spiele eine andere Startdatei von DVD. Um das Spiel dennoch erfolgreich 
  starten zu können kann hier eine alternative Startdatei ausgewählt werden.
Länder-Strings patchen: Dies ändert den Ländercode in der Spieledatei. Wer eine 
  japanische Wii hat, oder japanische Spiele starten möchte und Probleme hat, 
  kann diese Einstellung mal versuchen. Die Sprache wird dabei nicht umgestellt.
"Error 002" korrigieren: Einige neuere Spiele erfordern das Aktivieren dieser 
  Option, da sonst beim Starten der Fehler Nr. 002 auftritt.
Vipatch: Dies wird benötigt, um den Videomodus innerhalb der Startdatei zu 
  korrigieren. Wenn man Probleme mit der Darstellung von ausländischen Spielen 
  hat und eine Veränderung des Video-Modus nichts bringt, kann mit dieser 
  Einstellung vielleicht diese Probleme beheben.
cIOS-Version: Hier kann man auswählen, welches cIOS man für dieses Spiel 
  benutzen möchte. Wenn man aber ein anderes cIOS wählt, als das aktuelle, 
  dauert der Start des Spieles ein wenig länger.
Hüllen aktualisieren: Durch diesen Menüpunkt wird die Hülle für das ausgewählte 
  Spiel erneut versucht herunterzuladen und zu aktualisieren.
IOS-Wechsel blockieren: Einige neuere Spiele erfordern das Aktivieren dieser 
  Option, da diese ein anderes IOS laden möchten. Es wird aber ein cIOS 
  gebraucht, um auf die Festplatte zugreifen zu können.
Video-Modi patchen: Dies durchsucht die Startdatei nach Stellen, an denen der 
  Video-Modus geändert wird und korrigiert sie entsprechend der Einstellungen.
  Dafür muss unter Video-Modus unbedingt PAL-50, PAL-60 oder NTSC eingestellt 
  werden. Bei einigen Spielen funktioniert diese Einstellung nicht.
 -"keinen" verändert nichts an der Startdatei (Kein Video-Modus-Patch).
 -"normal" ersetzt Video-Modi mit der gleichen Auflösung, die erwartet wird 
           (gleiche Anzahl an Linien  und  derselbe Modus)
 -"mehr" ersetzt alle Video-Modi, die denselben Modus haben. Die Anzahl der 
         Linien wird ignoriert (Progressive oder Interlaced).
 -"alle" ersetzt alle Video-Modi ohne eine Überprüfung. Bei dieser Einstellung 
         können allerdings Fehler während des Spielablaufs auftreten.



  3.1 Hintergrundmusik einrichten
 ---------------------------------
Wenn man Hintergrundmusik in WiiFlow haben möchte, kann man ein oder mehrere 
Musikstücke im Verzeichnis \wiiflow\music\ abspeichern. WiiFlow wählt beim 
Start zufällig eine Datei aus und spielt diese dann ab. Wenn die Datei zu Ende 
ist, beginnt sie wieder von vorne. WiiFlow spielt erst beim nächsten Start eine 
andere Datei ab. Ein Neustart kann mit B + Home erzwungen werden.
Die Musik-Dateien müssen im OGG-Format gespeichert werden. WiiFlow spielt keine 
mp3-Dateien ab. Es gibt aber kostenlose Konvertierprogramme von mp3 zu ogg.

  3.2 Themenpakete verwalten
 ----------------------------
WiiFlow kann optisch verändert werden, indem man zusätzliche Themen installiert.
Diese Themen kann man entweder selbst erstellen oder als Paket heruntergeladen. 
Themen werden einfach in das Verzeichnis \wiiflow\themes\ kopiert. Sie bestehen 
aus einer ini-Datei und einem gleichnamigen Verzeichnis, welches die Grafiken 
und Schriftarten enthält. Hier ist darauf zu achten, das beide Dateien wirklich 
den gleichen Namen haben (z.B. gehört zur Datei \wiiflow\themes\Birne.ini das 
Verzeichnis \wiiflow\themes\Birne\ und dort drin befinden sich die Grafiken).
Wenn das Themenpaket installiert ist, kann es in den Einstellungen ausgewählt 
werden. Bitte speichern Sie in das themes-Verzeichnis nur Themenpakete ab.

  3.3 WiiFlow anpassen  (Haupt- und Spiele-Bildschirm)
 ----------------------
In den Optionen kann man auf Seite 2 WiiFlow optisch anpassen. Der Menüpunkt 
Coverflow einstellen erlaubt es, die Hüllen oder die Kamera zu verschieben. 
Außerdem kann man dort auch einstellen, wie stark die Hüllen rotieren sollen.
Die einzelnen Funktionen sind eigentlich selbsterklärend und werden hier nicht 
näher beschrieben. An der oberen linken Ecke des Bildschirms findet man Knöpfe, 
mit denen man zwischen den verschiedenen Ansichten wechseln kann. In der Mitte 
kann man zwischen den einzelnen Funktionen umschalten. Oben rechts sind die 
Knöpfe zum Speichern oder Abbrechen und an der unteren Kante findet man die 
einzelnen Funktionen an sich.
Man kann die Einstellungen kopieren, indem man B gedrückt hält und 1 drückt. 
Wenn man dann auf eine andere Ansicht wechselt, kann man die kopierten 
Einstellungen durch B und 2 an dieser Stelle einfügen. Gespeichert werden diese 
Änderungen in der entsprechenden ini-Datei des ausgewählten Themas.

  3.4 ini-Datei von WiiFlow anpassen
 ------------------------------------
Spätestens nach dem Erststart erstellt WiiFlow eine Datei, in welcher alle 
Einstellungen von WiiFlow gespeichert werden. Diese kann am PC geöffnet und 
bearbeitet werden. Allerdings zeigt der Windows Standard-Editor Notepad diese 
Datei nicht korrekt an. Man kann sie aber ohne Probleme im Wordpad öffnen, 
sollte dann allerdings keinerlei Formatierung verwenden. Besser ist aber auf 
jeden Fall, sich einen vernünftigen Texteditor zuzulegen (z.B. notepad++).
Da die meisten Einstellungen in WiiFlow gemacht werden können, erkläre ich hier 
nur die Einstellungen, die ausschließlich in der ini-Datei Verwendung finden. 
Alle hier erwähnten Einstellungen gelten für die Rubrik [GENERAL]:

data_on_usb=no : WiiFlow ignoriert eine eventuell vorhandene FAT-Partition auf 
  der externen USB-Festplatte (Wurde unter Installation bereits erklärt).
compress_cache=no : Nutzlose zlib-Komrimierung des Cache-Speichers. Ignoriert 
  diese Einstellung einfach und löscht diese Zeile falls sie vorhanden ist.
cover_buffer=120 : Maximale Anzahl an Hüllen, die im Arbeitsspeicher hinterlegt 
  bleiben sollen. 120 ist ein guter Wert für 512x512 oder 1024x1024 Pixel/Hülle.
keep_png=yes : WiiFlow arbeitet nur mit den Hüllen im Cache-Speicher. Hier kann 
  man einstellen, das die png-Bilder von der SD-Karte gelöscht werden, sobald 
  sie in den Cache-Speicher kopiert wurden.
max_fsaa=5 : Maximales Antialiasing (Weichzeichnen). Dies sollte in jedem Thema 
  begrenzt werden und bei einem Wert höher als fünf, ist WiiFlow selten noch 
  flüssig darzustellen.
url_flat_covers=... : Hier kann die URL für die Hüllen eingetragen werden.
url_full_covers=... : Dasselbe wie oben, lediglich für komplette Hüllen (also 
  Hüllen, die Vorder- und Rückseite haben).

Man kann in hier auch mehrere URL's angeben, indem man sie mit dem Zeichen "|" 
voneinander trennt. WiiFlow bietet auch Platzhalter an. Um die Spiele-ID in die 
URL einzufügen, geben Sie entweder {gameid4} oder {gameid6} ein (je nachdem ob 
Sie die ID 4- oder 6-stellig benötigen). Der Platzhalter {loc} steht für den 
geltenden Ländercode (wie es bei "wiitdb" verwendet wird) und {region} steht 
für die geltende Region (wie es bei "wiiboxart" verwendet wird).


=--------------------------------------------------- (geschrieben von Domi78) -=

