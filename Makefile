# Makefile for Vehicle Rental Centre Management System
CC ?= gcc
CFLAGS = -std=c99 -Wall -Wextra -O2
TARGET = vehicle_rental_centre
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe *.dat daily_business_report.txt

run: $(TARGET)
	./$(TARGET)
