all:
	gcc server.c csapp.c -o server
	gcc client.c csapp.c -o client

clean:
	rm *.exe
