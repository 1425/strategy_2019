CXXFLAGS+=-Wall -Wextra -Werror -std=c++1z -O2 -pg

test: table
	./table --strategy

.PHONY: clean
clean:
	rm -f table
	rm -rf output
