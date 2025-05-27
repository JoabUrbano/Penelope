# Penelope
Penelope é uma proposta de linguagem de programação alto nível

## Como executar

### Pré-requisitos
Para executar o analisador léxico da linguagem Penelope, você precisará ter o Flex instalado no seu sistema. Se ainda não tiver, instale-o com:

```bash
sudo apt update
sudo apt install flex
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

### Execução
Para analisar um código Penelope, execute:
```bash
./analisador_lexico < codigoPenelope.txt
```

Isso processará o arquivo de código e mostrará os tokens identificados pelo analisador léxico.

### Testando com outros arquivos
Você pode analisar qualquer arquivo de código Penelope usando:
```bash
./analisador_lexico < seu_arquivo.txt
```