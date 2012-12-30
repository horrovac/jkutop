FLAGS = -g -Wall -I/usr/src/linux-2.6.32.49-0.3/include

swapwatch: jkutop.o readstatus.o sorting.o
	${CC} ${FLAGS} -o jkutop jkutop.o readstatus.o sorting.o

jkutop.o: def.h jkutop.c
	${CC} ${FLAGS} -c jkutop.c -o jkutop.o

readstatus.o: def.h readstatus.c
	${CC} ${FLAGS} -c readstatus.c -o readstatus.o

sorting.o: def.h sorting.c
	${CC} ${FLAGS} -c sorting.c -o sorting.o

clean:
	rm *.o jkutop
