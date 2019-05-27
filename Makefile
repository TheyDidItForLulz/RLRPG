CC=g++ -std=c++17
DEBUG_COMPILE_FLAGS=-DDEBUG -g
LIBS=-lncurses -lfmt
OBJ_DIR=obj/
SOURCES=main unit enemy hero inventory utils item level gen_map log termlib/default_window_provider
OBJS=$(patsubst %,obj/%.o,$(SOURCES))
OBJS_DEBUG=$(patsubst %,obj/%_.o,$(SOURCES))
BIN_DIR=bin/
BINARY=$(BIN_DIR)game.out
BINARY_DEBUG=$(BIN_DIR)debug.out
INCLUDES=-Iinclude/

all: $(BINARY)

debug: $(BINARY_DEBUG)

clean: 
	-rm $(BINARY) $(BINARY_DEBUG) $(OBJS) $(OBJS_DEBUG) 2>/dev/null

$(BINARY): $(OBJS)
	$(CC) $(OBJS) -o $(BINARY) $(LIBS)

$(BINARY_DEBUG): $(OBJS_DEBUG)
	$(CC) -g $(OBJS_DEBUG) -o $(BINARY_DEBUG) $(LIBS)

$(OBJ_DIR)%.o: %.cpp
	$(CC) -c $< -o $@ $(INCLUDES)

$(OBJ_DIR)%_.o: %.cpp
	$(CC) $(DEBUG_COMPILE_FLAGS) -c $< -o $@ $(INCLUDES)

