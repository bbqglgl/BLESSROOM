CFLAGS = -W -Wall

all:
	gcc ${CFLAGS} -c net_lib.c
	ar crv ../libmynet.a net_lib.o

example:
	gcc -o ex_ser example_server.c -L../ -lmynet ${CFLAGS} -pthread
	gcc -o ex_cli example_client.c -L../ -lmynet ${CFLAGS} -pthread
