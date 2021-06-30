# ATools - FlyForFun resources editor

## Overview

ATools is a little software suite that I developed in order to replace Aeonsoft's tools:
* ModelEditor -> Cola
* SfxEditor -> SfxEditor
* GUIEditor -> Daisy
* GUIEditor_v19 -> (no equivalent)
* WorldEditor-> Beast
* ResEditor -> (no equivalent) 

What my tools have in addition:
* More beautiful and convenient interface
* More stable
* Less bugs
* Better rendering (antialiasing, Z-Buffer with better quality)
* Totally in french, english and german
* Custom keyboards shortcuts
* Lots of new features added
* The source code is available so you can fit all you need

## Screenshots

### WorldEditor

![alt tag](http://i.imgur.com/KNX9gu1.png)

![alt tag](http://i.imgur.com/Qb6jrPN.png)

### ModelEditor

![alt tag](http://i.imgur.com/AOc5cSx.png)

### SfxEditor

![alt tag](http://i.imgur.com/VTfc9Ow.png)

### GUIEditor (v15 and v19)

![alt tag](http://i.imgur.com/HZ3ZuUt.png)

### ResEditor

![alt tag](http://i.imgur.com/uTa5UJw.png)

## Build
    
To compile you will need:
* Visual Studio 2013 (not the express edition)
* DirectX SDK : http://www.microsoft.com/en-us/download/details.aspx?id=6812
* Visual Studio Add-in 1.2.4 for Qt5 : http://download.qt.io/official_releases/vsaddin/qt-vs-addin-1.2.4-opensource.exe
* Qt 5.4.1 for Windows 32 bits - VS 2013 : http://download.qt.io/official_releases/qt/5.4/5.4.1/qt-opensource-windows-x86-msvc2013-5.4.1.exe

You must also:
* Extract the file "Lib/Assimp/lib.7z"
* Copy all Qt's DLLs and your resources to the "Binary" folder.
* Install and configure Qt and his Add-in in order to have this in Visual Studio:

![alt tag](http://i.imgur.com/MAGPOjo.png)
