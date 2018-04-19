all: timer server resettimer

timer: timercreator.c
	gcc timercreator.c -o timer -lrt
 
resettimer: resettimer.c
	gcc resettimer.c -o resettimer -lrt
 
server: server.c
	gcc -O2 server.c -o server
  
clean:
	rm -rf *.o timer server resettimer