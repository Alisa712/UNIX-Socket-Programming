#makefile to run EE450 socket programming project

all: clean
	g++ -std=c++11 -pthread -o servers servers.cpp
	g++ -std=c++11 -o aws aws.cpp
	g++ -o client client.cpp

servers:
	./servers
	exec bash

aws:
	./aws
	exec bash

clean:
	rm -f servers
	rm -f aws
	rm -f client

test: all
	gnome-terminal -e "bash -c \"./servers;\""
	gnome-terminal -e "bash -c \"./aws\""
	
testinput: test
	gnome-terminal -e "bash -c \"./client $(COMMANDS) $(VALUES); exec bash\""

.PHONY: all servers aws clean test testinput

