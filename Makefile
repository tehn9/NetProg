#fisier folosit pentru compilarea serverului&clientului TCP iterativ

all:
	gcc servTcpChat.c -o servTcpChat
	gcc cliTcpChat.c -o cliTcpChat
clean:
	rm -f *~cliTcpChat servTcpChat