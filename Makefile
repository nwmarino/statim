CC := clang++
CXXFLAGS := -std=c++20 -g -O0 -stdlib=libstdc++ -Icompiler/include $(shell llvm-config --cxxflags)
LDFLAGS := -lstdc++ -lm $(shell llvm-config --ldflags)

MAIN := compiler/stmc.cpp
MAIN_OBJ := $(MAIN:.cpp=.o)
SRC := $(filter-out $(MAIN), $(shell find compiler -name "*.cpp"))
OBJ := $(SRC:.cpp=.o)
LLVM_IR := $(SRC:.cpp=.ll) $(MAIN:.cpp=.ll)

TEST_SRC := $(wildcard test/*.cpp)
TEST_OBJ := $(TEST_SRC:.cpp=.o)

TARGET := stmc
TEST_TARGET := stmc_test

GTEST_LIB := -L/usr/lib64 -lgtest -pthread
BOOST_LIB := -L/usr/lib64 -lboost_filesystem
LLVM_LIB  := $(shell llvm-config --libs)

all: $(TARGET)

$(TARGET): $(OBJ) $(MAIN_OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(BOOST_LIB) $(LLVM_LIB)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

llir: $(LLVM_IR)

%.ll: %.cpp
	$(CC) $(CXXFLAGS) -S -emit-llvm $< -o $@

test: $(TEST_TARGET)

$(TEST_TARGET): $(OBJ) $(TEST_OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(BOOST_LIB) $(GTEST_LIB) $(LLVM_LIB)

clean:
	rm -f $(OBJ) $(MAIN_OBJ) $(TEST_OBJ) $(TARGET) $(TEST_TARGET) $(LLVM_IR)
