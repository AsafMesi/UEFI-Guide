# Step-by-Step Guide to Creating and Running a UEFI Application

In this guide, we will carefully explain every step to create, build, and test a simple **Hello World** UEFI application using the **GNU-EFI** library, `ld` linker, `objcopy`, and QEMU with an OVMF UEFI BIOS. This process assumes you are a beginner and will explain everything in detail, including tools, paths, and commands.

Additionally, we will highlight how to automate the last steps using a **Makefile** for efficiency.

---

## **1. Prerequisites and Tool Installation**

Before starting, install all the tools required for building and testing the UEFI application:

### Command:
```bash
sudo apt update
sudo apt install gnu-efi gcc binutils qemu-system-x86 ovmf mtools
```

### Tools Explained:
- **gnu-efi**: Provides headers (`efi.h`, `efilib.h`) and static libraries (`libefi.a`, `libgnuefi.a`) for UEFI development.
- **gcc**: The GNU C compiler to compile the source code.
- **binutils**: Provides `ld` (linker) and `objcopy` for processing binaries.
- **qemu-system-x86**: QEMU emulator to test UEFI applications.
- **ovmf**: UEFI-compatible BIOS file for QEMU.
- **mtools**: Used to copy files into FAT-formatted virtual disks.

---

## **2. Writing the UEFI Application**

### Locate the UEFI Headers

The UEFI headers (e.g., `efi.h`) are required for compilation. These files may be located in different directories on different machines. Use the following command to find the path:

```bash
find /usr -name efi.h
```

On most systems, it is located at `/usr/include/efi`.

### Create a Source File: `main.c`

This UEFI program initializes the UEFI environment and prints "Hello, world!" to the screen.

```c
#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    // Initialize UEFI Library
    InitializeLib(ImageHandle, SystemTable);

    // Print Hello World
    Print(L"Hello, world!\n");

    // Exit successfully
    return EFI_SUCCESS;
}
```

### Key Points:
- **`efi.h` and `efilib.h`**: Header files from the GNU-EFI library.
- **`efi_main`**: Entry point for UEFI applications.
- **`InitializeLib`**: Initializes the UEFI environment.
- **`Print`**: Prints a wide string to the UEFI console.

---

## **3. Compilation**

The C code needs to be compiled into an object file using `gcc`.

### Command:
```bash
gcc -I/usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c main.c -o main.o
```

### Explanation of Flags:
- **`-I/usr/include/efi`**: Include path for UEFI headers (replace with the correct path if different on your system).
- **`-fpic`**: Generates position-independent code (mandatory for UEFI PE executables).
- **`-ffreestanding`**: Indicates a freestanding environment (no libc, hosted OS, etc.).
- **`-fno-stack-protector -fno-stack-check`**: Disables stack protections (no canaries, no extra checks).
- **`-fshort-wchar`**: Ensures `wchar_t` (used by UEFI) is 16-bit.
- **`-mno-red-zone`**: Prevents using the "red zone" on the stack.
- **`-maccumulate-outgoing-args`**: Ensures function call argument space is pre-allocated.

**Result**: `main.o` — a compiled object file.

---

## **4. Linking**

Link the object file into a shared object file (`main.so`) using the GNU linker.

### Locate Linker Script and Startup Code
The linker script (`elf_x86_64_efi.lds`) and startup code (`crt0-efi-x86_64.o`) may also be located in different places. Use:

```bash
find /usr -name elf_x86_64_efi.lds
find /usr -name crt0-efi-x86_64.o
```

### Command:
```bash
ld -shared -Bsymbolic -L/usr/lib -T/usr/lib/elf_x86_64_efi.lds /usr/lib/crt0-efi-x86_64.o main.o -o main.so -lgnuefi -lefi
```

### Key Points:
- **`-shared -Bsymbolic`**: Produces a shared object file with symbolic bindings.
- **`-L/usr/lib`**: Search path for libraries.
- **`-T/usr/lib/elf_x86_64_efi.lds`**: Uses the linker script for UEFI applications.
- **`/usr/lib/crt0-efi-x86_64.o`**: UEFI startup code (initializes the environment).
- **`-lgnuefi -lefi`**: Links with GNU-EFI libraries (`libgnuefi.a` and `libefi.a`).

**Result**: `main.so` — the linked shared object.

---

## **5. Converting to EFI Executable**

Convert the shared object into a UEFI-compliant PE32+ binary.

### Command:
```bash
objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 main.so main.efi
```

### Explanation:
- **`-j`**: Keeps only the necessary sections (code, data, relocations).
- **`--target efi-app-x86_64`**: Converts to a PE32+ executable suitable for x86_64 UEFI.
- **`--subsystem=10`**: Marks the binary as a UEFI application in the PE header.

**Result**: `main.efi` — the final UEFI executable.

---

## **6. Automating Steps 3–5 with a Makefile**

Instead of manually compiling, linking, and converting, use a Makefile to automate these tasks. Here is a sample Makefile:

### Makefile:
```makefile
# Compiler and tools
CC = gcc
LD = ld
OBJCOPY = objcopy

# Compiler flags
CFLAGS = -I/usr/include/efi \
         -fpic \
         -ffreestanding \
         -fno-stack-protector \
         -fno-stack-check \
         -fshort-wchar \
         -mno-red-zone \
         -maccumulate-outgoing-args

# Linker flags
LDFLAGS = -shared \
          -Bsymbolic \
          -L/usr/lib \
          -T/usr/lib/elf_x86_64_efi.lds

# Libraries
LIBS = -lgnuefi -lefi

# Source and output files
SRC = main.c
OBJ = main.o
SHARED_OBJ = main.so
TARGET = main.efi

# Default target
all: $(TARGET)

# Compile source to object file
$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object file to shared object
$(SHARED_OBJ): $(OBJ)
	$(LD) $(LDFLAGS) /usr/lib/crt0-efi-x86_64.o $< -o $@ $(LIBS)

# Convert shared object to EFI executable
$(TARGET): $(SHARED_OBJ)
	$(OBJCOPY) \
		-j .text \
		-j .sdata \
		-j .data \
		-j .rodata \
		-j .dynamic \
		-j .dynsym \
		-j .rel \
		-j .rela \
		-j .rel.* \
		-j .rela.* \
		-j .reloc \
		--target efi-app-x86_64 \
		--subsystem=10 \
		$< $@

# Clean up generated files
clean:
	rm -f $(OBJ) $(SHARED_OBJ) $(TARGET)

# Phony targets
.PHONY: all clean
```


### Usage:
Run the following commands:
```bash
make    # Builds main.efi
make clean # Cleans up build files
```

---

## **7. Running the EFI Application in QEMU**

### **7.1 Create a FAT-Formatted Virtual Disk**

UEFI applications must reside on a FAT-formatted disk to be bootable. Here’s how to create and prepare the disk:

1. **Copy the EFI executable to a folder**:
   ```bash
   mkdir fat_disk
   cp main.efi fat_disk/
   ```

2. **Create a virtual raw disk image**:
   ```bash
   qemu-img create -f raw fat.img 20M
   ```
   - **`qemu-img create`**: Creates a virtual disk image file.
   - **`-f raw`**: Specifies the image format as raw.
   - **`20M`**: Allocates 20 MB for the virtual disk.

3. **Format the disk as FAT**:
   ```bash
   mkfs.vfat fat.img
   ```
   - **`mkfs.vfat`**: Formats the virtual disk image with a FAT filesystem, which UEFI requires.

4. **Copy the EFI executable to the virtual disk**:
   ```bash
   mcopy -i fat.img main.efi ::
   ```
   - **`mcopy`**: Copies files to a FAT filesystem.
   - **`-i fat.img`**: Specifies the target disk image.
   - **`main.efi ::`**: Copies `main.efi` to the root directory of the FAT image.

### **7.2 Run QEMU with the EFI Application**

Create a bash script to launch QEMU with the required UEFI BIOS and virtual disk:

### Script: `run.sh`
```bash
#!/bin/bash

qemu-system-x86_64 \
  -bios /usr/share/OVMF/OVMF.fd \
  -drive file=fat.img,format=raw,if=virtio \
  -net none \
  -nographic
```

### Explanation of Script:
- **`-bios /usr/share/OVMF/OVMF.fd`**: Specifies the UEFI BIOS file (OVMF).
- **`-drive file=fat.img,format=raw,if=virtio`**: Specifies the FAT-formatted virtual disk (`fat.img`) created earlier.
   - **`file=fat.img`**: Points to the disk image file.
   - **`format=raw`**: Indicates that the disk image uses the raw format.
   - **`if=virtio`**: Configures the disk to use a `virtio` interface, which is an efficient virtualized device.
- **`-net none`**: Disables network emulation for simplicity.
- **`-nographic`**: Runs QEMU in text-only mode without a graphical display.

### **7.3 Start QEMU and Load the UEFI Shell**

Make the script executable and run it:
```bash
chmod +x run.sh
./run.sh
```

QEMU will start and boot into the UEFI environment provided by the OVMF BIOS. If successful, you will see output similar to this:
```
>>Start PXE over IPv4.
>>Start PXE over IPv6.
```
This indicates that the UEFI shell is ready.

### **7.4 Accessing the Virtual Disk in the UEFI Shell**

1. **List available file systems**:
   In the UEFI shell, type:
   ```shell
   fs0:
   ```
   - **`fs0:`**: Refers to the first mounted filesystem, which should be your FAT-formatted disk.

2. **List files on the disk**:
   ```shell
   ls
   ```
   You should see the `main.efi` file that was copied to the disk earlier.

3. **Run the UEFI application**:
   ```shell
   main.efi
   ```
   If everything was done correctly, the output will be:
   ```
   Hello, world!
   ```

### **7.5 Exiting QEMU**

To exit the QEMU session, press:
```bash
Ctrl + A, then X
```

This will stop the QEMU virtual machine.

---

## **8. Summary of Steps**

1. **Write the UEFI C code**: A simple `Hello, world!` program.
2. **Compile**: Convert the C code to an object file (`main.o`).
3. **Link**: Link the object file into a shared object (`main.so`).
4. **Convert**: Convert the shared object into a UEFI PE32+ executable (`main.efi`).
5. **Prepare a FAT-formatted virtual disk**: Store the UEFI application in a bootable FAT disk image.
6. **Run QEMU**: Use OVMF to boot into the UEFI shell and execute the UEFI application.
7. **Verify Output**: Confirm that the application prints "Hello, world!" successfully.

---

## **9. Troubleshooting**

If you encounter issues, here are common solutions:

1. **UEFI shell not detecting `main.efi`**:
   - Ensure the file was correctly copied to the FAT-formatted disk image using `mcopy`.
   - Check that you are accessing the correct filesystem (e.g., `fs0:`).

2. **`mkfs.vfat` or `mcopy` not found**:
   - Verify that `mtools` is installed:
     ```bash
     sudo apt install mtools
     ```

3. **QEMU BIOS file missing**:
   - Confirm the OVMF BIOS path:
     ```bash
     find /usr -name OVMF.fd
     ```
   - Update the `-bios` path in the script if necessary.

4. **Compilation or linking errors**:
   - Verify that `gnu-efi` is installed correctly and headers/libraries are available.
   - Use `find` to locate missing files:
     ```bash
     find /usr -name elf_x86_64_efi.lds
     ```

---

## **10. Further Exploration**

Now that you have successfully created and executed a UEFI application:
- Experiment with more complex UEFI programs (e.g., reading input, interacting with hardware).
- Explore the **UEFI Shell** capabilities (built-in commands and utilities).
- Use QEMU to test other UEFI-based operating systems or firmware.

---

Congratulations! You have completed a full workflow for writing, building, and running a UEFI application in a virtualized environment using QEMU and OVMF.

