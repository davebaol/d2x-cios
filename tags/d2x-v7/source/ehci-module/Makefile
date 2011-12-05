# devkitARM path
DEVKITARM ?=	/opt/devkitARM

# Prefix
PREFIX	=	$(DEVKITARM)/bin/arm-eabi-

# Executables
CC	=	$(PREFIX)gcc
LD	=	$(PREFIX)gcc
STRIP	=	../stripios

# Flags
ARCH	=	-mcpu=arm926ej-s -mthumb -mthumb-interwork -mbig-endian
CFLAGS	=	$(ARCH) -I. -Itinyehci -I../cios-lib -fomit-frame-pointer -Os -Wall -Wstrict-prototypes -ffunction-sections
LDFLAGS	=	$(ARCH) -nostartfiles -Wl,-T,link.ld,-Map,$(TARGET).map -Wl,--gc-sections -Wl,-static

# Libraries
LIBS	=	../cios-lib/cios-lib.a

# Target
TARGET	=	ehci-module

# Objects
OBJS	=	loop.o				\
		main.o				\
		start.o				\
		tinyehci.o			\
		usb_os.o			\
		wbfs.o


$(TARGET).elf: $(OBJS) $(LIBS)
	@echo -e " LD\t$@"
	@$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@.orig
	@$(STRIP) $@.orig $@

%.o: %.s
	@echo -e " CC\t$@"
	@$(CC) $(CFLAGS) -D_LANGUAGE_ASSEMBLY -c -x assembler-with-cpp -o $@ $<

%.o: %.c
	@echo -e " CC\t$@"
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo -e "Cleaning..."
	@rm -f $(OBJS) $(TARGET).elf $(TARGET).elf.orig $(TARGET).map
