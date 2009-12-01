CXXFLAGS=-I. -g -Wall -O2
LDFLAGS=-lboost_thread

atomictest: atomictest.o lib/atomic/src/fallback.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf atomictest atomictest.o lib/atomic/src/fallback.o
