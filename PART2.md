# Guide to Setting Up a UEFI Application on a Virtual Disk

This guide demonstrates how to set up a UEFI application (`main.efi`) on a small virtual disk with an `EFI ESP` partition, format it to `FAT32`, and test the application. Along the way, we will explain the concepts and tools used in each step.

---

## **1. Prerequisites**

Before starting, ensure the following:
- The `main.efi` file has been created and tested.
- Relevant tools such as `gdisk`, `mkfs.vfat`, `losetup`, `mount`, and `qemu-system-x86_64` are installed.

---

## **2. Create a Virtual Disk with a Single EFI ESP Partition**

### **What is a Partition?**
A partition is a logically separated section of a storage device. Each partition can have a specific filesystem, allowing multiple operating systems or data sections to coexist on a single disk.

### **What is `gdisk`?**
`gdisk` is a partition management tool for GUID Partition Table (GPT)-based disks. It allows creating, deleting, and modifying partitions. GPT is a modern partitioning scheme that supports large disks and is required for UEFI systems.

### **Steps to Create an EFI ESP Partition:**
1. **Create the Virtual Disk:**
   ```bash
   qemu-img create -f raw uefi_disk.img 50M
   ```
   - **`qemu-img create`**: Creates a virtual disk image.
   - **`-f raw`**: Specifies the raw format.
   - **`50M`**: Sets the size to 50 MB.

2. **Launch `gdisk` to Partition the Disk:**
   ```bash
   gdisk uefi_disk.img
   ```
   - **`gdisk`**: Opens the disk for partitioning without requiring elevated privileges.

3. **Create an EFI System Partition (ESP):**
   - Enter `n` to create a new partition.
   - Accept default values for partition number and sector sizes.
   - Enter `EF00` as the partition type (EFI System Partition).

4. **Write the Changes:**
   - Type `w` to write the partition table and exit.

### **What is a Single Partition? What is EFI ESP?**
- **Single Partition**: A disk with one partition only.
- **EFI ESP**: A partition type required for UEFI firmware. The UEFI specification mandates that the ESP be formatted as FAT32 to ensure compatibility with all UEFI implementations. This is why we use `mkfs.vfat` to format it explicitly, even if it is expected to be FAT32 by default.

---

## **3. Format the Partition to FAT32**

### **What is FAT32?**
FAT32 is a simple and widely compatible filesystem often used in boot environments like UEFI. Formatting a partition erases its content and prepares it to store files using a specific filesystem.

### **What is `mkfs.vfat`?**
`mkfs.vfat` creates a FAT32 filesystem on a partition. It is part of the `dosfstools` package. While the ESP is always FAT32, explicitly formatting it ensures proper initialization and prevents any pre-existing data from interfering.

### **Steps to Format the Partition:**
1. **Map the Partition to a Loop Device:**
   ```bash
   losetup -Pf --show uefi_disk.img
   ```
   - **`losetup`**: Associates the disk image with a loop device, making it accessible as a virtual block device.
   - **Why `losetup`?**: It allows tools like `mkfs.vfat` and `mount` to interact with partitions within the image as if they were real devices. The `-Pf` flags simplify this process by automatically mapping partitions.
   - **`--show`**: Displays the assigned loop device (e.g., `/dev/loop0`).
   - Replace `/dev/loop0` with the actual loop device you used.

2. **Format the EFI Partition:**
   ```bash
   sudo mkfs.vfat -F 32 /dev/loop0p1
   ```
   - **`mkfs.vfat`**: Creates a FAT filesystem.
   - **`-F 32`**: Specifies FAT32 format.
   - **`/dev/loop0p1`**: Refers to the first partition of the loop device.

---

## **4. Verify the Partition and Mount It**

### **What is Mounting?**
Mounting makes a filesystem accessible in the system's directory hierarchy.

### **Steps to Mount and Verify:**
1. **Create a Mount Point:**
   ```bash
   mkdir -p /mnt/uefi_disk
   ```
   - **`mkdir -p`**: Creates the directory and parent directories if they don't exist.

2. **Mount the Partition:**
   ```bash
   sudo mount /dev/loop0p1 /mnt/uefi_disk
   ```
   - **`mount`**: Attaches the filesystem to the directory.
   - **`/dev/loop0p1`**: The partition to mount.
   - **`/mnt/uefi_disk`**: The directory where the filesystem will be accessible.

3. **Verify the Mount:**
   ```bash
   df -h | grep /mnt/uefi_disk
   ```
   - **`df -h`**: Shows mounted filesystems in human-readable format.

---

## **5. Create the UEFI Boot Directory**

### **Steps to Create the Directory:**
1. **Navigate to the Mounted Partition:**
   ```bash
   cd /mnt/uefi_disk
   ```

2. **Create the Directory Path:**
   ```bash
   mkdir -p EFI/BOOT
   ```
   - **`mkdir -p`**: Creates the `EFI/BOOT` directory structure without requiring elevated privileges.

---

## **6. Copy and Rename the EFI File**

### **Steps to Copy and Rename:**
1. **Copy `main.efi` to the Boot Directory:**
   ```bash
   cp /path/to/main.efi /mnt/uefi_disk/EFI/BOOT/
   ```
   - **`cp`**: Copies files to the target directory.

2. **Rename the File to `BOOTX64.EFI`:**
   ```bash
   mv /mnt/uefi_disk/EFI/BOOT/main.efi /mnt/uefi_disk/EFI/BOOT/BOOTX64.EFI
   ```
   - **`mv`**: Moves or renames files.

### **Why Rename to BOOTX64.EFI?**
UEFI firmware expects the bootloader or application to be named `BOOTX64.EFI` for compatibility and automatic execution.

---

## **7. Verify and Run the Application**

### **Unmount the Partition:**
```bash
sudo umount /mnt/uefi_disk
```
- **Why Unmount?**: Unmounting ensures that all changes to the filesystem are written back to the disk image and prevents conflicts when the image is accessed later (e.g., by QEMU).

### **Detach the Loop Device:**
```bash
sudo losetup -d /dev/loop0
```
- **Why Detach?**: Detaching the loop device frees system resources and ensures no lingering processes are accessing the image. This step is crucial before running QEMU to avoid conflicts.

### **Run QEMU with the Disk:**
```bash
qemu-system-x86_64 \
  -bios /usr/share/OVMF/OVMF.fd \
  -drive file=uefi_disk.img,format=raw,if=virtio \
  -net none \
  -nographic
```

### **Check Output in the UEFI Shell:**
1. **Navigate to the Boot File:**
   ```shell
   fs0:\EFI\BOOT\BOOTX64.EFI
   ```
2. **Execute the File:**
   ```shell
   fs0:\EFI\BOOT\BOOTX64.EFI
   ```
   - You should see the output:
     ```
     Hello, world!
     ```

---

## **Summary**

This guide detailed how to:
1. Create and partition a virtual disk using `gdisk`.
2. Format the partition to FAT32 with `mkfs.vfat`.
3. Mount the partition to verify correctness.
4. Create the required UEFI directory structure.
5. Copy and rename the UEFI application file.
6. Test the application using QEMU.

By following these steps, you now understand how to prepare and run a UEFI application on a properly configured virtual disk.
