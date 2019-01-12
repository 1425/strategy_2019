CXXFLAGS+=-Wall -Wextra -Werror -std=c++1z -Ofast -flto

test: table
	time ./table --strategy

.PHONY: clean
clean:
	rm -f table
	rm -rf output
