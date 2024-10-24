# Sylent-X

## Overview

Sylent-X is a cross-platform application designed to run on both macOS and Windows. This guide provides detailed instructions on how to build the application on both operating systems.

## Build Instructions

### Build on macOS

1. **Build the application:**
    ```sh
    i686-w64-mingw32-windres resource.rc resource.o && \
    i686-w64-mingw32-g++ -Ilibs/imgui -o bin/Sylent-X.exe Sylent-X.cpp libs/imgui/imgui.cpp libs/imgui/imgui_draw.cpp libs/imgui/imgui_widgets.cpp libs/imgui/imgui_tables.cpp resource.o -Iincludes -mwindows -lurlmon -lwininet -ld3d9 -ldwmapi -static && wine bin/Sylent-X.exe
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
    3. Create a [`bin`](command:_github.copilot.openRelativePath?%5B%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2FUsers%2Fjoshua2504%2FProjekte%2FSylent-X%2Fbin%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%2C%222b759d69-5410-4d40-a098-c12a87cc9835%22%5D "/Users/joshua2504/Projekte/Sylent-X/bin") folder.
    4. Run the build command:
        ```sh
        windres resource.rc resource.o && g++ -o bin/Sylent-X.exe Sylent-X.cpp libs/imgui/imgui.cpp libs/imgui/imgui_draw.cpp libs/imgui/imgui_widgets.cpp libs/imgui/imgui_tables.cpp resource.o -Iincludes -Ilibs/imgui -mwindows -lurlmon -lwininet -ld3d9 -ldwmapi -static
        ```

## Update Process

To update the application, place the new file on the server and update `latest_version.txt` accordingly. Example `latest_version.txt`:

```
0.1.1
https://cor-forum.de/regnum/sylent/Sylent-X-0.1.1.exe
```

## License

Include licensing information here.

## Contributing

Include guidelines for contributing here.

## Contact

Provide contact information or links to relevant resources here.