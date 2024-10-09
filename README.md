## Build on macOS

```
i686-w64-mingw32-g++ -o Sylent-X-0.1.50.exe Sylent-X.cpp imgui/imgui_impl_dx9.cpp imgui/imgui_impl
_win32.cpp imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp imgui/imgui_tables.cpp -mwindows -lurlmon -lwininet -ld3d9 -l
dwmapi -static
```

The first part of the command will compile the resource file which will then be included in the program file.

You can also add ``&& wine Sylent-X-0.1.0.exe`` at the end while debugging.

## Build on Windows

1. Install https://www.msys2.org/
2. Open MSYS2 MINGW64 Shell
3. Run ``pacman -Syu`` to update all packages
4. After the terminal restart, run command ``pacman -Syu`` again
5. Install MinGW-w64 GCC toolchain for compiling 64-bit Windows applications
``pacman -S mingw-w64-x86_64-toolchain``

Now
1. Navigate to C:\ using ``cd /c``
2. Navigate to the project directory
3. Run the build command:
```
g++ -o Sylent-X.exe Sylent-X.cpp -mwindows -lurlmon -static -v
```


## Update process

Place the new file on the server and update ``latest_version.txt`` accordingly. Example ``latest_version.txt``:

```
0.1.1
https://cor-forum.de/regnum/sylent/Sylent-X-0.1.1.exe
```
