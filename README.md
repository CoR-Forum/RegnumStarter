## Build on macOS

```
x86_64-w64-mingw32-g++ -o Sylent-X-0.1.0.exe Sylent-X.cpp -mwindows -lurlmon -static
```

You can also add ``&& wine Sylent-X-0.1.0.exe`` at the end while debugging.

## Update process

Place the new file on the server and update ``latest_version.txt`` accordingly. Example ``latest_version.txt``:

```
0.1.1
https://cor-forum.de/regnum/sylent/Sylent-X-0.1.1.exe
```
