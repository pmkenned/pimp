all:
	gcc -pthread server.c csapp.c -o server
	gcc -pthread client.c csapp.c -o client

clean:
	rm *.exe
