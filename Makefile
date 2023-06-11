CC = g++
CXXFLAGS = -std=c++17
SRCS = core.cpp kernel.cpp main.cpp


project2 : $(SRCS)
	$(CC) $(CXXFLAGS) $(SRCS) -o project2

clean:
	rm project2