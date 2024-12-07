all: Task.o Scheduler.o System.o main.o
	g++ -o main Task.o Scheduler.o System.o main.o

Task.o: Task.cpp Task.h
	g++ -c Task.cpp
Scheduler.o: Scheduler.cpp Scheduler.h
	g++ -c Scheduler.cpp
System.o: System.cpp System.h
	g++ -c System.cpp

main.o: main.cpp Task.h Scheduler.h System.h
	g++ -c main.cpp

clean:
	rm *.o