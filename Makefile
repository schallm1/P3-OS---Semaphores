
all: master slave

master: master.o
	gcc -o master master.o

slave: slave.o
	gcc -o slave slave.o

master.o: master.c
	gcc -c master.c

slave.o: slave.c
	gcc -c slave.c


clean:
	rm -f *.o
	rm -f master
	rm -f slave
	rm -f logfile.*
	rm -f cstest

