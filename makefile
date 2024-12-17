ARCH=x86_64
CFLAGS=-I/usr/include/efi -fpic -ffreestanding -fno-stack-protector -fshort-wchar -mno-red-zone
LDFLAGS=-shared -Bsymbolic -T /usr/lib/elf_x86_64_efi.lds -L/usr/lib -lgnuefi -lefi

all: main.efi

main.efi: main.c
	gcc $(CFLAGS) -c main.c -o main.o
	ld $(LDFLAGS) /usr/lib/crt0-efi-$(ARCH).o main.o -o main.so
	objcopy --target efi-app-$(ARCH) --subsystem=10 main.so main.efi

clean:
	rm -f *.o *.so *.efi
