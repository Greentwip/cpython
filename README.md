# CPyton
Xbox One Python 3.4 port, this fork is directed towards game developers, but other uses might as well be possible. It can be used as a UWP framework for Microsoft Store applications, but the functionality is untested and some features such as os.walk or os.path.exists won't work for Windows (although they work for Xbox One)

We have ported Pygame with most of what's needed to run a complete game, so you'll know what to do if you have a game written in Python using that library.

To get started, clone this repo and init submodules, you can use a Powershell terminal or use chdir instead of cd

```
git clone https://github.com/greentwip/cpython
cd cpython
git submodule init
git submodule update
```

All the magic is inside the UWP folder, there you will find the UWPApp.sln solution file, you will need Visual Studio 2019 with C++ and UWP apps support enabled to open the projects.

The contents of the solution file are the Pygame extensions, SDL dependencies, code for Lex-Talionis, a fan game emulating all the features Fire Emblem for GameBoy Advance has, there are also embedded samples of Pygame running for Xbox One and a standalone Python interpreter. Those are located inside the Experiments solution folder (Interpreter and Pygame subfolders).

You will only need to setup your Xbox One as a development unit, there is information for that on the Microsoft Developer portals.

To run Lex-Talionis you either disable audio or download the audio files and place them in the Audio folder, see Lex-Talionis/Assets/Lex-Talionis/Code/Engine.py and look for NO_AUDIO = False, change it to True to make it run without music.

The audio files can be downloaded from [here](https://www.dropbox.com/sh/slbz2t7v1fc6uao/AACiznGLm442qcdOAGbQtnmwa?dl=1)