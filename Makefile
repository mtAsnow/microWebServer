MAINSOURCE := Main.cpp
SOURCE  := $(wildcard *.cpp base/*.cpp )
override SOURCE := $(filter-out $(MAINSOURCE),$(SOURCE))
OBJS    := $(patsubst %.cpp,%.o,$(SOURCE))

TARGET  := WebServer
CC      := g++
LIBS    := -lpthread #
INCLUDE:= -I./usr/local/lib
CFLAGS  := -std=c++11 -g -Wall -O0 -D_PTHREADS
CXXFLAGS:= $(CFLAGS)

.PHONY : objs clean veryclean rebuild all debug
all : $(TARGET) 
objs : $(OBJS)
rebuild: veryclean all

clean :
	find . -name '*.o' | xargs rm -f
veryclean :
	find . -name '*.o' | xargs rm -f
	find . -name $(TARGET) | xargs rm -f
debug:
	@echo $(SOURCE)

$(TARGET) : $(OBJS) Main.o
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
# $@代表目标，这里是$(TARGET)

