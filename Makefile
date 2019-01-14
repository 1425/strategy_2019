CXXFLAGS+=-Wall -Wextra -Werror -std=c++1z -Ofast -flto

test: table strategy
	time ./strategy --strategy

strategy: table.cpp util.cpp

table: util.cpp
	$(CXX) $(CXXFLAGS) -DTABLE_MAIN $< $@.cpp -o $@

.PHONY: clean
clean:
	rm -f table strategy
	rm -rf output
