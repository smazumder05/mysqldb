CFLAGS=-Wall -g
.PHONY

all:
	gcc -Wall db.c -o mysqlite
clean:
	rm -rf mysqlite
run:
	./mysqlite
