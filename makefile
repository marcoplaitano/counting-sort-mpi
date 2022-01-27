# File:   makefile
# Brief:  Makefile used to automate the process of compiling the source files.
# Author: Marco Plaitano
# Date:   27 Nov 2021
#
# COUNTING SORT MPI
# Parallelize and Evaluate Performances of "Counting Sort" Algorithm, by using
# MPI.
#
# Copyright (C) 2022 Plaitano Marco
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

BIN_DIR := bin
BUILD_DIR := build
INCLUDE_DIR := include
SRC_DIR := src
TEST_DIR := test

CC = mpicc
CFLAGS = -g -Wno-unused-result -I $(INCLUDE_DIR)/
OPT_LEVEL = 1
CLIBS =
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
EXEC := $(BIN_DIR)/main.out


# Create main executable file.
$(EXEC): dirs $(OBJS)
	$(CC) $(CFLAGS) -O$(OPT_LEVEL) $(OBJS) $(CLIBS) -o $@


# Create object files.
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -O$(OPT_LEVEL) -c $< $(CLIBS) -o $@


.PHONY: all parallel serial test dirs clean


# Compile sources to generate (parallelized) main executable.
all: $(EXEC)


# Compile parallel version.
parallel: all


# Compile serial version.
serial: CC = gcc
serial: CFLAGS = -g -Wno-unused-result -D_SERIAL
serial: dirs
	$(CC) $(CFLAGS) -O$(OPT_LEVEL) $(SRC_DIR)/serial.c $(CLIBS) -o $(EXEC)


# Compile test file(s).
test: dirs $(OBJS)
	$(CC) $(CFLAGS) -O$(OPT_LEVEL) -c $(TEST_DIR)/test.c $(CLIBS) -o $(BUILD_DIR)/test.o
	rm $(BUILD_DIR)/main.o
	$(CC) $(CFLAGS) -O$(OPT_LEVEL) $(BUILD_DIR)/*.o $(CLIBS) -o $(BIN_DIR)/test.out


# Create needed directories if they do not already exist.
dirs:
	$(shell if [ ! -d $(BIN_DIR) ]; then mkdir -p $(BIN_DIR); fi)
	$(shell if [ ! -d $(BUILD_DIR) ]; then mkdir -p $(BUILD_DIR); fi)


# Delete object files and executables.
# The '-' at the beginning of the line is used to ignore the return code of
# the command.
clean:
	-rm $(BIN_DIR)/* $(BUILD_DIR)/*
