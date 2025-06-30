all: penelope_parser

penelope_parser: parser.tab.c lex.yy.c semantica.o ./utils/hashMap/hashMap.c ./utils/uniqueIdentifier/uniqueIdentifier.c
	gcc -o penelope_parser parser.tab.c lex.yy.c semantica.o \
	    ./utils/hashMap/hashMap.c ./utils/uniqueIdentifier/uniqueIdentifier.c \
	    -lfl -luuid -lm

parser.tab.c parser.tab.h: parserPenelope.y
	bison -d -v -g -o parser.tab.c parserPenelope.y

lex.yy.c: lexerPenelope.I
	flex -o lex.yy.c lexerPenelope.I

semantica.o: ./semantica/semantica.c ./semantica/semantica.h
	gcc -c ./semantica/semantica.c -o semantica.o

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h parser.output penelope_parser semantica.o

.PHONY: all clean
