all: test
test: test.cpp suiElastic.cpp
	g++ -o $@ $^ -std=c++17 -lelasticlient -lcpr -ljsoncpp -lcurl