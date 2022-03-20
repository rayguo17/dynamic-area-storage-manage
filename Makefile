DRIVER = ./sdriver.pl
PROG = ./main
CC = gcc
FILES = $(PROG)
OUTPUT = ./main.out

all: $(FILES)

.PHONY: test 
test:  test01 test02 test03 test04 test05 test06 test07 test08 test09 test10 test11 test12 test13 test14
	
test01:
	$(DRIVER) -t trace01.txt -p $(PROG) > $(OUTPUT)
test02:
	$(DRIVER) -t trace02.txt -p $(PROG) >> $(OUTPUT)
test03:
	$(DRIVER) -t trace03.txt -p $(PROG) >> $(OUTPUT)
test04:
	$(DRIVER) -t trace04.txt -p $(PROG) >> $(OUTPUT)
test05:
	$(DRIVER) -t trace05.txt -p $(PROG) >> $(OUTPUT)
test06:
	$(DRIVER) -t trace06.txt -p $(PROG) >> $(OUTPUT)
test07:
	$(DRIVER) -t trace07.txt -p $(PROG) >> $(OUTPUT)
test08:
	$(DRIVER) -t trace08.txt -p $(PROG) >> $(OUTPUT)
test09:
	$(DRIVER) -t trace09.txt -p $(PROG) >> $(OUTPUT)
test10:
	$(DRIVER) -t trace10.txt -p $(PROG) >> $(OUTPUT)
test11:
	$(DRIVER) -t trace11.txt -p $(PROG) >> $(OUTPUT)
test12:
	$(DRIVER) -t trace12.txt -p $(PROG) >> $(OUTPUT)
test13:
	$(DRIVER) -t trace13.txt -p $(PROG) >> $(OUTPUT)
test14:
	$(DRIVER) -t trace14.txt -p $(PROG) >> $(OUTPUT)