/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parserPenelope.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "./utils/hashMap/hashMap.h"
#include "./utils/uniqueIdentifier/uniqueIdentifier.h"

#define MAX_SCOPE_DEPTH 100
char* scopeStack[MAX_SCOPE_DEPTH];
int scopeTop = -1;
int exec_block = 1;  // Flag de controle para if/else/while gustavo

HashMap symbolTable = { NULL };
char *currentScope = NULL;
int semantic_errors = 0; 
int last_condition_result = 0;  // serve pra lembrar o resultado da condição do IF



int are_types_compatible(const char* declaredType, const char* exprType) {
    if (strcmp(declaredType, exprType) == 0) return 1;
    if ((strcmp(declaredType, "float") == 0 && strcmp(exprType, "int") == 0) ||
      (strcmp(declaredType, "int") == 0 && strcmp(exprType, "float") == 0)) return 1;
    return 0;
}

// Função para extrair o tipo base de um tipo de array (ex: "int[]" -> "int")
char* get_array_element_type(const char* arrayType) {
    if (!arrayType || !strstr(arrayType, "[]")) {
        return NULL; // Não é um tipo de array
    }
    
    char* elementType = malloc(strlen(arrayType));
    strcpy(elementType, arrayType);
    
    // Remove o "[]" do final
    char* bracket = strstr(elementType, "[]");
    if (bracket) {
        *bracket = '\0';
    }
    
    return elementType;
}


void semantic_error(const char* format, ...) {
    extern int yylineno;
    fprintf(stderr, "Erro Semântico na linha %d: ", yylineno);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);  // Imprime a mensagem formatada
    va_end(args);

    fprintf(stderr, "\n");
    semantic_errors++;
}

Node* find_variable_in_scopes(char *name) {
    for (int i = scopeTop; i >= 0; i--) {
        char *scope = scopeStack[i];
        char *fullKey = malloc(strlen(scope) + strlen(name) + 2);
        sprintf(fullKey, "%s#%s", scope, name);
        
        Node *node = find_node(&symbolTable, fullKey);
        free(fullKey);

        if (node) {
            return node; // Variável encontrada
        }
    }
    return NULL; // Variável não encontrada em nenhum escopo
}

Node* find_variable_in_current_scope(char *name) {
    if (currentScope == NULL) return NULL;
    
    char *fullKey = malloc(strlen(currentScope) + strlen(name) + 2);
    sprintf(fullKey, "%s#%s", currentScope, name);
    
    Node *node = find_node(&symbolTable, fullKey);
    free(fullKey);
    
    return node;
}

double evaluate_number(char *str) {
    return atof(str);
}

// Function will be defined after struct declarations

void print_string(char *str) {
    // Remove aspas de literais de string
    if (str && str[0] == '"' && str[strlen(str)-1] == '"') {
        str[strlen(str)-1] = '\0';  // Remove aspas finais
        printf("%s ", str + 1);   // Pula aspas iniciais, adiciona espaço
    } else if (str) {
        printf("%s ", str);
    }
}

void print_newline() {
    printf("\n");
}

double power_operation(double base, double exponent) {
    return pow(base, exponent);
}

extern int yylex();
extern int yyparse();
extern FILE* yyin;
void yyerror(const char* s);

void push_scope(char *scope) {
    if (scopeTop < MAX_SCOPE_DEPTH - 1) {
        scopeStack[++scopeTop] = scope;
        currentScope = scope;
    } else {
        fprintf(stderr, "Erro: Estouro da pilha de escopos\n");
        exit(1);
    }
}

void pop_scope() {
    if (scopeTop >= 0) {
        free(scopeStack[scopeTop]);
        scopeStack[scopeTop--] = NULL;
        currentScope = (scopeTop >= 0) ? scopeStack[scopeTop] : NULL;
    } else {
        fprintf(stderr, "Erro: Pilha de escopos vazia\n");
        exit(1);
    }
}

#line 211 "parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_BOOL = 3,                       /* BOOL  */
  YYSYMBOL_ID = 4,                         /* ID  */
  YYSYMBOL_TYPE = 5,                       /* TYPE  */
  YYSYMBOL_STRING = 6,                     /* STRING  */
  YYSYMBOL_NUMBER = 7,                     /* NUMBER  */
  YYSYMBOL_BREAK = 8,                      /* BREAK  */
  YYSYMBOL_AND = 9,                        /* AND  */
  YYSYMBOL_OR = 10,                        /* OR  */
  YYSYMBOL_INT = 11,                       /* INT  */
  YYSYMBOL_FLOAT = 12,                     /* FLOAT  */
  YYSYMBOL_FUN = 13,                       /* FUN  */
  YYSYMBOL_WHILE = 14,                     /* WHILE  */
  YYSYMBOL_FOR = 15,                       /* FOR  */
  YYSYMBOL_IF = 16,                        /* IF  */
  YYSYMBOL_ELSE = 17,                      /* ELSE  */
  YYSYMBOL_LEN = 18,                       /* LEN  */
  YYSYMBOL_PRINT = 19,                     /* PRINT  */
  YYSYMBOL_RETURN = 20,                    /* RETURN  */
  YYSYMBOL_LBRACKET = 21,                  /* LBRACKET  */
  YYSYMBOL_RBRACKET = 22,                  /* RBRACKET  */
  YYSYMBOL_COMMA = 23,                     /* COMMA  */
  YYSYMBOL_LPAREN = 24,                    /* LPAREN  */
  YYSYMBOL_RPAREN = 25,                    /* RPAREN  */
  YYSYMBOL_COLON = 26,                     /* COLON  */
  YYSYMBOL_SEMICOLON = 27,                 /* SEMICOLON  */
  YYSYMBOL_NEWLINE = 28,                   /* NEWLINE  */
  YYSYMBOL_ASSIGNMENT = 29,                /* ASSIGNMENT  */
  YYSYMBOL_EQUALS = 30,                    /* EQUALS  */
  YYSYMBOL_SMALLEREQUALS = 31,             /* SMALLEREQUALS  */
  YYSYMBOL_BIGGEREQUALS = 32,              /* BIGGEREQUALS  */
  YYSYMBOL_SMALLER = 33,                   /* SMALLER  */
  YYSYMBOL_BIGGER = 34,                    /* BIGGER  */
  YYSYMBOL_INCREMENT = 35,                 /* INCREMENT  */
  YYSYMBOL_DECREMENT = 36,                 /* DECREMENT  */
  YYSYMBOL_EXPONENTIATION = 37,            /* EXPONENTIATION  */
  YYSYMBOL_MULTIPLICATION = 38,            /* MULTIPLICATION  */
  YYSYMBOL_DIVISION = 39,                  /* DIVISION  */
  YYSYMBOL_ADDITION = 40,                  /* ADDITION  */
  YYSYMBOL_SUBTRACTION = 41,               /* SUBTRACTION  */
  YYSYMBOL_LBRACE = 42,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 43,                    /* RBRACE  */
  YYSYMBOL_UMINUS = 44,                    /* UMINUS  */
  YYSYMBOL_LOWER_THAN_ELSE = 45,           /* LOWER_THAN_ELSE  */
  YYSYMBOL_YYACCEPT = 46,                  /* $accept  */
  YYSYMBOL_program = 47,                   /* program  */
  YYSYMBOL_list_decl_fun = 48,             /* list_decl_fun  */
  YYSYMBOL_decl_or_fun = 49,               /* decl_or_fun  */
  YYSYMBOL_fun = 50,                       /* fun  */
  YYSYMBOL_51_1 = 51,                      /* $@1  */
  YYSYMBOL_block = 52,                     /* block  */
  YYSYMBOL_53_2 = 53,                      /* $@2  */
  YYSYMBOL_list_stmt = 54,                 /* list_stmt  */
  YYSYMBOL_stmt = 55,                      /* stmt  */
  YYSYMBOL_simple_stmt = 56,               /* simple_stmt  */
  YYSYMBOL_compound_stmt = 57,             /* compound_stmt  */
  YYSYMBOL_while_stmt = 58,                /* while_stmt  */
  YYSYMBOL_if_stmt = 59,                   /* if_stmt  */
  YYSYMBOL_60_3 = 60,                      /* $@3  */
  YYSYMBOL_else_part = 61,                 /* else_part  */
  YYSYMBOL_62_4 = 62,                      /* $@4  */
  YYSYMBOL_63_5 = 63,                      /* $@5  */
  YYSYMBOL_for_stmt = 64,                  /* for_stmt  */
  YYSYMBOL_65_6 = 65,                      /* $@6  */
  YYSYMBOL_for_init = 66,                  /* for_init  */
  YYSYMBOL_decl = 67,                      /* decl  */
  YYSYMBOL_type = 68,                      /* type  */
  YYSYMBOL_list_param_opt = 69,            /* list_param_opt  */
  YYSYMBOL_list_param = 70,                /* list_param  */
  YYSYMBOL_param = 71,                     /* param  */
  YYSYMBOL_return_stmt = 72,               /* return_stmt  */
  YYSYMBOL_print_stmt = 73,                /* print_stmt  */
  YYSYMBOL_print_arg_list = 74,            /* print_arg_list  */
  YYSYMBOL_print_arg = 75,                 /* print_arg  */
  YYSYMBOL_assign_stmt = 76,               /* assign_stmt  */
  YYSYMBOL_lvalue = 77,                    /* lvalue  */
  YYSYMBOL_expression = 78,                /* expression  */
  YYSYMBOL_list_expression = 79,           /* list_expression  */
  YYSYMBOL_arg_list_opt = 80,              /* arg_list_opt  */
  YYSYMBOL_arg_list = 81                   /* arg_list  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */
#line 147 "parserPenelope.y"

    void print_value(ExpressionResult* expr) {
        if (!expr) return;
        
        if (strcmp(expr->type, "int") == 0) {
            printf("%d ", expr->intVal);
        } else if (strcmp(expr->type, "float") == 0) {
            printf("%.6g ", expr->doubleVal);
        } else if (strcmp(expr->type, "bool") == 0) {
            printf("%s ", expr->intVal ? "true" : "false");
        } else if (strcmp(expr->type, "string") == 0) {
            // Handle string literals - remove quotes if present
            if (expr->strVal && expr->strVal[0] == '"' && expr->strVal[strlen(expr->strVal)-1] == '"') {
                // Create a temporary string without quotes
                char* temp = strdup(expr->strVal);
                temp[strlen(temp)-1] = '\0';  // Remove closing quote
                printf("%s ", temp + 1);      // Skip opening quote
                free(temp);
            } else {
                printf("%s ", expr->strVal ? expr->strVal : "");
            }
        } else {
            printf("unknown ");
        }
    }

#line 354 "parser.tab.c"

#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  12
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   415

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  46
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  36
/* YYNRULES -- Number of rules.  */
#define YYNRULES  85
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  162

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   300


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   222,   222,   226,   227,   231,   232,   233,   237,   237,
     248,   248,   258,   259,   263,   264,   265,   269,   270,   271,
     272,   273,   279,   287,   288,   289,   293,   297,   297,   319,
     322,   322,   325,   325,   333,   333,   343,   344,   347,   348,
     375,   439,   440,   448,   449,   453,   454,   458,   476,   480,
     487,   488,   492,   499,   527,   556,   579,   582,   617,   623,
     629,   635,   641,   697,   700,   726,   752,   760,   773,   786,
     805,   824,   843,   862,   890,   926,   962,   978,   984,   991,
    1004,  1008,  1015,  1016,  1020,  1021
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "BOOL", "ID", "TYPE",
  "STRING", "NUMBER", "BREAK", "AND", "OR", "INT", "FLOAT", "FUN", "WHILE",
  "FOR", "IF", "ELSE", "LEN", "PRINT", "RETURN", "LBRACKET", "RBRACKET",
  "COMMA", "LPAREN", "RPAREN", "COLON", "SEMICOLON", "NEWLINE",
  "ASSIGNMENT", "EQUALS", "SMALLEREQUALS", "BIGGEREQUALS", "SMALLER",
  "BIGGER", "INCREMENT", "DECREMENT", "EXPONENTIATION", "MULTIPLICATION",
  "DIVISION", "ADDITION", "SUBTRACTION", "LBRACE", "RBRACE", "UMINUS",
  "LOWER_THAN_ELSE", "$accept", "program", "list_decl_fun", "decl_or_fun",
  "fun", "$@1", "block", "$@2", "list_stmt", "stmt", "simple_stmt",
  "compound_stmt", "while_stmt", "if_stmt", "$@3", "else_part", "$@4",
  "$@5", "for_stmt", "$@6", "for_init", "decl", "type", "list_param_opt",
  "list_param", "param", "return_stmt", "print_stmt", "print_arg_list",
  "print_arg", "assign_stmt", "lvalue", "expression", "list_expression",
  "arg_list_opt", "arg_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-138)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-39)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      46,   -14,     7,  -138,    38,    49,  -138,  -138,    15,    18,
      24,    52,  -138,  -138,  -138,    59,  -138,    44,    43,  -138,
      37,     7,  -138,    51,  -138,  -138,  -138,    55,    37,    37,
      37,    62,   334,    54,    64,  -138,    68,    37,    37,   334,
      30,   207,  -138,    37,    37,    37,    37,    37,    37,    37,
      37,    37,    37,    37,    37,    37,    90,    57,     7,   334,
      77,    82,   224,  -138,    37,  -138,   250,   363,   351,   374,
      47,    47,    47,    47,    72,    72,    72,    89,    89,  -138,
     194,  -138,  -138,    37,  -138,   334,  -138,    87,    91,  -138,
      93,    94,    37,  -138,    92,  -138,  -138,  -138,  -138,  -138,
    -138,    95,    97,   109,   112,   -15,   265,   334,  -138,    37,
     119,    37,    37,   334,  -138,  -138,  -138,  -138,  -138,  -138,
      37,  -138,  -138,  -138,   284,    61,   301,    -8,  -138,   334,
     334,    79,  -138,   121,  -138,  -138,   -15,  -138,    37,  -138,
    -138,  -138,    37,    79,  -138,   194,   318,   132,   126,   147,
     136,  -138,  -138,   128,    79,   139,   114,  -138,  -138,   194,
     160,  -138
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
      38,    41,     0,     7,     0,     2,     3,     5,     0,     0,
       0,     0,     1,     4,     6,     0,    42,     0,    39,     8,
       0,    43,    58,    56,    61,    59,    60,     0,     0,     0,
       0,    62,    40,     0,     0,    44,    45,    82,     0,    80,
       0,     0,    76,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
       0,    83,     0,    79,     0,    63,     0,    74,    75,    73,
      71,    72,    69,    70,    68,    66,    67,    64,    65,    47,
      38,    46,    77,     0,    78,    81,    57,     0,     0,    34,
       0,     0,     0,    16,    38,    12,    14,    15,    25,    23,
      24,     0,     0,     0,     0,    62,     0,    85,    22,     0,
       0,     0,     0,    48,     9,    13,    17,    19,    20,    18,
       0,    54,    55,    21,     0,    38,     0,     0,    50,    52,
      53,     0,    56,     0,    36,    37,     0,    27,     0,    49,
      10,    26,     0,     0,    51,    38,     0,    29,    38,     0,
      30,    28,    11,     0,     0,     0,     0,    31,    33,    38,
      38,    35
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -138,  -138,  -138,   152,  -138,  -138,  -104,  -138,  -137,   -91,
    -138,  -138,  -138,     3,  -138,  -138,  -138,  -138,  -138,  -138,
    -138,     0,     2,  -138,   101,  -138,  -138,  -138,  -138,    22,
    -112,   -78,   -19,  -138,  -138,  -138
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     4,     5,     6,     7,    21,   141,   145,    94,    95,
      96,    97,    98,    99,   143,   151,   154,   155,   100,   110,
     133,   101,     9,    34,    35,    36,   102,   103,   127,   128,
     104,    31,   106,    40,    60,    61
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       8,    32,   105,   115,    11,     8,    43,    10,   148,    39,
      41,    42,     1,   135,   120,   138,   105,   139,    59,    62,
     121,   122,   160,    33,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,   153,    12,   147,
      22,    23,    14,    24,    15,    85,    16,   136,    25,    26,
     157,     1,    63,    64,     1,    27,    17,   115,    28,     2,
      33,    29,     2,    18,   107,   132,     1,   105,    19,   115,
     105,   136,    20,   113,     3,    37,   -38,     3,    30,    38,
      56,   105,   105,    43,    51,    52,    53,    54,    55,    57,
     124,    58,   126,   129,    79,    22,    23,     1,    24,    80,
      87,   130,    82,    25,    26,    83,    88,    89,    90,    51,
      27,    91,    92,    28,   108,   109,    29,   111,   112,   129,
      93,   140,   116,   146,   117,   134,    51,    52,    53,    22,
      23,     1,    24,    30,    87,   114,   118,    25,    26,   119,
      88,    89,    90,   125,    27,    91,    92,    28,   142,   150,
      29,   132,   -32,   156,    93,    90,   159,    13,   158,    81,
     144,     0,     0,    22,    23,     1,    24,    30,    87,   152,
       0,    25,    26,     0,    88,    89,    90,     0,    27,    91,
      92,    28,     0,     0,    29,     0,     0,     0,    93,     0,
       0,     0,     0,     0,     0,     0,     0,    22,    23,     1,
      24,    30,    87,   161,     0,    25,    26,     0,    88,    89,
      90,     0,    27,    91,    92,    28,    44,    45,    29,     0,
       0,     0,    93,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    65,    44,    45,    30,     0,    46,    47,    48,
      49,    50,     0,     0,    51,    52,    53,    54,    55,    84,
       0,     0,     0,     0,    46,    47,    48,    49,    50,    44,
      45,    51,    52,    53,    54,    55,     0,     0,     0,     0,
       0,     0,    86,     0,    44,    45,     0,     0,     0,     0,
      46,    47,    48,    49,    50,     0,     0,    51,    52,    53,
      54,    55,   123,    44,    45,    46,    47,    48,    49,    50,
       0,     0,    51,    52,    53,    54,    55,     0,     0,   131,
      44,    45,     0,     0,    46,    47,    48,    49,    50,     0,
       0,    51,    52,    53,    54,    55,   137,    44,    45,     0,
       0,    46,    47,    48,    49,    50,     0,     0,    51,    52,
      53,    54,    55,    44,    45,   149,     0,     0,    46,    47,
      48,    49,    50,     0,     0,    51,    52,    53,    54,    55,
      44,     0,     0,     0,    46,    47,    48,    49,    50,     0,
       0,    51,    52,    53,    54,    55,     0,     0,     0,     0,
       0,    46,    47,    48,    49,    50,     0,     0,    51,    52,
      53,    54,    55,    46,    47,    48,    49,    50,     0,     0,
      51,    52,    53,    54,    55,    47,    48,    49,    50,     0,
       0,    51,    52,    53,    54,    55
};

static const yytype_int16 yycheck[] =
{
       0,    20,    80,    94,     2,     5,    21,    21,   145,    28,
      29,    30,     5,   125,    29,    23,    94,    25,    37,    38,
      35,    36,   159,    21,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,   149,     0,   143,
       3,     4,    27,     6,    26,    64,    22,   125,    11,    12,
     154,     5,    22,    23,     5,    18,     4,   148,    21,    13,
      58,    24,    13,     4,    83,     4,     5,   145,    24,   160,
     148,   149,    29,    92,    28,    24,    27,    28,    41,    24,
      26,   159,   160,    21,    37,    38,    39,    40,    41,    25,
     109,    23,   111,   112,     4,     3,     4,     5,     6,    42,
       8,   120,    25,    11,    12,    23,    14,    15,    16,    37,
      18,    19,    20,    21,    27,    24,    24,    24,    24,   138,
      28,    42,    27,   142,    27,   125,    37,    38,    39,     3,
       4,     5,     6,    41,     8,    43,    27,    11,    12,    27,
      14,    15,    16,    24,    18,    19,    20,    21,    27,    17,
      24,     4,    16,    25,    28,    16,    42,     5,   155,    58,
     138,    -1,    -1,     3,     4,     5,     6,    41,     8,    43,
      -1,    11,    12,    -1,    14,    15,    16,    -1,    18,    19,
      20,    21,    -1,    -1,    24,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,    41,     8,    43,    -1,    11,    12,    -1,    14,    15,
      16,    -1,    18,    19,    20,    21,     9,    10,    24,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,     9,    10,    41,    -1,    30,    31,    32,
      33,    34,    -1,    -1,    37,    38,    39,    40,    41,    25,
      -1,    -1,    -1,    -1,    30,    31,    32,    33,    34,     9,
      10,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    -1,     9,    10,    -1,    -1,    -1,    -1,
      30,    31,    32,    33,    34,    -1,    -1,    37,    38,    39,
      40,    41,    27,     9,    10,    30,    31,    32,    33,    34,
      -1,    -1,    37,    38,    39,    40,    41,    -1,    -1,    25,
       9,    10,    -1,    -1,    30,    31,    32,    33,    34,    -1,
      -1,    37,    38,    39,    40,    41,    25,     9,    10,    -1,
      -1,    30,    31,    32,    33,    34,    -1,    -1,    37,    38,
      39,    40,    41,     9,    10,    27,    -1,    -1,    30,    31,
      32,    33,    34,    -1,    -1,    37,    38,    39,    40,    41,
       9,    -1,    -1,    -1,    30,    31,    32,    33,    34,    -1,
      -1,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    30,    31,    32,    33,    34,    -1,    -1,    37,    38,
      39,    40,    41,    30,    31,    32,    33,    34,    -1,    -1,
      37,    38,    39,    40,    41,    31,    32,    33,    34,    -1,
      -1,    37,    38,    39,    40,    41
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     5,    13,    28,    47,    48,    49,    50,    67,    68,
      21,    68,     0,    49,    27,    26,    22,     4,     4,    24,
      29,    51,     3,     4,     6,    11,    12,    18,    21,    24,
      41,    77,    78,    68,    69,    70,    71,    24,    24,    78,
      79,    78,    78,    21,     9,    10,    30,    31,    32,    33,
      34,    37,    38,    39,    40,    41,    26,    25,    23,    78,
      80,    81,    78,    22,    23,    25,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,     4,
      42,    70,    25,    23,    25,    78,    22,     8,    14,    15,
      16,    19,    20,    28,    54,    55,    56,    57,    58,    59,
      64,    67,    72,    73,    76,    77,    78,    78,    27,    24,
      65,    24,    24,    78,    43,    55,    27,    27,    27,    27,
      29,    35,    36,    27,    78,    24,    78,    74,    75,    78,
      78,    25,     4,    66,    67,    76,    77,    25,    23,    25,
      42,    52,    27,    60,    75,    53,    78,    52,    54,    27,
      17,    61,    43,    76,    62,    63,    25,    52,    59,    42,
      54,    43
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    46,    47,    48,    48,    49,    49,    49,    51,    50,
      53,    52,    54,    54,    55,    55,    55,    56,    56,    56,
      56,    56,    56,    57,    57,    57,    58,    60,    59,    61,
      62,    61,    63,    61,    65,    64,    66,    66,    67,    67,
      67,    68,    68,    69,    69,    70,    70,    71,    72,    73,
      74,    74,    75,    76,    76,    76,    77,    77,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      79,    79,    80,    80,    81,    81
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     2,     1,     0,    10,
       0,     4,     1,     2,     1,     1,     1,     2,     2,     2,
       2,     2,     2,     1,     1,     1,     5,     0,     7,     0,
       0,     3,     0,     3,     0,    12,     1,     1,     0,     3,
       5,     1,     3,     0,     1,     1,     3,     3,     2,     4,
       1,     3,     1,     3,     2,     2,     1,     4,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     4,     4,     3,
       1,     3,     0,     1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 8: /* $@1: %empty  */
#line 237 "parserPenelope.y"
                       {
        // Cria o escopo da função antes de processar parâmetros
        char *scopeId = uniqueIdentifier();
        push_scope(scopeId);
    }
#line 1475 "parser.tab.c"
    break;

  case 9: /* fun: FUN type ID LPAREN $@1 list_param_opt RPAREN LBRACE list_stmt RBRACE  */
#line 241 "parserPenelope.y"
                                                    {
        // Remove o escopo da função
        pop_scope();
    }
#line 1484 "parser.tab.c"
    break;

  case 10: /* $@2: %empty  */
#line 248 "parserPenelope.y"
           {
        char *scopeId = uniqueIdentifier();

        push_scope(scopeId);
    }
#line 1494 "parser.tab.c"
    break;

  case 11: /* block: LBRACE $@2 list_stmt RBRACE  */
#line 252 "parserPenelope.y"
                       {
        pop_scope();
    }
#line 1502 "parser.tab.c"
    break;

  case 21: /* simple_stmt: expression SEMICOLON  */
#line 273 "parserPenelope.y"
                           {
        if (exec_block) {
            // Executa a expressão apenas se for dentro de bloco válido
        }
    }
#line 1512 "parser.tab.c"
    break;

  case 22: /* simple_stmt: BREAK SEMICOLON  */
#line 279 "parserPenelope.y"
                      {
    if (exec_block) {
        // break real ainda não implementado — só evitar erro
    }
}
#line 1522 "parser.tab.c"
    break;

  case 27: /* $@3: %empty  */
#line 297 "parserPenelope.y"
                                {
        // Avalia a expressão como condição booleana
        int condition = 0;
        if (strcmp((yyvsp[-1].exprResult)->type, "bool") == 0) {
            condition = (yyvsp[-1].exprResult)->intVal;
        } else if (strcmp((yyvsp[-1].exprResult)->type, "int") == 0) {
            condition = ((yyvsp[-1].exprResult)->intVal != 0);
        } else if (strcmp((yyvsp[-1].exprResult)->type, "float") == 0) {
            condition = ((yyvsp[-1].exprResult)->doubleVal != 0.0);
        } else {
            semantic_error("Condição if deve ser do tipo bool, int ou float, mas foi '%s'.", (yyvsp[-1].exprResult)->type);
        }
        
        last_condition_result = condition;
        exec_block = last_condition_result;
        free_expression_result((yyvsp[-1].exprResult));
    }
#line 1544 "parser.tab.c"
    break;

  case 28: /* if_stmt: IF LPAREN expression RPAREN $@3 block else_part  */
#line 313 "parserPenelope.y"
                      {
        exec_block = 1;  // reseta após o if
    }
#line 1552 "parser.tab.c"
    break;

  case 29: /* else_part: %empty  */
#line 319 "parserPenelope.y"
                {
        // No else clause
    }
#line 1560 "parser.tab.c"
    break;

  case 30: /* $@4: %empty  */
#line 322 "parserPenelope.y"
           {
        exec_block = !last_condition_result;  // ativa o bloco do else se falso
    }
#line 1568 "parser.tab.c"
    break;

  case 32: /* $@5: %empty  */
#line 325 "parserPenelope.y"
           {
        exec_block = !last_condition_result;  // ativa o bloco do else se falso
    }
#line 1576 "parser.tab.c"
    break;

  case 34: /* $@6: %empty  */
#line 333 "parserPenelope.y"
        {
        char *scopeId = uniqueIdentifier();

        push_scope(scopeId);
    }
#line 1586 "parser.tab.c"
    break;

  case 35: /* for_stmt: FOR $@6 LPAREN for_init SEMICOLON expression SEMICOLON assign_stmt RPAREN LBRACE list_stmt RBRACE  */
#line 337 "parserPenelope.y"
                                                                                                {
        pop_scope();
    }
#line 1594 "parser.tab.c"
    break;

  case 39: /* decl: type COLON ID  */
#line 348 "parserPenelope.y"
                      {
         if (find_variable_in_current_scope((yyvsp[0].str)) != NULL) {
              semantic_error("Variável '%s' já declarada no escopo atual.", (yyvsp[0].str));
              YYABORT;
          }


        // Cria chave completa com escopo: "escopo#variavel"
        char *fullKey = malloc(strlen(currentScope) + strlen((yyvsp[0].str)) + 2);
        
        if (!fullKey) {
            semantic_error("Erro de alocação de memória para chave do símbolo.\n");

            YYABORT;
        }
        

        sprintf(fullKey, "%s#%s", currentScope, (yyvsp[0].str));

        Data data;

        data.type = strdup((yyvsp[-2].str));

        insert_node(&symbolTable, fullKey, data);

        free(fullKey);
    }
#line 1626 "parser.tab.c"
    break;

  case 40: /* decl: type COLON ID ASSIGNMENT expression  */
#line 375 "parserPenelope.y"
                                          {
        if (find_variable_in_current_scope((yyvsp[-2].str)) != NULL) {
              semantic_error("Variável '%s' já declarada no escopo atual.", (yyvsp[-2].str));
              YYABORT;
        }
        
        char *fullKey = malloc(strlen(currentScope) + strlen((yyvsp[-2].str)) + 2);
        if (!fullKey) {
            semantic_error("Erro de alocação de memória para chave.\n");
            YYABORT;
        }
        sprintf(fullKey, "%s#%s", currentScope, (yyvsp[-2].str));

        // CORREÇÃO: $1 é uma string, não struct com campo .type
        if (!are_types_compatible((yyvsp[-4].str), (yyvsp[0].exprResult)->type)) {
            free(fullKey);
            semantic_error("Tipo incompatível: a variável de tipo %s não pode receber o tipo %s\n", (yyvsp[-4].str), (yyvsp[0].exprResult)->type);
            YYABORT;
        }

        Data newData;
        newData.type = strdup((yyvsp[-4].str));
        
        if (strcmp((yyvsp[-4].str), "int") == 0) {
            newData.value.intVal = (yyvsp[0].exprResult)->intVal;
        } else if (strcmp((yyvsp[-4].str), "float") == 0) {
            newData.value.doubleVal = (yyvsp[0].exprResult)->doubleVal;
        } else if (strcmp((yyvsp[-4].str), "bool") == 0) {
            newData.value.intVal = (yyvsp[0].exprResult)->intVal;
        } else if (strcmp((yyvsp[-4].str), "string") == 0) {
            newData.value.strVal = (yyvsp[0].exprResult)->strVal;
        } else if (strstr((yyvsp[-4].str), "[]") != NULL) {
            // Tipos de array - armazena um valor padrão por enquanto
            newData.value.intVal = 0; // Valor temporário para arrays
        } else {
            free(fullKey);
            semantic_error("Tipo '%s' não suportado para atribuição.\n", (yyvsp[-4].str));
            YYABORT;
        }

        printf("Inserido no symbolTable: %s com tipo %s e valor ", fullKey, newData.type);
        if (strcmp(newData.type, "int") == 0) {
            printf("%d\n", newData.value.intVal);
        } else if (strcmp(newData.type, "float") == 0) {
            printf("%f\n", newData.value.doubleVal);
        } else if (strcmp(newData.type, "bool") == 0) {
            printf("%d\n", newData.value.intVal);
        } else if (strcmp(newData.type, "string") == 0) {
            printf("%s\n", newData.value.strVal ? newData.value.strVal : "(null)");
        } else if (strstr(newData.type, "[]") != NULL) {
            printf("array\n");
        } else {
            printf("unknown\n");
        }
        
        insert_node(&symbolTable, fullKey, newData);

        free(newData.type); // insert_node já faz cópia
        free(fullKey);
    }
#line 1691 "parser.tab.c"
    break;

  case 41: /* type: TYPE  */
#line 439 "parserPenelope.y"
                                        { (yyval.str) = (yyvsp[0].str);}
#line 1697 "parser.tab.c"
    break;

  case 42: /* type: TYPE LBRACKET RBRACKET  */
#line 440 "parserPenelope.y"
                                        { 
                                            char *array_type = malloc(strlen((yyvsp[-2].str)) + 3);
                                            sprintf(array_type, "%s[]", (yyvsp[-2].str));
                                            (yyval.str) = array_type;
                                            free((yyvsp[-2].str));
                                        }
#line 1708 "parser.tab.c"
    break;

  case 47: /* param: type COLON ID  */
#line 458 "parserPenelope.y"
                  {
        // Adiciona o parâmetro à tabela de símbolos no escopo atual da função
        if (find_variable_in_current_scope((yyvsp[0].str)) != NULL) {
            semantic_error("Parâmetro '%s' já declarado na função.", (yyvsp[0].str));
        } else {
            char *fullKey = malloc(strlen(currentScope) + strlen((yyvsp[0].str)) + 2);
            sprintf(fullKey, "%s#%s", currentScope, (yyvsp[0].str));
            
            Data data;
            data.type = strdup((yyvsp[-2].str));
            
            insert_node(&symbolTable, fullKey, data);
            free(fullKey);
        }
    }
#line 1728 "parser.tab.c"
    break;

  case 48: /* return_stmt: RETURN expression  */
#line 476 "parserPenelope.y"
                      { /* Poderia adicionar verificação de tipo de retorno aqui */ }
#line 1734 "parser.tab.c"
    break;

  case 49: /* print_stmt: PRINT LPAREN print_arg_list RPAREN  */
#line 480 "parserPenelope.y"
                                       {
        if (exec_block) print_newline();
    }
#line 1742 "parser.tab.c"
    break;

  case 52: /* print_arg: expression  */
#line 492 "parserPenelope.y"
               {
        if (exec_block) print_value((yyvsp[0].exprResult));
        free_expression_result((yyvsp[0].exprResult));
    }
#line 1751 "parser.tab.c"
    break;

  case 53: /* assign_stmt: lvalue ASSIGNMENT expression  */
#line 499 "parserPenelope.y"
                                 {
        // Verifica se a variável do lvalue foi declarada antes de atribuir
        if ((yyvsp[-2].lvalueResult)->type == LVALUE_ARRAY_ACCESS) {
            // Acesso a array - verifica compatibilidade de tipos
            if (strcmp((yyvsp[-2].lvalueResult)->elementType, (yyvsp[0].exprResult)->type) != 0) {
                // Permite conversão implícita de int para float
                if (!(strcmp((yyvsp[-2].lvalueResult)->elementType, "float") == 0 && strcmp((yyvsp[0].exprResult)->type, "int") == 0)) {
                    semantic_error("Incompatibilidade de tipos: tentativa de atribuir '%s' a elemento de array do tipo '%s'.", 
                                   (yyvsp[0].exprResult)->type, (yyvsp[-2].lvalueResult)->elementType);
                    free_lvalue_result((yyvsp[-2].lvalueResult));
                    free_expression_result((yyvsp[0].exprResult));
                    YYABORT;
                }
            }
            // TODO: Implementar armazenamento real de valores em arrays
        } else if ((yyvsp[-2].lvalueResult)->type == LVALUE_VAR) {
            if (find_variable_in_scopes((yyvsp[-2].lvalueResult)->varName) == NULL) {
                semantic_error("Variável '%s' não declarada.", (yyvsp[-2].lvalueResult)->varName);
                free_lvalue_result((yyvsp[-2].lvalueResult));
                free_expression_result((yyvsp[0].exprResult));
                YYABORT;
            } else {
                // store_variable_value($1->varName, $3);
            }
        }
        free_lvalue_result((yyvsp[-2].lvalueResult));
        free_expression_result((yyvsp[0].exprResult));
    }
#line 1784 "parser.tab.c"
    break;

  case 54: /* assign_stmt: lvalue INCREMENT  */
#line 527 "parserPenelope.y"
                       {
        if ((yyvsp[-1].lvalueResult)->type == LVALUE_ARRAY_ACCESS) {
            // Incremento em elemento de array
            if (strcmp((yyvsp[-1].lvalueResult)->elementType, "int") != 0 && strcmp((yyvsp[-1].lvalueResult)->elementType, "float") != 0) {
                semantic_error("Não é possível usar o operador ++ para elementos de array do tipo '%s'.", (yyvsp[-1].lvalueResult)->elementType);
                YYABORT;
            }
            // TODO: Implementar incremento real em elementos de array
        } else if ((yyvsp[-1].lvalueResult)->type == LVALUE_VAR) {
            Node* node = find_variable_in_scopes((yyvsp[-1].lvalueResult)->varName);

            if (node == NULL) {
                semantic_error("Variável '%s' não declarada.", (yyvsp[-1].lvalueResult)->varName);
                YYABORT;
            } else {
                char* type = node->value.type;

                if (strcmp(type, "int") != 0 && strcmp(type, "float") != 0) {
                    semantic_error("Não é possível usar o operador ++ para tipos que não sejam int e float\n");
                    YYABORT;
                } else if (strcmp(type, "int") == 0) {
                    node->value.value.intVal += 1;
                } else if (strcmp(type, "float") == 0) {
                    node->value.value.doubleVal += 1.0;
                }
            }
        }
        free_lvalue_result((yyvsp[-1].lvalueResult));
    }
#line 1818 "parser.tab.c"
    break;

  case 55: /* assign_stmt: lvalue DECREMENT  */
#line 556 "parserPenelope.y"
                       {
        if ((yyvsp[-1].lvalueResult)->type == LVALUE_ARRAY_ACCESS) {
            // Decremento em elemento de array
            if (strcmp((yyvsp[-1].lvalueResult)->elementType, "int") != 0 && strcmp((yyvsp[-1].lvalueResult)->elementType, "float") != 0) {
                semantic_error("Não é possível usar o operador -- para elementos de array do tipo '%s'.", (yyvsp[-1].lvalueResult)->elementType);
                YYABORT;
            }
            // TODO: Implementar decremento real em elementos de array
        } else if ((yyvsp[-1].lvalueResult)->type == LVALUE_VAR) {
             if (find_variable_in_scopes((yyvsp[-1].lvalueResult)->varName) == NULL) {
                semantic_error("Variável '%s' não declarada.", (yyvsp[-1].lvalueResult)->varName);
                YYABORT;
            } else {
                // double current = get_variable_value($1->varName);
                // store_variable_value($1->varName, current - 1.0);
            }
        }
        free_lvalue_result((yyvsp[-1].lvalueResult));
    }
#line 1842 "parser.tab.c"
    break;

  case 56: /* lvalue: ID  */
#line 579 "parserPenelope.y"
       { 
        (yyval.lvalueResult) = create_lvalue_var((yyvsp[0].str));
    }
#line 1850 "parser.tab.c"
    break;

  case 57: /* lvalue: lvalue LBRACKET expression RBRACKET  */
#line 582 "parserPenelope.y"
                                          { 
        // Para acesso a array, verifica se o lvalue é um array e retorna o tipo do elemento
        
        // Se o lvalue já é um array_access, não podemos fazer double indexing nesta implementação
        if ((yyvsp[-3].lvalueResult)->type == LVALUE_ARRAY_ACCESS) {
            semantic_error("Acesso multidimensional a arrays não é suportado nesta versão.");
            YYABORT;
        }
        
        Node* arrayNode = find_variable_in_scopes((yyvsp[-3].lvalueResult)->varName);
        if (!arrayNode) {
            semantic_error("Variável '%s' não declarada.", (yyvsp[-3].lvalueResult)->varName);
            YYABORT;
        }
        
        if (!strstr(arrayNode->value.type, "[]")) {
            semantic_error("Tentativa de indexar uma variável que não é um array: '%s'.", (yyvsp[-3].lvalueResult)->varName);
            YYABORT;
        }
        
        // Verifica se o índice é um tipo válido (int)
        if (strcmp((yyvsp[-1].exprResult)->type, "int") != 0) {
            semantic_error("Índice de array deve ser do tipo int, mas foi '%s'.", (yyvsp[-1].exprResult)->type);
            YYABORT;
        }
        
        // Cria um resultado de array access com o tipo do elemento
        char* elementType = get_array_element_type(arrayNode->value.type);
        (yyval.lvalueResult) = create_lvalue_array_access((yyvsp[-3].lvalueResult)->varName, elementType);
        free(elementType);
        free_lvalue_result((yyvsp[-3].lvalueResult)); // Libera o lvalue anterior
    }
#line 1887 "parser.tab.c"
    break;

  case 58: /* expression: BOOL  */
#line 617 "parserPenelope.y"
         {
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        result->type = strdup("bool");
        result->intVal = (yyvsp[0].num);
        (yyval.exprResult) = result;
    }
#line 1898 "parser.tab.c"
    break;

  case 59: /* expression: INT  */
#line 623 "parserPenelope.y"
          {
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        result->type = strdup("int");
        result->intVal = (yyvsp[0].num);
        (yyval.exprResult) = result;
    }
#line 1909 "parser.tab.c"
    break;

  case 60: /* expression: FLOAT  */
#line 629 "parserPenelope.y"
            {
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        result->type = strdup("float");
        result->doubleVal = (yyvsp[0].num);
        (yyval.exprResult) = result;
    }
#line 1920 "parser.tab.c"
    break;

  case 61: /* expression: STRING  */
#line 635 "parserPenelope.y"
             {
          ExpressionResult* result = malloc(sizeof(ExpressionResult));
          result->type = strdup("string");
          result->strVal = strdup((yyvsp[0].str));
          (yyval.exprResult) = result;
      }
#line 1931 "parser.tab.c"
    break;

  case 62: /* expression: lvalue  */
#line 641 "parserPenelope.y"
             {
        ExpressionResult* result = malloc(sizeof(ExpressionResult));
        
        if ((yyvsp[0].lvalueResult)->type == LVALUE_ARRAY_ACCESS) {
            // Manipula acesso a array - usa o tipo do elemento especificado
            result->type = strdup((yyvsp[0].lvalueResult)->elementType);
            
            // Define valores padrão baseados no tipo do elemento
            if (strcmp((yyvsp[0].lvalueResult)->elementType, "int") == 0) {
                result->intVal = 0;
            } else if (strcmp((yyvsp[0].lvalueResult)->elementType, "float") == 0) {
                result->doubleVal = 0.0;
            } else if (strcmp((yyvsp[0].lvalueResult)->elementType, "bool") == 0) {
                result->intVal = 0; // false
            } else if (strcmp((yyvsp[0].lvalueResult)->elementType, "string") == 0) {
                result->strVal = strdup("");
            } else {
                semantic_error("Tipo de elemento de array não suportado: '%s'.", (yyvsp[0].lvalueResult)->elementType);
                free(result);
                free_lvalue_result((yyvsp[0].lvalueResult));
                YYABORT;
            }
        } else if ((yyvsp[0].lvalueResult)->type == LVALUE_VAR) {
            Node* varNode = find_variable_in_scopes((yyvsp[0].lvalueResult)->varName);
            
            if (!varNode) {
                semantic_error("Variável '%s' não declarada.", (yyvsp[0].lvalueResult)->varName);
                free(result);
                free_lvalue_result((yyvsp[0].lvalueResult));
                YYABORT;
            } else {
                result->type = strdup(varNode->value.type);

            if (strcmp(varNode->value.type, "int") == 0) {
                result->intVal = varNode->value.value.intVal;
            } else if (strcmp(varNode->value.type, "float") == 0) {
                result->doubleVal = varNode->value.value.doubleVal;
            } else if (strcmp(varNode->value.type, "bool") == 0) {
                result->intVal = varNode->value.value.intVal;
            } else if (strcmp(varNode->value.type, "string") == 0) {
                result->strVal = strdup(varNode->value.value.strVal);
            } else if (strstr(varNode->value.type, "[]") != NULL) {
                // Tipos de array são suportados - define valor padrão por enquanto
                result->intVal = 0; // Arrays avaliam para 0 por enquanto (valor temporário)
            } else {
                semantic_error("Tipo '%s' não suportado em expressões.", varNode->value.type);
                free(result);
                free_lvalue_result((yyvsp[0].lvalueResult));
                YYABORT;
            }
            }
        }

        free_lvalue_result((yyvsp[0].lvalueResult));
        (yyval.exprResult) = result;
    }
#line 1992 "parser.tab.c"
    break;

  case 63: /* expression: LPAREN expression RPAREN  */
#line 697 "parserPenelope.y"
                               {
          (yyval.exprResult) = (yyvsp[-1].exprResult);
      }
#line 2000 "parser.tab.c"
    break;

  case 64: /* expression: expression ADDITION expression  */
#line 700 "parserPenelope.y"
                                     {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        
        if (strcmp((yyvsp[-2].exprResult)->type, "float") == 0 || strcmp((yyvsp[0].exprResult)->type, "float") == 0) {
            res->type = strdup("float");

            double leftVal = (strcmp((yyvsp[-2].exprResult)->type, "int") == 0) ? (double)(yyvsp[-2].exprResult)->intVal : (yyvsp[-2].exprResult)->doubleVal;
            double rightVal = (strcmp((yyvsp[0].exprResult)->type, "int") == 0) ? (double)(yyvsp[0].exprResult)->intVal : (yyvsp[0].exprResult)->doubleVal;

            res->doubleVal = leftVal + rightVal;

        } else if (strcmp((yyvsp[-2].exprResult)->type, "int") == 0 && strcmp((yyvsp[0].exprResult)->type, "int") == 0) {
            res->type = strdup("int");
            res->intVal = (yyvsp[-2].exprResult)->intVal + (yyvsp[0].exprResult)->intVal;
        } else {
            // Caso tipos inesperados, força float como fallback
            res->type = strdup("float");
            res->doubleVal = 0.0;
            // Poderia emitir erro aqui, se quiser
        }

        free_expression_result((yyvsp[-2].exprResult));
        free_expression_result((yyvsp[0].exprResult));

        (yyval.exprResult) = res;
    }
#line 2031 "parser.tab.c"
    break;

  case 65: /* expression: expression SUBTRACTION expression  */
#line 726 "parserPenelope.y"
                                        {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        
        if (strcmp((yyvsp[-2].exprResult)->type, "float") == 0 || strcmp((yyvsp[0].exprResult)->type, "float") == 0) {
            res->type = strdup("float");

            double leftVal = (strcmp((yyvsp[-2].exprResult)->type, "int") == 0) ? (double)(yyvsp[-2].exprResult)->intVal : (yyvsp[-2].exprResult)->doubleVal;
            double rightVal = (strcmp((yyvsp[0].exprResult)->type, "int") == 0) ? (double)(yyvsp[0].exprResult)->intVal : (yyvsp[0].exprResult)->doubleVal;

            res->doubleVal = leftVal - rightVal;

        } else if (strcmp((yyvsp[-2].exprResult)->type, "int") == 0 && strcmp((yyvsp[0].exprResult)->type, "int") == 0) {
            res->type = strdup("int");
            res->intVal = (yyvsp[-2].exprResult)->intVal - (yyvsp[0].exprResult)->intVal;
        } else {
            // Caso tipos inesperados, força float como fallback
            res->type = strdup("float");
            res->doubleVal = 0.0;
            // Poderia emitir erro aqui, se quiser
        }

        free_expression_result((yyvsp[-2].exprResult));
        free_expression_result((yyvsp[0].exprResult));

        (yyval.exprResult) = res;
      }
#line 2062 "parser.tab.c"
    break;

  case 66: /* expression: expression MULTIPLICATION expression  */
#line 752 "parserPenelope.y"
                                           {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          res->doubleVal = (yyvsp[-2].exprResult)->doubleVal * (yyvsp[0].exprResult)->doubleVal;
          (yyval.exprResult) = res;
          free_expression_result((yyvsp[-2].exprResult));
          free_expression_result((yyvsp[0].exprResult));
      }
#line 2075 "parser.tab.c"
    break;

  case 67: /* expression: expression DIVISION expression  */
#line 760 "parserPenelope.y"
                                     {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          if ((yyvsp[0].exprResult)->doubleVal == 0) {
              semantic_error("Divisão por zero.", "");
              YYABORT;
          } else {
              res->doubleVal = (yyvsp[-2].exprResult)->doubleVal / (yyvsp[0].exprResult)->doubleVal;
          }
          (yyval.exprResult) = res;
          free_expression_result((yyvsp[-2].exprResult));
          free_expression_result((yyvsp[0].exprResult));
      }
#line 2093 "parser.tab.c"
    break;

  case 68: /* expression: expression EXPONENTIATION expression  */
#line 773 "parserPenelope.y"
                                           {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("float");

        double left = strcmp((yyvsp[-2].exprResult)->type, "int") == 0 ? (double)(yyvsp[-2].exprResult)->intVal : (yyvsp[-2].exprResult)->doubleVal;
        double right = strcmp((yyvsp[0].exprResult)->type, "int") == 0 ? (double)(yyvsp[0].exprResult)->intVal : (yyvsp[0].exprResult)->doubleVal;

        res->doubleVal = power_operation(left, right);

        (yyval.exprResult) = res;
        free_expression_result((yyvsp[-2].exprResult));
        free_expression_result((yyvsp[0].exprResult));
      }
#line 2111 "parser.tab.c"
    break;

  case 69: /* expression: expression SMALLER expression  */
#line 786 "parserPenelope.y"
                                    {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        if (!are_types_compatible((yyvsp[-2].exprResult)->type, (yyvsp[0].exprResult)->type) || strcmp((yyvsp[-2].exprResult)->type, "string") == 0) {
             semantic_error("Operador '<' inválido entre os tipos %s e %s.", (yyvsp[-2].exprResult)->type, (yyvsp[0].exprResult)->type);
             free(res);
             YYABORT;
        }

        double left = (strcmp((yyvsp[-2].exprResult)->type, "int") == 0) ? (double)(yyvsp[-2].exprResult)->intVal : (yyvsp[-2].exprResult)->doubleVal;
        double right = (strcmp((yyvsp[0].exprResult)->type, "int") == 0) ? (double)(yyvsp[0].exprResult)->intVal : (yyvsp[0].exprResult)->doubleVal;
        
        res->intVal = (left < right);
        
        free_expression_result((yyvsp[-2].exprResult));
        free_expression_result((yyvsp[0].exprResult));
        (yyval.exprResult) = res;
    }
#line 2135 "parser.tab.c"
    break;

  case 70: /* expression: expression BIGGER expression  */
#line 805 "parserPenelope.y"
                                   {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        if (!are_types_compatible((yyvsp[-2].exprResult)->type, (yyvsp[0].exprResult)->type) || strcmp((yyvsp[-2].exprResult)->type, "string") == 0) {
             semantic_error("Operador '>' inválido entre os tipos %s e %s.", (yyvsp[-2].exprResult)->type, (yyvsp[0].exprResult)->type);
             free(res);
             YYABORT;
        }

        double left = (strcmp((yyvsp[-2].exprResult)->type, "int") == 0) ? (double)(yyvsp[-2].exprResult)->intVal : (yyvsp[-2].exprResult)->doubleVal;
        double right = (strcmp((yyvsp[0].exprResult)->type, "int") == 0) ? (double)(yyvsp[0].exprResult)->intVal : (yyvsp[0].exprResult)->doubleVal;
        
        res->intVal = (left > right);
        
        free_expression_result((yyvsp[-2].exprResult));
        free_expression_result((yyvsp[0].exprResult));
        (yyval.exprResult) = res;
    }
#line 2159 "parser.tab.c"
    break;

  case 71: /* expression: expression SMALLEREQUALS expression  */
#line 824 "parserPenelope.y"
                                          {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        if (!are_types_compatible((yyvsp[-2].exprResult)->type, (yyvsp[0].exprResult)->type) || strcmp((yyvsp[-2].exprResult)->type, "string") == 0) {
             semantic_error("Operador '<=' inválido entre os tipos %s e %s.", (yyvsp[-2].exprResult)->type, (yyvsp[0].exprResult)->type);
             free(res);
             YYABORT;
        }

        double left = (strcmp((yyvsp[-2].exprResult)->type, "int") == 0) ? (double)(yyvsp[-2].exprResult)->intVal : (yyvsp[-2].exprResult)->doubleVal;
        double right = (strcmp((yyvsp[0].exprResult)->type, "int") == 0) ? (double)(yyvsp[0].exprResult)->intVal : (yyvsp[0].exprResult)->doubleVal;
        
        res->intVal = (left <= right);
        
        free_expression_result((yyvsp[-2].exprResult));
        free_expression_result((yyvsp[0].exprResult));
        (yyval.exprResult) = res;
    }
#line 2183 "parser.tab.c"
    break;

  case 72: /* expression: expression BIGGEREQUALS expression  */
#line 843 "parserPenelope.y"
                                         {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        if (!are_types_compatible((yyvsp[-2].exprResult)->type, (yyvsp[0].exprResult)->type) || strcmp((yyvsp[-2].exprResult)->type, "string") == 0) {
             semantic_error("Operador '>=' inválido entre os tipos %s e %s.", (yyvsp[-2].exprResult)->type, (yyvsp[0].exprResult)->type);
             free(res);
             YYABORT;
        }

        double left = (strcmp((yyvsp[-2].exprResult)->type, "int") == 0) ? (double)(yyvsp[-2].exprResult)->intVal : (yyvsp[-2].exprResult)->doubleVal;
        double right = (strcmp((yyvsp[0].exprResult)->type, "int") == 0) ? (double)(yyvsp[0].exprResult)->intVal : (yyvsp[0].exprResult)->doubleVal;
        
        res->intVal = (left >= right);
        
        free_expression_result((yyvsp[-2].exprResult));
        free_expression_result((yyvsp[0].exprResult));
        (yyval.exprResult) = res;
    }
#line 2207 "parser.tab.c"
    break;

  case 73: /* expression: expression EQUALS expression  */
#line 862 "parserPenelope.y"
                                   {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        int result = 0;

        // Caso 1: Comparação de strings
        if (strcmp((yyvsp[-2].exprResult)->type, "string") == 0 && strcmp((yyvsp[0].exprResult)->type, "string") == 0) {
            result = (strcmp((yyvsp[-2].exprResult)->strVal, (yyvsp[0].exprResult)->strVal) == 0);
        } 
        
        // Caso 2: Comparação de tipos numéricos compatíveis
        else if (are_types_compatible((yyvsp[-2].exprResult)->type, (yyvsp[0].exprResult)->type)) {
            double left = (strcmp((yyvsp[-2].exprResult)->type, "int") == 0) ? (double)(yyvsp[-2].exprResult)->intVal : (yyvsp[-2].exprResult)->doubleVal;
            double right = (strcmp((yyvsp[0].exprResult)->type, "int") == 0) ? (double)(yyvsp[0].exprResult)->intVal : (yyvsp[0].exprResult)->doubleVal;
            result = (left == right);
        } 
        // Caso 3: Tipos incompatíveis
        else {
            semantic_error("Operador '==' inválido entre os tipos %s e %s.", (yyvsp[-2].exprResult)->type, (yyvsp[0].exprResult)->type);
            free(res);
            YYABORT;
        }

        res->intVal = result;
        free_expression_result((yyvsp[-2].exprResult));
        free_expression_result((yyvsp[0].exprResult));
        (yyval.exprResult) = res;
    }
#line 2240 "parser.tab.c"
    break;

  case 74: /* expression: expression AND expression  */
#line 890 "parserPenelope.y"
                                {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        // Converte para valores booleanos
        int left = 0, right = 0;
        if (strcmp((yyvsp[-2].exprResult)->type, "bool") == 0) {
            left = (yyvsp[-2].exprResult)->intVal;
        } else if (strcmp((yyvsp[-2].exprResult)->type, "int") == 0) {
            left = ((yyvsp[-2].exprResult)->intVal != 0);
        } else if (strcmp((yyvsp[-2].exprResult)->type, "float") == 0) {
            left = ((yyvsp[-2].exprResult)->doubleVal != 0.0);
        } else {
            semantic_error("Operador '&&' inválido para o tipo %s.", (yyvsp[-2].exprResult)->type);
            free(res);
            YYABORT;
        }
        
        if (strcmp((yyvsp[0].exprResult)->type, "bool") == 0) {
            right = (yyvsp[0].exprResult)->intVal;
        } else if (strcmp((yyvsp[0].exprResult)->type, "int") == 0) {
            right = ((yyvsp[0].exprResult)->intVal != 0);
        } else if (strcmp((yyvsp[0].exprResult)->type, "float") == 0) {
            right = ((yyvsp[0].exprResult)->doubleVal != 0.0);
        } else {
            semantic_error("Operador '&&' inválido para o tipo %s.", (yyvsp[0].exprResult)->type);
            free(res);
            YYABORT;
        }
        
        res->intVal = (left && right);
        
        free_expression_result((yyvsp[-2].exprResult));
        free_expression_result((yyvsp[0].exprResult));
        (yyval.exprResult) = res;
    }
#line 2281 "parser.tab.c"
    break;

  case 75: /* expression: expression OR expression  */
#line 926 "parserPenelope.y"
                               {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        res->type = strdup("bool");
        
        // Converte para valores booleanos
        int left = 0, right = 0;
        if (strcmp((yyvsp[-2].exprResult)->type, "bool") == 0) {
            left = (yyvsp[-2].exprResult)->intVal;
        } else if (strcmp((yyvsp[-2].exprResult)->type, "int") == 0) {
            left = ((yyvsp[-2].exprResult)->intVal != 0);
        } else if (strcmp((yyvsp[-2].exprResult)->type, "float") == 0) {
            left = ((yyvsp[-2].exprResult)->doubleVal != 0.0);
        } else {
            semantic_error("Operador '||' inválido para o tipo %s.", (yyvsp[-2].exprResult)->type);
            free(res);
            YYABORT;
        }
        
        if (strcmp((yyvsp[0].exprResult)->type, "bool") == 0) {
            right = (yyvsp[0].exprResult)->intVal;
        } else if (strcmp((yyvsp[0].exprResult)->type, "int") == 0) {
            right = ((yyvsp[0].exprResult)->intVal != 0);
        } else if (strcmp((yyvsp[0].exprResult)->type, "float") == 0) {
            right = ((yyvsp[0].exprResult)->doubleVal != 0.0);
        } else {
            semantic_error("Operador '||' inválido para o tipo %s.", (yyvsp[0].exprResult)->type);
            free(res);
            YYABORT;
        }
        
        res->intVal = (left || right);
        
        free_expression_result((yyvsp[-2].exprResult));
        free_expression_result((yyvsp[0].exprResult));
        (yyval.exprResult) = res;
    }
#line 2322 "parser.tab.c"
    break;

  case 76: /* expression: SUBTRACTION expression  */
#line 962 "parserPenelope.y"
                                          {
        ExpressionResult* res = malloc(sizeof(ExpressionResult));
        if (strcmp((yyvsp[0].exprResult)->type, "int") == 0) {
            res->type = strdup("int");
            res->intVal = -(yyvsp[0].exprResult)->intVal;
        } else if (strcmp((yyvsp[0].exprResult)->type, "float") == 0) {
            res->type = strdup("float");
            res->doubleVal = -(yyvsp[0].exprResult)->doubleVal;
        } else {
            semantic_error("Operador unário '-' inválido para o tipo %s.", (yyvsp[0].exprResult)->type);
            free(res);
            YYABORT;
        }
        free_expression_result((yyvsp[0].exprResult));
        (yyval.exprResult) = res;
    }
#line 2343 "parser.tab.c"
    break;

  case 77: /* expression: ID LPAREN arg_list_opt RPAREN  */
#line 978 "parserPenelope.y"
                                    {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("float");
          res->doubleVal = 0.0; // Função não implementada ainda
          (yyval.exprResult) = res;
      }
#line 2354 "parser.tab.c"
    break;

  case 78: /* expression: LEN LPAREN expression RPAREN  */
#line 984 "parserPenelope.y"
                                   {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          res->type = strdup("int");
          res->intVal = 0; // len não implementado ainda
          free_expression_result((yyvsp[-1].exprResult));
          (yyval.exprResult) = res;
      }
#line 2366 "parser.tab.c"
    break;

  case 79: /* expression: LBRACKET list_expression RBRACKET  */
#line 991 "parserPenelope.y"
                                        {
          ExpressionResult* res = malloc(sizeof(ExpressionResult));
          // Determina o tipo do array baseado no primeiro elemento
          char* arrayType = malloc(strlen((yyvsp[-1].str)) + 3);
          sprintf(arrayType, "%s[]", (yyvsp[-1].str));
          res->type = arrayType;
          res->intVal = 0; // Valor temporário para literais de array
          (yyval.exprResult) = res;
      }
#line 2380 "parser.tab.c"
    break;

  case 80: /* list_expression: expression  */
#line 1004 "parserPenelope.y"
                                                   { 
        (yyval.str) = strdup((yyvsp[0].exprResult)->type); 
        free_expression_result((yyvsp[0].exprResult));
    }
#line 2389 "parser.tab.c"
    break;

  case 81: /* list_expression: list_expression COMMA expression  */
#line 1008 "parserPenelope.y"
                                                   { 
        (yyval.str) = (yyvsp[-2].str); // Mantém o tipo do primeiro elemento
        free_expression_result((yyvsp[0].exprResult));
    }
#line 2398 "parser.tab.c"
    break;

  case 82: /* arg_list_opt: %empty  */
#line 1015 "parserPenelope.y"
                                                   { /* vazio */ }
#line 2404 "parser.tab.c"
    break;

  case 83: /* arg_list_opt: arg_list  */
#line 1016 "parserPenelope.y"
                                                   { /* lista de argumentos */ }
#line 2410 "parser.tab.c"
    break;

  case 84: /* arg_list: expression  */
#line 1020 "parserPenelope.y"
                                                   { /* argumento único */ }
#line 2416 "parser.tab.c"
    break;

  case 85: /* arg_list: arg_list COMMA expression  */
#line 1021 "parserPenelope.y"
                                                   { /* múltiplos argumentos */ }
#line 2422 "parser.tab.c"
    break;


#line 2426 "parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 1024 "parserPenelope.y"


void yyerror(const char* s) {
    extern int yylineno;
    fprintf(stderr, "Erro de Sintaxe: erro de sintaxe na linha %d\n", yylineno);
    semantic_errors++; // Use semantic_errors as syntax error counter too for simplicity
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            perror(argv[1]);
            return 1;
        }
        yyin = file;
    } else {
        yyin = stdin;
    }

    push_scope(strdup("global"));

    int parse_result = yyparse();
    
    if (parse_result == 0 && semantic_errors == 0) {
        printf("Análise concluída com sucesso. A sintaxe e a semântica estão corretas!\n");
    } else {
        printf("Falha na análise. Foram encontrados %d erros.\n", semantic_errors);
    }

    print_map(&symbolTable);
    
    // Libera o escopo global
    pop_scope(); 

    free_map(&symbolTable);

    return (parse_result != 0 || semantic_errors > 0) ? 1 : 0;
}
