all: penelope_parser

# Source files
PARSER_SOURCES = ./parser/symbolTable.c ./parser/codeGenerator.c ./parser/semantics.c ./parser/expressionUtils.c
UTIL_SOURCES = ./utils/hashMap/hashMap.c ./utils/uniqueIdentifier/uniqueIdentifier.c
STRUCT_SOURCES = ./structs/expression/expressionResult.c ./structs/lvalue/lvalueResult.c
ALL_SOURCES = $(PARSER_SOURCES) $(UTIL_SOURCES) $(STRUCT_SOURCES)

penelope_parser: parser.tab.c lex.yy.c $(ALL_SOURCES)
	gcc -o penelope_parser parser.tab.c lex.yy.c $(ALL_SOURCES) -lfl -luuid -lm

parser.tab.c parser.tab.h: parserPenelope.y
	bison -d -v -g -o parser.tab.c parserPenelope.y

lex.yy.c: lexerPenelope.I
	flex -o lex.yy.c lexerPenelope.I

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h parser.output penelope_parser

.PHONY: all clean
