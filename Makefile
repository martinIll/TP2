# Directorios
SOURCE=sources
BINARY_DIR=bin
SIMPLE_BMP_DIR=simple_bmp
#Binarios
MAIN=main
SIMPLE_BMP=simple_bmp
# Opt de compilacion 
CC=gcc 
CFLAGS=-std=gnu11  -Wall -Werror -pedantic -Wextra -Wconversion -g -O3
INCLUDE=-I$(SIMPLE_BMP_DIR) 
LIB=-lm -fopenmp

all : build
build : $(SOURCE)/main.c  $(SIMPLE_BMP_DIR)/simple_bmp.h 	
	mkdir -p $(BINARY_DIR)
	cppcheck ./ --enable=all 
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BINARY_DIR)/$(SIMPLE_BMP).o -c $(SIMPLE_BMP_DIR)/simple_bmp.c  
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BINARY_DIR)/$(MAIN).o -c $(SOURCE)/main.c   $(LIB)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BINARY_DIR)/$(MAIN)   $(BINARY_DIR)/$(MAIN).o $(BINARY_DIR)/$(SIMPLE_BMP).o $(LIB)

.PHONY: clean
clean :
	rm  -Rf $(BINARY_DIR)
docs:
	doxygen doxyfile
 