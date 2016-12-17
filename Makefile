CFLAGS := -Wall -Werror -Os -nostdlib -nostartfiles -ffreestanding -fno-builtin

NAME := ntr

HEADER := header.raw

OFILES := $(NAME).o header.o

all: $(NAME)

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
	$(CC) -c header.S -DHEADER=\"$(HEADER)\"

$(NAME): $(OFILES)
	$(LD) $(OFILES) -o $(NAME)
