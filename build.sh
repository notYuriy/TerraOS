rm -rf Obj
mkdir Obj
# compiling assembly stuff
nasm -f elf64 Boot/boot.s -o Obj/boot
nasm -f elf64 Boot/multiboot.s -o Obj/multiboot
nasm -f elf64 Asm/asmutils.s -o Obj/asmutils
nasm -f elf64 Asm/inthandlers.s -o Obj/inthandlers
nasm -f elf64 Asm/spinlock.s -o Obj/spinlock
# compiling kernel
for name in Kernel/*.c
do
        obj="$(basename -- $name)"
        ~/opt/cross/bin/x86_64-elf-gcc -o Obj/$obj -c $name -ILib -std=gnu99 -ffreestanding -O2 -Wall -Wextra  -mcmodel=large -mno-sse -mno-sse2 -mno-sse3 -mno-red-zone
done
# linking everything together
ld -n -o Obj/kernel.bin -T Ld/linker.ld Obj/*
# moving os sources to init ramdisk
rm -rf Initrd
mkdir Initrd
#mkdir Initrd/Src
#cp -r Asm Initrd/Src/Asm
#cp -r Lib Initrd/Src/Lib
#cp -r Kernel Initrd/Src/Kernel
#cp -r Boot Initrd/Src/Boot
#cp -r Grub Initrd/Src/Grub
#cp build.sh Initrd/Src/build.sh
#cp test.sh Initrd/Src/build.sh
# compressing init ramdisk to tar file
rm -rf Tar
mkdir Tar
tar -cf Tar/initrd.tar Initrd --format=v7
# creating grub boot fs
rm -rf IsoTree
mkdir IsoTree
mkdir IsoTree/boot
mkdir IsoTree/boot/grub
# copy kernel, grub config and initrd to boot fs
cp Grub/grub.cfg IsoTree/boot/grub/grub.cfg
cp Obj/kernel.bin IsoTree/boot/kernel.bin
cp Tar/initrd.tar IsoTree/fs.initrd
rm -rf Iso
mkdir Iso
# creating iso file
grub-mkrescue IsoTree -o Iso/TerraOS.iso
