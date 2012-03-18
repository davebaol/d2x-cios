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
CFLAGS	=	$(ARCH) -I. -I./fatfs -I../cios-lib -fomit-frame-pointer -Os -Wall
LDFLAGS	=	$(ARCH) -nostartfiles -Wl,-T,link.ld,-Map,$(TARGET).map -Wl,--gc-sections -Wl,-static

# Libraries
LIBS	=	../cios-lib/cios-lib.a

# Target
TARGET	=	ffs-plugin

# Objects
OBJS	=	diskio.o		\
		iop_calls.o		\
		iop_plugin.o		\
		fat.o			\
		fatfs/ccsbcs.o		\
		fatfs/ff.o		\
		ff_mem.o		\
		fs_calls.o		\
		fs_dump.o		\
		fs_tools.o		\
		main.o			\
		patches.o		\
		plugin.o		\
		sdio_plugin.o		\
		start.o

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
