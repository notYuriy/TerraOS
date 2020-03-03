rm -rf Obj
mkdir Obj
declare CC="~/opt/cross/bin/x86_64-elf-gcc"
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
        eval "$CC -o Obj/$obj -c $name -IInclude -std=gnu99 -ffreestanding -O2 -Wall -Wextra -mno-sse -mno-sse2 -mno-sse3 -mcmodel=large -mno-red-zone -funroll-loops"
done
# linking everything together
ld -n -o Obj/kernel.bin -T Ld/linker.ld Obj/*
# creating copy of the source
cp -r Initrd TmpInitrd
mkdir TmpInitrd/Root/src
cp -r Kernel TmpInitrd/Root/src/Kernel
cp -r Asm TmpInitrd/Root/src/Asm
cp -r Include TmpInitrd/Root/src/Include
cp -r Ld TmpInitrd/Root/src/Ld
cp -r Boot TmpInitrd/Root/src/Boot
# creating ramdisk
rm -rf Disk
mkdir Disk
g++ -o ramdisk-util ramdisk.cpp -lstdc++fs
./ramdisk-util TmpInitrd Disk/initrd.img
rm -rf TmpInitrd
# creating grub boot fs
rm -rf IsoTree
mkdir IsoTree
mkdir IsoTree/boot
mkdir IsoTree/boot/grub
# copy kernel, grub config and initrd to boot fs
cp Grub/grub.cfg IsoTree/boot/grub/grub.cfg
cp Obj/kernel.bin IsoTree/boot/kernel.bin
cp Disk/initrd.img IsoTree/initrd.img
rm -rf Iso
mkdir Iso
# creating iso file
grub-mkrescue IsoTree -o Iso/TerraOS.iso
