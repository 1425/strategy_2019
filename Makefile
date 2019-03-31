CXXFLAGS+=-Wall -Wextra -Werror -std=c++1z -Ofast -flto -march=native -mtune=native

test: table strategy
	time ./strategy --team 2910 --file data/2019wamou.csv

strategy: table.cpp util.cpp climb.cpp data.cpp climb2.cpp

table: util.cpp
	$(CXX) $(CXXFLAGS) -DTABLE_MAIN $< $@.cpp -o $@

climb2: util.cpp data.cpp climb.cpp

explore: util.cpp

.PHONY: clean
clean:
	rm -f table strategy climb2
	rm -rf output
