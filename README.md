# desktop-character
A program built in SDL3 which displays a character moving around on your desktop and has an editor to configure the displayed character's animation.

This type of program is also known as a "desktop pet" or "shimeji" program.


# Running
Releases of the program are located on https://fractured-sky.itch.io/desktop-character.

For Windows, simply extract the files and run "out.exe" to start the main program.

For Linux, SDL3 and SDL3_image need to be installed onto the system through the system's package manager. Then, extract the files and run "out" to start the main program.

Further information can also be found in the provided README.txt file.


# Custom configurations
The configuration loaded on start is "config.charconf" in the current folder.

Currently, the configuration editor is only provided for Linux and can be ran by running "editor" in the extracted files.

On Windows, the provided spritesheets under example/ can be replaced with other 32x32 spritesheets as a temporary workaround before a Windows version of the editor can be built.


# Building
The main program under src/ was built using SDL3 and SDL3_image with the main loop located in src/main.c.

The editor program under src/config_editor was built using Raylib and Raygui with the main loop located in src/config_editor/main.c.

If any of these libraries are not installed on the system, they can be placed under thirdparty/ which will be included when building.

GCC and Make are used to build the project into the build/ directory.

Main program:
```sh
make release
```

Editor program:
```sh
make editor_release
```
