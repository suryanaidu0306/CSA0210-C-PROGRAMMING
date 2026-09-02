# Apex Auto Wheels - Vehicle Rental Centre Management System (in C)

A complete, high-performance, and robust Vehicle Rental Management System written in standard C (C99/C11 compatible). Designed to manage all aspects of a vehicle hire business in one unified software.

---

## 🚗 Core Features

### 1. Fleet & Vehicle Inventory Management
- Multi-class fleet support: Sedans, SUVs, Luxury Vehicles, Minivans, Motorcycles, Commercial / Trucks.
- Vehicle parameters: Plate number, brand, model, year, fuel type, transmission, seating capacity, daily & hourly rates, security deposit, odometer reading.
- Vehicle statuses: `AVAILABLE`, `RENTED`, `MAINTENANCE`, `RESERVED`.
- Fast searching, filtering (by budget, category, status, brand), addition, editing, and decommissioning.

### 2. Customer Relationship Management (CRM)
- Customer profiles: Full name, phone number, email address, national ID/passport, driver's license number, license expiry date, residential address.
- Tracks customer loyalty: Total lifetime rentals and cumulative revenue spent.
- Client search by name, phone number, or driver's license number.

### 3. Rental Operations (Check-Out & Check-In Contracts)
- **Check-Out (Hire Contract)**:
  - Duration calculator (Days/Hours), pickup and scheduled return dates.
  - Optional Add-on Protection & Accessories:
    - Basic Collision Damage Waiver ($15/day)
    - Comprehensive Full Protection ($30/day)
    - GPS Satellite Navigation ($8/day)
    - Child Safety Seat ($10/day)
    - Additional Authorized Driver ($12/day)
  - Upfront security deposit recording and printable POS Check-Out Agreement receipt.
  - Automatically updates vehicle status to `RENTED`.
- **Check-In (Return & Settlement)**:
  - Return date & odometer verification with automatic excess mileage surcharge ($0.35/km after complimentary 250 km/day allowance).
  - Overdue return penalty calculations ($50.00/day).
  - Fuel level inspection with refueling surcharges.
  - Physical damage inspection and repair deductions.
  - Final Settlement Invoice: Computes net balance due or refund returned to customer.
  - Automatically restores vehicle status to `AVAILABLE` (or `MAINTENANCE` if damaged).

### 4. Fleet Maintenance & Service Tracker
- Log maintenance events: Oil changes, brake repairs, tire replacements, routine inspections, detailing.
- Service cost tracking and mechanic/vendor logging.
- **Fleet Health Alerts**: Alerts when vehicles cross the 10,000 km routine service threshold.

### 5. Financial Analytics, POS & Business Reporting
- Executive dashboard: Gross Rental Revenue, Maintenance Expenses, Net Operating Profit, Deposits Held, Fleet Utilization Rate (%).
- General Transaction Ledger with chronological cashflow entries.
- One-click export to readable file: `daily_business_report.txt`.

### 6. User Authentication & Access Control
- Role-based accounts:
  - **Admin**: Full access including user account creation, password resets, and fleet decommissioning.
  - **Staff**: Operational access for front-desk bookings, check-in/out, and customer onboarding.

---

## 🔑 Default Login Credentials

| Role | Username | Password |
| :--- | :--- | :--- |
| **Administrator** | `admin` | `admin123` |
| **Front Desk Staff** | `staff` | `staff123` |

---

## 🛠️ How to Compile & Run

### Method 1: Using GCC (MinGW / Linux / macOS)
```bash
gcc -std=c99 -Wall -Wextra -O2 main.c -o vehicle_rental_centre
./vehicle_rental_centre
```

### Method 2: On Windows (Batch Script)
Simply double-click `build.bat` or run:
```cmd
build.bat
```

### Method 3: Using Clang
```bash
clang -std=c99 -O2 main.c -o vehicle_rental_centre
./vehicle_rental_centre
```

### Method 4: Using Microsoft Visual C++ (MSVC)
```cmd
cl /O2 /Fe:vehicle_rental_centre.exe main.c
vehicle_rental_centre.exe
```

---

## 💾 Data Persistence Files
The system automatically creates and maintains binary database files in the execution directory:
- `vehicles.dat` - Fleet inventory
- `customers.dat` - Customer profiles
- `rentals.dat` - Rental contracts & booking archives
- `maintenance.dat` - Service records
- `transactions.dat` - Financial ledger records
- `users.dat` - User credentials & roles
