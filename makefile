all: penelope_parser

# Diretórios
SRC_DIR = src
BUILD_DIR = build
LEXER_DIR = $(SRC_DIR)/lexer

# Arquivos fonte
PARSER_SOURCES = $(SRC_DIR)/parser/symbolTable/symbolTable.c $(SRC_DIR)/parser/codeGenerator/codeGenerator.c $(SRC_DIR)/parser/semantics/semantics.c $(SRC_DIR)/parser/expressionUtils/expressionUtils.c $(SRC_DIR)/parser/grammarActions/grammarActions.c
UTIL_SOURCES = $(SRC_DIR)/utils/hashMap/hashMap.c $(SRC_DIR)/utils/uniqueIdentifier/uniqueIdentifier.c
STRUCT_SOURCES = $(SRC_DIR)/structs/expression/expressionResult.c $(SRC_DIR)/structs/lvalue/lvalueResult.c
MAIN_SOURCE = $(SRC_DIR)/main.c
ALL_SOURCES = $(PARSER_SOURCES) $(UTIL_SOURCES) $(STRUCT_SOURCES) $(MAIN_SOURCE)

penelope_parser: $(BUILD_DIR)/parser.tab.c $(BUILD_DIR)/lex.yy.c $(ALL_SOURCES) | $(BUILD_DIR)
	gcc -o penelope_parser $(BUILD_DIR)/parser.tab.c $(BUILD_DIR)/lex.yy.c $(ALL_SOURCES) -lfl -luuid -lm

$(BUILD_DIR)/parser.tab.c $(BUILD_DIR)/parser.tab.h: $(SRC_DIR)/parser/penelope.y | $(BUILD_DIR)
	bison -d -v -g -o $(BUILD_DIR)/parser.tab.c $(SRC_DIR)/parser/penelope.y

$(BUILD_DIR)/lex.yy.c: $(LEXER_DIR)/penelope.l | $(BUILD_DIR)
	flex -o $(BUILD_DIR)/lex.yy.c $(LEXER_DIR)/penelope.l

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -f penelope_parser
	rm -rf $(BUILD_DIR)

.PHONY: all clean
