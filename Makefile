CXXFLAGS=-I.
LDFLAGS=-lboost_thread

atomictest: atomictest.o lib/fallback.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf atomictest atomictest.o lib/fallback.o