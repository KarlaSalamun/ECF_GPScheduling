CXX = g++
WARN = -Wall
CFLAGS = $(WARN) -ggdb3
LDFLAGS = $(WARN) -lecf

OBJS = TaskEvalOp.o \
       main.o \
       Task.o \
       PeriodicSimulator.o \
       Task_p.o \
       Scheduler.o

BINFILE = sched.elf

.PHONY = all clean

all: $(BINFILE)

$(BINFILE): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(BINFILE)

%.o: %.cpp TaskEvalOp.h Nodes/ddNode.h Nodes/AddNode.h Nodes/DivNode.h Nodes/MulNode.h Nodes/NrNode.h Nodes/PosNode.h Nodes/ptNode.h Nodes/SDNode.h Nodes/SLNode.h Nodes/SPrNode.h Nodes/SubNode.h Nodes/WNode.h PeriodicSimulator.h Task_p.h Scheduler.h
	$(CXX) $(CFLAGS) $< -c -o $@

clean:
	rm -f *~ $(OBJS) $(BINFILE)
