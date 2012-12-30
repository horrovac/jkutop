FLAGS = -g -Wall -I/usr/src/linux-2.6.32.49-0.3/include

swapwatch: getpids.o readstatus.o
	${CC} ${FLAGS} -o swapwatch readstatus.o getpids.o

getpids.o: def.h getpids.c
	${CC} ${FLAGS} -c getpids.c -o getpids.o

readstatus.o: def.h readstatus.c
	${CC} ${FLAGS} -c readstatus.c -o readstatus.o

clean:
	rm *.o
