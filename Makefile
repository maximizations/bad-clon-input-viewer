# SCUFFED ASS MAKEFILE -- it works though lOL
# ill write an actual competent one later this is basically a placeholder

CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=-lraylib


# as non-flexible as it gets !!
SRC=inputviewer.c
TARGET_NAME=inputviewer

# definitely yoinked that one btw
ifeq ($(OS),Windows_NT)
	TARGET_NAME=$(addsuffix .exe,$(TARGET_NAME))
endif

TARGET_DIR=bin

TARGET=$(TARGET_DIR)/$(TARGET_NAME)

.PHONY: default debug run clean install uninstall

default:
	$(CC) -Ofast $(CFLAGS) $(LDFLAGS) $(SRC) -o $(TARGET)

debug:
	$(CC) -Og $(CFLAGS) $(LDFLAGS) $(SRC) -o $(TARGET)

run: 
	$(TARGET)

clean:
	rm -f $(TARGET)

install:
	cp $(TARGET) /usr/local/bin

uninstall:
	rm /usr/local/bin/$(TARGET_FILE)
