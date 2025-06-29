# Penelope

Penelope é uma proposta de linguagem de programação no contexto da disciplina de engenharia de linguagens. No repositório encontra-se o analisador lexico e o síntatico, parser, e tem como objetivo traduzir os códigos penelope para C.

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

### Compilação

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
gcc -o penelope_parser parser.tab.c lex.yy.c hashMap.c -lfl
```

### Execução

Foram preparados três exemplos de código penélope, abrangendo todos os construtores sintáticos da linguagem.

Para analisar o código de um dos exemplos, execute:

```bash
./penelope_parser tests/codigoPenelope.txt
```
ou
```bash
./penelope_parser tests/comParenteses.txt
```
ou
```bash
./penelope_parser tests/quantidadeVendas.txt
```

### Testando com outros arquivos

Você pode analisar qualquer arquivo de código Penelope usando:

```bash
./penelope_parser seu_arquivo.txt
```

<br>Autores:<br>
<a href="https://github.com/JoabUrbano">Joab Urbano</a><br>
<a href="https://github.com/luizgustavoou">Luiz Gustavo</a><br>
<a href="https://github.com/Jorgelino328">Jorge William</a><br>
<a href="https://github.com/Gustavobiz">Gustavo Sousa</a><br>
