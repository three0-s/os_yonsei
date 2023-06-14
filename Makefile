CC = g++
CXXFLAGS = -std=c++17 -g
SRCS = core.cpp kernel.cpp main.cpp


project3 : $(SRCS)
	$(CC) $(CXXFLAGS) $(SRCS) -o project3

clean:
	rm project3