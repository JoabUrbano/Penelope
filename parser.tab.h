/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 142 "parserPenelope.y"

    #include "./structs/expression/expressionResult.h"
    #include "./structs/lvalue/lvalueResult.h"

#line 54 "parser.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    BOOL = 258,                    /* BOOL  */
    ID = 259,                      /* ID  */
    TYPE = 260,                    /* TYPE  */
    STRING = 261,                  /* STRING  */
    NUMBER = 262,                  /* NUMBER  */
    BREAK = 263,                   /* BREAK  */
    AND = 264,                     /* AND  */
    OR = 265,                      /* OR  */
    INT = 266,                     /* INT  */
    FLOAT = 267,                   /* FLOAT  */
    FUN = 268,                     /* FUN  */
    WHILE = 269,                   /* WHILE  */
    FOR = 270,                     /* FOR  */
    IF = 271,                      /* IF  */
    ELSE = 272,                    /* ELSE  */
    LEN = 273,                     /* LEN  */
    PRINT = 274,                   /* PRINT  */
    RETURN = 275,                  /* RETURN  */
    LBRACKET = 276,                /* LBRACKET  */
    RBRACKET = 277,                /* RBRACKET  */
    COMMA = 278,                   /* COMMA  */
    LPAREN = 279,                  /* LPAREN  */
    RPAREN = 280,                  /* RPAREN  */
    COLON = 281,                   /* COLON  */
    SEMICOLON = 282,               /* SEMICOLON  */
    NEWLINE = 283,                 /* NEWLINE  */
    ASSIGNMENT = 284,              /* ASSIGNMENT  */
    EQUALS = 285,                  /* EQUALS  */
    SMALLEREQUALS = 286,           /* SMALLEREQUALS  */
    BIGGEREQUALS = 287,            /* BIGGEREQUALS  */
    SMALLER = 288,                 /* SMALLER  */
    BIGGER = 289,                  /* BIGGER  */
    INCREMENT = 290,               /* INCREMENT  */
    DECREMENT = 291,               /* DECREMENT  */
    EXPONENTIATION = 292,          /* EXPONENTIATION  */
    MULTIPLICATION = 293,          /* MULTIPLICATION  */
    DIVISION = 294,                /* DIVISION  */
    ADDITION = 295,                /* ADDITION  */
    SUBTRACTION = 296,             /* SUBTRACTION  */
    LBRACE = 297,                  /* LBRACE  */
    RBRACE = 298,                  /* RBRACE  */
    UMINUS = 299,                  /* UMINUS  */
    LOWER_THAN_ELSE = 300          /* LOWER_THAN_ELSE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 174 "parserPenelope.y"

    char *str;
    double num;
    ExpressionResult* exprResult;
    LValueResult* lvalueResult;

#line 123 "parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
