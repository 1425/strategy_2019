CXXFLAGS+=-Wall -Wextra -Werror -std=c++1z -g

test: table strategy
	time ./strategy --picklist

strategy: table.cpp util.cpp

table: util.cpp
	$(CXX) $(CXXFLAGS) -DTABLE_MAIN $< $@.cpp -o $@

.PHONY: clean
clean:
	rm -f table strategy
	rm -rf output
