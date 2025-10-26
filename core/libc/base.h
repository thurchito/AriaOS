#ifndef BASE_H
#define BASE_H
#define KERNEL_CS 0x08
#define NULL 0

typedef unsigned int   u32;
typedef          int   s32;
typedef unsigned short u16;
typedef          short s16;
typedef unsigned char  u8;
typedef          char  s8;
typedef unsigned long size_t;

#define UNUSED(x) (void)(x)
#define low_16(address) (u16)((address) & 0xFFFF)
#define high_16(address) (u16)(((address) >> 16) & 0xFFFF)

char *strcat(char *dest, const char *src);
void int2ascii(int n, char str[]);
void hex2ascii(int n, char str[]);
void reverse(char s[]);
int strlen(char s[]);
void pbackspace();
void backspace(char s[]);
void append(char s[], char n);
int strcmp(char s1[], char s2[]);

void memcmp(u8 *source, u8 *dest, int nbytes);
void memset(u8 *dest, u8 val, u32 len);

unsigned char pbin (u16 port);
void pbout (u16 port, u8 data);
unsigned short pwin (u16 port);
void pwout (u16 port, u16 data);

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f
#define RED_ON_WHITE 0xf4

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

void clear();
void printat(char *message, int col, int row);
void print(char *message);

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

typedef struct {
   u32 ds;
   u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
   u32 int_no, err_code;
   u32 eip, cs, eflags, useresp, ss;
} registers_t;

void isr_install();
void isr_handler(registers_t r);

typedef void (*isr_t)(registers_t);
void register_interrupt_handler(u8 n, isr_t handler);

#define KERNEL_CS 0x08

typedef struct {
    u16 low_offset;
    u16 sel;
    u8 always0;
    u8 flags;
    u16 high_offset;
} __attribute__((packed)) idt_gate_t ;

typedef struct {
    u16 limit;
    u32 base;
} __attribute__((packed)) idt_register_t;

void set_idt_gate(int n, u32 handler);
void set_idt();

void init_main(u32 freq);
void init_keyboard();
void user_input(char *input);

#endif