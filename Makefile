firefly:
	gcc socket_server.c firefly_main.c -o firefly -lpthread 
client:	
	gcc client.c -o client -lpthread
clean:
	rm -rf firefly
	rm -rf client
