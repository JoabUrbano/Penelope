all: penelope_parser

penelope_parser: parser.tab.c lex.yy.c ./utils/hashMap/hashMap.c ./utils/uniqueIdentifier/uniqueIdentifier.c
	gcc -o penelope_parser parser.tab.c lex.yy.c ./utils/hashMap/hashMap.c ./utils/uniqueIdentifier/uniqueIdentifier.c -lfl -luuid -lm

parser.tab.c parser.tab.h: parserPenelope.y
	bison -d -v -g -o parser.tab.c parserPenelope.y

lex.yy.c: lexerPenelope.I
	flex -o lex.yy.c lexerPenelope.I

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h parser.output penelope_parser

.PHONY: all clean
