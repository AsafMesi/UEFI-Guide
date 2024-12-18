# Step-by-Step Guide: UEFI Application for IDE Controller Interaction

This guide will walk you through the process of creating a UEFI application that:
1. Initializes an IDE controller.
2. Reads the first sector of a disk (using 28-bit LBA).
3. Returns and prints the sector's content.

We'll also cover testing the application in a virtual environment using QEMU.

---

## **1. Prerequisites**

Before starting, ensure the following tools and components are ready:
- A working installation of QEMU.
- The `gnu-efi` library for developing UEFI applications.
- A minimal raw disk image (`uefi_disk.img`) created with `qemu-img`.
- The `mkfs.vfat` tool for formatting the disk.
- Basic understanding of UEFI application development.

---

## **2. Create a Raw Disk Image**

1. **Create the disk image:**
   ```bash
   qemu-img create -f raw uefi_disk.img 50M
   ```
   - This creates a 50 MB raw disk image.

2. **Format the disk as FAT32:**
   ```bash
   mkfs.vfat -F 32 uefi_disk.img
   ```
   - Formats the disk image to the FAT32 filesystem, which is required by UEFI.

---

## **3. Develop the UEFI Application**

1. **Write the UEFI Application:** Create a file `main.c` with the following code:

   ```c
   #include <efi.h>
   #include <efilib.h>

   EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
       EFI_STATUS Status;
       EFI_BLOCK_IO_PROTOCOL *BlockIo;
       EFI_HANDLE *Handles;
       UINTN HandleCount;
       VOID *Buffer;
       UINTN BufferSize = 512; // Size of one sector (512 bytes)

       // Initialize the UEFI library
       InitializeLib(ImageHandle, SystemTable);

       // Locate all handles supporting Block IO Protocol
       Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleCount, &Handles);
       if (EFI_ERROR(Status)) {
           Print(L"Failed to locate Block IO Protocol: %r\n", Status);
           return Status;
       }

       // Get the first handle supporting Block IO
       Status = gBS->HandleProtocol(Handles[0], &gEfiBlockIoProtocolGuid, (void **)&BlockIo);
       if (EFI_ERROR(Status)) {
           Print(L"Failed to open Block IO Protocol: %r\n", Status);
           return Status;
       }

       // Allocate memory for the buffer
       Status = gBS->AllocatePool(EfiLoaderData, BufferSize, &Buffer);
       if (EFI_ERROR(Status)) {
           Print(L"Failed to allocate buffer: %r\n", Status);
           return Status;
       }

       // Read the first sector (LBA 0)
       Status = BlockIo->ReadBlocks(BlockIo, BlockIo->Media->MediaId, 0, BufferSize, Buffer);
       if (EFI_ERROR(Status)) {
           Print(L"Failed to read sector: %r\n", Status);
           gBS->FreePool(Buffer);
           return Status;
       }

       // Print the content of the first sector
       Print(L"Sector Content: \n");
       for (UINTN i = 0; i < BufferSize; i++) {
           Print(L"%02x ", ((UINT8 *)Buffer)[i]);
           if ((i + 1) % 16 == 0) Print(L"\n");
       }

       // Free allocated memory
       gBS->FreePool(Buffer);

       return EFI_SUCCESS;
   }
   ```

2. **Compile the Application:** Use the following Makefile to build `main.efi`:

   ```makefile
   CC = gcc
   LD = ld
   OBJCOPY = objcopy

   CFLAGS = -I/usr/include/efi -fpic -ffreestanding -fno-stack-protector -fshort-wchar -mno-red-zone
   LDFLAGS = -shared -Bsymbolic -T /usr/lib/elf_x86_64_efi.lds -L/usr/lib -lgnuefi -lefi

   all: main.efi

   main.efi: main.c
       gcc $(CFLAGS) -c main.c -o main.o
       ld $(LDFLAGS) /usr/lib/crt0-efi-x86_64.o main.o -o main.so
       objcopy --target=efi-app-x86_64 --subsystem=10 main.so main.efi

   clean:
       rm -f *.o *.so *.efi
   ```

   Run `make` to generate the `main.efi` file.

---

## **4. Prepare the Disk for Testing**

1. **Create the UEFI Boot Path:**
   ```bash
   mkdir /mnt/uefi_disk
   sudo mount uefi_disk.img /mnt/uefi_disk
   sudo mkdir -p /mnt/uefi_disk/EFI/BOOT
   sudo cp main.efi /mnt/uefi_disk/EFI/BOOT/BOOTX64.EFI
   sudo umount /mnt/uefi_disk
   ```

---

## **5. Create a QEMU Script for Testing**

1. **Create `run.sh`:**
   ```bash
   #!/bin/bash
   qemu-system-x86_64 \
   --enable-kvm \
   -m 100M \
   -device piix3-ide,id=ide \
   -drive id=disk,file=uefi_disk.img,format=raw,if=none \
   -bios /usr/share/ovmf/OVMF.fd \
   -net none
   ```

2. **Make the Script Executable:**
   ```bash
   chmod +x run.sh
   ```

3. **Run the Script:**
   ```bash
   ./run.sh
   ```

---

## **6. Test the Application**

1. When the UEFI shell appears in QEMU, it should automatically run the `BOOTX64.EFI` application.
2. The content of the first sector of the disk will be printed to the screen.

---

## **Summary**
This guide demonstrated how to:
- Initialize an IDE controller using UEFI.
- Read the first sector of a disk using the `EFI_BLOCK_IO_PROTOCOL`.
- Print the content of the sector to the screen.
- Test the application using QEMU.
