#!/bin/bash

echo "Membuat berkas imej floppy..."
dd if=/dev/zero of=floppy.img bs=512 count=2880

echo "Kompilasi Bootloader..."
nasm -f bin bootloader.asm -o bootloader.bin
dd if=bootloader.bin of=floppy.img bs=512 count=1 conv=notrunc

echo "Kompilasi Kernel C dan Assembly secara Lokal..."
# Mengompilasi C dengan arsitektur 16-bit 8086 menggunakan bcc lokal
bcc -ansi -c -o kernel.o kernel.c
# Mengompilasi file assembly pendukung menggunakan as86 lokal
as86 kernel.asm -o kernel_asm.o
# Melakukan linking object file menjadi format binary kernel akhir
ld86 -o kernel -d kernel.o kernel_asm.o

echo "Memasukkan Kernel ke dalam Floppy Image..."
dd if=kernel of=floppy.img bs=512 seek=1 conv=notrunc

echo "Proses Build Selesai! Berkas floppy.img siap digunakan."
