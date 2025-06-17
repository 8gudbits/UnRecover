# UnRecover

UnRecover is a **drive-type aware secure wiping tool** that permanently erases data from **HDDs, SSDs, and USB drives**. It detects the drive type and applies the most effective wiping method to ensure data cannot be recovered.

## How Does Secure Wiping Work?

When you delete a file, the data is **not immediately erased**, only the reference to it is removed. Specialized recovery tools can restore deleted files unless they are **overwritten multiple times**.

### **Different Wiping Methods for Different Drives**

- **HDD (Hard Disk Drive)** → Uses **multiple overwrite passes** (`cipher /w`) to ensure data is unrecoverable.
- **SSD (Solid State Drive)** → Uses **TRIM** (`defrag /L /O`) to optimize flash memory before and after wiping.
- **USB (Flash Drives)** → Uses **quick format** (`format /Q`) followed by **multiple overwrites**.

## Usage

```bash
UnRecover <drive letter> [options]
```

### Options

- `-p, --pass <number>` Set the number of wipe passes (1-99, default: 3).
- `-s, --suppress-warning` Skip confirmation prompts.
- `-n, --nobanner` Suppress the banner output.
- `-h, --help` Display this help message.

## Download exe for Windows

This tool is part of the [8gudbitsKit](https://github.com/8gudbits/8gudbitsKit) project. To download the executable for Windows, visit the [8gudbitsKit](https://github.com/8gudbits/8gudbitsKit) repository.

## For the Tech People

- Detects **drive type** using `DeviceIoControl()` and `IOCTL_STORAGE_QUERY_PROPERTY`.
- Uses **cipher** for **secure overwriting** on HDDs and USB drives.
- Optimizes SSD wiping with **TRIM commands** (`defrag /L /O`).
- Ensures **safe drive formatting** for USB drives (`format /FS:FAT32 /Q`).

Note: This tool is intended for advanced users and should be used with caution.
