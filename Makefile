ARMGNU ?= arm-none-eabi

ASFLAGS := --warn --fatal-warnings
CFLAGS := -Wall -Werror -Os -nostdlib -nostartfiles -ffreestanding -fno-builtin

NAME := ntrcard

HEADER := header.raw

OFILES := $(NAME).o header.o

all: $(NAME).hex $(NAME).bin

clean:
	rm -f *.o
	rm -f *.bin
	rm -f *.hex
	rm -f *.elf
	rm -f *.list
	rm -f *.img
	rm -f *.bc
	rm -f *.clang.opt.s
	rm -f header.c

$(TARGET).o: payload.h

header.o: header.S $(HEADER)
	$(ARMGNU)-gcc -c header.S -DHEADER=\"$(HEADER)\"

%.o: %.c
	$(ARMGNU)-gcc -c -o $@ $< $(CFLAGS)

vectors.o: vectors.s
	$(ARMGNU)-as $(ASFLAGS) vectors.s -o vectors.o

$(NAME).elf: memmap vectors.o $(OFILES)
	$(ARMGNU)-ld vectors.o $(OFILES) -T memmap -o $(NAME).elf
	$(ARMGNU)-objdump -D $(NAME).elf > $(NAME).list

$(NAME).bin: $(NAME).elf
	$(ARMGNU)-objcopy $(NAME).elf -O binary $(NAME).bin

$(NAME).hex: $(NAME).elf
	$(ARMGNU)-objcopy $(NAME).elf -O ihex $(NAME).hex
