FLAGS = -g -Wall

jkutop: jkutop.o readstatus.o readsmaps.o readmeminfo.o sorting.o printing.o
	${CC} ${FLAGS} -lncurses -o jkutop jkutop.o readstatus.o readsmaps.o readmeminfo.o sorting.o printing.o

jkutop.o: jkutop.h jkutop.c
	${CC} ${FLAGS} -c jkutop.c -o jkutop.o

readstatus.o: jkutop.h readstatus.c
	${CC} ${FLAGS} -c readstatus.c -o readstatus.o

readsmaps.o: jkutop.h readsmaps.c
	${CC} ${FLAGS} -c readsmaps.c -o readsmaps.o

readmeminfo.o: jkutop.h readmeminfo.c
	${CC} ${FLAGS} -c readmeminfo.c -o readmeminfo.o

sorting.o: jkutop.h sorting.c
	${CC} ${FLAGS} -c sorting.c -o sorting.o

printing.o: jkutop.h printing.c
	${CC} ${FLAGS} -c printing.c -o printing.o

clean:
	rm *.o jkutop
