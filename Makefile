CC=g++
DEFINES=-DDEBUG
LIBS=-lncurses
OBJ_DIR=obj/
OBJS=$(OBJ_DIR)main.o $(OBJ_DIR)gen_map.o
OBJS_DEBUG=$(OBJ_DIR)_main.o $(OBJ_DIR)_gen_map.o
BIN_DIR=bin/
BINARY=$(BIN_DIR)game.out
BINARY_DEBUG=$(BIN_DIR)debug.out

HV=$(shell ls -l obj/*)

all: $(BINARY)

debug: $(BINARY_DEBUG)

clean: 
	if [[ -e $(BINARY) ]]; then rm $(BINARY); fi
	if [[ -e $(BINARY_DEBUG) ]]; then rm $(BINARY_DEBUG); fi
	if [[ '$(HV)' != '' ]]; then rm obj/*; fi

$(BINARY): $(OBJS)
	$(CC) $(OBJS) -o $(BINARY) $(LIBS)

$(OBJ_DIR)main.o: main.cpp
	$(CC) -c $< -o $@

$(OBJ_DIR)gen_map.o: gen_map.cpp
	$(CC) -c $< -o $@

$(BINARY_DEBUG): $(OBJS_DEBUG)
	$(CC) -g $(OBJS_DEBUG) -o $(BINARY_DEBUG) $(LIBS)

$(OBJ_DIR)_main.o: main.cpp
	$(CC) $(DEFINES) -c -g $< -o $@

$(OBJ_DIR)_gen_map.o: gen_map.cpp
	$(CC) $(DEFINES) -c -g $< -o $@

