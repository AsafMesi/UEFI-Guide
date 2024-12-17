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
