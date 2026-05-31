int cursor = 0;
int color = 0x07; // Warna teks abu-abu terang, latar hitam default

extern char getChar();
extern void putInMemory(int segment, int address, char character);

// ==========================================
// FUNGSI UTILITAS BAWAAN KERNEL
// ==========================================

void printChar(char c) {
    putInMemory(0xB800, cursor, c);
    cursor = cursor + 1;
    putInMemory(0xB800, cursor, color);
    cursor = cursor + 1;
}

void printString(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        printChar(str[i]);
        i = i + 1;
    }
}

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

void newline() {
    int baris_sekarang = bagi(cursor, 160);
    cursor = (baris_sekarang + 1) * 160;
}

void readString(char *buf) {
    int i = 0;
    char c;
    while (1) {
        c = getChar();
        if (c == '\r' || c == '\n') {
            buf[i] = '\0';
            break;
        }
        else if (c == '\b') {
            if (i > 0) {
                i = i - 1;
                cursor = cursor - 2;
                printChar(' ');
                cursor = cursor - 2;
            }
        }
        else {
            if (i < 63) {
                buf[i] = c;
                i = i + 1;
            }
        }
    }
}

int strcmp(char *str1, char *str2) {
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) return 0;
        i = i + 1;
    }
    if (str1[i] == '\0' && str2[i] == '\0') return 1;
    return 0;
}

int stringToInt(char *str) {
    int hasil = 0;
    int i = 0;
    while (str[i] >= '0' && str[i] <= '9') {
        hasil = (hasil * 10) + (str[i] - '0');
        i = i + 1;
    }
    return hasil;
}

void printInt(int angka) {
    char buf[7];
    int i = 0;
    int j;
    if (angka == 0) {
        printChar('0');
        return;
    }
    if (angka < 0) {
        printChar('-');
        angka = -angka;
    }
    while (angka > 0) {
        buf[i] = modulo(angka, 10) + '0';
        angka = bagi(angka, 10);
        i = i + 1;
    }
    for (j = i - 1; j >= 0; j = j - 1) {
        printChar(buf[j]);
    }
}

// ==========================================
// IMPLEMENTASI KELOMPOK FITUR UTAMA
// ==========================================

void clearScreen() {
    int i;
    cursor = 0;
    for (i = 0; i < 2000; i = i + 1) {
        printChar(' ');
    }
    cursor = 0;
}

void parseDuaAngka(char *cmd, int *a, int *b) {
    int i = 0, idx1 = 0, idx2 = 0;
    char s1[16], s2[16];

    while (cmd[i] != ' ' && cmd[i] != '\0') i = i + 1;
    if (cmd[i] == ' ') i = i + 1;

    while (cmd[i] != ' ' && cmd[i] != '\0') {
        s1[idx1] = cmd[i];
        idx1 = idx1 + 1;
        i = i + 1;
    }
    s1[idx1] = '\0';

    if (cmd[i] == ' ') i = i + 1;

    while (cmd[i] != '\0') {
        s2[idx2] = cmd[i];
        idx2 = idx2 + 1;
        i = i + 1;
    }
    s2[idx2] = '\0';

    *a = stringToInt(s1);
    *b = stringToInt(s2);
}

void handleFac(char *cmd) {
    int i = 0, n, idx = 0;
    char s[16];
    int hasil = 1; 

    while (cmd[i] != ' ' && cmd[i] != '\0') i = i + 1;
    if (cmd[i] == ' ') i = i + 1;

    while (cmd[i] != '\0') {
        s[idx] = cmd[i];
        idx = idx + 1;
        i = i + 1;
    }
    s[idx] = '\0';
    n = stringToInt(s);

    // Kasus dasar untuk 0! atau 1!
    if (n == 0 || n == 1) {
        printInt(1);
        newline();
        return;
    }

    for (i = 2; i <= n; i = i + 1) {
        // Pengecekan overflow sebelum perkalian dilakukan:
        // Jika (hasil > 32767 / i), maka hasil * i pasti melebihi limit 16-bit signed integer
        if (hasil > bagi(32767, i)) { 
            printString("know your limit little bro.");
            newline();
            return;
        }
        hasil = hasil * i;
    }
    printInt(hasil);
    newline();
}

void handleSeason(char *cmd) {
    int i = 0, idx = 0;
    char tema[16];

    while (cmd[i] != ' ' && cmd[i] != '\0') i = i + 1;
    if (cmd[i] == ' ') i = i + 1;

    while (cmd[i] != '\0') {
        tema[idx] = cmd[i];
        idx = idx + 1;
        i = i + 1;
    }
    tema[idx] = '\0';

    if (strcmp(tema, "winter")) { color = 0x09; printString("Ubah warna ke Winter."); } 
    else if (strcmp(tema, "spring")) { color = 0x0A; printString("Ubah warna ke Spring."); } 
    else if (strcmp(tema, "summer")) { color = 0x0E; printString("Ubah warna ke Summer."); } 
    else if (strcmp(tema, "fall")) { color = 0x06; printString("Ubah warna ke Fall."); } 
    else if (strcmp(tema, "radiant")) { color = 0x0D; printString("Ubah warna ke Radiant."); } 
    else { printString("Tema season tidak dikenal!"); }
    newline();
}

void handleTriangle(char *cmd) {
    int i = 0, j = 0, tinggi, idx = 0;
    char s[16];

    while (cmd[i] != ' ' && cmd[i] != '\0') i = i + 1;
    if (cmd[i] == ' ') i = i + 1;

    while (cmd[i] != '\0') {
        s[idx] = cmd[i];
        idx = idx + 1;
        i = i + 1;
    }
    s[idx] = '\0';
    tinggi = stringToInt(s);

    for (i = 1; i <= tinggi; i = i + 1) {
        for (j = 1; j <= i; j = j + 1) {
            printChar('*');
        }
        newline();
    }
}

// ==========================================
// MAIN SHELL KERNEL LOOP
// ==========================================

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

        if (cmd[0] == 'a' && cmd[1] == 'd' && cmd[2] == 'd') {
            parseDuaAngka(cmd, &a, &b);
            printInt(a + b);
            newline();
        } 
        else if (cmd[0] == 's' && cmd[1] == 'u' && cmd[2] == 'b') {
            parseDuaAngka(cmd, &a, &b);
            printInt(a - b);
            newline();
        }
        else if (cmd[0] == 'f' && cmd[1] == 'a' && cmd[2] == 'c') {
            handleFac(cmd);
        }
        else if (cmd[0] == 's' && cmd[1] == 'e' && cmd[2] == 'a' && cmd[3] == 's') {
            handleSeason(cmd);
        }
        else if (cmd[0] == 't' && cmd[1] == 'r' && cmd[2] == 'i') {
            handleTriangle(cmd);
        }
        else if (strcmp(cmd, "check")) {
            printString("Sistem berjalan dengan baik.");
            newline();
        } 
        else if (strcmp(cmd, "clear")) {
            clearScreen();
        }
        else if (strcmp(cmd, "help")) {
            printString("Commands: check, add [a] [b], sub [a] [b], fac [n], season [nama], triangle [n], clear");
            newline();
        }
        else {
            printString("Command tidak dikenal!");
            newline();
        }
        newline();
    }
}
