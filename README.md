## WIP Server for MungPlex

### Setup project
- clone the repo
- install DevkitPro
- run `devkitPro/msys2/usr/bin/pacman -Syu --needed wiiu-dev` to install the Wii U toolchain
- run `devkitPro/msys2/usr/bin/pacman cmake` to install cmake with all required dependencies

### Build Instructions
- in terminal navigate to the `build` folder and run `cmake ..`
- then run `make`

### Misc.

Thanks to [Matthew Lopez](https://github.com/MatthewL246) for the hello world sample!
