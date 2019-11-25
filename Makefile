firefly:
	gcc socket_server.c firefly_main.c -o firefly -lpthread 
client:	
	gcc client.c -o client
clean:
	rm -rf firefly
	rm -rf client
