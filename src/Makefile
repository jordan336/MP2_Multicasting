compiler=gcc
flags=-g -Wall

OBJS = networking.o multicast.o
OBJ_DIR = obj
OBJ = $(patsubst %,$(OBJ_DIR)/%,$(OBJS))

all: directories run

run: $(OBJ)
	$(compiler) $^ -o $@

$(OBJ_DIR)/%.o : %.c networking.h
	$(compiler) $(flags) -c $< -o $@

directories:
	@mkdir -p $(OBJ_DIR)

clean:
	rm -rf run $(OBJ_DIR)/*.o

