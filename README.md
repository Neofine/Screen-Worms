# Computer Networks - Screen Worms

Big project of Computer Networks course, the goal was to make server and client to given GUI to make fully functional online game supporting up to 25 players.

Connection protocols were:
- Client to GUI (and vice versa) - UDP
- Client to Server (and vice versa) - TCP

Game is started when every connected client has made non-forward move.

Game is played in turns, every second there are played tens of turns (depending on given flags while deploying a server).

Last worm alive wins!

To start a game you need to first type:

```
make
```

Server is started by:

```
./screen-worms-server [-p n] [-s n] [-t n] [-v n] [-w n] [-h n]
```

where:
- -p flag is port number (default 2021)
- -s flag is seed to deterministic number generator (default time(NULL))
- -t flag is the turning speed of a worm (default 6)
- -v flag is the amount of rounds per second (default 50)
- -w and -h flags are width and height of map in pixels respectively (default 640x480)

Client is started by:

```
./screen-worms-client game_server [-n player_name] [-p n] [-i gui_server] [-r n]
```

where:
- game_server is IPv4 or IPv6 server address (or its name)
- -n flag is player name
- -p flag is game server port number (default 2021)
- -i is IPv4 or IPv6 GUI address (or its name, default localhost)
- -r is GUI port number (default 20210)

GUI is started by typing 

```
./gui2 [-p n]
```

where -p is its port number (default 20210).