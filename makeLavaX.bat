PATH=.\devkitadv\bin;%PATH%

LavaXGba
as -o font.o font.s

gcc -O3 -c main.c lcd.c hardware.c lava.c file.c py2gb.c
as -o arm.o arm.s

gcc -o main.elf main.o lcd.o lava.o hardware.o file.o py2gb.o arm.o font.o

objcopy -v -O binary main.elf LavaX.bin

del main.bin main.elf *.o font.s

pause

