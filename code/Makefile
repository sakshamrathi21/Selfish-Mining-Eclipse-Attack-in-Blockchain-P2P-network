OS := $(shell uname -s)

ifeq ($(OS), Darwin)  # macOS
    OPENSSL_PATH=$(shell brew --prefix openssl)
else  # Linux (assumes OpenSSL is installed in system paths)
    OPENSSL_PATH=/usr
endif

CXX=g++
CXXFLAGS=-w -std=c++23 -O2 -I$(OPENSSL_PATH)/include
LDFLAGS=-L$(OPENSSL_PATH)/lib -lssl -lcrypto

run: *.cpp *.h
	$(CXX) $(CXXFLAGS) block.cpp blockchain.cpp transaction.cpp peer.cpp simulator.cpp helper.cpp hash.cpp main.cpp -o run $(LDFLAGS)

.PHONY: clean
clean:
	rm -f run log.txt
	rm -rf blockchain_data blockchain_graphs logFiles
