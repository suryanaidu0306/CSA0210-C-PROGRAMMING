/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *    Description:  All-in-One Vehicle Rental Centre Management System
 *         Author:  Antigravity AI
 *        Version:  1.0
 *       Standard:  C99 / C11 compatible (Cross-Platform: Windows / Linux / macOS)
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#define strcasecmp _stricmp
#endif

/* ========================================================================= */
/*                         CONSTANTS & MACROS                                */
/* ========================================================================= */

#define MAX_VEHICLES     200
#define MAX_CUSTOMERS    300
#define MAX_RENTALS      500
#define MAX_MAINTENANCE  300
#define MAX_TRANSACTIONS 1000
#define MAX_USERS        20

#define FILE_VEHICLES     "vehicles.dat"
#define FILE_CUSTOMERS    "customers.dat"
#define FILE_RENTALS      "rentals.dat"
#define FILE_MAINTENANCE  "maintenance.dat"
#define FILE_TRANSACTIONS "transactions.dat"
#define FILE_USERS        "users.dat"

/* ANSI Colors */
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_WHITE   "\033[1;37m"

/* Daily add-on pricing */
#define RATE_BASIC_INSURANCE  15.00
#define RATE_FULL_INSURANCE   30.00
#define RATE_GPS              8.00
#define RATE_CHILD_SEAT       10.00
#define RATE_EXTRA_DRIVER     12.00
#define FREE_KM_PER_DAY       250.0
#define EXCESS_KM_RATE        0.35
#define LATE_PENALTY_PER_DAY  50.00

/* ========================================================================= */
/*                             DATA STRUCTURES                               */
/* ========================================================================= */

typedef struct {
    int insurance_type;   /* 0 = None, 1 = Basic ($15/day), 2 = Full Coverage ($30/day) */
    int gps;              /* 1 = Yes ($8/day), 0 = No */
    int child_seat;       /* 1 = Yes ($10/day), 0 = No */
    int extra_driver;     /* 1 = Yes ($12/day), 0 = No */
} AddOnOptions;

typedef struct {
    int id;
    char plate[20];
    char brand[30];
    char model[30];
    int year;
    char category[25];    /* Sedan, SUV, Luxury, Minivan, Motorcycle, Commercial */
    char transmission[15];/* Automatic, Manual */
    char fuel_type[15];   /* Petrol, Diesel, Electric, Hybrid */
    int seats;
    double daily_rate;
    double hourly_rate;
    double deposit_req;
    double mileage;       /* Current Odometer in KM */
    char status[20];      /* AVAILABLE, RENTED, MAINTENANCE, RESERVED */
    int is_deleted;
} Vehicle;

typedef struct {
    int id;
    char name[60];
    char phone[25];
    char email[60];
    char id_passport[30];
    char license_num[30];
    char license_expiry[15];
    char address[100];
    int total_rentals;
    double total_spent;
    int is_deleted;
} Customer;

typedef struct {
    int booking_id;
    int customer_id;
    char customer_name[60];
    char customer_phone[25];
    int vehicle_id;
    char vehicle_info[70];
    char plate[20];
    char start_date[20];
    char expected_return_date[20];
    char actual_return_date[20];
    int duration_days;
    double daily_rate;
    double base_cost;
    AddOnOptions addons;
    double addons_cost;
    double deposit_paid;
    double start_mileage;
    double return_mileage;
    double excess_mileage_fee;
    double late_fee;
    double damage_fee;
    double fuel_charge;
    double total_cost;
    double balance_settled; /* Positive: Customer paid extra, Negative: Deposit refunded */
    char status[20];        /* ACTIVE, COMPLETED, CANCELLED */
    char notes[120];
} RentalBooking;

typedef struct {
    int log_id;
    int vehicle_id;
    char vehicle_info[70];
    char date[20];
    char service_type[50];
    double cost;
    char description[120];
    double odometer_at_service;
    char vendor_mechanic[60];
} MaintenanceLog;

typedef struct {
    int trans_id;
    char date[20];
    char type[35];          /* RENTAL_DEPOSIT_ADVANCE, FINAL_SETTLEMENT, REFUND_ISSUED, MAINTENANCE_EXPENSE */
    int booking_id;
    int vehicle_id;
    double amount;
    char description[120];
} Transaction;

typedef struct {
    char username[30];
    char password[30];
    char full_name[60];
    char role[15];          /* ADMIN, STAFF */
} UserAccount;

/* ========================================================================= */
/*                          GLOBAL STATE & INVENTORIES                       */
/* ========================================================================= */

Vehicle vehicles[MAX_VEHICLES];
int vehicle_count = 0;

Customer customers[MAX_CUSTOMERS];
int customer_count = 0;

RentalBooking rentals[MAX_RENTALS];
int rental_count = 0;

MaintenanceLog maintenance_logs[MAX_MAINTENANCE];
int maintenance_count = 0;

Transaction transactions[MAX_TRANSACTIONS];
int transaction_count = 0;

UserAccount users[MAX_USERS];
int user_count = 0;

UserAccount current_user;
int is_authenticated = 0;

/* ========================================================================= */
/*                             FUNCTION PROTOTYPES                           */
/* ========================================================================= */

/* Setup & Utilities */
void initConsoleColors(void);
void getCurrentDateStr(char *buffer, size_t size);
void clearScreen(void);
void pausePrompt(void);
void drawHeader(const char *title);
void drawSubHeader(const char *subtitle);
int getIntInput(const char *prompt, int min_val, int max_val);
double getDoubleInput(const char *prompt, double min_val, double max_val);
void getStringInput(const char *prompt, char *dest, size_t max_len);
void formatCurrency(char *buffer, size_t size, double amount);

/* Storage & Persistence */
void loadAllData(void);
void saveAllData(void);
void seedInitialDataIfEmpty(void);

/* Authentication */
int loginMenu(void);
void manageUsers(void);
void changePassword(void);

/* Fleet Inventory Management */
void fleetMenu(void);
void viewAllVehicles(void);
void addVehicle(void);
void editVehicle(void);
void deleteVehicle(void);
void searchVehicles(void);
void viewAvailableVehicles(void);
int findVehicleIndexById(int id);
int findVehicleIndexByPlate(const char *plate);

/* Customer CRM */
void customerMenu(void);
void viewAllCustomers(void);
void addCustomer(void);
void editCustomer(void);
void searchCustomers(void);
void viewCustomerHistory(void);
int findCustomerIndexById(int id);
int findCustomerIndexByLicense(const char *license);

/* Rental Operations (Check-Out & Check-In) */
void rentalOperationsMenu(void);
void checkOutVehicle(void);
void checkInVehicle(void);
void viewActiveRentals(void);
void viewAllRentalHistory(void);
void printRentalContractReceipt(const RentalBooking *b);
void printFinalSettlementInvoice(const RentalBooking *b);
int findRentalIndexById(int id);

/* Maintenance Management */
void maintenanceMenu(void);
void logMaintenanceRecord(void);
void viewMaintenanceLogs(void);
void fleetHealthAlerts(void);

/* Billing, POS & Business Analytics */
void billingReportsMenu(void);
void viewFinancialSummary(void);
void viewTransactionLedger(void);
void exportDailyBusinessReport(void);

/* Main Dashboard */
void mainDashboard(void);

/* ========================================================================= */
/*                               MAIN ENTRY POINT                            */
/* ========================================================================= */

int main(void) {
    initConsoleColors();
    loadAllData();
    seedInitialDataIfEmpty();

    clearScreen();
    printf(COLOR_CYAN);
    printf("================================================================================\n");
    printf("        *** WELCOME TO APEX AUTO WHEELS - VEHICLE RENTAL CENTRE ***             \n");
    printf("            Enterprise Fleet & Rental Operations Management Software            \n");
    printf("================================================================================\n");
    printf(COLOR_RESET);

    if (loginMenu()) {
        mainDashboard();
    }

    saveAllData();
    clearScreen();
    printf(COLOR_GREEN "\nThank you for using Apex Auto Wheels Management System. Have a productive day!\n\n" COLOR_RESET);
    return 0;
}

/* ========================================================================= */
/*                          UTILITY & UI FUNCTIONS                           */
/* ========================================================================= */

void initConsoleColors(void) {
#ifdef _WIN32
    /* Enable Virtual Terminal Processing for ANSI escape codes on Windows 10/11 */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= 0x0004; /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

void getCurrentDateStr(char *buffer, size_t size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, size, "%Y-%m-%d", tm_info);
}

void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausePrompt(void) {
    printf(COLOR_YELLOW "\nPress [ENTER] to return to the menu..." COLOR_RESET);
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void drawHeader(const char *title) {
    clearScreen();
    printf(COLOR_BLUE);
    printf("================================================================================\n");
    printf("   APEX VEHICLE RENTAL CENTRE  |  User: %s (%s)\n", current_user.full_name, current_user.role);
    printf("================================================================================\n" COLOR_RESET);
    printf(COLOR_CYAN COLOR_BOLD " >> %s\n" COLOR_RESET, title);
    printf(COLOR_BLUE "--------------------------------------------------------------------------------\n" COLOR_RESET);
}

void drawSubHeader(const char *subtitle) {
    printf(COLOR_MAGENTA "\n--- [ %s ] ---\n" COLOR_RESET, subtitle);
}

int getIntInput(const char *prompt, int min_val, int max_val) {
    char buffer[100];
    int value;
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            char *endptr;
            value = (int)strtol(buffer, &endptr, 10);
            if (endptr != buffer && (*endptr == '\n' || *endptr == '\0')) {
                if (value >= min_val && value <= max_val) {
                    return value;
                }
            }
        }
        printf(COLOR_RED "  [!] Invalid input. Please enter an integer between %d and %d.\n" COLOR_RESET, min_val, max_val);
    }
}

double getDoubleInput(const char *prompt, double min_val, double max_val) {
    char buffer[100];
    double value;
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            char *endptr;
            value = strtod(buffer, &endptr);
            if (endptr != buffer && (*endptr == '\n' || *endptr == '\0')) {
                if (value >= min_val && value <= max_val) {
                    return value;
                }
            }
        }
        printf(COLOR_RED "  [!] Invalid input. Please enter a numerical value between %.2f and %.2f.\n" COLOR_RESET, min_val, max_val);
    }
}

void getStringInput(const char *prompt, char *dest, size_t max_len) {
    char buffer[512];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            /* Trim trailing newline */
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
                len--;
            }
            /* Trim leading spaces */
            char *start = buffer;
            while (*start == ' ' || *start == '\t') start++;

            if (strlen(start) > 0) {
                strncpy(dest, start, max_len - 1);
                dest[max_len - 1] = '\0';
                return;
            }
        }
        printf(COLOR_RED "  [!] Input cannot be empty. Please try again.\n" COLOR_RESET);
    }
}

void formatCurrency(char *buffer, size_t size, double amount) {
    snprintf(buffer, size, "$%.2f", amount);
}

/* ========================================================================= */
/*                        STORAGE & PERSISTENCE                              */
/* ========================================================================= */

void loadAllData(void) {
    FILE *f;

    /* Users */
    f = fopen(FILE_USERS, "rb");
    if (f) {
        fread(&user_count, sizeof(int), 1, f);
        fread(users, sizeof(UserAccount), user_count, f);
        fclose(f);
    }

    /* Vehicles */
    f = fopen(FILE_VEHICLES, "rb");
    if (f) {
        fread(&vehicle_count, sizeof(int), 1, f);
        fread(vehicles, sizeof(Vehicle), vehicle_count, f);
        fclose(f);
    }

    /* Customers */
    f = fopen(FILE_CUSTOMERS, "rb");
    if (f) {
        fread(&customer_count, sizeof(int), 1, f);
        fread(customers, sizeof(Customer), customer_count, f);
        fclose(f);
    }

    /* Rentals */
    f = fopen(FILE_RENTALS, "rb");
    if (f) {
        fread(&rental_count, sizeof(int), 1, f);
        fread(rentals, sizeof(RentalBooking), rental_count, f);
        fclose(f);
    }

    /* Maintenance */
    f = fopen(FILE_MAINTENANCE, "rb");
    if (f) {
        fread(&maintenance_count, sizeof(int), 1, f);
        fread(maintenance_logs, sizeof(MaintenanceLog), maintenance_count, f);
        fclose(f);
    }

    /* Transactions */
    f = fopen(FILE_TRANSACTIONS, "rb");
    if (f) {
        fread(&transaction_count, sizeof(int), 1, f);
        fread(transactions, sizeof(Transaction), transaction_count, f);
        fclose(f);
    }
}

void saveAllData(void) {
    FILE *f;

    /* Users */
    f = fopen(FILE_USERS, "wb");
    if (f) {
        fwrite(&user_count, sizeof(int), 1, f);
        fwrite(users, sizeof(UserAccount), user_count, f);
        fclose(f);
    }

    /* Vehicles */
    f = fopen(FILE_VEHICLES, "wb");
    if (f) {
        fwrite(&vehicle_count, sizeof(int), 1, f);
        fwrite(vehicles, sizeof(Vehicle), vehicle_count, f);
        fclose(f);
    }

    /* Customers */
    f = fopen(FILE_CUSTOMERS, "wb");
    if (f) {
        fwrite(&customer_count, sizeof(int), 1, f);
        fwrite(customers, sizeof(Customer), customer_count, f);
        fclose(f);
    }

    /* Rentals */
    f = fopen(FILE_RENTALS, "wb");
    if (f) {
        fwrite(&rental_count, sizeof(int), 1, f);
        fwrite(rentals, sizeof(RentalBooking), rental_count, f);
        fclose(f);
    }

    /* Maintenance */
    f = fopen(FILE_MAINTENANCE, "wb");
    if (f) {
        fwrite(&maintenance_count, sizeof(int), 1, f);
        fwrite(maintenance_logs, sizeof(MaintenanceLog), maintenance_count, f);
        fclose(f);
    }

    /* Transactions */
    f = fopen(FILE_TRANSACTIONS, "wb");
    if (f) {
        fwrite(&transaction_count, sizeof(int), 1, f);
        fwrite(transactions, sizeof(Transaction), transaction_count, f);
        fclose(f);
    }
}

void seedInitialDataIfEmpty(void) {
    /* Default Users */
    if (user_count == 0) {
        strcpy(users[0].username, "admin");
        strcpy(users[0].password, "admin123");
        strcpy(users[0].full_name, "Master Administrator");
        strcpy(users[0].role, "ADMIN");

        strcpy(users[1].username, "staff");
        strcpy(users[1].password, "staff123");
        strcpy(users[1].full_name, "Front Desk Officer");
        strcpy(users[1].role, "STAFF");

        user_count = 2;
    }

    /* Default Fleet Inventory */
    if (vehicle_count == 0) {
        Vehicle sample_vehicles[] = {
            {101, "CAR-8821", "Toyota", "Camry SE", 2023, "Sedan", "Automatic", "Petrol", 5, 55.00, 10.00, 200.00, 18500.0, "AVAILABLE", 0},
            {102, "SUV-4930", "Ford", "Explorer XLT", 2023, "SUV", "Automatic", "Petrol", 7, 85.00, 15.00, 300.00, 24200.0, "AVAILABLE", 0},
            {103, "LUX-1102", "BMW", "530i M-Sport", 2024, "Luxury", "Automatic", "Hybrid", 5, 140.00, 25.00, 500.00, 9400.0, "AVAILABLE", 0},
            {104, "EV-7741", "Tesla", "Model 3 Long Range", 2023, "Sedan", "Automatic", "Electric", 5, 75.00, 14.00, 300.00, 15300.0, "AVAILABLE", 0},
            {105, "SUV-9912", "Toyota", "RAV4 Hybrid", 2022, "SUV", "Automatic", "Hybrid", 5, 65.00, 12.00, 250.00, 31200.0, "RENTED", 0},
            {106, "VAN-3345", "Mercedes", "Sprinter Passenger", 2022, "Minivan", "Automatic", "Diesel", 12, 130.00, 22.00, 400.00, 42000.0, "AVAILABLE", 0},
            {107, "BIK-6619", "Yamaha", "MT-07", 2023, "Motorcycle", "Manual", "Petrol", 2, 45.00, 8.00, 150.00, 6800.0, "AVAILABLE", 0},
            {108, "TRK-5520", "Ford", "F-150 SuperCrew", 2022, "Commercial", "Automatic", "Diesel", 5, 95.00, 18.00, 350.00, 38600.0, "AVAILABLE", 0},
            {109, "CAR-2219", "Honda", "Civic Touring", 2023, "Sedan", "Automatic", "Petrol", 5, 50.00, 9.00, 200.00, 21000.0, "AVAILABLE", 0},
            {110, "LUX-7788", "Porsche", "Macan GTS", 2024, "Luxury", "Automatic", "Petrol", 5, 180.00, 30.00, 600.00, 5100.0, "MAINTENANCE", 0}
        };
        int n = sizeof(sample_vehicles) / sizeof(sample_vehicles[0]);
        for (int i = 0; i < n; i++) {
            vehicles[i] = sample_vehicles[i];
        }
        vehicle_count = n;
    }

    /* Default Customers */
    if (customer_count == 0) {
        Customer sample_customers[] = {
            {201, "Alexander Wright", "+1-555-0143", "a.wright@example.com", "US-P987123", "DL-TX-99882", "2027-10-15", "742 Evergreen Terrace, Austin TX", 4, 1350.00, 0},
            {202, "Sophia Martinez", "+1-555-0188", "sophia.m@example.com", "US-P443321", "DL-CA-44551", "2028-04-20", "1204 Sunset Blvd, Los Angeles CA", 2, 620.00, 0},
            {203, "David Chen", "+1-555-0199", "chen.d@example.com", "US-P665544", "DL-NY-11223", "2026-11-30", "45 Wall St, New York NY", 1, 385.00, 0}
        };
        int n = sizeof(sample_customers) / sizeof(sample_customers[0]);
        for (int i = 0; i < n; i++) {
            customers[i] = sample_customers[i];
        }
        customer_count = n;
    }

    /* Default Sample Active Rental (matches vehicle 105) */
    if (rental_count == 0) {
        RentalBooking sample_rental = {
            301, 202, "Sophia Martinez", "+1-555-0188",
            105, "Toyota RAV4 Hybrid", "SUV-9912",
            "2026-09-01", "2026-09-05", "",
            4, 65.00, 260.00,
            {1, 1, 0, 0}, 92.00, /* Basic Insurance ($60) + GPS ($32) */
            250.00,
            31200.0, 0.0,
            0.0, 0.0, 0.0, 0.0,
            352.00, 0.0,
            "ACTIVE", "Customer requested early pickup."
        };
        rentals[0] = sample_rental;
        rental_count = 1;

        Transaction t1 = {
            1001, "2026-09-01", "RENTAL_DEPOSIT_ADVANCE", 301, 105, 250.00,
            "Security deposit collected for Booking #301 (SUV-9912)"
        };
        transactions[0] = t1;
        transaction_count = 1;
    }

    /* Default Maintenance Log */
    if (maintenance_count == 0) {
        MaintenanceLog m1 = {
            401, 110, "Porsche Macan GTS [LUX-7788]",
            "2026-08-28", "Brake Pad & Rotor Replacement",
            420.00, "OEM performance brake pads installed at authorized service centre",
            5100.0, "Porsche Centre North"
        };
        maintenance_logs[0] = m1;
        maintenance_count = 1;

        Transaction t2 = {
            1002, "2026-08-28", "MAINTENANCE_EXPENSE", 0, 110, -420.00,
            "Maintenance expense for Porsche Macan GTS (Log #401)"
        };
        transactions[transaction_count++] = t2;
    }

    saveAllData();
}

/* ========================================================================= */
/*                          AUTHENTICATION MODULE                            */
/* ========================================================================= */

int loginMenu(void) {
    char u_input[30];
    char p_input[30];
    int attempts = 0;

    while (attempts < 3) {
        drawSubHeader("SYSTEM LOGIN");
        printf(COLOR_WHITE "Enter administrative or staff credentials to proceed.\n" COLOR_RESET);
        printf("Default Admin: " COLOR_YELLOW "admin / admin123" COLOR_RESET "  |  Default Staff: " COLOR_YELLOW "staff / staff123\n\n" COLOR_RESET);

        getStringInput("Username: ", u_input, sizeof(u_input));
        getStringInput("Password: ", p_input, sizeof(p_input));

        for (int i = 0; i < user_count; i++) {
            if (strcmp(users[i].username, u_input) == 0 && strcmp(users[i].password, p_input) == 0) {
                current_user = users[i];
                is_authenticated = 1;
                printf(COLOR_GREEN "\n[+] Login successful! Welcome, %s (%s).\n" COLOR_RESET, current_user.full_name, current_user.role);
                printf("Loading business workspace...\n");
#ifdef _WIN32
                Sleep(800);
#endif
                return 1;
            }
        }

        attempts++;
        printf(COLOR_RED "\n[-] Invalid username or password. Attempt %d of 3.\n\n" COLOR_RESET, attempts);
    }

    printf(COLOR_RED "\n[!] Maximum login attempts exceeded. Access restricted for security.\n" COLOR_RESET);
    return 0;
}

void manageUsers(void) {
    if (strcmp(current_user.role, "ADMIN") != 0) {
        printf(COLOR_RED "\n[!] Access Denied: Only Administrator accounts can manage users.\n" COLOR_RESET);
        pausePrompt();
        return;
    }

    while (1) {
        drawHeader("USER ACCOUNT MANAGEMENT (ADMIN ONLY)");
        printf(COLOR_CYAN "%-5s | %-18s | %-25s | %-10s\n", "No.", "Username", "Full Name", "Role");
        printf("--------------------------------------------------------------------------------\n" COLOR_RESET);
        for (int i = 0; i < user_count; i++) {
            printf("%-5d | %-18s | %-25s | %-10s\n", i + 1, users[i].username, users[i].full_name, users[i].role);
        }
        printf("--------------------------------------------------------------------------------\n");
        printf("1. Create New Staff/Admin Account\n");
        printf("2. Reset User Password\n");
        printf("3. Back to Main Dashboard\n");

        int choice = getIntInput("\nSelect option (1-3): ", 1, 3);
        if (choice == 3) break;

        if (choice == 1) {
            if (user_count >= MAX_USERS) {
                printf(COLOR_RED "[!] Maximum user limit reached.\n" COLOR_RESET);
                pausePrompt();
                continue;
            }
            UserAccount newUser;
            getStringInput("Enter New Username: ", newUser.username, sizeof(newUser.username));
            
            /* Check uniqueness */
            int exists = 0;
            for (int i = 0; i < user_count; i++) {
                if (strcmp(users[i].username, newUser.username) == 0) {
                    exists = 1;
                    break;
                }
            }
            if (exists) {
                printf(COLOR_RED "[!] Username already taken. Please choose another.\n" COLOR_RESET);
                pausePrompt();
                continue;
            }

            getStringInput("Enter Password: ", newUser.password, sizeof(newUser.password));
            getStringInput("Enter Full Name: ", newUser.full_name, sizeof(newUser.full_name));
            
            printf("Select Role:\n 1. STAFF (Front Desk & Bookings)\n 2. ADMIN (Full System Access)\n");
            int role_choice = getIntInput("Role (1-2): ", 1, 2);
            if (role_choice == 1) {
                strcpy(newUser.role, "STAFF");
            } else {
                strcpy(newUser.role, "ADMIN");
            }

            users[user_count++] = newUser;
            saveAllData();
            printf(COLOR_GREEN "\n[+] User account '%s' created successfully!\n" COLOR_RESET, newUser.username);
            pausePrompt();
        } else if (choice == 2) {
            char target_user[30];
            getStringInput("Enter username to reset password: ", target_user, sizeof(target_user));
            int found = -1;
            for (int i = 0; i < user_count; i++) {
                if (strcmp(users[i].username, target_user) == 0) {
                    found = i;
                    break;
                }
            }
            if (found == -1) {
                printf(COLOR_RED "[!] User '%s' not found.\n" COLOR_RESET, target_user);
            } else {
                char new_pwd[30];
                getStringInput("Enter new password: ", new_pwd, sizeof(new_pwd));
                strcpy(users[found].password, new_pwd);
                saveAllData();
                printf(COLOR_GREEN "\n[+] Password for '%s' updated successfully!\n" COLOR_RESET, target_user);
            }
            pausePrompt();
        }
    }
}

void changePassword(void) {
    drawHeader("CHANGE MY PASSWORD");
    char current_pwd[30];
    getStringInput("Enter Current Password: ", current_pwd, sizeof(current_pwd));
    if (strcmp(current_pwd, current_user.password) != 0) {
        printf(COLOR_RED "\n[!] Incorrect current password.\n" COLOR_RESET);
        pausePrompt();
        return;
    }

    char new_pwd[30], confirm_pwd[30];
    getStringInput("Enter New Password: ", new_pwd, sizeof(new_pwd));
    getStringInput("Confirm New Password: ", confirm_pwd, sizeof(confirm_pwd));

    if (strcmp(new_pwd, confirm_pwd) != 0) {
        printf(COLOR_RED "\n[!] Passwords do not match. Action aborted.\n" COLOR_RESET);
        pausePrompt();
        return;
    }

    /* Update current user in users array */
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, current_user.username) == 0) {
            strcpy(users[i].password, new_pwd);
            strcpy(current_user.password, new_pwd);
            break;
        }
    }
    saveAllData();
    printf(COLOR_GREEN "\n[+] Your password has been successfully updated!\n" COLOR_RESET);
    pausePrompt();
}

/* ========================================================================= */
/*                    FLEET & VEHICLE INVENTORY MODULE                       */
/* ========================================================================= */

int findVehicleIndexById(int id) {
    for (int i = 0; i < vehicle_count; i++) {
        if (vehicles[i].id == id && !vehicles[i].is_deleted) {
            return i;
        }
    }
    return -1;
}

int findVehicleIndexByPlate(const char *plate) {
    for (int i = 0; i < vehicle_count; i++) {
        if (strcasecmp(vehicles[i].plate, plate) == 0 && !vehicles[i].is_deleted) {
            return i;
        }
    }
    return -1;
}

void viewAllVehicles(void) {
    drawHeader("FLEET INVENTORY DIRECTORY");
    printf(COLOR_CYAN "%-5s | %-10s | %-22s | %-10s | %-8s | %-9s | %-9s | %-12s\n",
           "ID", "Plate No", "Make & Model", "Category", "Fuel", "Daily Rate", "Odometer", "Status");
    printf("----------------------------------------------------------------------------------------------------\n" COLOR_RESET);

    int active_count = 0;
    for (int i = 0; i < vehicle_count; i++) {
        if (vehicles[i].is_deleted) continue;
        active_count++;

        char status_color[20] = COLOR_GREEN;
        if (strcmp(vehicles[i].status, "RENTED") == 0) strcpy(status_color, COLOR_YELLOW);
        else if (strcmp(vehicles[i].status, "MAINTENANCE") == 0) strcpy(status_color, COLOR_RED);
        else if (strcmp(vehicles[i].status, "RESERVED") == 0) strcpy(status_color, COLOR_MAGENTA);

        char model_buf[40];
        snprintf(model_buf, sizeof(model_buf), "%s %s", vehicles[i].brand, vehicles[i].model);

        printf("%-5d | %-10s | %-22s | %-10s | %-8s | $%7.2f/d | %7.0f km | %s%-12s" COLOR_RESET "\n",
               vehicles[i].id,
               vehicles[i].plate,
               model_buf,
               vehicles[i].category,
               vehicles[i].fuel_type,
               vehicles[i].daily_rate,
               vehicles[i].mileage,
               status_color,
               vehicles[i].status);
    }
    printf("----------------------------------------------------------------------------------------------------\n");
    printf(COLOR_WHITE "Total Active Fleet: %d vehicles\n" COLOR_RESET, active_count);
}

void viewAvailableVehicles(void) {
    drawHeader("AVAILABLE VEHICLES READY FOR HIRE");
    printf(COLOR_CYAN "%-5s | %-10s | %-22s | %-10s | %-5s | %-9s | %-9s | %-9s\n",
           "ID", "Plate No", "Make & Model", "Category", "Seats", "Daily Rate", "Hourly", "Deposit");
    printf("----------------------------------------------------------------------------------------------------\n" COLOR_RESET);

    int available_count = 0;
    for (int i = 0; i < vehicle_count; i++) {
        if (vehicles[i].is_deleted) continue;
        if (strcmp(vehicles[i].status, "AVAILABLE") == 0) {
            available_count++;
            char model_buf[40];
            snprintf(model_buf, sizeof(model_buf), "%s %s (%d)", vehicles[i].brand, vehicles[i].model, vehicles[i].year);

            printf("%-5d | %-10s | %-22s | %-10s | %-5d | $%7.2f/d | $%6.2f/h | $%7.2f\n",
                   vehicles[i].id,
                   vehicles[i].plate,
                   model_buf,
                   vehicles[i].category,
                   vehicles[i].seats,
                   vehicles[i].daily_rate,
                   vehicles[i].hourly_rate,
                   vehicles[i].deposit_req);
        }
    }
    printf("----------------------------------------------------------------------------------------------------\n");
    if (available_count == 0) {
        printf(COLOR_YELLOW "No vehicles currently available for hire.\n" COLOR_RESET);
    } else {
        printf(COLOR_GREEN "Available Fleet Count: %d vehicles ready for rental.\n" COLOR_RESET, available_count);
    }
}

void addVehicle(void) {
    if (vehicle_count >= MAX_VEHICLES) {
        printf(COLOR_RED "[!] Vehicle capacity limit reached.\n" COLOR_RESET);
        pausePrompt();
        return;
    }

    drawHeader("ADD NEW VEHICLE TO FLEET");
    Vehicle v;
    memset(&v, 0, sizeof(Vehicle));

    /* Auto generate next ID */
    int max_id = 100;
    for (int i = 0; i < vehicle_count; i++) {
        if (vehicles[i].id > max_id) max_id = vehicles[i].id;
    }
    v.id = max_id + 1;

    printf(COLOR_YELLOW "Generated Vehicle ID: %d\n\n" COLOR_RESET, v.id);

    while (1) {
        getStringInput("Enter License Plate Number (e.g., CAR-1234): ", v.plate, sizeof(v.plate));
        if (findVehicleIndexByPlate(v.plate) != -1) {
            printf(COLOR_RED "[!] A vehicle with plate '%s' already exists in the system.\n" COLOR_RESET, v.plate);
        } else {
            break;
        }
    }

    getStringInput("Enter Make / Brand (e.g., Toyota, BMW, Ford): ", v.brand, sizeof(v.brand));
    getStringInput("Enter Model (e.g., Camry SE, X5, Mustang): ", v.model, sizeof(v.model));
    v.year = getIntInput("Enter Manufacturing Year (1990 - 2027): ", 1990, 2027);

    printf("\nSelect Vehicle Category:\n");
    printf(" 1. Sedan\n 2. SUV\n 3. Luxury\n 4. Minivan\n 5. Motorcycle / Bike\n 6. Commercial / Truck\n");
    int cat_c = getIntInput("Category choice (1-6): ", 1, 6);
    switch (cat_c) {
        case 1: strcpy(v.category, "Sedan"); break;
        case 2: strcpy(v.category, "SUV"); break;
        case 3: strcpy(v.category, "Luxury"); break;
        case 4: strcpy(v.category, "Minivan"); break;
        case 5: strcpy(v.category, "Motorcycle"); break;
        case 6: strcpy(v.category, "Commercial"); break;
    }

    printf("\nSelect Transmission:\n 1. Automatic\n 2. Manual\n");
    int trans_c = getIntInput("Transmission (1-2): ", 1, 2);
    strcpy(v.transmission, trans_c == 1 ? "Automatic" : "Manual");

    printf("\nSelect Fuel Type:\n 1. Petrol\n 2. Diesel\n 3. Electric\n 4. Hybrid\n");
    int fuel_c = getIntInput("Fuel Type (1-4): ", 1, 4);
    switch (fuel_c) {
        case 1: strcpy(v.fuel_type, "Petrol"); break;
        case 2: strcpy(v.fuel_type, "Diesel"); break;
        case 3: strcpy(v.fuel_type, "Electric"); break;
        case 4: strcpy(v.fuel_type, "Hybrid"); break;
    }

    v.seats = getIntInput("Enter Seating Capacity (1 - 50): ", 1, 50);
    v.daily_rate = getDoubleInput("Enter Daily Rental Rate ($): ", 10.0, 5000.0);
    v.hourly_rate = getDoubleInput("Enter Hourly Rental Rate ($): ", 2.0, 1000.0);
    v.deposit_req = getDoubleInput("Enter Required Security Deposit ($): ", 50.0, 10000.0);
    v.mileage = getDoubleInput("Enter Current Odometer Reading (KM): ", 0.0, 1000000.0);
    strcpy(v.status, "AVAILABLE");
    v.is_deleted = 0;

    vehicles[vehicle_count++] = v;
    saveAllData();

    printf(COLOR_GREEN "\n[+] Vehicle '%s %s' [Plate: %s] successfully registered into fleet!\n" COLOR_RESET,
           v.brand, v.model, v.plate);
    pausePrompt();
}

void editVehicle(void) {
    drawHeader("MODIFY VEHICLE DETAILS & RATES");
    viewAllVehicles();

    int id = getIntInput("\nEnter Vehicle ID to edit (0 to cancel): ", 0, 99999);
    if (id == 0) return;

    int idx = findVehicleIndexById(id);
    if (idx == -1) {
        printf(COLOR_RED "[!] Vehicle ID #%d not found.\n" COLOR_RESET, id);
        pausePrompt();
        return;
    }

    Vehicle *v = &vehicles[idx];
    printf(COLOR_YELLOW "\nEditing Vehicle: %s %s [%s] (Current Status: %s)\n" COLOR_RESET,
           v->brand, v->model, v->plate, v->status);

    printf("1. Update Daily Rental Rate (Current: $%.2f)\n", v->daily_rate);
    printf("2. Update Hourly Rate (Current: $%.2f)\n", v->hourly_rate);
    printf("3. Update Security Deposit (Current: $%.2f)\n", v->deposit_req);
    printf("4. Update Odometer / Mileage (Current: %.0f km)\n", v->mileage);
    printf("5. Change Vehicle Status (Current: %s)\n", v->status);
    printf("6. Cancel\n");

    int choice = getIntInput("\nSelect field to update (1-6): ", 1, 6);
    switch (choice) {
        case 1:
            v->daily_rate = getDoubleInput("Enter New Daily Rate ($): ", 10.0, 5000.0);
            break;
        case 2:
            v->hourly_rate = getDoubleInput("Enter New Hourly Rate ($): ", 2.0, 1000.0);
            break;
        case 3:
            v->deposit_req = getDoubleInput("Enter New Security Deposit ($): ", 50.0, 10000.0);
            break;
        case 4:
            v->mileage = getDoubleInput("Enter New Odometer (KM): ", v->mileage, 1000000.0);
            break;
        case 5:
            printf("\nSelect New Status:\n 1. AVAILABLE\n 2. MAINTENANCE\n 3. RESERVED\n");
            int sc = getIntInput("Status (1-3): ", 1, 3);
            if (sc == 1) strcpy(v->status, "AVAILABLE");
            else if (sc == 2) strcpy(v->status, "MAINTENANCE");
            else if (sc == 3) strcpy(v->status, "RESERVED");
            break;
        case 6:
            return;
    }

    saveAllData();
    printf(COLOR_GREEN "\n[+] Vehicle records updated successfully!\n" COLOR_RESET);
    pausePrompt();
}

void deleteVehicle(void) {
    if (strcmp(current_user.role, "ADMIN") != 0) {
        printf(COLOR_RED "\n[!] Access Restricted: Only Administrator accounts can decommission vehicles.\n" COLOR_RESET);
        pausePrompt();
        return;
    }

    drawHeader("DECOMMISSION / REMOVE VEHICLE");
    viewAllVehicles();

    int id = getIntInput("\nEnter Vehicle ID to remove (0 to cancel): ", 0, 99999);
    if (id == 0) return;

    int idx = findVehicleIndexById(id);
    if (idx == -1) {
        printf(COLOR_RED "[!] Vehicle ID #%d not found.\n" COLOR_RESET, id);
        pausePrompt();
        return;
    }

    if (strcmp(vehicles[idx].status, "RENTED") == 0) {
        printf(COLOR_RED "[!] Cannot remove vehicle currently marked as 'RENTED'. Please process check-in first.\n" COLOR_RESET);
        pausePrompt();
        return;
    }

    printf(COLOR_YELLOW "Are you sure you want to decommission %s %s [Plate: %s]?\n" COLOR_RESET,
           vehicles[idx].brand, vehicles[idx].model, vehicles[idx].plate);
    printf("1. Yes, Decommission & Remove\n2. Cancel\n");
    int confirm = getIntInput("Choice (1-2): ", 1, 2);

    if (confirm == 1) {
        vehicles[idx].is_deleted = 1;
        saveAllData();
        printf(COLOR_GREEN "\n[+] Vehicle successfully removed from active fleet inventory.\n" COLOR_RESET);
    }
    pausePrompt();
}

void searchVehicles(void) {
    drawHeader("SEARCH FLEET INVENTORY");
    printf("Search By:\n");
    printf("1. Make / Model / Keyword\n");
    printf("2. Category (Sedan, SUV, Luxury, etc.)\n");
    printf("3. Status (Available, Rented, Maintenance)\n");
    printf("4. Maximum Daily Budget\n");
    printf("5. Back\n");

    int option = getIntInput("\nSelect search filter (1-5): ", 1, 5);
    if (option == 5) return;

    int matches = 0;

    if (option == 1) {
        char keyword[50];
        getStringInput("Enter search keyword (e.g., Toyota, Camry, Ford): ", keyword, sizeof(keyword));
        printf("\n" COLOR_CYAN "%-5s | %-10s | %-22s | %-10s | %-9s | %-12s\n", "ID", "Plate", "Make & Model", "Category", "Daily Rate", "Status");
        printf("--------------------------------------------------------------------------------\n" COLOR_RESET);
        for (int i = 0; i < vehicle_count; i++) {
            if (vehicles[i].is_deleted) continue;
            char combined[80];
            snprintf(combined, sizeof(combined), "%s %s %s", vehicles[i].brand, vehicles[i].model, vehicles[i].plate);
            /* Case insensitive substring search */
            char lower_combined[80], lower_key[50];
            for (int k = 0; combined[k]; k++) lower_combined[k] = (char)tolower(combined[k]);
            lower_combined[strlen(combined)] = '\0';
            for (int k = 0; keyword[k]; k++) lower_key[k] = (char)tolower(keyword[k]);
            lower_key[strlen(keyword)] = '\0';

            if (strstr(lower_combined, lower_key) != NULL) {
                matches++;
                printf("%-5d | %-10s | %-22s | %-10s | $%7.2f/d | %-12s\n",
                       vehicles[i].id, vehicles[i].plate, combined, vehicles[i].category, vehicles[i].daily_rate, vehicles[i].status);
            }
        }
    } else if (option == 2) {
        char cat[30];
        getStringInput("Enter Category (Sedan, SUV, Luxury, Minivan, Motorcycle, Commercial): ", cat, sizeof(cat));
        printf("\n" COLOR_CYAN "%-5s | %-10s | %-22s | %-10s | %-9s | %-12s\n", "ID", "Plate", "Make & Model", "Category", "Daily Rate", "Status");
        printf("--------------------------------------------------------------------------------\n" COLOR_RESET);
        for (int i = 0; i < vehicle_count; i++) {
            if (vehicles[i].is_deleted) continue;
            if (strcasecmp(vehicles[i].category, cat) == 0) {
                matches++;
                char m_buf[50];
                snprintf(m_buf, sizeof(m_buf), "%s %s", vehicles[i].brand, vehicles[i].model);
                printf("%-5d | %-10s | %-22s | %-10s | $%7.2f/d | %-12s\n",
                       vehicles[i].id, vehicles[i].plate, m_buf, vehicles[i].category, vehicles[i].daily_rate, vehicles[i].status);
            }
        }
    } else if (option == 3) {
        char st[30];
        getStringInput("Enter Status (AVAILABLE, RENTED, MAINTENANCE, RESERVED): ", st, sizeof(st));
        printf("\n" COLOR_CYAN "%-5s | %-10s | %-22s | %-10s | %-9s | %-12s\n", "ID", "Plate", "Make & Model", "Category", "Daily Rate", "Status");
        printf("--------------------------------------------------------------------------------\n" COLOR_RESET);
        for (int i = 0; i < vehicle_count; i++) {
            if (vehicles[i].is_deleted) continue;
            if (strcasecmp(vehicles[i].status, st) == 0) {
                matches++;
                char m_buf[50];
                snprintf(m_buf, sizeof(m_buf), "%s %s", vehicles[i].brand, vehicles[i].model);
                printf("%-5d | %-10s | %-22s | %-10s | $%7.2f/d | %-12s\n",
                       vehicles[i].id, vehicles[i].plate, m_buf, vehicles[i].category, vehicles[i].daily_rate, vehicles[i].status);
            }
        }
    } else if (option == 4) {
        double max_budget = getDoubleInput("Enter Maximum Daily Rate ($): ", 1.0, 10000.0);
        printf("\n" COLOR_CYAN "%-5s | %-10s | %-22s | %-10s | %-9s | %-12s\n", "ID", "Plate", "Make & Model", "Category", "Daily Rate", "Status");
        printf("--------------------------------------------------------------------------------\n" COLOR_RESET);
        for (int i = 0; i < vehicle_count; i++) {
            if (vehicles[i].is_deleted) continue;
            if (vehicles[i].daily_rate <= max_budget) {
                matches++;
                char m_buf[50];
                snprintf(m_buf, sizeof(m_buf), "%s %s", vehicles[i].brand, vehicles[i].model);
                printf("%-5d | %-10s | %-22s | %-10s | $%7.2f/d | %-12s\n",
                       vehicles[i].id, vehicles[i].plate, m_buf, vehicles[i].category, vehicles[i].daily_rate, vehicles[i].status);
            }
        }
    }

    printf("--------------------------------------------------------------------------------\n");
    printf(COLOR_WHITE "Found %d matching vehicles.\n" COLOR_RESET, matches);
    pausePrompt();
}

void fleetMenu(void) {
    while (1) {
        drawHeader("FLEET & VEHICLE INVENTORY MANAGEMENT");
        printf("1. View All Fleet Vehicles (Full Table)\n");
        printf("2. View Only Available Vehicles (Ready to Hire)\n");
        printf("3. Register New Vehicle into Fleet\n");
        printf("4. Edit Vehicle Rates / Status / Mileage\n");
        printf("5. Search / Filter Fleet Vehicles\n");
        printf("6. Decommission / Remove Vehicle\n");
        printf("7. Return to Main Dashboard\n");

        int choice = getIntInput("\nSelect an option (1-7): ", 1, 7);
        switch (choice) {
            case 1: viewAllVehicles(); pausePrompt(); break;
            case 2: viewAvailableVehicles(); pausePrompt(); break;
            case 3: addVehicle(); break;
            case 4: editVehicle(); break;
            case 5: searchVehicles(); break;
            case 6: deleteVehicle(); break;
            case 7: return;
        }
    }
}

/* ========================================================================= */
/*                          CUSTOMER CRM MODULE                              */
/* ========================================================================= */

int findCustomerIndexById(int id) {
    for (int i = 0; i < customer_count; i++) {
        if (customers[i].id == id && !customers[i].is_deleted) {
            return i;
        }
    }
    return -1;
}

int findCustomerIndexByLicense(const char *license) {
    for (int i = 0; i < customer_count; i++) {
        if (strcasecmp(customers[i].license_num, license) == 0 && !customers[i].is_deleted) {
            return i;
        }
    }
    return -1;
}

void viewAllCustomers(void) {
    drawHeader("CUSTOMER RELATIONSHIP DIRECTORY (CRM)");
    printf(COLOR_CYAN "%-5s | %-20s | %-15s | %-15s | %-12s | %-8s | %-10s\n",
           "ID", "Customer Name", "Phone", "Driver License", "Expiry", "Rentals", "Total Spent");
    printf("----------------------------------------------------------------------------------------------------\n" COLOR_RESET);

    int count = 0;
    for (int i = 0; i < customer_count; i++) {
        if (customers[i].is_deleted) continue;
        count++;
        printf("%-5d | %-20s | %-15s | %-15s | %-12s | %-8d | $%8.2f\n",
               customers[i].id,
               customers[i].name,
               customers[i].phone,
               customers[i].license_num,
               customers[i].license_expiry,
               customers[i].total_rentals,
               customers[i].total_spent);
    }
    printf("----------------------------------------------------------------------------------------------------\n");
    printf(COLOR_WHITE "Total Registered Clients: %d\n" COLOR_RESET, count);
}

void addCustomer(void) {
    if (customer_count >= MAX_CUSTOMERS) {
        printf(COLOR_RED "[!] Customer database limit reached.\n" COLOR_RESET);
        pausePrompt();
        return;
    }

    drawHeader("REGISTER NEW CLIENT PROFILE");
    Customer c;
    memset(&c, 0, sizeof(Customer));

    int max_id = 200;
    for (int i = 0; i < customer_count; i++) {
        if (customers[i].id > max_id) max_id = customers[i].id;
    }
    c.id = max_id + 1;
    printf(COLOR_YELLOW "Assigned Customer ID: %d\n\n" COLOR_RESET, c.id);

    getStringInput("Enter Full Legal Name: ", c.name, sizeof(c.name));
    getStringInput("Enter Phone Number: ", c.phone, sizeof(c.phone));
    getStringInput("Enter Email Address: ", c.email, sizeof(c.email));
    getStringInput("Enter National ID / Passport No: ", c.id_passport, sizeof(c.id_passport));

    while (1) {
        getStringInput("Enter Driver's License Number: ", c.license_num, sizeof(c.license_num));
        if (findCustomerIndexByLicense(c.license_num) != -1) {
            printf(COLOR_RED "[!] Driver's License '%s' is already registered to another customer profile.\n" COLOR_RESET, c.license_num);
        } else {
            break;
        }
    }

    getStringInput("Enter License Expiry Date (YYYY-MM-DD): ", c.license_expiry, sizeof(c.license_expiry));
    getStringInput("Enter Residential Address: ", c.address, sizeof(c.address));

    c.total_rentals = 0;
    c.total_spent = 0.0;
    c.is_deleted = 0;

    customers[customer_count++] = c;
    saveAllData();

    printf(COLOR_GREEN "\n[+] Client '%s' (ID #%d) registered successfully!\n" COLOR_RESET, c.name, c.id);
    pausePrompt();
}

void editCustomer(void) {
    drawHeader("UPDATE CLIENT PROFILE");
    viewAllCustomers();

    int id = getIntInput("\nEnter Customer ID to edit (0 to cancel): ", 0, 99999);
    if (id == 0) return;

    int idx = findCustomerIndexById(id);
    if (idx == -1) {
        printf(COLOR_RED "[!] Customer ID #%d not found.\n" COLOR_RESET, id);
        pausePrompt();
        return;
    }

    Customer *c = &customers[idx];
    printf(COLOR_YELLOW "\nEditing Profile: %s (ID #%d)\n" COLOR_RESET, c->name, c->id);
    printf("1. Update Phone Number (Current: %s)\n", c->phone);
    printf("2. Update Email (Current: %s)\n", c->email);
    printf("3. Update Address (Current: %s)\n", c->address);
    printf("4. Update License Expiry (Current: %s)\n", c->license_expiry);
    printf("5. Cancel\n");

    int ch = getIntInput("\nSelect field to update (1-5): ", 1, 5);
    switch (ch) {
        case 1: getStringInput("Enter New Phone: ", c->phone, sizeof(c->phone)); break;
        case 2: getStringInput("Enter New Email: ", c->email, sizeof(c->email)); break;
        case 3: getStringInput("Enter New Address: ", c->address, sizeof(c->address)); break;
        case 4: getStringInput("Enter New Expiry Date (YYYY-MM-DD): ", c->license_expiry, sizeof(c->license_expiry)); break;
        case 5: return;
    }

    saveAllData();
    printf(COLOR_GREEN "\n[+] Customer profile updated successfully!\n" COLOR_RESET);
    pausePrompt();
}

void searchCustomers(void) {
    drawHeader("SEARCH CLIENT DATABASE");
    char term[60];
    getStringInput("Enter search term (Name, Phone, or License No): ", term, sizeof(term));

    printf("\n" COLOR_CYAN "%-5s | %-20s | %-15s | %-15s | %-12s | %-8s\n",
           "ID", "Customer Name", "Phone", "License No", "Expiry", "Rentals");
    printf("--------------------------------------------------------------------------------\n" COLOR_RESET);

    int found = 0;
    char lower_term[60];
    for (int k = 0; term[k]; k++) lower_term[k] = (char)tolower(term[k]);
    lower_term[strlen(term)] = '\0';

    for (int i = 0; i < customer_count; i++) {
        if (customers[i].is_deleted) continue;

        char combined[200];
        snprintf(combined, sizeof(combined), "%s %s %s %s",
                 customers[i].name, customers[i].phone, customers[i].license_num, customers[i].email);
        char lower_comb[200];
        for (int k = 0; combined[k]; k++) lower_comb[k] = (char)tolower(combined[k]);
        lower_comb[strlen(combined)] = '\0';

        if (strstr(lower_comb, lower_term) != NULL) {
            found++;
            printf("%-5d | %-20s | %-15s | %-15s | %-12s | %-8d\n",
                   customers[i].id, customers[i].name, customers[i].phone,
                   customers[i].license_num, customers[i].license_expiry, customers[i].total_rentals);
        }
    }
    printf("--------------------------------------------------------------------------------\n");
    printf(COLOR_WHITE "Found %d matching client records.\n" COLOR_RESET, found);
    pausePrompt();
}

void viewCustomerHistory(void) {
    drawHeader("CLIENT RENTAL HISTORY RECORD");
    int id = getIntInput("Enter Customer ID: ", 1, 99999);
    int idx = findCustomerIndexById(id);
    if (idx == -1) {
        printf(COLOR_RED "[!] Customer ID #%d not found.\n" COLOR_RESET, id);
        pausePrompt();
        return;
    }

    printf(COLOR_YELLOW "\nRental Ledger for: %s (Phone: %s, License: %s)\n" COLOR_RESET,
           customers[idx].name, customers[idx].phone, customers[idx].license_num);
    printf(COLOR_CYAN "%-8s | %-10s | %-22s | %-10s | %-10s | %-10s | %-10s\n",
           "Book ID", "Plate", "Vehicle Model", "Start Date", "End Date", "Total Cost", "Status");
    printf("----------------------------------------------------------------------------------------------------\n" COLOR_RESET);

    int count = 0;
    for (int i = 0; i < rental_count; i++) {
        if (rentals[i].customer_id == id) {
            count++;
            printf("#%-7d | %-10s | %-22s | %-10s | %-10s | $%9.2f | %-10s\n",
                   rentals[i].booking_id, rentals[i].plate, rentals[i].vehicle_info,
                   rentals[i].start_date, rentals[i].expected_return_date,
                   rentals[i].total_cost, rentals[i].status);
        }
    }
    printf("----------------------------------------------------------------------------------------------------\n");
    printf(COLOR_WHITE "Total Bookings on File: %d  |  Lifetime Value: $%.2f\n" COLOR_RESET,
           count, customers[idx].total_spent);
    pausePrompt();
}

void customerMenu(void) {
    while (1) {
        drawHeader("CUSTOMER CRM & CLIENT MANAGEMENT");
        printf("1. View All Client Profiles\n");
        printf("2. Register New Customer\n");
        printf("3. Search Client Database (Name/Phone/License)\n");
        printf("4. Update Existing Client Profile\n");
        printf("5. View Individual Client Rental History\n");
        printf("6. Return to Main Dashboard\n");

        int choice = getIntInput("\nSelect an option (1-6): ", 1, 6);
        switch (choice) {
            case 1: viewAllCustomers(); pausePrompt(); break;
            case 2: addCustomer(); break;
            case 3: searchCustomers(); break;
            case 4: editCustomer(); break;
            case 5: viewCustomerHistory(); break;
            case 6: return;
        }
    }
}

/* ========================================================================= */
/*                   RENTAL OPERATIONS (CHECK-OUT & CHECK-IN)                */
/* ========================================================================= */

int findRentalIndexById(int id) {
    for (int i = 0; i < rental_count; i++) {
        if (rentals[i].booking_id == id) {
            return i;
        }
    }
    return -1;
}

void printRentalContractReceipt(const RentalBooking *b) {
    printf(COLOR_CYAN);
    printf("\n================================================================================\n");
    printf("                  APEX AUTO WHEELS - VEHICLE RENTAL CONTRACT                    \n");
    printf("                       Official Check-Out Agreement Receipt                     \n");
    printf("================================================================================\n" COLOR_RESET);
    printf("  Booking Contract ID : " COLOR_YELLOW "#%d" COLOR_RESET "\n", b->booking_id);
    printf("  Contract Status     : " COLOR_GREEN "%s" COLOR_RESET "\n", b->status);
    printf("  Customer Name       : %s\n", b->customer_name);
    printf("  Customer Phone      : %s\n", b->customer_phone);
    printf("  Rented Vehicle      : %s (Plate: %s)\n", b->vehicle_info, b->plate);
    printf("  Pickup Date         : %s\n", b->start_date);
    printf("  Scheduled Return    : %s (%d Days Duration)\n", b->expected_return_date, b->duration_days);
    printf("  Starting Odometer   : %.0f KM\n", b->start_mileage);
    printf("--------------------------------------------------------------------------------\n");
    printf("  Base Rental Charge  : %d days x $%.2f/day = $%.2f\n",
           b->duration_days, b->daily_rate, b->base_cost);

    printf("  Add-On Options      :\n");
    if (b->addons.insurance_type == 1) printf("    + Basic Collision Damage Waiver ($%.2f/day) = $%.2f\n", RATE_BASIC_INSURANCE, RATE_BASIC_INSURANCE * b->duration_days);
    else if (b->addons.insurance_type == 2) printf("    + Comprehensive Full Protection ($%.2f/day) = $%.2f\n", RATE_FULL_INSURANCE, RATE_FULL_INSURANCE * b->duration_days);
    if (b->addons.gps) printf("    + GPS Satellite Navigation ($%.2f/day) = $%.2f\n", RATE_GPS, RATE_GPS * b->duration_days);
    if (b->addons.child_seat) printf("    + Child Safety Seat ($%.2f/day) = $%.2f\n", RATE_CHILD_SEAT, RATE_CHILD_SEAT * b->duration_days);
    if (b->addons.extra_driver) printf("    + Additional Authorized Driver ($%.2f/day) = $%.2f\n", RATE_EXTRA_DRIVER, RATE_EXTRA_DRIVER * b->duration_days);
    if (b->addons_cost == 0) printf("    (No optional add-ons selected)\n");

    printf("  Total Add-On Fees   : $%.2f\n", b->addons_cost);
    printf("--------------------------------------------------------------------------------\n");
    printf("  " COLOR_BOLD "Estimated Total Cost: $%.2f\n" COLOR_RESET, b->total_cost);
    printf("  " COLOR_GREEN "Security Deposit Paid: $%.2f\n" COLOR_RESET, b->deposit_paid);
    printf("================================================================================\n");
    printf("  * Complimentary Mileage: %.0f KM/day (Excess charged at $%.2f/km upon return)\n", FREE_KM_PER_DAY, EXCESS_KM_RATE);
    printf("  * Standard Late Return Penalty: $%.2f per day overdue.\n", LATE_PENALTY_PER_DAY);
    printf("================================================================================\n\n");
}

void printFinalSettlementInvoice(const RentalBooking *b) {
    printf(COLOR_CYAN);
    printf("\n================================================================================\n");
    printf("                  APEX AUTO WHEELS - FINAL SETTLEMENT INVOICE                   \n");
    printf("                         Official Check-In Settlement                           \n");
    printf("================================================================================\n" COLOR_RESET);
    printf("  Booking Contract ID : #%d\n", b->booking_id);
    printf("  Customer Name       : %s\n", b->customer_name);
    printf("  Vehicle             : %s [%s]\n", b->vehicle_info, b->plate);
    printf("  Rental Period       : %s to %s\n", b->start_date, b->actual_return_date);
    printf("  Odometer Log        : Start: %.0f KM  ->  Return: %.0f KM (Total Travelled: %.0f KM)\n",
           b->start_mileage, b->return_mileage, (b->return_mileage - b->start_mileage));
    printf("--------------------------------------------------------------------------------\n");
    printf("  Base Rental Cost    : $%.2f\n", b->base_cost);
    printf("  Add-Ons Package     : $%.2f\n", b->addons_cost);
    if (b->excess_mileage_fee > 0) printf("  Excess Mileage Fee  : $%.2f\n", b->excess_mileage_fee);
    if (b->late_fee > 0)           printf("  Late Return Penalty : $%.2f\n", b->late_fee);
    if (b->fuel_charge > 0)        printf("  Refueling Charge    : $%.2f\n", b->fuel_charge);
    if (b->damage_fee > 0)         printf("  Vehicle Damage Fee  : $%.2f\n", b->damage_fee);
    printf("--------------------------------------------------------------------------------\n");
    printf("  " COLOR_BOLD "FINAL TOTAL CHARGES : $%.2f\n" COLOR_RESET, b->total_cost);
    printf("  Advance Deposit Paid: $%.2f\n", b->deposit_paid);
    printf("--------------------------------------------------------------------------------\n");
    if (b->balance_settled >= 0) {
        printf(COLOR_YELLOW "  NET BALANCE DUE FROM CUSTOMER: $%.2f (PAID IN FULL)\n" COLOR_RESET, b->balance_settled);
    } else {
        printf(COLOR_GREEN "  NET REFUND RETURNED TO CUSTOMER: $%.2f (PROCESSED)\n" COLOR_RESET, -b->balance_settled);
    }
    printf("================================================================================\n");
    printf("  Transaction Status  : " COLOR_GREEN "COMPLETED & CLOSED\n" COLOR_RESET);
    printf("================================================================================\n\n");
}

void checkOutVehicle(void) {
    if (rental_count >= MAX_RENTALS) {
        printf(COLOR_RED "[!] Maximum rental records limit reached.\n" COLOR_RESET);
        pausePrompt();
        return;
    }

    drawHeader("CHECK-OUT PROCESS: NEW RENTAL CONTRACT");

    /* Step 1: Select Customer */
    drawSubHeader("STEP 1: SELECT CUSTOMER PROFILE");
    viewAllCustomers();
    int cust_id = getIntInput("\nEnter Customer ID (0 to cancel, -1 to register new customer): ", -1, 99999);
    if (cust_id == 0) return;
    if (cust_id == -1) {
        addCustomer();
        cust_id = customers[customer_count - 1].id;
    }

    int c_idx = findCustomerIndexById(cust_id);
    if (c_idx == -1) {
        printf(COLOR_RED "[!] Invalid Customer ID.\n" COLOR_RESET);
        pausePrompt();
        return;
    }

    /* Step 2: Select Vehicle */
    drawSubHeader("STEP 2: SELECT AVAILABLE VEHICLE");
    viewAvailableVehicles();
    int veh_id = getIntInput("\nEnter Vehicle ID to hire (0 to cancel): ", 0, 99999);
    if (veh_id == 0) return;

    int v_idx = findVehicleIndexById(veh_id);
    if (v_idx == -1) {
        printf(COLOR_RED "[!] Vehicle ID #%d not found.\n" COLOR_RESET, veh_id);
        pausePrompt();
        return;
    }

    if (strcmp(vehicles[v_idx].status, "AVAILABLE") != 0) {
        printf(COLOR_RED "[!] Vehicle #%d is currently %s and cannot be hired out.\n" COLOR_RESET,
               veh_id, vehicles[v_idx].status);
        pausePrompt();
        return;
    }

    /* Step 3: Booking Details */
    drawSubHeader("STEP 3: RENTAL DURATION & DATES");
    RentalBooking b;
    memset(&b, 0, sizeof(RentalBooking));

    int max_b_id = 300;
    for (int i = 0; i < rental_count; i++) {
        if (rentals[i].booking_id > max_b_id) max_b_id = rentals[i].booking_id;
    }
    b.booking_id = max_b_id + 1;
    b.customer_id = customers[c_idx].id;
    strcpy(b.customer_name, customers[c_idx].name);
    strcpy(b.customer_phone, customers[c_idx].phone);
    b.vehicle_id = vehicles[v_idx].id;
    snprintf(b.vehicle_info, sizeof(b.vehicle_info), "%s %s", vehicles[v_idx].brand, vehicles[v_idx].model);
    strcpy(b.plate, vehicles[v_idx].plate);
    b.daily_rate = vehicles[v_idx].daily_rate;
    b.start_mileage = vehicles[v_idx].mileage;

    char today_str[20];
    getCurrentDateStr(today_str, sizeof(today_str));
    getStringInput("Enter Pickup Date (YYYY-MM-DD, e.g., today): ", b.start_date, sizeof(b.start_date));
    if (strcmp(b.start_date, "today") == 0) strcpy(b.start_date, today_str);

    b.duration_days = getIntInput("Enter Rental Duration (Days, 1 - 90): ", 1, 90);
    getStringInput("Enter Expected Return Date (YYYY-MM-DD): ", b.expected_return_date, sizeof(b.expected_return_date));

    /* Step 4: Add-On Packages */
    drawSubHeader("STEP 4: OPTIONAL PROTECTION & ACCESSORIES");
    printf("Insurance Coverage Options:\n");
    printf(" 0. None (Standard Third-Party Liability Only)\n");
    printf(" 1. Basic CDW - Collision Damage Waiver ($%.2f/day)\n", RATE_BASIC_INSURANCE);
    printf(" 2. Comprehensive Full Protection - Zero Excess ($%.2f/day)\n", RATE_FULL_INSURANCE);
    b.addons.insurance_type = getIntInput("Select Insurance (0-2): ", 0, 2);

    printf("\nAccessories & Services:\n");
    b.addons.gps = getIntInput("Include GPS Satellite Navigation ($8.00/day)? (1=Yes, 0=No): ", 0, 1);
    b.addons.child_seat = getIntInput("Include Child Safety Seat ($10.00/day)? (1=Yes, 0=No): ", 0, 1);
    b.addons.extra_driver = getIntInput("Add Additional Authorized Driver ($12.00/day)? (1=Yes, 0=No): ", 0, 1);

    /* Compute Financials */
    b.base_cost = b.daily_rate * b.duration_days;
    double add_rate = 0.0;
    if (b.addons.insurance_type == 1) add_rate += RATE_BASIC_INSURANCE;
    else if (b.addons.insurance_type == 2) add_rate += RATE_FULL_INSURANCE;
    if (b.addons.gps) add_rate += RATE_GPS;
    if (b.addons.child_seat) add_rate += RATE_CHILD_SEAT;
    if (b.addons.extra_driver) add_rate += RATE_EXTRA_DRIVER;

    b.addons_cost = add_rate * b.duration_days;
    b.total_cost = b.base_cost + b.addons_cost;
    b.deposit_paid = vehicles[v_idx].deposit_req;

    getStringInput("Any special notes or trip instructions (optional): ", b.notes, sizeof(b.notes));
    strcpy(b.status, "ACTIVE");

    /* Review & Confirm */
    printRentalContractReceipt(&b);
    printf(COLOR_YELLOW "Confirm Check-Out and Collect Security Deposit of $%.2f?\n" COLOR_RESET, b.deposit_paid);
    printf("1. Confirm Check-Out & Generate Contract\n2. Cancel\n");
    int confirm = getIntInput("Choice (1-2): ", 1, 2);

    if (confirm == 1) {
        /* Update vehicle status */
        strcpy(vehicles[v_idx].status, "RENTED");
        /* Update customer records */
        customers[c_idx].total_rentals++;
        
        /* Save Booking */
        rentals[rental_count++] = b;

        /* Record Financial Deposit Transaction */
        Transaction t;
        int max_t_id = 1000;
        for (int i = 0; i < transaction_count; i++) {
            if (transactions[i].trans_id > max_t_id) max_t_id = transactions[i].trans_id;
        }
        t.trans_id = max_t_id + 1;
        getCurrentDateStr(t.date, sizeof(t.date));
        strcpy(t.type, "RENTAL_DEPOSIT_ADVANCE");
        t.booking_id = b.booking_id;
        t.vehicle_id = b.vehicle_id;
        t.amount = b.deposit_paid;
        snprintf(t.description, sizeof(t.description), "Deposit collected for Booking #%d (%s)", b.booking_id, b.plate);
        transactions[transaction_count++] = t;

        saveAllData();

        printf(COLOR_GREEN "\n[+] CHECK-OUT SUCCESSFUL! Booking #%d is now ACTIVE.\n" COLOR_RESET, b.booking_id);
        printf(COLOR_GREEN "[+] Vehicle '%s' status switched to RENTED.\n" COLOR_RESET, b.plate);
    } else {
        printf(COLOR_YELLOW "\n[!] Check-out cancelled.\n" COLOR_RESET);
    }
    pausePrompt();
}

void checkInVehicle(void) {
    drawHeader("CHECK-IN PROCESS: RETURN VEHICLE & SETTLEMENT");

    /* Show active rentals */
    printf(COLOR_CYAN "%-8s | %-20s | %-12s | %-20s | %-12s | %-12s\n",
           "Book ID", "Customer", "Plate", "Vehicle", "Start Date", "Due Date");
    printf("----------------------------------------------------------------------------------------------------\n" COLOR_RESET);

    int active_found = 0;
    for (int i = 0; i < rental_count; i++) {
        if (strcmp(rentals[i].status, "ACTIVE") == 0) {
            active_found++;
            printf("#%-7d | %-20s | %-12s | %-20s | %-12s | %-12s\n",
                   rentals[i].booking_id, rentals[i].customer_name, rentals[i].plate,
                   rentals[i].vehicle_info, rentals[i].start_date, rentals[i].expected_return_date);
        }
    }
    printf("----------------------------------------------------------------------------------------------------\n");

    if (active_found == 0) {
        printf(COLOR_YELLOW "There are currently no active vehicle hires to check in.\n" COLOR_RESET);
        pausePrompt();
        return;
    }

    int b_id = getIntInput("\nEnter Booking ID to return (0 to cancel): ", 0, 99999);
    if (b_id == 0) return;

    int r_idx = findRentalIndexById(b_id);
    if (r_idx == -1 || strcmp(rentals[r_idx].status, "ACTIVE") != 0) {
        printf(COLOR_RED "[!] Active booking with ID #%d not found.\n" COLOR_RESET, b_id);
        pausePrompt();
        return;
    }

    RentalBooking *b = &rentals[r_idx];
    int v_idx = findVehicleIndexById(b->vehicle_id);
    int c_idx = findCustomerIndexById(b->customer_id);

    drawSubHeader("RETURN INSPECTION & ODOMETER VERIFICATION");
    printf("Customer        : %s\n", b->customer_name);
    printf("Vehicle         : %s [%s]\n", b->vehicle_info, b->plate);
    printf("Pickup Odometer : %.0f KM\n", b->start_mileage);

    char today_str[20];
    getCurrentDateStr(today_str, sizeof(today_str));
    getStringInput("Enter Actual Return Date (YYYY-MM-DD, e.g. today): ", b->actual_return_date, sizeof(b->actual_return_date));
    if (strcmp(b->actual_return_date, "today") == 0) strcpy(b->actual_return_date, today_str);

    b->return_mileage = getDoubleInput("Enter Current Return Odometer Reading (KM): ", b->start_mileage, 1000000.0);
    double km_travelled = b->return_mileage - b->start_mileage;
    double allowed_km = b->duration_days * FREE_KM_PER_DAY;

    if (km_travelled > allowed_km) {
        double excess_km = km_travelled - allowed_km;
        b->excess_mileage_fee = excess_km * EXCESS_KM_RATE;
        printf(COLOR_YELLOW "  [!] Excess Mileage: %.1f km travelled beyond %.0f km allowance -> Excess Fee: $%.2f\n" COLOR_RESET,
               excess_km, allowed_km, b->excess_mileage_fee);
    } else {
        b->excess_mileage_fee = 0.0;
        printf(COLOR_GREEN "  [+] Mileage within complimentary allowance (%.1f / %.0f KM).\n" COLOR_RESET, km_travelled, allowed_km);
    }

    int late_days = getIntInput("Enter number of overdue / late days (0 if returned on time): ", 0, 60);
    b->late_fee = late_days * LATE_PENALTY_PER_DAY;

    printf("\nFuel Tank Inspection:\n");
    printf(" 1. Full Tank (No charge)\n 2. Missing Fuel (Apply $45.00 refuel surcharge)\n 3. Empty / Low (Apply $90.00 refuel surcharge)\n");
    int fuel_opt = getIntInput("Fuel Status (1-3): ", 1, 3);
    if (fuel_opt == 2) b->fuel_charge = 45.00;
    else if (fuel_opt == 3) b->fuel_charge = 90.00;
    else b->fuel_charge = 0.0;

    printf("\nPhysical Damage & Condition Inspection:\n");
    printf(" 1. Clean / No Damage\n 2. Minor Scratches / Dents (Deduct $150.00)\n 3. Major Damage / Mechanical Issue\n");
    int dmg_opt = getIntInput("Vehicle Condition (1-3): ", 1, 3);
    if (dmg_opt == 2) {
        b->damage_fee = 150.00;
    } else if (dmg_opt == 3) {
        b->damage_fee = getDoubleInput("Enter Assessed Damage Repair Deduction ($): ", 50.0, 5000.0);
    } else {
        b->damage_fee = 0.0;
    }

    /* Final Financial Calculations */
    b->total_cost = b->base_cost + b->addons_cost + b->excess_mileage_fee + b->late_fee + b->fuel_charge + b->damage_fee;
    b->balance_settled = b->total_cost - b->deposit_paid;
    strcpy(b->status, "COMPLETED");

    /* Display Settlement */
    printFinalSettlementInvoice(b);

    printf(COLOR_YELLOW "Confirm Final Settlement & Release Vehicle?\n1. Confirm Settlement\n2. Abort\n" COLOR_RESET);
    int confirm = getIntInput("Choice (1-2): ", 1, 2);

    if (confirm == 1) {
        /* Update vehicle odometer and status */
        if (v_idx != -1) {
            vehicles[v_idx].mileage = b->return_mileage;
            if (dmg_opt >= 2) {
                strcpy(vehicles[v_idx].status, "MAINTENANCE");
                printf(COLOR_YELLOW "[!] Vehicle %s placed in MAINTENANCE due to reported inspection damage.\n" COLOR_RESET, vehicles[v_idx].plate);
            } else {
                strcpy(vehicles[v_idx].status, "AVAILABLE");
            }
        }

        /* Update customer spending */
        if (c_idx != -1) {
            customers[c_idx].total_spent += b->total_cost;
        }

        /* Record Settlement Transaction */
        Transaction t;
        int max_t_id = 1000;
        for (int i = 0; i < transaction_count; i++) {
            if (transactions[i].trans_id > max_t_id) max_t_id = transactions[i].trans_id;
        }
        t.trans_id = max_t_id + 1;
        getCurrentDateStr(t.date, sizeof(t.date));
        strcpy(t.type, "FINAL_SETTLEMENT");
        t.booking_id = b->booking_id;
        t.vehicle_id = b->vehicle_id;
        t.amount = b->total_cost - b->deposit_paid;
        snprintf(t.description, sizeof(t.description), "Final settlement closed for Booking #%d (%s)", b->booking_id, b->plate);
        transactions[transaction_count++] = t;

        saveAllData();
        printf(COLOR_GREEN "\n[+] CHECK-IN & SETTLEMENT COMPLETED SUCCESSFULLY!\n" COLOR_RESET);
    } else {
        strcpy(b->status, "ACTIVE");
        printf(COLOR_YELLOW "\n[!] Check-in aborted. Booking remains ACTIVE.\n" COLOR_RESET);
    }
    pausePrompt();
}

void viewActiveRentals(void) {
    drawHeader("CURRENTLY ACTIVE HIRES");
    printf(COLOR_CYAN "%-8s | %-20s | %-12s | %-20s | %-12s | %-12s | %-10s\n",
           "Book ID", "Customer", "Plate", "Vehicle", "Start Date", "Due Date", "Deposit");
    printf("----------------------------------------------------------------------------------------------------\n" COLOR_RESET);

    int count = 0;
    for (int i = 0; i < rental_count; i++) {
        if (strcmp(rentals[i].status, "ACTIVE") == 0) {
            count++;
            printf("#%-7d | %-20s | %-12s | %-20s | %-12s | %-12s | $%8.2f\n",
               rentals[i].booking_id, rentals[i].customer_name, rentals[i].plate,
               rentals[i].vehicle_info, rentals[i].start_date, rentals[i].expected_return_date,
               rentals[i].deposit_paid);
        }
    }
    printf("----------------------------------------------------------------------------------------------------\n");
    printf(COLOR_WHITE "Total On-Road Vehicles: %d\n" COLOR_RESET, count);
    pausePrompt();
}

void viewAllRentalHistory(void) {
    drawHeader("ALL RENTAL BOOKINGS & CONTRACT ARCHIVES");
    printf(COLOR_CYAN "%-8s | %-18s | %-10s | %-18s | %-10s | %-10s | %-10s | %-10s\n",
           "Book ID", "Customer", "Plate", "Vehicle", "Start", "Return", "Total", "Status");
    printf("----------------------------------------------------------------------------------------------------\n" COLOR_RESET);

    for (int i = 0; i < rental_count; i++) {
        char status_col[20] = COLOR_GREEN;
        if (strcmp(rentals[i].status, "ACTIVE") == 0) strcpy(status_col, COLOR_YELLOW);
        else if (strcmp(rentals[i].status, "CANCELLED") == 0) strcpy(status_col, COLOR_RED);

        printf("#%-7d | %-18s | %-10s | %-18s | %-10s | %-10s | $%8.2f | %s%-10s" COLOR_RESET "\n",
               rentals[i].booking_id, rentals[i].customer_name, rentals[i].plate,
               rentals[i].vehicle_info, rentals[i].start_date,
               strlen(rentals[i].actual_return_date) > 0 ? rentals[i].actual_return_date : rentals[i].expected_return_date,
               rentals[i].total_cost, status_col, rentals[i].status);
    }
    printf("----------------------------------------------------------------------------------------------------\n");
    printf(COLOR_WHITE "Total Recorded Rental Contracts: %d\n" COLOR_RESET, rental_count);
    pausePrompt();
}

void rentalOperationsMenu(void) {
    while (1) {
        drawHeader("RENTAL CONTRACTS & FLEET OPERATIONS");
        printf("1. Process New Vehicle Hire (Check-Out Contract)\n");
        printf("2. Return Vehicle & Final Settlement (Check-In & Invoice)\n");
        printf("3. View All Currently Active Hires (On-Road)\n");
        printf("4. View Complete Booking History & Archives\n");
        printf("5. Return to Main Dashboard\n");

        int choice = getIntInput("\nSelect operation (1-5): ", 1, 5);
        switch (choice) {
            case 1: checkOutVehicle(); break;
            case 2: checkInVehicle(); break;
            case 3: viewActiveRentals(); break;
            case 4: viewAllRentalHistory(); break;
            case 5: return;
        }
    }
}

/* ========================================================================= */
/*                     FLEET MAINTENANCE MANAGEMENT                          */
/* ========================================================================= */

void logMaintenanceRecord(void) {
    if (maintenance_count >= MAX_MAINTENANCE) {
        printf(COLOR_RED "[!] Maintenance log archive is full.\n" COLOR_RESET);
        pausePrompt();
        return;
    }

    drawHeader("LOG VEHICLE MAINTENANCE & SERVICE EVENT");
    viewAllVehicles();

    int v_id = getIntInput("\nEnter Vehicle ID serviced (0 to cancel): ", 0, 99999);
    if (v_id == 0) return;

    int idx = findVehicleIndexById(v_id);
    if (idx == -1) {
        printf(COLOR_RED "[!] Vehicle ID #%d not found.\n" COLOR_RESET, v_id);
        pausePrompt();
        return;
    }

    MaintenanceLog log;
    memset(&log, 0, sizeof(MaintenanceLog));

    int max_m_id = 400;
    for (int i = 0; i < maintenance_count; i++) {
        if (maintenance_logs[i].log_id > max_m_id) max_m_id = maintenance_logs[i].log_id;
    }
    log.log_id = max_m_id + 1;
    log.vehicle_id = vehicles[idx].id;
    snprintf(log.vehicle_info, sizeof(log.vehicle_info), "%s %s [%s]",
             vehicles[idx].brand, vehicles[idx].model, vehicles[idx].plate);

    char today_str[20];
    getCurrentDateStr(today_str, sizeof(today_str));
    getStringInput("Enter Service Date (YYYY-MM-DD, e.g. today): ", log.date, sizeof(log.date));
    if (strcmp(log.date, "today") == 0) strcpy(log.date, today_str);

    printf("\nSelect Service Type:\n");
    printf(" 1. Engine Oil & Filter Change\n 2. Brake Pads & Rotor Overhaul\n 3. Tire Replacement & Wheel Alignment\n 4. Comprehensive Multi-Point Inspection\n 5. Bodywork & Detailing Repair\n 6. Other Mechanical Repair\n");
    int st_c = getIntInput("Service Type (1-6): ", 1, 6);
    switch (st_c) {
        case 1: strcpy(log.service_type, "Oil & Filter Change"); break;
        case 2: strcpy(log.service_type, "Brake System Service"); break;
        case 3: strcpy(log.service_type, "Tire Replacement & Balance"); break;
        case 4: strcpy(log.service_type, "Multi-Point Safety Inspection"); break;
        case 5: strcpy(log.service_type, "Bodywork & Detailing"); break;
        case 6: strcpy(log.service_type, "Mechanical Repair"); break;
    }

    log.cost = getDoubleInput("Enter Total Service Cost ($): ", 0.0, 50000.0);
    log.odometer_at_service = getDoubleInput("Enter Odometer Reading at Service (KM): ", vehicles[idx].mileage, 1000000.0);
    vehicles[idx].mileage = log.odometer_at_service;

    getStringInput("Enter Service Description / Notes: ", log.description, sizeof(log.description));
    getStringInput("Enter Workshop / Mechanic Vendor Name: ", log.vendor_mechanic, sizeof(log.vendor_mechanic));

    printf("\nUpdate Vehicle Status:\n 1. Mark as AVAILABLE (Ready for hire)\n 2. Keep in MAINTENANCE (Work ongoing)\n");
    int stat_c = getIntInput("Choice (1-2): ", 1, 2);
    if (stat_c == 1) strcpy(vehicles[idx].status, "AVAILABLE");
    else strcpy(vehicles[idx].status, "MAINTENANCE");

    maintenance_logs[maintenance_count++] = log;

    /* Record Expense Transaction */
    Transaction t;
    int max_t_id = 1000;
    for (int i = 0; i < transaction_count; i++) {
        if (transactions[i].trans_id > max_t_id) max_t_id = transactions[i].trans_id;
    }
    t.trans_id = max_t_id + 1;
    getCurrentDateStr(t.date, sizeof(t.date));
    strcpy(t.type, "MAINTENANCE_EXPENSE");
    t.booking_id = 0;
    t.vehicle_id = vehicles[idx].id;
    t.amount = -log.cost;
    snprintf(t.description, sizeof(t.description), "%s for %s (Log #%d)", log.service_type, vehicles[idx].plate, log.log_id);
    transactions[transaction_count++] = t;

    saveAllData();

    printf(COLOR_GREEN "\n[+] Maintenance event #%d logged successfully and expense recorded!\n" COLOR_RESET, log.log_id);
    pausePrompt();
}

void viewMaintenanceLogs(void) {
    drawHeader("FLEET MAINTENANCE & SERVICE ARCHIVES");
    printf(COLOR_CYAN "%-6s | %-24s | %-12s | %-22s | %-10s | %-10s\n",
           "Log ID", "Vehicle Info", "Date", "Service Type", "Cost", "Odometer");
    printf("----------------------------------------------------------------------------------------------------\n" COLOR_RESET);

    double total_maint_cost = 0.0;
    for (int i = 0; i < maintenance_count; i++) {
        total_maint_cost += maintenance_logs[i].cost;
        printf("#%-5d | %-24s | %-12s | %-22s | $%8.2f | %7.0f km\n",
               maintenance_logs[i].log_id, maintenance_logs[i].vehicle_info,
               maintenance_logs[i].date, maintenance_logs[i].service_type,
               maintenance_logs[i].cost, maintenance_logs[i].odometer_at_service);
    }
    printf("----------------------------------------------------------------------------------------------------\n");
    printf(COLOR_WHITE "Total Service Events: %d  |  Total Fleet Maintenance Expenditure: $%.2f\n" COLOR_RESET,
           maintenance_count, total_maint_cost);
    pausePrompt();
}

void fleetHealthAlerts(void) {
    drawHeader("FLEET HEALTH & ROUTINE SERVICE ALERTS");
    printf(COLOR_WHITE "Routine Service Threshold: Every 10,000 KM.\n\n" COLOR_RESET);

    printf(COLOR_CYAN "%-5s | %-10s | %-22s | %-10s | %-12s | %-16s\n",
           "ID", "Plate", "Make & Model", "Odometer", "Status", "Health Alert");
    printf("----------------------------------------------------------------------------------------------------\n" COLOR_RESET);

    int alert_count = 0;
    for (int i = 0; i < vehicle_count; i++) {
        if (vehicles[i].is_deleted) continue;

        /* Check odometer modulo 10000 */
        double km = vehicles[i].mileage;
        int due = ((int)km % 10000) > 8500;
        int overdue = ((int)km % 10000) < 500 && km > 10000;

        char alert_msg[30] = "OK - In Good Standing";
        char color[20] = COLOR_GREEN;

        if (strcmp(vehicles[i].status, "MAINTENANCE") == 0) {
            strcpy(alert_msg, "UNDER REPAIR");
            strcpy(color, COLOR_RED);
            alert_count++;
        } else if (due || overdue) {
            strcpy(alert_msg, "SERVICE DUE SOON");
            strcpy(color, COLOR_YELLOW);
            alert_count++;
        }

        char m_buf[50];
        snprintf(m_buf, sizeof(m_buf), "%s %s", vehicles[i].brand, vehicles[i].model);

        printf("%-5d | %-10s | %-22s | %7.0f km | %-12s | %s%-16s" COLOR_RESET "\n",
               vehicles[i].id, vehicles[i].plate, m_buf,
               vehicles[i].mileage, vehicles[i].status, color, alert_msg);
    }
    printf("----------------------------------------------------------------------------------------------------\n");
    printf("Vehicles requiring service attention: %d\n", alert_count);
    pausePrompt();
}

void maintenanceMenu(void) {
    while (1) {
        drawHeader("FLEET HEALTH & MAINTENANCE TRACKER");
        printf("1. Log New Vehicle Service / Repair Event\n");
        printf("2. View All Maintenance History & Expenses\n");
        printf("3. Fleet Health & Service Due Alerts\n");
        printf("4. Return to Main Dashboard\n");

        int choice = getIntInput("\nSelect an option (1-4): ", 1, 4);
        switch (choice) {
            case 1: logMaintenanceRecord(); break;
            case 2: viewMaintenanceLogs(); break;
            case 3: fleetHealthAlerts(); break;
            case 4: return;
        }
    }
}

/* ========================================================================= */
/*                   BILLING, POS & FINANCIAL ANALYTICS                      */
/* ========================================================================= */

void viewFinancialSummary(void) {
    drawHeader("FINANCIAL PERFORMANCE & BUSINESS ANALYTICS");

    double total_rental_revenue = 0.0;
    double total_maintenance_expenses = 0.0;
    double deposits_in_hand = 0.0;
    int completed_rentals = 0;
    int active_rentals = 0;

    for (int i = 0; i < rental_count; i++) {
        if (strcmp(rentals[i].status, "COMPLETED") == 0) {
            total_rental_revenue += rentals[i].total_cost;
            completed_rentals++;
        } else if (strcmp(rentals[i].status, "ACTIVE") == 0) {
            deposits_in_hand += rentals[i].deposit_paid;
            active_rentals++;
        }
    }

    for (int i = 0; i < maintenance_count; i++) {
        total_maintenance_expenses += maintenance_logs[i].cost;
    }

    double net_operating_profit = total_rental_revenue - total_maintenance_expenses;

    int total_active_vehicles = 0;
    int rented_vehicles = 0;
    for (int i = 0; i < vehicle_count; i++) {
        if (!vehicles[i].is_deleted) {
            total_active_vehicles++;
            if (strcmp(vehicles[i].status, "RENTED") == 0) {
                rented_vehicles++;
            }
        }
    }
    double fleet_utilization = total_active_vehicles > 0 ? ((double)rented_vehicles / total_active_vehicles) * 100.0 : 0.0;

    printf(COLOR_CYAN "================================================================================\n");
    printf("                       EXECUTIVE FINANCIAL OVERVIEW                             \n");
    printf("================================================================================\n" COLOR_RESET);
    printf("  " COLOR_BOLD "Total Gross Rental Revenue  : " COLOR_GREEN "$%12.2f\n" COLOR_RESET, total_rental_revenue);
    printf("  " COLOR_BOLD "Total Maintenance Expenses  : " COLOR_RED "$%12.2f\n" COLOR_RESET, total_maintenance_expenses);
    printf("  -----------------------------------------------------------------------------\n");
    if (net_operating_profit >= 0) {
        printf("  " COLOR_BOLD "NET OPERATING PROFIT        : " COLOR_GREEN "$%12.2f\n" COLOR_RESET, net_operating_profit);
    } else {
        printf("  " COLOR_BOLD "NET OPERATING PROFIT (LOSS) : " COLOR_RED "$%12.2f\n" COLOR_RESET, net_operating_profit);
    }
    printf("  Active Security Deposits Held : " COLOR_YELLOW "$%12.2f\n" COLOR_RESET, deposits_in_hand);
    printf("================================================================================\n");
    printf(COLOR_CYAN "                       OPERATIONAL METRICS & KPI                                \n");
    printf("================================================================================\n" COLOR_RESET);
    printf("  Total Fleet Size             : %d Vehicles\n", total_active_vehicles);
    printf("  Currently On-Road (Rented)   : %d Vehicles\n", rented_vehicles);
    printf("  Fleet Utilization Rate       : " COLOR_YELLOW "%.1f %%\n" COLOR_RESET, fleet_utilization);
    printf("  Completed Rental Bookings    : %d contracts\n", completed_rentals);
    printf("  Active Ongoing Contracts     : %d contracts\n", active_rentals);
    printf("  Registered Client Profiles   : %d clients\n", customer_count);
    printf("================================================================================\n");

    pausePrompt();
}

void viewTransactionLedger(void) {
    drawHeader("GENERAL TRANSACTION & BILLING LEDGER");
    printf(COLOR_CYAN "%-6s | %-12s | %-24s | %-10s | %-28s\n",
           "TX ID", "Date", "Transaction Type", "Amount", "Description");
    printf("----------------------------------------------------------------------------------------------------\n" COLOR_RESET);

    for (int i = 0; i < transaction_count; i++) {
        char col[20] = COLOR_GREEN;
        if (transactions[i].amount < 0) strcpy(col, COLOR_RED);
        else if (strcmp(transactions[i].type, "RENTAL_DEPOSIT_ADVANCE") == 0) strcpy(col, COLOR_YELLOW);

        printf("#%-5d | %-12s | %-24s | %s$%9.2f" COLOR_RESET " | %-28s\n",
               transactions[i].trans_id, transactions[i].date, transactions[i].type,
               col, transactions[i].amount, transactions[i].description);
    }
    printf("----------------------------------------------------------------------------------------------------\n");
    printf(COLOR_WHITE "Total Ledger Entries: %d\n" COLOR_RESET, transaction_count);
    pausePrompt();
}

void exportDailyBusinessReport(void) {
    drawHeader("EXPORT BUSINESS DAILY REPORT");
    char filename[50] = "daily_business_report.txt";

    FILE *f = fopen(filename, "w");
    if (!f) {
        printf(COLOR_RED "[!] Failed to generate report file.\n" COLOR_RESET);
        pausePrompt();
        return;
    }

    char date_str[20];
    getCurrentDateStr(date_str, sizeof(date_str));

    fprintf(f, "================================================================================\n");
    fprintf(f, "               APEX AUTO WHEELS - COMPREHENSIVE BUSINESS REPORT                 \n");
    fprintf(f, "                     Generated on: %s                                           \n", date_str);
    fprintf(f, "================================================================================\n\n");

    double gross_rev = 0.0, maint_exp = 0.0;
    for (int i = 0; i < rental_count; i++) {
        if (strcmp(rentals[i].status, "COMPLETED") == 0) gross_rev += rentals[i].total_cost;
    }
    for (int i = 0; i < maintenance_count; i++) {
        maint_exp += maintenance_logs[i].cost;
    }

    fprintf(f, "1. FINANCIAL SUMMARY\n");
    fprintf(f, "   - Gross Rental Revenue  : $%.2f\n", gross_rev);
    fprintf(f, "   - Fleet Maintenance Cost: $%.2f\n", maint_exp);
    fprintf(f, "   - Net Operating Profit  : $%.2f\n\n", (gross_rev - maint_exp));

    fprintf(f, "2. ACTIVE FLEET INVENTORY (%d vehicles)\n", vehicle_count);
    fprintf(f, "   %-5s | %-10s | %-22s | %-10s | %-10s | %-12s\n", "ID", "Plate", "Model", "Category", "Rate/Day", "Status");
    fprintf(f, "   -----------------------------------------------------------------------------\n");
    for (int i = 0; i < vehicle_count; i++) {
        if (vehicles[i].is_deleted) continue;
        char m_buf[50];
        snprintf(m_buf, sizeof(m_buf), "%s %s", vehicles[i].brand, vehicles[i].model);
        fprintf(f, "   %-5d | %-10s | %-22s | %-10s | $%8.2f | %-12s\n",
                vehicles[i].id, vehicles[i].plate, m_buf, vehicles[i].category, vehicles[i].daily_rate, vehicles[i].status);
    }
    fprintf(f, "\n");

    fprintf(f, "3. RECENT RENTAL CONTRACTS (%d records)\n", rental_count);
    fprintf(f, "   %-8s | %-18s | %-10s | %-10s | %-10s | %-10s\n", "Book ID", "Customer", "Plate", "Start", "Return", "Cost");
    fprintf(f, "   -----------------------------------------------------------------------------\n");
    for (int i = 0; i < rental_count; i++) {
        fprintf(f, "   #%-7d | %-18s | %-10s | %-10s | %-10s | $%8.2f\n",
                rentals[i].booking_id, rentals[i].customer_name, rentals[i].plate,
                rentals[i].start_date, rentals[i].expected_return_date, rentals[i].total_cost);
    }
    fprintf(f, "\n================================ END OF REPORT =================================\n");

    fclose(f);
    printf(COLOR_GREEN "\n[+] Business report successfully exported to '%s'!\n" COLOR_RESET, filename);
    pausePrompt();
}

void billingReportsMenu(void) {
    while (1) {
        drawHeader("FINANCIAL ANALYTICS, POS & REPORTS");
        printf("1. Executive Financial Summary & KPI Dashboard\n");
        printf("2. General Transaction Ledger\n");
        printf("3. Export Full Business Report to File (.txt)\n");
        printf("4. Return to Main Dashboard\n");

        int choice = getIntInput("\nSelect an option (1-4): ", 1, 4);
        switch (choice) {
            case 1: viewFinancialSummary(); break;
            case 2: viewTransactionLedger(); break;
            case 3: exportDailyBusinessReport(); break;
            case 4: return;
        }
    }
}

/* ========================================================================= */
/*                             MAIN DASHBOARD                                */
/* ========================================================================= */

void mainDashboard(void) {
    while (1) {
        clearScreen();
        printf(COLOR_BLUE);
        printf("================================================================================\n");
        printf("      APEX AUTO WHEELS - ALL-IN-ONE VEHICLE RENTAL MANAGEMENT SYSTEM            \n");
        printf("================================================================================\n" COLOR_RESET);
        printf(COLOR_WHITE " Active User: " COLOR_YELLOW "%s " COLOR_WHITE "| Role: " COLOR_YELLOW "%s " COLOR_WHITE "| Date: " COLOR_CYAN, current_user.full_name, current_user.role);
        char date_str[20];
        getCurrentDateStr(date_str, sizeof(date_str));
        printf("%s\n" COLOR_RESET, date_str);
        printf(COLOR_BLUE "--------------------------------------------------------------------------------\n" COLOR_RESET);

        /* Live status glance */
        int available_v = 0, rented_v = 0, maint_v = 0;
        for (int i = 0; i < vehicle_count; i++) {
            if (vehicles[i].is_deleted) continue;
            if (strcmp(vehicles[i].status, "AVAILABLE") == 0) available_v++;
            else if (strcmp(vehicles[i].status, "RENTED") == 0) rented_v++;
            else if (strcmp(vehicles[i].status, "MAINTENANCE") == 0) maint_v++;
        }

        printf(" Fleet Status: " COLOR_GREEN "%d Available " COLOR_RESET "| "
               COLOR_YELLOW "%d Rented (On-Road) " COLOR_RESET "| "
               COLOR_RED "%d In Service\n" COLOR_RESET,
               available_v, rented_v, maint_v);
        printf(COLOR_BLUE "--------------------------------------------------------------------------------\n" COLOR_RESET);

        printf(COLOR_BOLD "MAIN OPERATIONS:\n" COLOR_RESET);
        printf("  1. " COLOR_CYAN "Rental Operations (Check-Out & Check-In Contracts)" COLOR_RESET "\n");
        printf("  2. " COLOR_CYAN "Fleet & Vehicle Inventory Management" COLOR_RESET "\n");
        printf("  3. " COLOR_CYAN "Customer Relationship Management (CRM)" COLOR_RESET "\n");
        printf("  4. " COLOR_CYAN "Fleet Maintenance & Health Tracker" COLOR_RESET "\n");
        printf("  5. " COLOR_CYAN "Financial Analytics, POS Invoicing & Reports" COLOR_RESET "\n\n");

        printf(COLOR_BOLD "SYSTEM SETTINGS:\n" COLOR_RESET);
        printf("  6. User Accounts Management " COLOR_YELLOW "(Admin Only)" COLOR_RESET "\n");
        printf("  7. Change My Password\n");
        printf("  8. Save & Logout / Exit System\n");
        printf(COLOR_BLUE "================================================================================\n" COLOR_RESET);

        int choice = getIntInput("Enter selection (1-8): ", 1, 8);
        switch (choice) {
            case 1: rentalOperationsMenu(); break;
            case 2: fleetMenu(); break;
            case 3: customerMenu(); break;
            case 4: maintenanceMenu(); break;
            case 5: billingReportsMenu(); break;
            case 6: manageUsers(); break;
            case 7: changePassword(); break;
            case 8:
                saveAllData();
                printf(COLOR_GREEN "\nSaving records and logging out...\n" COLOR_RESET);
                return;
        }
    }
}
