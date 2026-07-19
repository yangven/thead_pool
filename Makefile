CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -pthread
TARGET := thread_pool_test
SOURCES := test.cpp ThreadPool.cpp TaskQueue.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)

.PHONY: run clean
