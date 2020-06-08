CFLAGS = -W -Wall

all:
	gcc ${CFLAGS} -c net_lib.c
	ar crv ../libmynet.a net_lib.o

example:
	gcc ${CFLAGS} -c example_server.c
	gcc ${CFLAGS} -c example_client.c
	gcc -o ex_ser example_server.o -L../ -lmynet ${CFLAGS} -pthread
	gcc -o ex_cli example_client.o -L../ -lmynet ${CFLAGS} -pthread
