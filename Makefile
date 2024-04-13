SUBMITTERS := <student1-ID>_<student2-ID>
COMPILER := g++
COMPILER_FLAGS := --std=c++11 -Wall -g3
SRCS := Commands.cpp signals.cpp smash.cpp
OBJS=$(subst .cpp,.o,$(SRCS))
HDRS := Commands.h signals.h
SMASH_BIN := smash

$(SMASH_BIN): $(OBJS)
	$(COMPILER) $(COMPILER_FLAGS) $^ -o $@

$(OBJS): %.o: %.cpp
	$(COMPILER) $(COMPILER_FLAGS) -c $^

clean:
	rm -rf $(SMASH_BIN) $(OBJS) $(TESTS_OUTPUTS)
