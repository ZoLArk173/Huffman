COMPILER = g++
CPPSTD = c++17

NAME = hfc

# .PHONY: all 

all:
	$(COMPILER) -o$(NAME) main.cpp -std=$(CPPSTD) 

timer:
	$(COMPILER) -o$(NAME) main.cpp -std=$(CPPSTD) -DTIMER 

debug:
	$(COMPILER) -o$(NAME) main.cpp -std=$(CPPSTD) -DDEBUG 

pmf:
	$(COMPILER) -o$(NAME) main.cpp -std=$(CPPSTD) -DPMF 

clean:
	rm $(NAME)