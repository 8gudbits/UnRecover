#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <winioctl.h>
#include <ntddscsi.h>

#define DRIVE_TYPE_UNKNOWN 0
#define DRIVE_TYPE_HDD 1
#define DRIVE_TYPE_SSD 2
#define DRIVE_TYPE_USB 3

// Detect the type of drive (HDD, SSD, or USB)
int detect_drive_type(char drive_letter)
{
    char drive_path[10];
    sprintf(drive_path, "%c:\\", drive_letter);

    UINT drive_type = GetDriveTypeA(drive_path);

    // USB drives are easily identifiable
    if (drive_type == DRIVE_REMOVABLE)
        return DRIVE_TYPE_USB;

    // For fixed drives, we need deeper inspection
    char device_path[20];
    sprintf(device_path, "\\\\.\\%c:", drive_letter);

    HANDLE hDevice = CreateFileA(device_path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE)
        return DRIVE_TYPE_UNKNOWN;

    STORAGE_PROPERTY_QUERY query = {0};
    query.PropertyId = StorageDeviceSeekPenaltyProperty;
    query.QueryType = PropertyStandardQuery;

    DEVICE_SEEK_PENALTY_DESCRIPTOR result = {0};
    DWORD bytesReturned = 0;

    BOOL success = DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
                                   &query, sizeof(query), &result, sizeof(result),
                                   &bytesReturned, NULL);
    CloseHandle(hDevice);

    return (success) ? (result.IncursSeekPenalty ? DRIVE_TYPE_HDD : DRIVE_TYPE_SSD)
                     : DRIVE_TYPE_UNKNOWN;
}

// Wipe HDD with multiple passes
int wipe_hdd(char drive_letter, int pass_count)
{
    printf("Starting HDD wipe on %c: drive...\n", drive_letter);
    for (int i = 1; i <= pass_count; i++)
    {
        printf("Pass %d/%d...\n", i, pass_count);
        char command[50];
        sprintf(command, "cipher /w:%c:", drive_letter);
        if (system(command))
        {
            fprintf(stderr, "Error during pass %d\n", i);
            return -1;
        }
    }
    return 0;
}

// Optimized SSD wiping procedure
int wipe_ssd(char drive_letter)
{
    printf("Starting optimized SSD wipe on %c: drive...\n", drive_letter);

    // First TRIM operation
    printf("Running initial TRIM...\n");
    char trim_cmd[50];
    sprintf(trim_cmd, "defrag /L /O %c:", drive_letter);
    if (system(trim_cmd))
        return -1;

    // Secure wipe
    printf("Performing secure wipe...\n");
    char cipher_cmd[50];
    sprintf(cipher_cmd, "cipher /w:%c:", drive_letter);
    if (system(cipher_cmd))
        return -1;

    // Final TRIM
    printf("Running final TRIM...\n");
    if (system(trim_cmd))
        return -1;

    return 0;
}

// USB drive wiping procedure
int wipe_usb(char drive_letter, int pass_count)
{
    printf("Starting USB drive wipe on %c: drive...\n", drive_letter);

    // Step 1: Perform a quick format first
    printf("Performing initial format...\n");
    char format_cmd[100];
    sprintf(format_cmd, "format %c: /FS:FAT32 /Q /V:ERASED", drive_letter);
    if (system(format_cmd))
    {
        fprintf(stderr, "Error during format operation.\n");
        return -1;
    }

    // Step 2: Multiple overwrite passes after formatting
    for (int i = 1; i <= pass_count; i++)
    {
        printf("Overwrite pass %d/%d...\n", i, pass_count);
        char cipher_cmd[50];
        sprintf(cipher_cmd, "cipher /w:%c:", drive_letter);
        if (system(cipher_cmd))
        {
            fprintf(stderr, "Error during pass %d\n", i);
            return -1;
        }
    }

    printf("\nUSB wipe complete.\n");
    return 0;
}

// Check if drive is valid for wiping
int check_drive(char *drive_path)
{
    UINT drive_type = GetDriveTypeA(drive_path);
    return (drive_type == DRIVE_FIXED || drive_type == DRIVE_REMOVABLE);
}

void show_banner()
{
    printf("\nUnRecover 2.0 (x64) : (c) 8gudbits - All rights reserved.\n");
    printf("Source - \"https://github.com/8gudbits/8gudbitsKit\"\n");
    printf("Drive-type aware secure wiping tool\n\n");
}

void show_help(char *program_name)
{
    show_banner();
    printf("Usage: %s <drive letter> [options]\n\n", program_name);
    printf("Options:\n");
    printf("  -n, --nobanner            Hide banner\n");
    printf("  -s, --suppress-warning    Skip confirmation prompts\n");
    printf("  -p, --pass <number>       Set number of wipe passes (1-99, default:3)\n");
    printf("  -h, --help                Show this help menu\n\n");
    printf("Examples:\n");
    printf("  %s C --pass 5    # Wipe C: drive with 5 passes\n", program_name);
    printf("  %s D -s          # Wipe D: drive without confirmation\n", program_name);
}

int main(int argc, char *argv[])
{
    int show_banner_flag = 1;
    int suppress_warning = 0;
    int pass_count = 3;
    int drive_letter_found = 0;
    char drive_path[4] = {0};
    char drive_letter = 0;

    // Parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "--nobanner") || !strcmp(argv[i], "-n"))
        {
            show_banner_flag = 0;
        }
        else if (!strcmp(argv[i], "--suppress-warning") || !strcmp(argv[i], "-s"))
        {
            suppress_warning = 1;
        }
        else if (!strcmp(argv[i], "--pass") || !strcmp(argv[i], "-p"))
        {
            if (i + 1 < argc)
            {
                pass_count = atoi(argv[++i]);
                if (pass_count < 1 || pass_count > 99)
                    pass_count = 3;
            }
        }
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
        {
            show_help(argv[0]);
            return 0;
        }
        else if (isalpha(argv[i][0]))
        {
            drive_letter = toupper(argv[i][0]);
            drive_letter_found = 1;
            snprintf(drive_path, sizeof(drive_path), "%c:\\", drive_letter);
        }
    }

    // Validate we got a drive letter
    if (!drive_letter_found)
    {
        fprintf(stderr, "ERROR: No drive letter specified\n");
        show_help(argv[0]);
        return 1;
    }

    if (show_banner_flag)
        show_banner();

    // Check drive validity
    if (!check_drive(drive_path))
    {
        fprintf(stderr, "\nERROR: Unsupported drive type\n");
        return 1;
    }

    // Detect drive type
    int drive_type = detect_drive_type(drive_letter);
    if (drive_type == DRIVE_TYPE_UNKNOWN)
    {
        fprintf(stderr, "\nERROR: Could not determine drive type\n");
        return 1;
    }

    // Show warning unless suppressed
    if (!suppress_warning)
    {
        printf("WARNING: This will PERMANENTLY ERASE ALL DATA on drive %c:\n", drive_letter);
        printf("Type 'YES' to confirm and continue: ");

        char confirmation[10];
        if (scanf("%9s", confirmation) != 1 || strcmp(confirmation, "YES") != 0)
        {
            printf("Aborted.\n");
            return 0;
        }
    }

    // Perform the appropriate wipe
    int result = -1;
    switch (drive_type)
    {
    case DRIVE_TYPE_HDD:
        result = wipe_hdd(drive_letter, pass_count);
        break;
    case DRIVE_TYPE_SSD:
        result = wipe_ssd(drive_letter);
        break;
    case DRIVE_TYPE_USB:
        result = wipe_usb(drive_letter, pass_count);
        break;
    }

    printf(result == 0 ? "\nWIPE COMPLETE\n" : "\nWIPE FAILED\n");
    return result;
}
