# Penelope

Penelope é uma proposta de linguagem de programação no contexto da disciplina de engenharia de linguagens. No repositório encontra-se o analisador léxico e o sintático (parser), com o objetivo de traduzir os códigos Penelope para C.

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

Além disso, para suportar a geração de identificadores únicos via UUID, é necessário instalar a biblioteca de desenvolvimento do UUID:

```bash
sudo apt install uuid-dev
```

---

### Compilação e execução com Makefile

Agora o projeto conta com um **Makefile** que automatiza o processo de compilação.

Para compilar o parser, basta executar:

```bash
make
```

Isso irá compilar o analisador léxico e sintático, além das dependências, e gerar o executável `penelope_parser`.

Para limpar os arquivos gerados, use:

```bash
make clean
```

---

### Execução

Foram preparados três exemplos de código Penelope, abrangendo todos os construtores sintáticos da linguagem.

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

---

### Testando com outros arquivos

Você pode analisar qualquer arquivo de código Penelope usando:

```bash
./penelope_parser seu_arquivo.txt
```

---

<br>Autores:<br>
<a href="https://github.com/JoabUrbano">Joab Urbano</a><br>
<a href="https://github.com/luizgustavoou">Luiz Gustavo</a><br>
<a href="https://github.com/Jorgelino328">Jorge William</a><br>
<a href="https://github.com/Gustavobiz">Gustavo Sousa</a><br>