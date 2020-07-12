CFLAGS=-Wall -g
MYSQL=./mysqlite

all:
	cc $(CFLAGS) db.c -o mysqlite
clean:
	rm -rf mysqlite
run:
	$(MYSQL)
