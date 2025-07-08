# Penelope

Penelope é uma proposta de linguagem de programação no contexto da disciplina de engenharia de linguagens. No repositório encontra-se o analisador léxico e o sintático (parser), com o objetivo de traduzir os códigos Penelope para C.

## Como executar

### Pré-requisitos

Para executar o analisador léxico da linguagem Penelope, você precisará ter o Flex instalado no seu sistema. Se ainda não tiver, instale-o com:

```bash
sudo apt update
sudo apt install flex
```

Para compilar o parser, você também precisará do Bison:

```bash
sudo apt install bison
```

Além disso, para suportar a geração de identificadores únicos via UUID, é necessário instalar a biblioteca de desenvolvimento do UUID:

```bash
sudo apt install libuuid1 uuid-dev
```

---

### Compilação

O projeto conta com um **Makefile** que automatiza o processo de compilação.

Para compilar o parser, basta executar:

```bash
make
```

Este comando irá:
1. Criar automaticamente o diretório `build/` (se não existir)
2. Gerar o analisador léxico (`lex.yy.c`) a partir de `src/lexer/penelope.l` usando o Flex
3. Gerar o analisador sintático (`parser.tab.c` e `parser.tab.h`) a partir de `src/penelope.y` usando o Bison
4. Compilar todos os módulos do projeto
5. Criar o executável `penelope_parser` no diretório raiz

Para limpar os arquivos gerados, use:

```bash
make clean
```


## Executando Programas Penelope

### Execução Rápida (Recomendado)
```bash
# Compila e cria executável automaticamente
./penelope_parser -c testes/Problemas/Problema1.txt

# Executa o programa
./output/Problema1
```

### Execução Manual (se necessário)
Se você gerou apenas o código C com `-S`, pode compilar manualmente:

```bash
# Gerar apenas código C
./penelope_parser -S testes/Problemas/Problema1.txt

# Compilar manualmente 
gcc -o output/Problema1 output/Problema1.c -lm

# Executar
./output/Problema1
```

## Exemplos de Código Penelope

O repositório inclui vários exemplos funcionais de código Penelope:

### Problemas Principais (Totalmente Funcionais):
- **`testes/Problemas/Problema1.txt`** - Cálculos matemáticos com exponenciação
- **`testes/Problemas/Problema2.txt`** - Classificação de números com estruturas condicionais aninhadas  
- **`testes/Problemas/Problema3.txt`** - Operações com matrizes 2D (soma e multiplicação)

### Exemplos de Conceitos:
- `testes/CodigosExemplos/codigoPenelope.txt` - Exemplo com funções e arrays 1D
- `testes/CodigosExemplos/quantidadeVendas.txt` - Processamento de dados com arrays
- `testes/CodigosExemplos/comParenteses.txt` - Exemplo de código com parênteses e operações matemáticas 

## Características da Linguagem Penelope

A linguagem Penelope suporta:

### Tipos de Dados
- **Tipos básicos**: `int`, `float`, `bool`, `string`
- **Arrays 1D**: `int[]`, `float[]`, `string[]`
- **Arrays 2D**: `int[][]`, `float[][]` (com alocação dinâmica automática)
- **Declaração com tipagem explícita**: `int: x = 5;`

### Estruturas de Controle
- **Condicionais**: `if-else` (com suporte a aninhamento profundo)
- **Loops**: `while`, `for` (com declaração inline de variáveis)
- **Controle de fluxo**: `break`, `return`

### Funcionalidades Avançadas
- **Funções**: Com parâmetros tipados e valores de retorno
- **Operações matemáticas**: Incluindo exponenciação (`**`) 
- **Arrays literais**: `int[]: nums = [1, 2, 3, 4, 5];`
- **E/S**: Comandos `read()` e `print()` com formatação inteligente
- **Matrizes**: Operações automáticas em arrays 2D com inferência de dimensões

---

## Ajduda

Para mais informações sobre como usar o Penelope, você pode acessar a ajuda do parser com o seguinte comando:

```bash
./penelope_parser --help
```

<br>Autores:<br>
<a href="https://github.com/JoabUrbano">Joab Urbano</a><br>
<a href="https://github.com/luizgustavoou">Luiz Gustavo</a><br>
<a href="https://github.com/Jorgelino328">Jorge William</a><br>
<a href="https://github.com/Gustavobiz">Gustavo Sousa</a><br>