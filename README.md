# Sylent-X

This tool requires the [Sylent-X-API](https://github.com/Sylent-X/Sylent-X-API).

## Overview

Sylent-X is an application designed to run on Windows, although development is also possible on macOS.

This guide provides detailed instructions on how to build the application on both operating systems.

## Branches

* **main** - Latest stable version
* **dev** - Current development branch

## Build Instructions

### Build on macOS

1. **Build the application:**
    ```sh
    i686-w64-mingw32-windres resource.rc resource.o && \
    i686-w64-mingw32-g++ -Ilibs/imgui -o bin/Sylent-X.exe Sylent-X.cpp libs/imgui/imgui.cpp libs/imgui/imgui_draw.cpp libs/imgui/imgui_widgets.cpp libs/imgui/imgui_tables.cpp resource.o -lurlmon -lwininet -ld3d9 -ldwmapi -static -mwindows && wine bin/Sylent-X.exe
    ```

    The first part of the command compiles the resource file, which is then included in the program file.

    **Note:** You can also add `&& wine Sylent-X.exe` at the end while debugging.

### Build on Windows

1. **Install MSYS2:**
    - Download and install from [MSYS2](https://www.msys2.org/).

2. **Open MSYS2 MINGW64 Shell.**

3. **Update all packages:**
    ```sh
    pacman -Syu
    ```
    After the terminal restarts, run the command again:
    ```sh
    pacman -Syu
    ```

4. **Install MinGW-w64 GCC toolchain:**
    ```sh
    pacman -S mingw-w64-x86_64-toolchain
    ```

5. **Build the application:**
    1. Navigate to `C:\`:
        ```sh
        cd /c
        ```
    2. Navigate to the project directory.
    3. Create a `bin` folder.
    4. Run the build command:
        ```sh
        windres resource.rc resource.o && g++ -o bin/Sylent-X.exe Sylent-X.cpp libs/imgui/imgui.cpp libs/imgui/imgui_draw.cpp libs/imgui/imgui_widgets.cpp libs/imgui/imgui_tables.cpp resource.o -lurlmon -lwininet -ld3d9 -ldwmapi -static -mwindows && ./bin/Sylent-X.exe
        ```

## Update Process

To update the application, place the new file on the server and update `latest_version.txt` accordingly. Example `latest_version.txt`:

```
0.1.1
https://cor-forum.de/regnum/sylent/Sylent-X-0.1.1.exe
```