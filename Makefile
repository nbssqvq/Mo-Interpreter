CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Iinclude

ifeq ($(OS),Windows_NT)
    TARGET := Mo.exe
    RM     := del /Q
    NULL   := nul
else
    TARGET := Mo
    RM     := rm -f
    NULL   := /dev/null
endif

SRCS = src/main.cpp \
       include/MoExpression.cpp \
       include/MoStatement.cpp \
       include/MoRun.cpp

OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(TARGET) 2>$(NULL) || true