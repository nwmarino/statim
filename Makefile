CC := clang
CFLAGS := -g -O0 -Icompiler
LDFLAGS := -lm

MAIN := compiler/skvoz.c
MAIN_OBJ := $(MAIN:.c=.o)
SRC := $(filter-out $(MAIN), $(shell find compiler -name "*.c"))
OBJ := $(SRC:.c=.o)

TARGET := skvoz

all: $(TARGET)

$(TARGET): $(OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS)  -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(MAIN_OBJ) $(TARGET)
