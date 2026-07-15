# compiler
CC=clang++ -Werror -O3 -std=c++14 -Isrc/external/asio-1.36.0/include/

all: src/build/program

# src/build/program: src/code/sources/main.cpp src/code/sources/ProxyUtils.cpp src/code/sources/AtomicSPSCQueue.cpp src/code/sources/AsioSocket.cpp src/code/sources/SimpleSocket.cpp
# 	$(CC) $^ -o $@

src/build/program: src/build/main.o src/build/ProxyUtils.o src/build/AsioSocket.o
	$(CC) $^ -o $@

src/build/main.o: src/code/sources/main.cpp
	$(CC) -c $^ -o $@

src/build/ProxyUtils.o: src/code/sources/ProxyUtils.cpp
	$(CC) -c $^ -o $@

src/build/AsioSocket.o: src/code/sources/AsioSocket.cpp
	$(CC) -c $^ -o $@

clean:
	rm -rf src/build/*