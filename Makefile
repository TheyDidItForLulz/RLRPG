CC=g++ -std=c++17
DEBUG_COMPILE_FLAGS=-DDEBUG -g
LIBS=-lncurses -lfmt
OBJ_DIR=obj/
SOURCES=main unit utils item level gen_map log termlib/default_window_provider
#OBJS=$(OBJ_DIR)main.o $(OBJ_DIR)unit.o $(OBJ_DIR)item.o $(OBJ_DIR)gen_map.o $(OBJ_DIR)log.o
OBJS=$(patsubst %,obj/%.o,$(SOURCES))
#OBJS_DEBUG=$(OBJ_DIR)_main.o $(OBJ_DIR)_unit.o $(OBJ_DIR)_item.o $(OBJ_DIR)_gen_map.o $(OBJ_DIR)_log.o
OBJS_DEBUG=$(patsubst %,obj/%_.o,$(SOURCES))
BIN_DIR=bin/
BINARY=$(BIN_DIR)game.out
BINARY_DEBUG=$(BIN_DIR)debug.out
BINARY_EDITOR=$(BIN_DIR)editor.out
BINARY_EDITOR_DEBUG=$(BIN_DIR)editor_debug.out
INCLUDES=-Iinclude/

all: $(BINARY)

debug: $(BINARY_DEBUG)

clean: 
	-rm $(BINARY) $(BINARY_DEBUG) $(BINARY_EDITOR) $(BINARY_EDITOR_DEBUG) $(OBJS) $(OBJS_DEBUG) 2>/dev/null

$(BINARY): $(OBJS)
	$(CC) $(OBJS) -o $(BINARY) $(LIBS)

$(BINARY_DEBUG): $(OBJS_DEBUG)
	$(CC) -g $(OBJS_DEBUG) -o $(BINARY_DEBUG) $(LIBS)

$(OBJ_DIR)%.o: %.cpp
	$(CC) -c $< -o $@ $(INCLUDES)

$(OBJ_DIR)%_.o: %.cpp
	$(CC) $(DEBUG_COMPILE_FLAGS) -c $< -o $@ $(INCLUDES)

$(BINARY_EDITOR): $(OBJ_DIR)map_editor.o
	$(CC) $< -o $@ $(LIBS)

$(BINARY_EDITOR_DEBUG): $(OBJ_DIR)map_editor_.o
	$(CC) -g $< -o $@ $(LIBS)

