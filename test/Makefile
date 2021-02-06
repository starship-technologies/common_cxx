CPPFLAGS += -std=c++14
CPPFLAGS += -I..
CPPFLAGS += -g
CPPFLAGS += -O2
CPPFLAGS += -W -Wall
CPPFLAGS += $(ARCH_CPPFLAGS)
CPPFLAGS += $(EXTRA_CPPFLAGS)

LDFLAGS_GTEST := -lgtest -lgtest_main

ARCH_CPPFLAGS := -march=native

all: run_tests

test_string_view: test_string_view.cpp ../common/string_view.hpp ../common/array_view.hpp
	$(CXX) -o $@ $(CPPFLAGS) $< $(LDFLAGS)
TESTS += test_string_view

test_array_view: test_array_view.cpp ../common/array_view.hpp ../common/reverse_iterable.hpp
	$(CXX) -o $@ $(CPPFLAGS) $< $(LDFLAGS)
TESTS += test_array_view

$(TESTS): LDFLAGS += $(LDFLAGS_GTEST)

run_tests: $(TESTS)
	set -e
	for test in $(TESTS); do \
	  ./$$test; \
	done

tests: $(TESTS)

clean:
	$(RM) $(TESTS)

