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
        eval "$CC -g -o Obj/$obj -c $name -IInclude -std=gnu99 -ffreestanding -Og -Wall -Wextra -mno-sse -mno-sse2 -mno-sse3 -mcmodel=large -mno-red-zone -funroll-loops"
done
# linking everything together
ld -n -o Obj/kernel.bin -T Ld/linker.ld Obj/*
# compressing init ramdisk to tar file
rm -rf Disk
mkdir Disk
./ramdisk-util Initrd Disk/initrd.img
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
qemu-system-x86_64 -cdrom Iso/TerraOS.iso -m 128M -no-reboot -s -curses