export ARCH:=i386
NASM:=nasm
CC:=gcc
SRC_NASM:=main.S
SRC_LIBC:=$(wildcard libc/*.c)
SRC_C:=main.c
HEADERS = $(wildcard libc/*.h)
LINKER:=linker.ld
BIN:=boot.bin
OBJ = ${SRC_LIBC:.c=.o}
OBJ_NASM:=boot.o
CFLAGS:=-Wall -Werror -m32 -fno-pie -ffreestanding -mno-red-zone -fno-exceptions -nostdlib -nostartfiles -I./src/include
LDFLAGS:=-Wl,--build-id=none
OC:=objcopy
DD:=dd
ELF:=boot.elf

all: $(DEPENDENCIES)
	$(NASM) $(SRC_NASM) -f elf32 -o $(OBJ_NASM)
	$(NASM) libc/interrupt.S -f elf32 -o interrupt.elf
	$(CC) $(SRC_C) $(SRC_LIBC) $(OBJ_NASM) interrupt.elf -o $(ELF) $(CFLAGS) -T $(LINKER) $(LDFLAGS)
	$(OC) -O binary $(ELF) $(BIN)
	$(DD) if=/dev/zero of=$(BIN).tmp count=1440 bs=1024
	$(DD) if=$(BIN) of=$(BIN).tmp conv=notrunc
	mv $(BIN).tmp $(BIN)

clean:
	rm ${OBJ_NASM} boot.elf boot.bin interrupt.elf