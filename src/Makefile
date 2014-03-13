compiler=gcc
flags=-g -Wall

OBJS = networking.o multicast.o run.o threads.o
OBJ_DIR = obj
OBJ = $(patsubst %,$(OBJ_DIR)/%,$(OBJS))

all: directories chat

chat: $(OBJ)
	$(compiler) $^ -o $@ -pthread

$(OBJ_DIR)/%.o : %.c networking.h multicast.h threads.h
	$(compiler) $(flags) -c $< -o $@

directories:
	@mkdir -p $(OBJ_DIR)

clean:
	rm -rf chat $(OBJ_DIR)/*.o

