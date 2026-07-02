ALL : put get
put : put.cpp suiSerSearch.cpp
	g++ $^ -o $@ -std=c++17 -letcd-cpp-api -lcpprest
get : get.cpp suiSerSearch.cpp
	g++ $^ -o $@ -std=c++17 -letcd-cpp-api -lcpprest

