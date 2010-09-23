

    WiiFlow  Manual


  Content
 =========

1.1) Installation from WiiFlow
1.2) Installation from WiiFlow on SD-Card
1.3) Installation from WiiFlow on Harddisk
1.4) Starting WiiFlow

2.1) Handling the Main-Screen "Coverflow"
2.2) Handling the Game-Select-Screen
2.3) WiiFlow Settings
2.4) Installing Games
2.5) Download Covers
2.6) Password-Administration
2.7) Game-Settings

3.1) Playing background-music
3.2) Administrating Themes
3.3) WiiFlow Setup (Main- and Game-Select-Screen)
3.4) Setting up the ini-File



  1.1 Installation of WiiFlow
 -------------------------------
You can either install WiiFlow to a SD-Card or on your Harddisk. Both types has 
advantages and disadvantages: If you install on a SD-Card you will need the 
SD-Card to operate with WiiFlow. You should always let the SD-Card inside your 
Wii and should not write-protect it. Otherwise WiiFlow will not work properly. 
It is better to install WiiFlow on your Harddisk but you will need a FAT
partition for it and a second WBFS partition for your Wii-Games. If you have a 
WBFS only Harddisk, you need to repartition it. That will delete all of your 
Wii-Games on it. So I'll explain both types now:

  1.2 Installation of WiiFlow on SD-Card
 ------------------------------------------
Just copy the content of the archive to the Root of your SD-Card. You should 
get the following Path:
SD:\wiiflow\language.ini
WiiFlow will save all settings and files into the directory SD:\wiiflow\.
Exception: If you connect a harddisk with a FAT- and a WBFS-Partition, WiiFlow 
will save its files to the FAT-Partition. To avoid this, you should open the 
file SD:\wiiflow\wiiflow.ini and add the entry "data_on_usb=no" in the category 
[GENERAL]. WiiFlow will ignore FAT-Partitions then. If wiiflow.ini isn't there, 
copy the one that has been created on the HDD.

  1.3 Installation of WiiFlow on Harddisk
 -------------------------------------------
It is necessary to split your Harddisk into at least two partitions to install 
WiiFlow on it. One is the WBFS-partition which contains all Wii-Games 
and the other is a FAT16- or FAT32-partition and contains the WiiFlow-data.
 (If you already have a single WBFS-partition on your harddisk, you need to 
  reformat it into two parts. You will lose all installed games then).
If you got this, you can just copy the content of the archive to your external 
harddisk. You should get the following path:
USB:\wiiflow\language.ini
WiiFlow will save all settings and files into the directory USB:\wiiflow\.
Exception: If you put a SD-Card with the entry "data_on_usb=no" in wiiflow.ini 
into your Wii, WiiFlow will use the SD-Folder instead of your harddisk.

  1.4 Starting WiiFlow
 ----------------------
You can start WiiFlow by executing the boot.dol from the archive. WiiFlow will 
start from everywhere. It doesn't care about the current directory. It will 
work inside the directory you've installed it  (SD:\wiiflow  or  USB:\wiiflow).
But I will explain some options to start WiiFlow:
1.4.1) Starting WiiFlow via Homebrew-Channel
You need to save the file "boot.dol" into the directory SD:\apps\wiiflow\ that 
the Homebrew-Channel can find it. The files meta.xml and icon.png belongs to 
that directory too, but they are not necessary for the execution.
1.4.2) Starting WiiFlow through a WiiFlow-Channel
If you have a WiiFlow-Channel (not a forwarder) the boot.dol is already inside 
it and will be launched there. There is no need to save it anywhere else. 
You can install such a channel with help of a WAD-Manager.
1.4.3) Starting WiiFlow through a Forwarder-Channel
A forwarder is a channel which executes a specific file in a specific directory. 
So you may need to rename the boot.dol and place it on that directory where the 
forwarder can find it. Ask the author of the forwarder which file it will start.
1.4.4) Starting WiiFlow with help of Preloader
Rename boot.dol to WiiFlow.dol and save it inside the root of your SD-Card. Now 
start Preloader and select "Load/Install File" and then the "WiiFlow.dol". 
After a short time the file is installed and you can automatically start 
WiiFlow by powering on your Wii if you set this in the settings of Preloader.



  2.1 - 2.7   Not translated  (sorry)
 ----------------------------
!!!  I've skipped the translation of these points because WiiFlow should be  !!!
!!!  self-explaining. I will hand this in later to avoid a delayed release.  !!!



  3.1 Playing background-music
 ------------------------------
If you want to have some music in WiiFlow, you can save one or more audio 
files inside of \wiiflow\music\. WiiFlow will randomly play one of these 
files each time you start it. If the file is played to the end of it, it 
will start over again. If you want WiiFlow to play another file, you need to 
do a restart by holding the B-button and pressing Home. WiiFlow will only play 
OGG-files and is not compatible with mp3's but there are free converters.

  3.2 Installing Themes
 ---------------------------
You can change the look of WiiFlow by installing an additional theme. You can 
either download a theme or create one by yourself. To install a theme just copy 
it inside the directory \wiiflow\themes\. Themes consist of a ini-file and a 
belonging directory, which contains the graphics and fonts. You only have to 
check if both has the same name (e.g. to the file "\wiiflow\themes\pear.ini" 
belongs the directory "\wiiflow\themes\pear\" and all files inside of it). 
After you have copied both things, you can choose this theme in the settings. 

  3.3 WiiFlow Setup  (Main- and Game-Select-Screen)
 -------------------
This function was made to help theme designers, so it might look a little 
complicated.
You can customize WiiFlow by clicking on "Adjust Coverflow" in the Settings on 
page two. You can move the covers, the camera and much more with this.
You can even setup the oscillation of the covers in there. I will not explain 
the various functions here. Just try it by yourself. You will find some buttons 
in the upper left edge. These are for changing the actual view. On the upper 
border you can switch between the various functions and on the upper right edge 
are the buttons for saving or exit without saving your changes. The settings
themselves are on the bottom of the screen.
You can copy all by holding down the B-Button and pressing 1. Now you can 
switch to another view, hold down the B-Button again and press 2 to "paste".
Your changes will be saved inside of the ini-file that belongs to the theme.
While you're adjusting a parameter, press B to speed up (so press B+A instead 
of just A when you click the button)

  3.4 Setting up the ini-File
 -----------------------------
WiiFlow will create the file \wiiflow\wiiflow.ini at its first run. It will 
save all settings in there. You can open and edit these files with your PC but 
Windows default editor "notepad" can not display them correctly. So use 
either Windows Wordpad (and avoid any text-formattings) or any better text 
editor like notepad++. I will only explain the settings you can not find inside 
WiiFlow here. All settings have to be inside the category [GENERAL]:

data_on_usb=no : WiiFlow will ignore a FAT-Partition on your Harddisk and 
  use the SD-Card instead (as explained under installation).
compress_cache=no : This is a useless zlib-compression of the covers cache. 
  Just ignore this setting and leave it to "no".
cover_buffer=120 : Maximum number of covers stored in memory. 120 is a good 
  value for 512x512 pixels or 1024x1024 pixels per cover
keep_png=yes : WiiFlow is only working with the covers inside the cache 
  directory. Here you can force WiiFlow to delete the png file after a 
  cache file has been created.
max_fsaa=5 : Maximum antialiasing (Here: 5x). This should be limited in each 
  theme and a value higher than five is rarely fast enough for WiiFlow. So 
  the antiliasing applied is the minimum between this value and the one found 
  in the theme's description.
url_flat_covers=... : Here you can enter the URL for the front covers.
url_full_covers=... : Same as above, but for full covers (front- and backside).

You can enter more URLs here by separating them with the char "|". WiiFlow uses 
variables you can use to specify the required cover. Use either {gameid4} or 
{gameid6} to indicate the Game-ID (with 4 or 6 letters). Use {loc} for the 
console's country code (like it is used at wiitdb) and {region} for game region 
(like it is used at wiiboxart).


=-------------------------------------------------------- (written by Domi78) -=

