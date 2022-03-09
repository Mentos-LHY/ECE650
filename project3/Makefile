all: ringmaster player

ringmaster: ringmaster.cpp tcp_actions.cpp potato.h
	g++ -g -o ringmaster ringmaster.cpp tcp_actions.cpp 

player: player.cpp tcp_actions.cpp potato.h
	g++ -g -o player player.cpp tcp_actions.cpp 

.PHONY: clean
clean:
	rm -f *.o *~ *# ringmaster player
