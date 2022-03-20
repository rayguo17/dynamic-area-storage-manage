DRIVER = ./sdriver.pl
PROG = ./main
CC = gcc
FILES = $(PROG)
OUTPUT = ./main.out

all: $(FILES)

.PHONY:test01 test02 test
test: test01 test02
	
test01:
	$(DRIVER) -t trace01.txt -p $(PROG) >> $(OUTPUT)
test02:
	$(DRIVER) -t trace02.txt -p $(PROG) >> $(OUTPUT)