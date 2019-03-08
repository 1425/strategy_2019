CXXFLAGS+=-Wall -Wextra -Werror -std=c++1z

test: table strategy
	time ./strategy --picklist

strategy: table.cpp util.cpp climb.cpp data.cpp climb2.cpp

table: util.cpp
	$(CXX) $(CXXFLAGS) -DTABLE_MAIN $< $@.cpp -o $@

climb2: util.cpp data.cpp climb.cpp

.PHONY: clean
clean:
	rm -f table strategy climb2
	rm -rf output
