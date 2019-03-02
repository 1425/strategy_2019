CXXFLAGS+=-Wall -Wextra -Werror -std=c++1z -g

test: table strategy
	time ./strategy --picklist

strategy: table.cpp util.cpp climb.cpp data.cpp

table: util.cpp
	$(CXX) $(CXXFLAGS) -DTABLE_MAIN $< $@.cpp -o $@

climb2: util.cpp data.cpp climb.cpp

.PHONY: clean
clean:
	rm -f table strategy
	rm -rf output
