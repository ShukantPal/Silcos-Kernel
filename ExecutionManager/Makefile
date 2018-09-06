# S i l c o s   (E x e c u t i o n   M a n a g e r)   M a k e f i l e
#    C o p y r i g h t ( C ) 2 0 1 7  -  S h u k a n t   P a l

# I f c  -  H e a d e r   D i r e c t o r i e s
IfcKern = ../Interface/KERNEL.h ../Interface/TYPE.h
IfcArch = ../Interface/$(ArchDir)
IfcACPI = ../Interface/ACPI
IfcHAL = ../Interface/HAL
IfcRunnable = ../Interface/Executable
IfcMemory = ../Interface/Memory
IfcModule = ../Interface/Module
IfcUtil = ../Interface/Util

# C o m p i l a t i o n   U n i t s
#     (O b j e c t   F i l e s)

COM_IRQ = Compile/IRQ
SRC_IRQ = Source/IRQ

COM_SCHED = Compile/Scheduler
SRC_SCHED = Source/Scheduler

COM_TIME = Compile/Timer
SRC_TIME = Source/Timer

COM_TSK = Compile/Task
SRC_TSK = Source/Task

#
# I R Q   S u b s y s t e m
#

$(COM_IRQ)/IRQHandler.o: $(SRC_IRQ)/IRQHandler.cpp
	$(CC) $(CFLAGS) $(SRC_IRQ)/IRQHandler.cpp -o $(COM_IRQ)/IRQHandler.o

IRQ_Build = $(COM_IRQ)/IRQHandler.o

#
# S c h e d u l i n g   S u b s y s t e m
#

Sched_Build = $(COM_SCHED)/Scheduler.o $(COM_SCHED)/ScheduleRoller.o \
$(COM_SCHED)/RoundRobin.o $(COM_SCHED)/RunqueueBalancer.o

Tsk_Build = $(COM_TSK)/AVLTree.o $(COM_TSK)/Thread.o

#
# T i m e r   M a n a g e m e n t   S u b s y s t e m
#

$(COM_TIME)/EventGroup.o: $(SRC_TIME)/EventGroup.cpp
	$(CC) $(CFLAGS) $(SRC_TIME)/EventGroup.cpp -o $(COM_TIME)/EventGroup.o

$(COM_TIME)/PIT.o: $(SRC_TIME)/PIT.cpp
	$(CC) $(CFLAGS) $(SRC_TIME)/PIT.cpp -o $(COM_TIME)/PIT.o

$(COM_TIME)/NodeSorter.o: $(SRC_TIME)/NodeSorter.cpp
	$(CC) $(CFLAGS) $(SRC_TIME)/NodeSorter.cpp -o $(COM_TIME)/NodeSorter.o

$(COM_TIME)/HardwareTimer.o: $(SRC_TIME)/HardwareTimer.cpp
	$(CC) $(CFLAGS) $(SRC_TIME)/HardwareTimer.cpp -o $(COM_TIME)/HardwareTimer.o

$(COM_TIME)/HPET.o: $(SRC_TIME)/HPET.cpp
	$(CC) $(CFLAGS) $(SRC_TIME)/HPET.cpp -o $(COM_TIME)/HPET.o

$(COM_TIME)/Timeline.o: $(SRC_TIME)/Timeline.cpp
	$(CC) $(CFLAGS) $(SRC_TIME)/Timeline.cpp -o $(COM_TIME)/Timeline.o

Time_Build = $(COM_TIME)/EventGroup.o $(COM_TIME)/Timeline.o \
$(COM_TIME)/NodeSorter.o \
$(COM_TIME)/HPET.o $(COM_TIME)/PIT.o $(COM_TIME)/HardwareTimer.o

#
# S c h e d u l e r   C o m p i l e   R u l e s
#

$(COM_SCHED)/Scheduler.o: $(SRC_SCHED)/Scheduler.cpp
	$(CC) $(CFLAGS) $(SRC_SCHED)/Scheduler.cpp -o $(COM_SCHED)/Scheduler.o

$(COM_SCHED)/ScheduleRoller.o: $(SRC_SCHED)/ScheduleRoller.cpp
	$(CC) $(CFLAGS) $(SRC_SCHED)/ScheduleRoller.cpp -o $(COM_SCHED)/ScheduleRoller.o

$(COM_SCHED)/RoundRobin.o: $(SRC_SCHED)/RoundRobin.cpp
	$(CC) $(CFLAGS) $(SRC_SCHED)/RoundRobin.cpp -o $(COM_SCHED)/RoundRobin.o

$(COM_SCHED)/RunqueueBalancer.o: $(SRC_SCHED)/RunqueueBalancer.cpp
	$(CC) $(CFLAGS) $(SRC_SCHED)/RunqueueBalancer.cpp -o $(COM_SCHED)/RunqueueBalancer.o

$(COM_SCHED)/RR.o: $(IfcHAL)/Processor.h $(IfcRunnable)/Scheduler.h $(IfcRunnable)/RoundRobin.h $(SRC_SCHED)/RR.cpp
	$(CC) $(CFLAGS) $(SRC_SCHED)/RR.cpp -o $(COM_SCHED)/RR.o
	
#
# T a s k   S u b s y s t e m
#

$(COM_TSK)/AVLTree.o: $(SRC_TSK)/AVLTree.cpp
	$(CC) $(CFLAGS) $(SRC_TSK)/AVLTree.cpp -o $(COM_TSK)/AVLTree.o

$(COM_TSK)/Thread.o: $(SRC_TSK)/Thread.cpp
	$(CC) $(CFLAGS) $(SRC_TSK)/Thread.cpp -o $(COM_TSK)/Thread.o
	
ExMake: $(IRQ_Build) $(Sched_Build) $(Time_Build) $(Tsk_Build)
	$(CC) $(Sched_Build) $(Time_Build) \
	$(Tsk_Build) $(IRQ_Build) $(LFLAGS) -o Build/kernel.silcos.excmgr
	cp Build/kernel.silcos.excmgr ../Modules/Builtin