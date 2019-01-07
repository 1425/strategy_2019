CXXFLAGS+=-Wall -Wextra -Werror -std=c++1z

test: table
	./table --standard

.PHONY: clean
clean:
	rm -f table
	rm -rf output
