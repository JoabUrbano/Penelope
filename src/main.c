#include "parser/parser_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yyparse();
extern FILE* yyin;
extern void yyerror(const char* s);

void print_help() {
    printf("Uso: penelope_parser [OPÇÕES] [ARQUIVO]\n");
    printf("\nAnalisador léxico e sintático da linguagem Penelope\n");
    printf("\nOpções:\n");
    printf("  -S, --generate-c    Gera código C apenas (não compila)\n");
    printf("  -c                  Compila para executável\n");
    printf("  -o ARQUIVO          Especifica o nome do executável (implica -c)\n");
    printf("  -h, --help          Mostra esta mensagem de ajuda\n");
    printf("\nExemplos:\n");
    printf("  penelope_parser programa.txt           # Apenas análise sintática/semântica\n");
    printf("  penelope_parser -S programa.txt        # Gera código C em output/\n");
    printf("  penelope_parser -c programa.txt        # Compila para executável\n");
    printf("  penelope_parser -o meuapp programa.txt # Compila para executável 'meuapp'\n");
    printf("\nSe nenhum arquivo for especificado, lê da entrada padrão.\n");
}

int main(int argc, char **argv) {
    int output_c_code = 0;
    int compile_code = 0;
    char *input_file = NULL;
    char *output_file = NULL;
    
    // Analisa argumentos de linha de comando
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--generate-c") == 0 || strcmp(argv[i], "-S") == 0) {
            output_c_code = 1;
        } else if (strcmp(argv[i], "-c") == 0) {
            output_c_code = 1;
            compile_code = 1;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[i + 1];
            output_c_code = 1;
            compile_code = 1;  // -o implies compilation
            i++; // Pula o próximo argumento pois é o nome do arquivo de saída
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        } else {
            fprintf(stderr, "Opção desconhecida: %s\n", argv[i]);
            fprintf(stderr, "Use --help para ver as opções disponíveis.\n");
            return 1;
        }
    }
    
    if (input_file) {
        FILE *file = fopen(input_file, "r");
        if (!file) {
            perror(input_file);
            return 1;
        }
        yyin = file;
    } else {
        yyin = stdin;
    }

    // Inicializa geração de código
    if (output_c_code) {
        generate_code = 1;
        init_code_generation();
        // função main será emitida pela regra de função
    } else {
        generate_code = 0;
    }

    push_scope(strdup("global"));

    int parse_result = yyparse();
    
    if (output_c_code) {
        // Finaliza geração de código C
        finalize_code_generation();
        
        if (parse_result == 0 && semantic_errors == 0 && syntax_errors == 0) {
            // Determina o nome do arquivo fonte C (sempre vai para o diretório output/)
            char *c_output_file;
            char default_c_output[256];
            
            // Gera nome padrão do arquivo de saída C baseado no arquivo de entrada
            if (input_file) {
                char *dot = strrchr(input_file, '.');
                char *slash = strrchr(input_file, '/');
                char *basename = slash ? slash + 1 : input_file;
                
                if (dot && dot > basename) {
                    int base_len = dot - basename;
                    snprintf(default_c_output, sizeof(default_c_output), "output/%.*s.c", base_len, basename);
                } else {
                    snprintf(default_c_output, sizeof(default_c_output), "output/%s.c", basename);
                }
            } else {
                strcpy(default_c_output, "output/output.c");
            }
            c_output_file = default_c_output;
            
            // Escreve o código C gerado para o arquivo
            // Primeiro, cria o diretório se necessário
            char* output_dir = get_directory_from_path(c_output_file);
            if (output_dir && !create_directory_if_not_exists(output_dir)) {
                fprintf(stderr, "Erro: Não foi possível criar o diretório '%s'\n", output_dir);
                free(output_dir);
                return 1;
            }
            if (output_dir) free(output_dir);
            
            FILE *output_fp = fopen(c_output_file, "w");
            if (output_fp) {
                fprintf(output_fp, "%s\n", generated_code);
                fclose(output_fp);
                printf("Código C gerado com sucesso: %s\n", c_output_file);
                
                // Compilação automática se solicitada
                if (compile_code) {
                    // Determina nome do executável
                    char exe_name[256];
                    if (output_file) {
                        // Usa o nome de arquivo de saída especificado para o executável
                        strncpy(exe_name, output_file, sizeof(exe_name) - 1);
                        exe_name[sizeof(exe_name) - 1] = '\0';
                    } else {
                        // Gera nome do executável removendo extensão .c do arquivo C
                        strncpy(exe_name, c_output_file, sizeof(exe_name) - 1);
                        exe_name[sizeof(exe_name) - 1] = '\0';
                        
                        char* dot = strrchr(exe_name, '.');
                        if (dot && strcmp(dot, ".c") == 0) {
                            *dot = '\0'; // Remove extensão .c
                        }
                    }
                    
                    // Constrói comando de compilação com biblioteca math
                    char compile_cmd[512];
                    snprintf(compile_cmd, sizeof(compile_cmd), "gcc -o \"%s\" \"%s\" -lm", exe_name, c_output_file);
                    
                    printf("Compilando: %s\n", compile_cmd);
                    int compile_result = system(compile_cmd);
                    
                    if (compile_result == 0) {
                        printf("Compilação bem-sucedida: %s\n", exe_name);
                    } else {
                        fprintf(stderr, "Erro na compilação (código: %d)\n", compile_result);
                        return 1;
                    }
                }
            } else {
                fprintf(stderr, "Erro: Não foi possível criar o arquivo de saída '%s'\n", c_output_file);
                return 1;
            }
        } else {
            printf("Falha na geração de código. Foram encontrados ");
            
            int total_errors = semantic_errors + syntax_errors;
            if (total_errors > 0) {
                if (semantic_errors > 0 && syntax_errors > 0) {
                    printf("%d erros semânticos e %d erros de sintaxe.\n", semantic_errors, syntax_errors);
                } else if (semantic_errors > 0) {
                    printf("%d erro%s semântico%s.\n", semantic_errors, 
                           semantic_errors > 1 ? "s" : "", semantic_errors > 1 ? "s" : "");
                } else if (syntax_errors > 0) {
                    printf("%d erro%s de sintaxe.\n", syntax_errors, 
                           syntax_errors > 1 ? "s" : "");
                } else {
                    printf("erros desconhecidos.\n");
                }
            } else {
                printf("falha no parser (código de retorno %d).\n", parse_result);
            }
        }
    } else {
        if (parse_result == 0 && semantic_errors == 0 && syntax_errors == 0) {
            printf("Análise concluída com sucesso. A sintaxe e a semântica estão corretas!\n");
        } else {
            printf("Falha na análise. Foram encontrados ");
            
            int total_errors = semantic_errors + syntax_errors;
            if (total_errors > 0) {
                if (semantic_errors > 0 && syntax_errors > 0) {
                    printf("%d erros semânticos e %d erros de sintaxe.\n", semantic_errors, syntax_errors);
                } else if (semantic_errors > 0) {
                    printf("%d erro%s semântico%s.\n", semantic_errors, 
                           semantic_errors > 1 ? "s" : "", semantic_errors > 1 ? "s" : "");
                } else if (syntax_errors > 0) {
                    printf("%d erro%s de sintaxe.\n", syntax_errors, 
                           syntax_errors > 1 ? "s" : "");
                } else {
                    printf("erros desconhecidos.\n");
                }
            } else {
                printf("falha no parser (código de retorno %d).\n", parse_result);
            }
        }
        print_map(&symbolTable);
    }
    
    // Libera escopo global
    pop_scope(); 

    free_map(&symbolTable);

    return (parse_result != 0 || semantic_errors > 0 || syntax_errors > 0) ? 1 : 0;
}
