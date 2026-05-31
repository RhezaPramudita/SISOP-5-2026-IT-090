! kernel.asm
! Menggunakan sintaksis as86 (komentar menggunakan tanda seru)

.global _putInMemory
.global _getChar

_putInMemory:
    push bp
    mov bp, sp
    
    push ds
    push si

    ! Mengambil argumen dari stack sesuai konvensi C bcc
    mov ax, [bp+4]    ! Argumen 1: segment
    mov si, [bp+6]    ! Argumen 2: address
    mov cl, [bp+8]    ! Argumen 3: character

    mov ds, ax        ! Set Segment Register
    mov [si], cl      ! Pindahkan karakter ke memori alamat tujuan

    pop si
    pop ds
    pop bp
    ret

_getChar:
    push bp
    mov bp, sp

    ! 1. Membaca input satu karakter dari keyboard via BIOS Int 16h
    mov ah, #0x00
    int 0x16          ! Karakter ASCII otomatis masuk ke register AL

    ! 2. Menampilkan (echo) karakter tersebut ke layar via BIOS Int 10h
    mov ah, #0x0E
    mov bh, #0x00     ! Page number 0
    mov bl, #0x07     ! Warna teks default
    int 0x10

    ! 3. Mengembalikan nilai karakter ke fungsi C (dikembalikan via register AX)
    mov ah, #0x00     ! Bersihkan AH agar nilai murni ASCII berada di AX
    pop bp
    ret
