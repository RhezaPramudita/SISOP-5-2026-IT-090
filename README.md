LAPRES PRAKTIKUM MODUL 5

RHEZA PRAMUDITA ADI PUTRA
5027251090

SOAL 2

---

## kernel.asm

```asm
.global _putInMemory
.global _getChar
```

Dua baris ini mendeklarasikan fungsi `_putInMemory` dan `_getChar` sebagai simbol global agar bisa dipanggil dari file C (`kernel.c`) melalui proses linking. Tanpa deklarasi `.global` ini, linker tidak akan bisa menemukan fungsi-fungsi tersebut saat menghubungkan `kernel.o` dan `kernel_asm.o`.

```asm
_putInMemory:
    push bp
    mov bp, sp
    
    push ds
    push si

    mov ax, [bp+4]    ! Argumen 1: segment
    mov si, [bp+6]    ! Argumen 2: address
    mov cl, [bp+8]    ! Argumen 3: character

    mov ds, ax
    mov [si], cl

    pop si
    pop ds
    pop bp
    ret
```

Fungsi `_putInMemory` bertugas menulis satu karakter ke alamat memori tertentu secara langsung, yang digunakan untuk menampilkan teks ke layar melalui Video RAM (VRAM) pada segment `0xB800`. Sebelum mengakses argumen, register `bp` disimpan ke stack lalu diisi dengan nilai `sp` (stack pointer saat ini) — ini adalah pola standar stack frame pada arsitektur x86 16-bit. Register `ds` dan `si` juga disimpan karena akan dimodifikasi. Argumen diambil dari stack: `[bp+4]` adalah segment, `[bp+6]` adalah address (offset dalam segment), dan `[bp+8]` adalah karakter yang akan ditulis. `mov ds, ax` mengatur segment register ke segment tujuan, lalu `mov [si], cl` menulis karakter ke alamat memori tersebut. Setelah selesai, semua register yang disimpan dikembalikan dan fungsi kembali dengan `ret`.

```asm
_getChar:
    push bp
    mov bp, sp

    mov ah, #0x00
    int 0x16

    mov ah, #0x0E
    mov bh, #0x00
    mov bl, #0x07
    int 0x10

    mov ah, #0x00
    pop bp
    ret
```

Fungsi `_getChar` bertugas membaca satu karakter dari keyboard dan menampilkannya (echo) ke layar. Setelah stack frame disiapkan, `int 0x16` dengan `ah = 0x00` adalah BIOS interrupt untuk membaca keystroke — program akan berhenti (blocking) menunggu pengguna menekan tombol, dan karakter ASCII-nya akan tersimpan di register `AL`. Setelah karakter dibaca, `int 0x10` dengan `ah = 0x0E` adalah BIOS interrupt untuk menampilkan karakter tersebut ke layar (teletype output). `bh = 0x00` menentukan page number, dan `bl = 0x07` adalah atribut warna teks default (abu-abu terang di atas latar hitam). Sebelum `ret`, `ah` dikosongkan dengan `mov ah, #0x00` agar nilai kembalian di register `AX` murni hanya berisi karakter ASCII dari `AL`.

---

## kernel.c

```c
int cursor = 0;
int color = 0x07;

extern char getChar();
extern void putInMemory(int segment, int address, char character);
```

`cursor` menyimpan posisi penulisan saat ini di VRAM dalam satuan byte. Setiap karakter di VRAM mengambil 2 byte: satu untuk karakter ASCII dan satu untuk atributnya (warna), sehingga `cursor` selalu bertambah 2 setiap kali satu karakter ditulis. `color` menyimpan byte atribut warna teks yang berlaku saat ini, dengan nilai default `0x07` yang berarti teks abu-abu terang di atas latar hitam. Kata kunci `extern` memberitahu compiler bahwa `getChar` dan `putInMemory` diimplementasikan di luar file ini (yaitu di `kernel.asm`) dan akan dihubungkan saat proses linking.

```c
void printChar(char c) {
    putInMemory(0xB800, cursor, c);
    cursor = cursor + 1;
    putInMemory(0xB800, cursor, color);
    cursor = cursor + 1;
}
```

Fungsi ini menulis satu karakter ke VRAM. Segment `0xB800` adalah alamat standar Video RAM pada mode teks x86. Pertama ditulis karakter ASCII-nya di offset `cursor`, lalu byte warna (`color`) ditulis di offset berikutnya. Setelah dua penulisan ini, `cursor` telah bertambah 2, siap untuk karakter berikutnya.

```c
void printString(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        printChar(str[i]);
        i = i + 1;
    }
}
```

Fungsi ini mencetak string ke layar dengan cara iterasi karakter demi karakter hingga menemukan null terminator `'\0'`. Setiap karakter diteruskan ke `printChar` untuk ditulis ke VRAM.

```c
int bagi(int pembilang, int penyebut) {
    int hasil = 0;
    if (penyebut == 0) return 0;
    while (pembilang >= penyebut) {
        pembilang = pembilang - penyebut;
        hasil = hasil + 1;
    }
    return hasil;
}

int modulo(int pembilang, int penyebut) {
    if (penyebut == 0) return 0;
    while (pembilang >= penyebut) {
        pembilang = pembilang - penyebut;
    }
    return pembilang;
}
```

Karena kernel ini berjalan dalam mode real 16-bit tanpa dukungan library standar C maupun instruksi pembagian hardware yang aman, operasi pembagian dan modulo diimplementasikan secara manual menggunakan pengurangan berulang. `bagi` menghitung berapa kali `penyebut` bisa dikurangkan dari `pembilang`, sedangkan `modulo` mengembalikan sisa pengurangan tersebut. Keduanya menjaga diri dari pembagian dengan nol dengan pengecekan di awal.

```c
void newline() {
    int baris_sekarang = bagi(cursor, 160);
    cursor = (baris_sekarang + 1) * 160;
}
```

Setiap baris pada layar teks 80 kolom mengambil `80 karakter × 2 byte = 160 byte` di VRAM. Fungsi ini menghitung baris saat ini dengan membagi `cursor` dengan 160, lalu melompat `cursor` ke awal baris berikutnya.

```c
void readString(char *buf) {
    int i = 0;
    char c;
    while (1) {
        c = getChar();
        if (c == 0x0D || c == 0x0A || c == '\r' || c == '\n') {
            buf[i] = '\0';
            break;
        }
        else if (c == 0x08 || c == '\b') {
            if (i > 0) {
                i = i - 1;
                cursor = cursor - 2;
                printChar(' ');
                cursor = cursor - 2;
            }
        }
        else if (c >= 32 && c <= 126) {
            if (i < 63) {
                buf[i] = c;
                printChar(c);
                i = i + 1;
            }
        }
    }
}
```

Fungsi ini membaca input pengguna karakter demi karakter dan menyimpannya ke buffer. Tiga jenis karakter ditangani: Enter (`0x0D`/`0x0A`) mengakhiri input dengan menempatkan null terminator; Backspace (`0x08`) menghapus karakter terakhir dengan cara memundurkan `cursor` dua langkah, menulis spasi untuk menghapus tampilan, lalu memundurkan `cursor` lagi; dan karakter printable (ASCII 32–126) disimpan ke buffer dan langsung ditampilkan ke layar. Batas 63 karakter dijaga agar buffer tidak overflow.

```c
int strcmp(char *str1, char *str2) {
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) return 0;
        i = i + 1;
    }
    if (str1[i] == '\0' && str2[i] == '\0') return 1;
    return 0;
}
```

Implementasi `strcmp` khusus kernel ini mengembalikan `1` jika dua string identik dan `0` jika tidak — kebalikan dari konvensi `strcmp` standar C. Fungsi membandingkan karakter per karakter dan mengembalikan `0` segera saat ada perbedaan. Hanya jika kedua string habis pada saat yang sama (keduanya `'\0'`) maka dianggap sama.

```c
int stringToInt(char *str) {
    int hasil = 0;
    int i = 0;
    while (str[i] >= '0' && str[i] <= '9') {
        hasil = (hasil * 10) + (str[i] - '0');
        i = i + 1;
    }
    return hasil;
}
```

Fungsi ini mengonversi string digit menjadi integer menggunakan metode Horner: setiap iterasi mengalikan hasil sebelumnya dengan 10 lalu menambahkan nilai digit karakter saat ini. Konversi karakter ke angka dilakukan dengan mengurangkan kode ASCII `'0'` (48) dari karakter.

```c
void printInt(int angka) {
    char buf[7];
    int i = 0, j;
    if (angka == 0) { printChar('0'); return; }
    if (angka < 0) { printChar('-'); angka = -angka; }
    while (angka > 0) {
        buf[i] = modulo(angka, 10) + '0';
        angka = bagi(angka, 10);
        i = i + 1;
    }
    for (j = i - 1; j >= 0; j = j - 1) {
        printChar(buf[j]);
    }
}
```

Fungsi ini mencetak integer ke layar. Karena digit dihasilkan dari belakang (digit paling kecil dulu), digit-digit disimpan sementara di `buf` lalu dicetak dalam urutan terbalik. Kasus khusus untuk `0` dan angka negatif ditangani di awal.

```c
void clearScreen() {
    int i;
    cursor = 0;
    for (i = 0; i < 2000; i = i + 1) {
        printChar(' ');
    }
    cursor = 0;
}
```

Layar teks standar berukuran 80×25 = 2000 karakter. Fungsi ini menimpa seluruh VRAM dengan karakter spasi menggunakan `printChar`, lalu mereset `cursor` ke posisi awal `0`.

```c
void parseDuaAngka(char *cmd, int *a, int *b) {
    int i = 0, idx1 = 0, idx2 = 0;
    char s1[16], s2[16];

    while (cmd[i] != ' ' && cmd[i] != '\0') i = i + 1;
    if (cmd[i] == ' ') i = i + 1;

    while (cmd[i] != ' ' && cmd[i] != '\0') { s1[idx1] = cmd[i]; idx1++; i++; }
    s1[idx1] = '\0';

    if (cmd[i] == ' ') i = i + 1;

    while (cmd[i] != '\0') { s2[idx2] = cmd[i]; idx2++; i++; }
    s2[idx2] = '\0';

    *a = stringToInt(s1);
    *b = stringToInt(s2);
}
```

Fungsi parser ini memisahkan dua angka dari string perintah seperti `"add 10 5"`. Pertama ia melompati token perintah (kata pertama) hingga menemukan spasi, lalu mengambil token kedua (angka pertama) ke `s1`, dan token ketiga (angka kedua) ke `s2`. Hasil konversi ke integer disimpan melalui pointer `*a` dan `*b`.

```c
void handleFac(char *cmd) {
    ...
    if (hasil > bagi(32767, i)) {
        printString("know your limit little bro.");
        newline();
        return;
    }
    hasil = hasil * i;
    ...
}
```

Fungsi ini menghitung faktorial dari angka `n` yang diambil dari perintah. Karena kernel beroperasi dalam mode 16-bit dengan batas integer signed `32767`, dilakukan pengecekan overflow sebelum setiap perkalian: jika `hasil > 32767 / i` maka `hasil * i` pasti melebihi batas, sehingga program menampilkan pesan `"know your limit little bro."` dan berhenti. Untuk `n = 0` atau `n = 1`, hasil langsung dikembalikan sebagai `1`.

```c
void handleSeason(char *cmd) {
    ...
    if (strcmp(tema, "winter"))       { color = 0x09; printString("Ubah warna ke Winter."); }
    else if (strcmp(tema, "spring"))  { color = 0x0A; printString("Ubah warna ke Spring."); }
    else if (strcmp(tema, "summer"))  { color = 0x0E; printString("Ubah warna ke Summer."); }
    else if (strcmp(tema, "fall"))    { color = 0x06; printString("Ubah warna ke Fall."); }
    else if (strcmp(tema, "radiant")) { color = 0x0D; printString("Ubah warna ke Radiant."); }
    else { printString("Tema season tidak dikenal!"); }
    ...
}
```

Perintah `season` mengubah variabel global `color` yang menentukan atribut warna teks pada seluruh tampilan berikutnya. Setiap tema memetakan ke kode atribut warna VRAM yang berbeda: `0x09` biru terang (winter), `0x0A` hijau terang (spring), `0x0E` kuning (summer), `0x06` coklat/oranye redup (fall), dan `0x0D` magenta terang (radiant).

```c
void handleTriangle(char *cmd) {
    ...
    for (i = 1; i <= tinggi; i = i + 1) {
        for (j = 1; j <= i; j = j + 1) {
            printChar('*');
        }
        newline();
    }
}
```

Perintah `triangle` mencetak segitiga siku-siku dari karakter `*` dengan tinggi `n` baris. Baris pertama mencetak 1 bintang, baris kedua 2 bintang, dan seterusnya hingga baris ke-`n` mencetak `n` bintang. Pola ini dihasilkan oleh nested loop di mana loop luar mengontrol baris dan loop dalam mengontrol jumlah bintang per baris.

```c
void main() {
    char cmd[64];
    int a, b;

    clearScreen();
    printString("Welcome to Season OS!");
    newline();
    printString("Type 'help' for commands list.");
    newline();
    newline();

    while (1) {
        printString("> ");
        readString(cmd);
        newline();

        if (cmd[0] == 'a' && cmd[1] == 'd' && cmd[2] == 'd') { ... }
        else if (cmd[0] == 's' && cmd[1] == 'u' && cmd[2] == 'b') { ... }
        else if (cmd[0] == 'f' && cmd[1] == 'a' && cmd[2] == 'c') { handleFac(cmd); }
        else if (cmd[0] == 's' && cmd[1] == 'e' && cmd[2] == 'a' && cmd[3] == 's') { handleSeason(cmd); }
        else if (cmd[0] == 't' && cmd[1] == 'r' && cmd[2] == 'i') { handleTriangle(cmd); }
        else if (strcmp(cmd, "check")) { printString("Sistem berjalan dengan baik."); newline(); }
        else if (strcmp(cmd, "clear")) { clearScreen(); }
        else if (strcmp(cmd, "help")) { ... }
        else { printString("Command tidak dikenal!"); newline(); }

        newline();
    }
}
```

Ini adalah shell loop utama kernel. Setelah layar dibersihkan dan pesan selamat datang ditampilkan, kernel masuk ke loop tak terbatas `while(1)` yang menampilkan prompt `"> "`, membaca input pengguna dengan `readString`, lalu memeriksa perintah apa yang dimasukkan. Pengecekan perintah dilakukan karakter demi karakter (bukan dengan `strcmp` penuh) untuk perintah multi-kata seperti `add`, `sub`, `fac`, `season`, dan `triangle`, karena perintah-perintah ini diikuti argumen. Perintah satu kata seperti `check`, `clear`, dan `help` menggunakan `strcmp`. Jika tidak ada yang cocok, ditampilkan pesan error.

---

## bootloader.asm

```asm
bits 16
org 0x7C00
```

`bits 16` memberitahu NASM bahwa kode ini dikompilasi untuk mode 16-bit real mode, yang merupakan mode CPU saat komputer baru booting. `org 0x7C00` adalah alamat awal di mana BIOS memuat bootloader — semua referensi alamat dalam kode dihitung relatif dari titik ini.

```asm
KERNEL_SEGMENT equ 0x1000
KERNEL_SECTORS equ 15
```

Dua konstanta ini mendefinisikan tujuan pemuatan kernel. `KERNEL_SEGMENT = 0x1000` berarti kernel akan dimuat ke alamat fisik `0x10000` (segment × 16). `KERNEL_SECTORS = 15` adalah jumlah sektor floppy yang dibaca untuk memuat kernel ke memori.

```asm
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
```

Bagian inisialisasi ini menyiapkan environment bootloader. `cli` menonaktifkan interrupt sementara agar tidak terjadi gangguan selama setup. Semua segment register (`ds`, `es`, `ss`) dikosongkan ke `0` dengan menggunakan `xor ax, ax` sebagai sumbernya. Stack pointer `sp` diset ke `0x7C00` karena stack tumbuh ke bawah dan lokasi bootloader sendiri ada di `0x7C00`. Setelah selesai, `sti` mengaktifkan kembali interrupt.

```asm
    mov ax, KERNEL_SEGMENT
    mov es, ax
    xor bx, bx

    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00

    int 0x13
    jc disk_error
```

Bagian ini memuat kernel dari floppy disk ke memori menggunakan BIOS interrupt `0x13` fungsi `0x02` (Read Sectors). Tujuan pembacaan adalah `ES:BX = 0x1000:0x0000`. Parameter pembacaan: `AL = 15` sektor, `CH = 0` (cylinder 0), `CL = 2` (mulai dari sektor 2 karena sektor 1 adalah bootloader itu sendiri), `DH = 0` (head 0). Register `DL` dibiarkan apa adanya karena BIOS sudah mengisi nomor drive boot di sana. Jika terjadi error (carry flag set), program melompat ke `disk_error`.

```asm
    cli
    mov ax, KERNEL_SEGMENT
    mov ds, ax
    mov es, ax

    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xFFFF
    mov bp, 0xFFFF
    sti

    push word KERNEL_SEGMENT
    push word 0x0000
    retf
```

Setelah kernel berhasil dimuat, segment register diperbarui ke `KERNEL_SEGMENT` agar cocok dengan lokasi kernel. Stack dipindahkan ke `0x9000:0xFFFF` yang merupakan area aman jauh dari kernel dan bootloader. Far jump ke kernel dilakukan dengan teknik `push + retf`: dengan mendorong segment `0x1000` dan offset `0x0000` ke stack lalu mengeksekusi `retf`, CPU akan melompat ke `0x1000:0x0000` — tepat di awal kernel yang baru dimuat.

```asm
disk_error:
    mov si, msg
.print:
    lodsb
    or al, al
    jz $
    mov ah, 0x0E
    mov bh, 0x00
    int 0x10
    jmp .print

msg db 'DISK ERROR',0
times 510-($-$$) db 0
dw 0xAA55
```

Jika disk gagal dibaca, program mencetak pesan `"DISK ERROR"` ke layar menggunakan BIOS interrupt `0x10`. `lodsb` memuat karakter dari `[SI]` ke `AL` dan menginkremen `SI`. Loop berhenti saat `AL = 0` (null terminator). `times 510-($-$$) db 0` mengisi sisa bootloader dengan byte nol hingga tepat 510 byte, lalu `dw 0xAA55` adalah magic number yang wajib ada di dua byte terakhir sektor boot agar BIOS mengenalinya sebagai bootloader yang valid.

---

## Makefile & build.sh

```makefile
prepare:
    dd if=/dev/zero of=floppy.img bs=512 count=2880

bootloader:
    nasm -f bin bootloader.asm -o bootloader.bin
    dd if=bootloader.bin of=floppy.img bs=512 count=1 conv=notrunc

kernel:
    nasm -f as86 kernel.asm -o kernel-asm.o
    bcc -ansi -c kernel.c -o kernel.o
    ld86 -o kernel.bin -d kernel.o kernel-asm.o
    dd if=kernel.bin of=floppy.img bs=512 seek=1 conv=notrunc

build: prepare bootloader kernel

run:
    bochs -f bochsrc.txt
```

`prepare` membuat file image floppy kosong berukuran `512 × 2880 = 1.44MB` (ukuran standar floppy 1.44MB) menggunakan `dd`. `bootloader` mengompilasi `bootloader.asm` menjadi binary flat dengan NASM, lalu menyalinnya ke sektor pertama image floppy. `kernel` mengompilasi `kernel.asm` dengan assembler `as86`, mengompilasi `kernel.c` dengan compiler C 16-bit `bcc`, lalu menghubungkan keduanya dengan `ld86` menjadi `kernel.bin`. Kernel kemudian ditulis mulai dari sektor kedua (`seek=1`) agar tidak menimpa bootloader. `run` menjalankan emulator Bochs dengan konfigurasi dari `bochsrc.txt`.

`build.sh` adalah skrip alternatif untuk pengguna macOS yang melakukan langkah-langkah yang sama secara berurutan dalam satu skrip bash karena toolchain tersebut mungkin tidak tersedia langsung via `make` di macOS.

---

## bochsrc.txt

```
megs: 32
floppya: 1_44=floppy.img, status=inserted
boot: floppy
log: bochsout.txt
mouse: enabled=0
```

File konfigurasi Bochs ini mendefinisikan spesifikasi mesin virtual: RAM sebesar 32MB, floppy drive A berisi `floppy.img` dengan ukuran 1.44MB dalam keadaan terpasang, urutan boot dari floppy, log output ditulis ke `bochsout.txt`, dan mouse dinonaktifkan karena kernel tidak mengimplementasikan driver mouse.

---

## Tree

```
soal_2/
├── bootloader.asm
├── bochsrc.txt
├── build.sh
├── kernel.asm
├── kernel.c
└── Makefile
```
