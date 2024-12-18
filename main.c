#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    EFI_HANDLE *HandleBuffer;
    UINTN HandleCount;
    EFI_PCI_IO_PROTOCOL *PciIo;
    EFI_BLOCK_IO_PROTOCOL *BlockIo;
    UINTN Index;
    UINT8 ClassCode[3];
    UINT32 LBA = 0; // Logical Block Address to read
    UINTN BufferSize = 512; // Size of a sector
    VOID *Buffer;

    InitializeLib(ImageHandle, SystemTable);

    // Locate all handles that support the PCI I/O protocol
    Status = uefi_call_wrapper(BS->LocateHandleBuffer, 5, ByProtocol, &gEfiPciIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to locate PCI I/O handles\n");
        return Status;
    }

    // Iterate through all handles to find the PCI IDE controller
    for (Index = 0; Index < HandleCount; Index++) {
        Status = uefi_call_wrapper(BS->HandleProtocol, 3, HandleBuffer[Index], &gEfiPciIoProtocolGuid, (VOID**)&PciIo);
        if (EFI_ERROR(Status)) {
            continue;
        }

        // Read the Class Code from the PCI configuration space
        Status = uefi_call_wrapper(PciIo->Pci.Read, 5, PciIo, EfiPciIoWidthUint8, PCI_CLASSCODE_OFFSET, sizeof(ClassCode), ClassCode);
        if (EFI_ERROR(Status)) {
            continue;
        }

        // Check if the Class Code matches that of an IDE controller (0x01, 0x01, 0x80)
        if (ClassCode[2] == PCI_CLASS_MASS_STORAGE && ClassCode[1] == PCI_CLASS_MASS_STORAGE_IDE) {
            Print(L"Found PCI IDE controller\n");

            // Locate the Block I/O protocol on the same handle
            Status = uefi_call_wrapper(BS->HandleProtocol, 3, HandleBuffer[Index], &gEfiBlockIoProtocolGuid, (VOID**)&BlockIo);
            if (EFI_ERROR(Status)) {
                Print(L"Failed to locate Block I/O protocol\n");
                continue;
            }

            // Allocate buffer for reading the sector
            Buffer = AllocateZeroPool(BufferSize);
            if (Buffer == NULL) {
                Print(L"Failed to allocate buffer\n");
                return EFI_OUT_OF_RESOURCES;
            }

            // Read the sector
            Status = uefi_call_wrapper(BlockIo->ReadBlocks, 5, BlockIo, BlockIo->Media->MediaId, LBA, BufferSize, Buffer);
            if (EFI_ERROR(Status)) {
                Print(L"Failed to read sector\n");
                FreePool(Buffer);
                return Status;
            }

            // Print the content of the sector
            Print(L"Sector content:\n");
            for (UINTN i = 0; i < BufferSize; i++) {
                Print(L"%02x ", ((UINT8*)Buffer)[i]);
                if ((i + 1) % 16 == 0) {
                    Print(L"\n");
                }
            }

            FreePool(Buffer);
            break;
        }
    }

    if (Index == HandleCount) {
        Print(L"No PCI IDE controller found\n");
        return EFI_NOT_FOUND;
    }

    return EFI_SUCCESS;
}