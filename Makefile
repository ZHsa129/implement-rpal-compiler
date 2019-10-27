all: p2

p2:  token.o parser.o p2.o  
	g++ -std=gnu++0x *.o -o p2
%.o: %.cpp 
	g++ -c -std=gnu++0x $^
clean:
	rm -rf *.o p2