FLAGS = -g -Wall

jkutop: jkutop.o readstatus.o sorting.o printing.o
	${CC} ${FLAGS} -lncurses -o jkutop jkutop.o readstatus.o sorting.o printing.o

jkutop.o: def.h jkutop.c
	${CC} ${FLAGS} -c jkutop.c -o jkutop.o

readstatus.o: def.h readstatus.c
	${CC} ${FLAGS} -c readstatus.c -o readstatus.o

sorting.o: def.h sorting.c
	${CC} ${FLAGS} -c sorting.c -o sorting.o

printing.o: def.h printing.c
	${CC} ${FLAGS} -c printing.c -o printing.o

clean:
	rm *.o jkutop
