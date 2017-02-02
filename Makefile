CC=g++
LIBS=-lncurses
BINARY=a.out
BINARY_DEBUG=debug.out

all: $(BINARY)

debug: $(BINARY_DEBUG)

clean: 
	if [[ -e $(BINARY) ]]; then rm $(BINARY); fi
	if [[ -e $(BINARY_DEBUG) ]]; then rm $(BINARY_DEBUG); fi

$(BINARY): main.cpp
	$(CC) main.cpp -o $(BINARY) $(LIBS)

$(BINARY_DEBUG): main.cpp
	$(CC) -g main.cpp -o $(BINARY_DEBUG) $(LIBS)


