assembler: assembler.o output.o readings.o var.o helper.o instruction.o command.o main_header.h
	gcc -g -ansi -pedantic -Wall assembler.o output.o readings.o var.o helper.o instruction.o command.o -o assembler

assembler.o: assembler.c main_header.h
	gcc -g -c -ansi -pedantic -Wall assembler.c -o assembler.o
var.o: var.c main_header.h
	gcc -g -c -ansi -pedantic -Wall var.c -o var.o
helper.o: helper.c main_header.h
	gcc -g -c -ansi -pedantic -Wall helper.c -o helper.o
instruction.o: instruction.c main_header.h
	gcc -g -c -ansi -pedantic -Wall instruction.c -o instruction.o
command.o: command.c main_header.h
	gcc -g -c -ansi -pedantic -Wall command.c -o command.o
readings.o: readings.c main_header.h
	gcc -g -c -ansi -pedantic -Wall readings.c -o readings.o
output.o: output.c main_header.h
	gcc -g -c -ansi -pedantic -Wall output.c -o output.o
