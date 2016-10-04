%: %.cpp
	g++-5 -std=c++11 $< -o $@

%: %.c
	gcc $< -o $@

