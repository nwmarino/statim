CC := clang
CFLAGS := -g -O0 -Icompiler/include
LDFLAGS := -lm

MAIN := compiler/stmc.c
MAIN_OBJ := $(MAIN:.c=.o)
SRC := $(filter-out $(MAIN), $(shell find compiler -name "*.c"))
OBJ := $(SRC:.c=.o)

TARGET := stmc

all: $(TARGET)

$(TARGET): $(OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS)  -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(MAIN_OBJ) $(TARGET)
