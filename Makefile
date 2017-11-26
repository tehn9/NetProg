#fisier folosit pentru compilarea serverului&clientului TCP iterativ

all:
	gcc -pthread servTcpChat.c -o servTcpChat
	gcc cliTcpChat.c -o cliTcpChat
clean:
	rm -f *~cliTcpChat servTcpChat
