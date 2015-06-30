# Introduction #

This is a guide which helps you to compile d2x-cios correctly.

# Details #

# Windows #
**REQUIREMENTS**:
  * DevkitPro environment, get it with the [devkitpro installer](http://www.sourceforge.net/projects/devkitpro/files/latest/download).

  * [devkitARM r32](http://mirror.transact.net.au/pub/sourceforge/d/project/de/devkitpro/devkitARM/previous/devkitARM_r32-win32.exe).

  * A Subversion Client like [SlikSVN](http://www.sliksvn.com/en/download) or [TortoiseSVN](http://tortoisesvn.net/downloads.html).

  * Stripios from [here](http://usbloader-gui.googlecode.com/svn/branches/ehcmodule_rodries/stripios/stripios.exe) or compile it by yourself (see the utils folder into svn repository).

  * Gawk from [here](http://gnuwin32.sourceforge.net/packages/gawk.htm).

**PREPARATION**:
At first use the devkitpro installer to install devkitARM and devkitPPC. After installing devkitPro, you will need to install the older devkitARM [r32](https://code.google.com/p/d2x-cios/source/detail?r=32), move the devkitARM\_r32-win32.exe to "C:\devkitpro" delete the devkitARM folder, start the devkitARM\_r32-win32.exe and extract. Then install the subversion client, after installing open a command prompt and type "svn checkout http://d2x-cios.googlecode.com/svn/trunk/ d2x-cios-read-only".
After this you should have a "d2x-cios-read-only" in the directory where you started the command prompt. Then move the "stripios.exe" and the "awk.exe" from "gawk-3.1.6-1-bin.zip\bin" to the "d2x-cios-read-only" folder.

**COMPILE**:
Open a command prompt in "d2x-cios-read-only" and type "maked2x -h" to see the help.
If the source version is for example v6 final type "maked2x 6 final".
Then it should compile d2x v6 final. The compiled files can be found in the "build" folder.


# Linux #
**REQUIREMENTS**:
  * [devkitARM r32 x86](http://mirror.transact.net.au/pub/sourceforge/d/project/de/devkitpro/devkitARM/previous/devkitARM_r32-i686-linux.tar.bz2).

  * Stripios from [here](http://www.multiupload.com/AW7S2SAMJX) or compile it by yourself (see the utils folder into svn repository).

  * Gawk using "sudo apt-get install gawk".

**PREPARATION**:
Create a "devkitpro" folder on your linux drive root and extract the
"devkitARM" folder from the devkitARM archive downloaded into it. Now you need to edit the file ".bashrc", it's in your home folder. Add these lines into it:

export DEVKITPRO=$home/devkitpro

export DEVKITARM=$DEVKITPRO/devkitARM

Save the file.
Open a terminal and type in "source ~/.bashrc" then "svn checkout http://d2x-cios.googlecode.com/svn/trunk/ d2x-cios-read-only". Go into the "d2x-cios-read-only" folder, extract the "Makefile" and "stripios" file into it, and make the "stripios" executable using a terminal and the command "chmod +x stripios".

**COMPILE**:
Open a terminal in the "d2x-cios-read-only" folder and type "maked2x.sh -h" to see the help.
If the source version is for example v6 final type "maked2x.sh 6 final".
Then it should compile d2x v6 final. The compiled files can be found in the "build" folder.