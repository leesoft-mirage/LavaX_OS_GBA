PATH=.\devkitadv\bin;%PATH%

Larm 中国麻将GBA.lav -o 中国麻将GBA.s

as -o lav.o 中国麻将GBA.s

gcc -o main.elf lav.o

objcopy -v -O binary main.elf 中国麻将.exe

del main.elf lav.o 中国麻将GBA.s

pause

