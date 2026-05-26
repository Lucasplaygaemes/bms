#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BATTERY_CAPACITY "/sys/class/power_supply/BAT0/capacity"
#define RAPL_PL1_PATH "/sys/class/powercap/intel-rapl/intel-rapl:0/constraint_0_power_limit_uw"
#define MAXIMUM_CLOCK_PATH "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq"

// Global Variables
int current_battery;
unsigned long long default_tdp_pl1;
unsigned long default_max_clock;
long nprocs;

/**
 * Helper function to write an unsigned long long value to a sysfs path.
 */
int write_sysfs_ull(const char *path, unsigned long long value) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("Error writing to sysfs (try running with sudo)");
        return -1;
    }
    fprintf(file, "%llu", value);
    fclose(file);
    return 0;
}

/**
 * Helper function to read an unsigned long long value from a sysfs path.
 */
unsigned long long read_sysfs_ull(const char *path) {
    unsigned long long value = 0;
    FILE *file = fopen(path, "r");
    if (file) {
        if (fscanf(file, "%llu", &value) != 1) {
            value = 0;
        }
        fclose(file);
    }
    return value;
}

/**
 * Backs up the current system power and clock defaults.
 */
void backup_system_defaults() {
    default_tdp_pl1 = read_sysfs_ull(RAPL_PL1_PATH);
    
    char path[128];
    sprintf(path, MAXIMUM_CLOCK_PATH, 0);
    default_max_clock = (unsigned long)read_sysfs_ull(path);
    
    if (default_tdp_pl1 == 0 || default_max_clock == 0) {
        fprintf(stderr, "Warning: Could not read default system values. Some features might not work.\n");
    }
}

/**
 * Sets the CPU maximum clock limit across all cores.
 * @param percentage The factor to multiply the default max clock by (0.0 to 1.0).
 */
void set_cpu_clock_limit(double percentage) {
    unsigned long new_clock = (unsigned long)(default_max_clock * percentage);
    for (int i = 0; i < nprocs; i++) {
        char path[128];
        sprintf(path, MAXIMUM_CLOCK_PATH, i);
        write_sysfs_ull(path, new_clock);
    }
    printf("CPU Clock capped at %.0f%%: %lu kHz\n", percentage * 100, new_clock);
}

/**
 * Applies a specific power saving mode.
 */
void apply_power_mode(const char *mode) {
    if (strcmp(mode, "nsave") == 0) {
        printf("Applying Mode: No Power Save (Performance)\n");
        write_sysfs_ull(RAPL_PL1_PATH, default_tdp_pl1);
        set_cpu_clock_limit(1.0);
    } else if (strcmp(mode, "msave") == 0) {
        printf("Applying Mode: Medium Power Save\n");
        write_sysfs_ull(RAPL_PL1_PATH, default_tdp_pl1 * 0.5);
        set_cpu_clock_limit(0.5);
    } else if (strcmp(mode, "hsave") == 0) {
        printf("Applying Mode: High Power Save\n");
        write_sysfs_ull(RAPL_PL1_PATH, default_tdp_pl1 * 0.2);
        set_cpu_clock_limit(0.2);
    } else {
        printf("Unknown mode: %s\n", mode);
    }
}

int main(int argc, char *argv[]) {
    // Check for root privileges
    if (geteuid() != 0) {
        fprintf(stderr, "This program must be run as root (sudo).\n");
        return 1;
    }

    nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs < 1) {
        nprocs = 1; // Fallback
    }

    backup_system_defaults();
    
    // Read battery capacity
    current_battery = (int)read_sysfs_ull(BATTERY_CAPACITY);
    printf("Current Battery Level: %d%%\n", current_battery);

    if (argc < 2) {
        printf("Usage: %s [nsave|msave|hsave|auto]\n", argv[0]);
        printf("Options:\n");
        printf("  nsave - No power saving (Default limits)\n");
        printf("  msave - Medium power saving (50%% limit)\n");
        printf("  hsave - High power saving (20%% limit)\n");
        printf("  auto  - Automatically select mode based on battery level\n");
        return 1;
    }

    if (strcmp(argv[1], "auto") == 0) {
        if (current_battery >= 75) {
            apply_power_mode("nsave");
        } else if (current_battery >= 40) {
            apply_power_mode("msave");
        } else {
            apply_power_mode("hsave");
        }
    } else {
        apply_power_mode(argv[1]);
    }

    printf("Operation complete.\n");
    return 0;
}
