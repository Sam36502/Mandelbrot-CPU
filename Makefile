#
#	SDL Project Makefile
#

VERSION = 0.1.0
BIN = mandelbrot

CC = gcc
CFLAGS = -Wall -g
REL_CFLAGS = -Wall -O2 -mwindows

LIBS = mingw32 SDL2main SDL2

SRC = src
OBJ = objects
SRCS = $(wildcard $(SRC)/*.c) 
HDRS = $(wildcard $(SRC)/*.h) 
OBJS = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))

run: build
	@echo -e '### Running... ###\n'
	@./$(BIN)

build: $(OBJ) $(OBJS)
	@echo '\n### Linking... ###'
	${CC} ${CFLAGS} -o ${BIN}.exe $(OBJS) $(addprefix -l,$(LIBS))

release: 
	@echo '### Creating Release... ###\n'
	${CC} ${REL_CFLAGS} -o ${BIN}_v${VERSION}.exe main.c src/* $(addprefix -l,${LIBS})

clean:
	@echo -e '### Cleaning... ###\n'
	-rm $(BIN).exe
	-rm -r $(OBJ)
	mkdir -p $(OBJ)

$(OBJ):
	@echo -e '### Creating object directory... ###\n'
	mkdir -p $(OBJ)

$(OBJ)/%.o: $(SRC)/%.c $(SRC)/%.h
	@echo -e 'Building $@...'
	@${CC} ${CFLAGS} -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.c
	@echo -e 'Building $@...'
	@${CC} ${CFLAGS} -c $< -o $@

.PHONY: run build release clean
