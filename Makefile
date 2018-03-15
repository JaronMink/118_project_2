all: main server client

JJP: JJP.cpp Receiver.cpp Sender.cpp Packer.cpp
	g++ -std=c++11 -pthread -Wall -O2 -c JJP.cpp Receiver.cpp Sender.cpp Packer.cpp
main: JJP main.cpp JJP.o
	g++ -std=c++11 -pthread -Wall -O2 -o main main.cpp JJP.o Receiver.o Packer.o Sender.o
server:
	g++ -std=c++11 -pthread -Wall -O2 -o server server.cpp JJP.o Receiver.o Packer.o Sender.o
client:
	g++ -std=c++11 -pthread -Wall -O2 -o client client.cpp JJP.o Receiver.o Packer.o Sender.o
clean:
	rm -f JJP.o Receiver.o Packer.o Sender.o main server client
