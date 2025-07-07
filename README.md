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

**Nota:** O diretório `build/` contém apenas arquivos gerados automaticamente e não está incluído no repositório Git. Ele será criado automaticamente durante a compilação.

---

## Uso

Para ver todas as opções disponíveis:

```bash
./penelope_parser --help
```

## Modos de Execução

O parser da linguagem Penelope possui dois modos principais de operação:

### 1. Análise Semântica e Sintática

Para apenas analisar a sintaxe e semântica de um programa Penelope sem gerar código C:

```bash
./penelope_parser arquivo_penelope.txt
```

**Exemplo:**
```bash
./penelope_parser input/CodigosExemplos/codigoPenelope.txt
```

Este modo irá:
- Verificar a sintaxe do código Penelope
- Realizar análise semântica (verificação de tipos, declaração de variáveis, etc.)
- Exibir a tabela de símbolos
- Reportar erros encontrados

### 2. Geração de Código C

Para gerar código C a partir de um programa Penelope:

```bash
./penelope_parser -c arquivo_penelope.txt
```
ou
```bash
./penelope_parser --generate-c arquivo_penelope.txt
```

**Exemplo:**
```bash
./penelope_parser -c input/CodigosExemplos/codigoPenelope.txt
```

Este modo irá:
- Realizar análise sintática e semântica
- Gerar código C equivalente
- Salvar o código C gerado na pasta `output/` com o mesmo nome do arquivo de entrada, mas com extensão `.c`

#### Especificando arquivo de saída

Você também pode especificar um nome personalizado para o arquivo C gerado:

```bash
./penelope_parser -o output/meu_programa.c arquivo_penelope.txt
```

### Opções de Linha de Comando

- `-c, --generate-c`: Gera código C a partir do código Penelope
- `-o ARQUIVO`: Especifica o arquivo de saída (automaticamente habilita geração de código C)
- `-h, --help`: Mostra a mensagem de ajuda com todas as opções

### Observações sobre Uso

- Se nenhum arquivo for especificado, o parser lê da entrada padrão
- A opção `-o` automaticamente habilita a geração de código C (não precisa usar `-c` junto)
- Por padrão, quando usando `-c` sem `-o`, o código C é salvo em `output/` com o mesmo nome do arquivo de entrada mas com extensão `.c`

## Executando o Código C Gerado

Após gerar o código C, você pode compilá-lo e executá-lo:

### Passo 1: Compilar o código C gerado
```bash
gcc -o output/programa output/meu_programa.c
```

### Passo 2: Executar o programa
```bash
./output/programa
``` 

## Exemplos de Código Penelope

O repositório inclui vários exemplos de código Penelope em diferentes locais:

### Exemplos principais:
- `input/CodigosExemplos/codigoPenelope.txt` - Exemplo com funções e arrays
- `input/CodigosExemplos/comParenteses.txt` - Exemplo com expressões parentetizadas
- `input/CodigosExemplos/quantidadeVendas.txt` - Exemplo de processamento de dados

### Problemas:
- `input/Problemas/Problema1.txt`
- `input/Problemas/Problema2.txt`
- `input/Problemas/Problema3.txt`

## Características da Linguagem Penelope

A linguagem Penelope suporta:
- Declaração de variáveis com tipagem explícita (`int: x = 5;`)
- Tipos básicos: `int`, `float`, `bool`, `string`
- Arrays unidimensionais e multidimensionais
- Estruturas de controle: `if-else`, `while`, `for`
- Funções com parâmetros e valores de retorno
- Operações aritméticas, lógicas e de comparação
- Comandos de entrada (`read`) e saída (`print`)

---

<br>Autores:<br>
<a href="https://github.com/JoabUrbano">Joab Urbano</a><br>
<a href="https://github.com/luizgustavoou">Luiz Gustavo</a><br>
<a href="https://github.com/Jorgelino328">Jorge William</a><br>
<a href="https://github.com/Gustavobiz">Gustavo Sousa</a><br>