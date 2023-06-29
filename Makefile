# Compiler
CC := g++
# Compiler flags
CFLAGS := -std=c++20 -Wall -Wextra -O3 -I /mnt/c/libraries/fastflow-master/fastflow-master/

# Source files (excluding main.cpp)
SRCS := par_fastflow.cpp sequential.cpp utimer.cpp new_par_threads.cpp new_queue.cpp par_threads.cpp queue.cpp util.cpp
# Object files (excluding main.o)
OBJS := $(patsubst %.cpp,obj/%.o,$(SRCS))
# Header files
HDRS := src/utimer.h src/util.h

# Target executable
TARGET := bin/prog bin/seq bin/par_threads bin/par_ff bin/par_threads_old

.PHONY: all clean

all: $(TARGET)

bin/prog: obj/main.o $(OBJS)
	$(CC) -g obj/main.o $(OBJS) -o bin/prog

bin/seq: obj/main_seq.o $(OBJS)
	$(CC) -g obj/main_seq.o $(OBJS) -o bin/seq

bin/par_threads: obj/main_par_threads.o $(OBJS)
	$(CC) -g obj/main_par_threads.o $(OBJS) -o bin/par_threads

bin/par_ff: obj/main_par_ff.o $(OBJS)
	$(CC) -g obj/main_par_ff.o $(OBJS) -o bin/par_ff

bin/par_threads_old: obj/main_par_threads_old.o $(OBJS)
	$(CC) -g obj/main_par_threads_old.o $(OBJS) -o bin/par_threads_old

obj/%.o: src/%.cpp $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/* $(TARGET)
