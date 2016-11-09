# PORTSETTER - A PROGRAM FOR SETTING PORTS

## Installation

__IMPORTANT:__

To compile porttester.cpp, do `g++-5 -std=c++11 portsetter.cpp -o portsetter.cpp.o`

If you don't use __g++ -5__, the RegEx will not compile.

1. Copy this repository to Cloud9 or your favorite Linux machine.
2. Compile portsetter.cpp and porttester.cpp.
3. To add setport as a command in the terminal, run install.sh (./install.sh).
    * If it says "bash: ./install.sh: Permission denied", then do chmod +x install.sh and run install.sh again.
4. You are now free to use setport in the terminal as much as you please, from any directory!


## Uninstallation

1. To remove setport from the commands in the terminal, run uninstall.sh.
    * If it says "bash: ./uninstall.sh: Permission denied", then do "chmod +x uninstall.sh" and run uninstall.sh again.

## How to use
1. Use setport like any other command on the command line. Set ports all day.
2. Portsetter can run in spanish. It will automatically read in the settings from environmental variables LANGUAGE, 
  LC_ALL, LC_MESSAGES, and LANG, in that order. If available, that language will be used.
3. For further instructions, type setport -? into the command line.
4. To add new languages, add .txt files to the language_files/ directory. Follow the naming conventions and you'll be fine.


## HOW TO RUN TESTS
1. porttester.cpp is the testing program. It depends on setport being added as a command to the terminal, so run install.sh before running this program.
2. porttester.cpp also depends on some text files existing in the language_files/ directory. So if you delete those or change their location, the test program won't work. Portsetter also won't work, so don't do that.
3. Compile porttester.cpp, and run ./porttester.cpp.o. It will run through some tests, display some output, and return 0 if the portsetter program passed everything and 1 otherwise.


## What's new in version 1.02?
* New flags you can use!
    * -?
    * -!, --about
    * -v, --version
    * -e (to be used with -p or --port)
    * For more information, use setport -h
* External files for portsetter usage, and information about the developer. Easily forward them to your friends and family.
* An abundance of new tests in porttester.cpp.

## What's new in version 1.03?
* Support for languages besides English, including an easy framework for adding new languages
* Free Spanish translation packs.

## What's new in version 1.04?
* Hundreds of new comments
* Mild bug fixes, including properly checking the environmental variables and allowing setport to run from any directory.

## What's new in version 1.05?
* Added --environment flag as an alias for -e
* Made the usage message slightly prettier
* Added a new developer whose unvetted changes may or may not violate any third-party software licenses

Portsetter is now on GitHub! Come check it out!