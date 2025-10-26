#include "base.h"

#define IDT_ENTRIES 256
idt_gate_t idt[IDT_ENTRIES];
idt_register_t idt_reg;

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

void memcpy(u8 *source, u8 *dest, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        *(dest + i) = *(source + i);
    }
}

void memset(u8 *dest, u8 val, u32 len) {
    u8 *temp = (u8 *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

u8 pbin (u16 port) {
    u8 result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void pbout (u16 port, u8 data) {
    __asm__ __volatile__("out %%al, %%dx" : : "a" (data), "d" (port));
}

u16 pwin (u16 port) {
    u16 result;
    __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void pwout (u16 port, u16 data) {
    __asm__ __volatile__("out %%ax, %%dx" : : "a" (data), "d" (port));
}

int gcoffset();
void scoffset(int offset);
int printchar(char c, int col, int row, char attr);
int goffset(int col, int row);
int gorow(int offset);
int gocol(int offset);

void printat(char *message, int col, int row) {
    int offset;
    if (col >= 0 && row >= 0)
        offset = goffset(col, row);
    else {
        offset = gcoffset();
        row = gorow(offset);
        col = gocol(offset);
    }

    int i = 0;
    while (message[i] != 0) {
        offset = printchar(message[i++], col, row, WHITE_ON_BLACK);
        row = gorow(offset);
        col = gocol(offset);
    }
}

void print(char *message) {
    printat(message, -1, -1);
}

void backspace(char s[]) {
    int len = strlen(s);
    s[len-1] = '\0';
}

void pbackspace() {
    int offset = gcoffset()-2;
    int row = gorow(offset);
    int col = gocol(offset);
    printchar(0x08, col, row, WHITE_ON_BLACK);
}

int printchar(char c, int col, int row, char attr) {
    u8 *vidmem = (u8*) VIDEO_ADDRESS;
    if (!attr) attr = WHITE_ON_BLACK;

    if (col >= MAX_COLS || row >= MAX_ROWS) {
        vidmem[2*(MAX_COLS)*(MAX_ROWS)-2] = 'E';
        vidmem[2*(MAX_COLS)*(MAX_ROWS)-1] = RED_ON_WHITE;
        return goffset(col, row);
    }

    int offset;
    if (col >= 0 && row >= 0) offset = goffset(col, row);
    else offset = gcoffset();

    if (c == '\n') {
        row = gorow(offset);
        offset = goffset(0, row+1);
    } else if (c == 0x08) {
        vidmem[offset] = ' ';
        vidmem[offset+1] = attr;
    } else {
        vidmem[offset] = c;
        vidmem[offset+1] = attr;
        offset += 2;
    }

    if (offset >= MAX_ROWS * MAX_COLS * 2) {
        int i;
        for (i = 1; i < MAX_ROWS; i++)
            memcpy((u8*)(goffset(0, i) + VIDEO_ADDRESS),
                        (u8*)(goffset(0, i-1) + VIDEO_ADDRESS),
                        MAX_COLS * 2);

        char *last_line = (char*) (goffset(0, MAX_ROWS-1) + (u8*) VIDEO_ADDRESS);
        for (i = 0; i < MAX_COLS * 2; i++) last_line[i] = 0;

        offset -= 2 * MAX_COLS;
    }

    scoffset(offset);
    return offset;
}

int gcoffset() {
    pbout(REG_SCREEN_CTRL, 14);
    int offset = pbin(REG_SCREEN_DATA) << 8;
    pbout(REG_SCREEN_CTRL, 15);
    offset += pbin(REG_SCREEN_DATA);
    return offset * 2;
}

void scoffset(int offset) {
    offset /= 2;
    pbout(REG_SCREEN_CTRL, 14);
    pbout(REG_SCREEN_DATA, (u8)(offset >> 8));
    pbout(REG_SCREEN_CTRL, 15);
    pbout(REG_SCREEN_DATA, (u8)(offset & 0xff));
}

void clear() {
    int screen_size = MAX_COLS * MAX_ROWS;
    int i;
    u8 *screen = (u8*) VIDEO_ADDRESS;

    for (i = 0; i < screen_size; i++) {
        screen[i*2] = ' ';
        screen[i*2+1] = WHITE_ON_BLACK;
    }
    scoffset(goffset(0, 0));
}

void int2ascii(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

    reverse(str);
}

void hex2ascii(int n, char str[]) {
    append(str, '0');
    append(str, 'x');
    char zeros = 0;

    s32 tmp;
    int i;
    for (i = 28; i > 0; i -= 4) {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && zeros == 0) continue;
        zeros = 1;
        if (tmp > 0xA) append(str, tmp - 0xA + 'a');
        else append(str, tmp + '0');
    }

    tmp = n & 0xF;
    if (tmp >= 0xA) append(str, tmp - 0xA + 'a');
    else append(str, tmp + '0');
}

void reverse(char s[]) {
    int c, i, j;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int strlen(char s[]) {
    int i = 0;
    while (s[i] != '\0') ++i;
    return i;
}

void append(char s[], char n) {
    int len = strlen(s);
    s[len] = n;
    s[len+1] = '\0';
}

int strcmp(char s1[], char s2[]) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

int goffset(int col, int row) { return 2 * (row * MAX_COLS + col); }
int gorow(int offset) { return offset / (2 * MAX_COLS); }
int gocol(int offset) { return (offset - (gorow(offset)*2*MAX_COLS))/2; }

void set_idt_gate(int n, u32 handler) {
    idt[n].low_offset = low_16(handler);
    idt[n].sel = KERNEL_CS;
    idt[n].always0 = 0;
    idt[n].flags = 0x8E;
    idt[n].high_offset = high_16(handler);
}

void set_idt() {
    idt_reg.base = (u32) &idt;
    idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
    __asm__ __volatile__("lidtl (%0)" : : "r" (&idt_reg));
}

isr_t interrupt_handlers[256];

void isr_install() {
    set_idt_gate(0, (u32)isr0);
    set_idt_gate(1, (u32)isr1);
    set_idt_gate(2, (u32)isr2);
    set_idt_gate(3, (u32)isr3);
    set_idt_gate(4, (u32)isr4);
    set_idt_gate(5, (u32)isr5);
    set_idt_gate(6, (u32)isr6);
    set_idt_gate(7, (u32)isr7);
    set_idt_gate(8, (u32)isr8);
    set_idt_gate(9, (u32)isr9);
    set_idt_gate(10, (u32)isr10);
    set_idt_gate(11, (u32)isr11);
    set_idt_gate(12, (u32)isr12);
    set_idt_gate(13, (u32)isr13);
    set_idt_gate(14, (u32)isr14);
    set_idt_gate(15, (u32)isr15);
    set_idt_gate(16, (u32)isr16);
    set_idt_gate(17, (u32)isr17);
    set_idt_gate(18, (u32)isr18);
    set_idt_gate(19, (u32)isr19);
    set_idt_gate(20, (u32)isr20);
    set_idt_gate(21, (u32)isr21);
    set_idt_gate(22, (u32)isr22);
    set_idt_gate(23, (u32)isr23);
    set_idt_gate(24, (u32)isr24);
    set_idt_gate(25, (u32)isr25);
    set_idt_gate(26, (u32)isr26);
    set_idt_gate(27, (u32)isr27);
    set_idt_gate(28, (u32)isr28);
    set_idt_gate(29, (u32)isr29);
    set_idt_gate(30, (u32)isr30);
    set_idt_gate(31, (u32)isr31);

    pbout(0x20, 0x11);
    pbout(0xA0, 0x11);
    pbout(0x21, 0x20);
    pbout(0xA1, 0x28);
    pbout(0x21, 0x04);
    pbout(0xA1, 0x02);
    pbout(0x21, 0x01);
    pbout(0xA1, 0x01);
    pbout(0x21, 0x0);
    pbout(0xA1, 0x0);

    set_idt_gate(32, (u32)irq0);
    set_idt_gate(33, (u32)irq1);
    set_idt_gate(34, (u32)irq2);
    set_idt_gate(35, (u32)irq3);
    set_idt_gate(36, (u32)irq4);
    set_idt_gate(37, (u32)irq5);
    set_idt_gate(38, (u32)irq6);
    set_idt_gate(39, (u32)irq7);
    set_idt_gate(40, (u32)irq8);
    set_idt_gate(41, (u32)irq9);
    set_idt_gate(42, (u32)irq10);
    set_idt_gate(43, (u32)irq11);
    set_idt_gate(44, (u32)irq12);
    set_idt_gate(45, (u32)irq13);
    set_idt_gate(46, (u32)irq14);
    set_idt_gate(47, (u32)irq15);

    set_idt();
}

char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_handler(registers_t r) {
    print("received interrupt: ");
    char s[3];
    int2ascii(r.int_no, s);
    print(s);
    print("\n");
    print(exception_messages[r.int_no]);
    print("\n");
}

void register_interrupt_handler(u8 n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

void irq_handler(registers_t r) {
    if (r.int_no >= 40) pbout(0xA0, 0x20);
    pbout(0x20, 0x20);

    if (interrupt_handlers[r.int_no] != 0) {
        isr_t handler = interrupt_handlers[r.int_no];
        handler(r);
    }
}

#define BACKSPACE 0x0E
#define ENTER 0x1C
#define SC_MAX 57

u32 tick = 0;
u8 previous = 0;
int caps = 0;
char buffer[256];
int array_position = 0;
char *temp = 0;

char *strcat(char *dest, const char *src) {
    size_t i,j;
    for (i = 0; dest[i] != '\0'; i++)
        ;
    for (j = 0; src[j] != '\0'; j++)
        dest[i+j] = src[j];
    dest[i+j] = '\0';
    return dest;
}

int compareArrays(char a[], char b[], int n) {
  int ii;
  for(ii = 1; ii <= n; ii++) {
    if (a[ii] != b[ii]) return 0;
  }
  return 1;
}

static void main_callback(registers_t regs) {
    tick++;
    print("\nroot@AriaOS:$ ");
    while(1) {
        u8 scancode = pbin(0x60);
        if (scancode == 0x2a || scancode == 0x36) {
            caps = 1;
        } else if (previous == 170 || previous == 182) {
            caps = 1;
        } else {
            caps = 0;
        }
        if (caps == 0 && previous != scancode) {
            switch (scancode) {
                case 0x1:
                    print("\nCPU Halted. Safe for poweroff...");
                    asm volatile ("hlt");
                case 0x2:
                    print("1");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"1"), array_position);
                    array_position++;
                    break;
                case 0x3:
                    print("2");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"2"), array_position);
                    array_position++;
                    break;
                case 0x4:
                    print("3");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"3"), array_position);
                    array_position++;
                    break;
                case 0x5:
                    print("4");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"4"), array_position);
                    array_position++;
                    break;
                case 0x6:
                    print("5");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"5"), array_position);
                    array_position++;
                    break;
                case 0x7:
                    print("6");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"6"), array_position);
                    array_position++;
                    break;
                case 0x8:
                    print("7");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"7"), array_position);
                    array_position++;
                    break;
                case 0x9:
                    print("8");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"8"), array_position);
                    array_position++;
                    break;
                case 0x0A:
                    print("9");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"9"), array_position);
                    array_position++;
                    break;
                case 0x0B:
                    print("0");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"0"), array_position);
                    array_position++;
                    break;
                case 0x0C:
                    print("-");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"-"), array_position);
                    array_position++;
                    break;
                case 0x0D:
                    print("=");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"="), array_position);
                    array_position++;
                    break;
                case 0x0E:
                    --array_position;
                    int offset = gcoffset()-2;
                    int noffset = gcoffset()-0;
                    int row = gorow(noffset);
                    int prevcol = gocol(offset);
                    if (prevcol < 14) { prevcol = 14; }
                    char null = 0;
                    int col = gocol(null);
                    printchar(0x72, col, row, WHITE_ON_BLACK);
                    print("oot@AriaOS:$ ");
                    printchar(0x08, prevcol, row, WHITE_ON_BLACK);
                    buffer[array_position - 1] = 0;
                    break;
                case 0x0F:
                    print("   ");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"   "), array_position);
                    array_position++;
                    break;
                case 0x10:
                    print("q");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"q"), array_position);
                    array_position++;
                    break;
                case 0x11:
                    print("w");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"w"), array_position);
                    array_position++;
                    break;
                case 0x12:
                    print("e");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"e"), array_position);
                    array_position++;
                    break;
                case 0x13:
                    print("r");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"r"), array_position);
                    array_position++;
                    break;
                case 0x14:
                    print("t");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"t"), array_position);
                    array_position++;
                    break;
                case 0x15:
                    print("y");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"y"), array_position);
                    array_position++;
                    break;
                case 0x16:
                    print("u");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"u"), array_position);
                    array_position++;
                    break;
                case 0x17:
                    print("i");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"i"), array_position);
                    array_position++;
                    break;
                case 0x18:
                    print("o");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"o"), array_position);
                    array_position++;
                    break;
                case 0x19:
                    print("p");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"p"), array_position);
                    array_position++;
                    break;
	    	    case 0x1A:
		        	print("[");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"["), array_position);
                    array_position++;
			        break;
		        case 0x1B:
		  	        print("]");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"]"), array_position);
                    array_position++;
		  	        break;
	    	    case 0x1C:
                    print("\n");
                    char clear_input[256] = "clear";
                    int clear_check = compareArrays(buffer, clear_input, 5);
                    if (clear_check == 1) {
                    clear();
                    }
                    print(buffer);
	       		    print("\nroot@AriaOS:$ ");
                    memset((unsigned char*)buffer, 0, sizeof buffer);
                    array_position = 1;
	       		    break;
	    	    case 0x1D:
	      		    /* print("LCtrl"); */
	      		    break;
	    	    case 0x1E:
	      		    print("a");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"a"), array_position);
                    array_position++;
	       		    break;
	    	    case 0x1F:
	      		    print("s");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"s"), array_position);
                    array_position++;
		       	    break;
                case 0x20:
                    print("d");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"d"), array_position);
                    array_position++;
                    break;
                case 0x21:
                    print("f");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"f"), array_position);
                    array_position++;
                    break;
                case 0x22:
                    print("g");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"g"), array_position);
                    array_position++;
                    break;
                case 0x23:
                    print("h");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"h"), array_position);
                    array_position++;
                    break;
                case 0x24:
                    print("j");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"j"), array_position);
                    array_position++;
                    break;
                case 0x25:
                    print("k");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"k"), array_position);
                    array_position++;
                    break;
                case 0x26:
                    print("l");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"l"), array_position);
                    array_position++;
                    break;
                case 0x27:
                    print(";");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&";"), array_position);
                    array_position++;
                    break;
                case 0x28:
                    print("'");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"'"), array_position);
                    array_position++;
                    break;
                case 0x29:
                    print("´");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"´"), array_position);
                    array_position++;
                    break;
    		    case 0x2A:
    			    /* print("LShift"); */
    		  	    break;
    		    case 0x2B:
    	  		    print("\\");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"\\"), array_position);
                    array_position++;
      			    break;
    		    case 0x2C:
      			    print("z");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"z"), array_position);
                    array_position++;
       			    break;
    		    case 0x2D:
    			    print("x");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"x"), array_position);
                    array_position++;
    		  	    break;
    		    case 0x2E:
    	  		    print("c");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"c"), array_position);
                    array_position++;
      			    break;
    		    case 0x2F:
      			    print("v");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"v"), array_position);
                    array_position++;
  	    		    break;
                case 0x30:
                    print("b");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"b"), array_position);
                    array_position++;
                    break;
                case 0x31:
                    print("n");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"n"), array_position);
                    array_position++;
                    break;
                case 0x32:
                    print("m");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"m"), array_position);
                    array_position++;
                    break;
                case 0x33:
                    print(",");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&","), array_position);
                    array_position++;
                    break;
                case 0x34:
                    print(".");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"."), array_position);
                    array_position++;
                    break;
                case 0x35:
                    print("/");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"/"), array_position);
                    array_position++;
                    break;
                case 0x36:
                    /* print("Rshift"); */
                    break;
                case 0x37:
                    /* print("Keypad *"); */
                    break;
                case 0x38:
                    /* print("LAlt"); */
                    break;
                case 0x39:
                    print(" ");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&" "), array_position);
                    array_position++;
                    break;
            }
        }
        if (caps == 1 && previous != scancode) {
            switch (scancode) {
                case 0x1:
                    print("\nCPU Halted. Safe for poweroff...");
                    asm volatile ("hlt");
                case 0x2:
                    print("!");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"!"), array_position);
                    array_position++;
                    break;
                case 0x3:
                    print("@");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"@"), array_position);
                    array_position++;
                    break;
                case 0x4:
                    print("#");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"#"), array_position);
                    array_position++;
                    break;
                case 0x5:
                    print("$");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"$"), array_position);
                    array_position++;
                    break;
                case 0x6:
                    print("%");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"%"), array_position);
                    array_position++;
                    break;
                case 0x7:
                    print("¨");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"¨"), array_position);
                    array_position++;
                    break;
                case 0x8:
                    print("&");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"&"), array_position);
                    array_position++;
                    break;
                case 0x9:
                    print("*");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"*"), array_position);
                    array_position++;
                    break;
                case 0x0A:
                    print("(");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&")"), array_position);
                    array_position++;
                    break;
                case 0x0B:
                    print(")");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&")"), array_position);
                    array_position++;
                    break;
                case 0x0C:
                    print("_");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"_"), array_position);
                    array_position++;
                    break;
                case 0x0D:
                    print("+");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"+"), array_position);
                    array_position++;
                    break;
                case 0x0E:
                    --array_position;
                    int offset = gcoffset()-2;
                    int noffset = gcoffset()-0;
                    int row = gorow(noffset);
                    int prevcol = gocol(offset);
                    if (prevcol < 14) { prevcol = 14; }
                    char null = 0;
                    int col = gocol(null);
                    printchar(0x72, col, row, WHITE_ON_BLACK);
                    print("oot@AriaOS:$ ");
                    printchar(0x08, prevcol, row, WHITE_ON_BLACK);
                    buffer[array_position - 1] = 0;
                    break;
                case 0x0F:
                    print("   ");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"   "), array_position);
                    array_position++;
                    break;
                case 0x10:
                    print("Q");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"Q"), array_position);
                    array_position++;
                    break;
                case 0x11:
                    print("W");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"W"), array_position);
                    array_position++;
                    break;
                case 0x12:
                    print("E");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"E"), array_position);
                    array_position++;
                    break;
                case 0x13:
                    print("R");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"R"), array_position);
                    array_position++;
                    break;
                case 0x14:
                    print("T");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"T"), array_position);
                    array_position++;
                    break;
                case 0x15:
                    print("Y");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"Y"), array_position);
                    array_position++;
                    break;
                case 0x16:
                    print("U");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"U"), array_position);
                    array_position++;
                    break;
                case 0x17:
                    print("I");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"I"), array_position);
                    array_position++;
                    break;
                case 0x18:
                    print("O");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"O"), array_position);
                    array_position++;
                    break;
                case 0x19:
                    print("P");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"P"), array_position);
                    array_position++;
                    break;
	    	    case 0x1A:
		        	print("{");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"{"), array_position);
                    array_position++;
			        break;
		        case 0x1B:
		  	        print("}");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"}"), array_position);
                    array_position++;
		  	        break;
	    	    case 0x1C:
                    print("\n");
                    char clear_input[256] = "clear";
                    int clear_check = compareArrays(buffer, clear_input, 5);
                    if (clear_check == 1) {
                    clear();
                    }
                    print(buffer);
	       		    print("\nroot@AriaOS:$ ");
                    memset((unsigned char*)buffer, 0, sizeof buffer);
                    array_position = 1;
	       		    break;
	    	    case 0x1D:
	      		    /* print("LCtrl"); */
	      		    break;
	    	    case 0x1E:
	      		    print("A");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"A"), array_position);
                    array_position++;
	       		    break;
	    	    case 0x1F:
	      		    print("S");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"S"), array_position);
                    array_position++;
		       	    break;
                case 0x20:
                    print("D");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"D"), array_position);
                    array_position++;
                    break;
                case 0x21:
                    print("F");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"F"), array_position);
                    array_position++;
                    break;
                case 0x22:
                    print("G");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"G"), array_position);
                    array_position++;
                    break;
                case 0x23:
                    print("H");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"H"), array_position);
                    array_position++;
                    break;
                case 0x24:
                    print("J");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"J"), array_position);
                    array_position++;
                    break;
                case 0x25:
                    print("K");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"K"), array_position);
                    array_position++;
                    break;
                case 0x26:
                    print("L");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"L"), array_position);
                    array_position++;
                    break;
                case 0x27:
                    print(":");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&":"), array_position);
                    array_position++;
                    break;
                case 0x28:
                    print("\"");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"\""), array_position);
                    array_position++;
                    break;
                case 0x29:
                    print("`");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"`"), array_position);
                    array_position++;
                    break;
    		    case 0x2A:
    			    /* print("LShift"); */
    		  	    break;
    		    case 0x2B:
    	  		    print("|");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"|"), array_position);
                    array_position++;
      			    break;
    		    case 0x2C:
      			    print("Z");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"Z"), array_position);
                    array_position++;
       			    break;
    		    case 0x2D:
    			    print("X");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"X"), array_position);
                    array_position++;
    		  	    break;
    		    case 0x2E:
    	  		    print("C");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"C"), array_position);
                    array_position++;
      			    break;
    		    case 0x2F:
      			    print("V");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"V"), array_position);
                    array_position++;
  	    		    break;
                case 0x30:
                    print("B");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"B"), array_position);
                    array_position++;
                    break;
                case 0x31:
                    print("N");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"N"), array_position);
                    array_position++;
                    break;
                case 0x32:
                    print("M");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"M"), array_position);
                    array_position++;
                    break;
                case 0x33:
                    print("<");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"<"), array_position);
                    array_position++;
                    break;
                case 0x34:
                    print(">");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&">"), array_position);
                    array_position++;
                    break;
                case 0x35:
                    print("?");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&"?"), array_position);
                    array_position++;
                    break;
                case 0x36:
                    /* print("Rshift"); */
                    break;
                case 0x37:
                    /* print("Keypad *"); */
                    break;
                case 0x38:
                    /* print("LAlt"); */
                    break;
                case 0x39:
                    print(" ");
                    memcpy((unsigned char*)buffer, (unsigned char*)strcat(buffer, (char*)&" "), array_position);
                    array_position++;
                    break;
            }
        }
        previous = scancode;
    }
}

void init_main(u32 freq) {
    register_interrupt_handler(IRQ0, main_callback);
    u32 divisor = 1193180 / freq;
    u8 low  = (u8)(divisor & 0xFF);
    u8 high = (u8)( (divisor >> 8) & 0xFF);
    pbout(0x43, 0x36);
    pbout(0x40, low);
    pbout(0x40, high);
}