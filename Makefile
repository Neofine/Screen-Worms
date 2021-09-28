.PHONY: all clean

all: server client

server: communicator.o utility.o server.o events.o
	g++ -Wall -Wextra -O2 -std=c++17 -o screen-worms-server server.o communicator.o utility.o events.o -lstdc++fs

utility.o: Server/utility.cpp Server/utility.h Server/structures.h
	g++ -Wall -Wextra -O2 -std=c++17 -lstdc++fs -c $<

events.o: Server/events.cpp Server/events.h Server/structures.h
	g++ -Wall -Wextra -O2 -std=c++17 -lstdc++fs -c $<

communicator.o: Server/communicator.cpp Server/communicator.h Server/structures.h Server/utility.h
	g++ -Wall -Wextra -O2 -std=c++17 -lstdc++fs -c $<

server.o: Server/server.cpp Server/communicator.h
	g++ -Wall -Wextra -O2 -std=c++17 -lstdc++fs -c $<



client: client.o setup.o client_utility.o client_events.o sending_info.o
	g++ -Wall -Wextra -O2 -std=c++17 -o screen-worms-client client.o setup.o client_utility.o client_events.o sending_info.o -lstdc++fs

setup.o: setup.cpp setup.h Client/client_structures.h
	g++ -Wall -Wextra -O2 -std=c++17 -lstdc++fs -c $<

client_utility.o: Client/client_utility.cpp Client/client_utility.h Client/client_structures.h
	g++ -Wall -Wextra -O2 -std=c++17 -lstdc++fs -c $<

client_events.o: Client/client_events.cpp Client/client_events.h Client/client_structures.h
	g++ -Wall -Wextra -O2 -std=c++17 -lstdc++fs -c $<

sending_info.o: Client/sending_info.cpp Client/sending_info.h Client/client_structures.h
	g++ -Wall -Wextra -O2 -std=c++17 -lstdc++fs -c $<

client.o: Client/client.cpp
	g++ -Wall -Wextra -O2 -std=c++17 -lstdc++fs -c $<
	
clean:
	rm -f *.o screen-worms-server
	rm -f *.o screen-worms-client