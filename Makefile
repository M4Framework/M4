CXX = g++
CXXFLAGS = -g -Wall -O2 -std=c++17 -lm

all: tdigest mreq dd

tdigest:
	rm -f tdigest
	$(CXX) $(CXXFLAGS) -D TEST_TD main.cpp -o tdigest

mreq:
	rm -f mreq
	$(CXX) $(CXXFLAGS) -D TEST_MREQ main.cpp -o mreq

dd:
	rm -f dd
	$(CXX) $(CXXFLAGS) -D TEST_DD main.cpp -o dd

clean:
	rm -f tdigest mreq dd

.PHONY: all tdigest mreq dd clean
