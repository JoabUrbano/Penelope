# Penelope

Penelope é uma proposta de linguagem de programação alto nível

## Como executar

### Pré-requisitos

Para executar o analisador léxico da linguagem Penelope, você precisará ter o Flex instalado no seu sistema. Se ainda não tiver, instale-o com:

```bash
sudo apt update
sudo apt install flex
```

Para o analisador sintático, você também precisará do Bison:

```bash
sudo apt install bison
```

### Compilação do analisador léxico

1. Compile o arquivo lexerPenelope.I usando o Flex:

```bash
flex lexerPenelope.I
```

2. Compile o código C gerado (lex.yy.c):

```bash
gcc lex.yy.c -o analisador_lexico -lfl
```

### Compilação do analisador sintático (parser)

1. Compile o arquivo de gramática usando o Bison:

```bash
bison -d -o parser.tab.c parserPenelope.y
```

2. Compile o arquivo léxico usando o Flex:

```bash
flex -o lex.yy.c lexerPenelope.I
```

3. Compile o parser completo:

```bash
gcc -o penelope_parser parser.tab.c lex.yy.c -lfl
```

### Execução do analisador léxico

Para analisar um código Penelope com o analisador léxico, execute:

```bash
./analisador_lexico < codigoPenelope.txt
```

### Execução do analisador sintático

Para analisar um código Penelope com o analisador sintático, execute:

```bash
./penelope_parser codigoPenelope.txt
```

### Testando com outros arquivos

Você pode analisar qualquer arquivo de código Penelope usando:

```bash
./analisador_lexico < seu_arquivo.txt
```

Ou para o analisador sintático:

```bash
./penelope_parser seu_arquivo.txt
```
