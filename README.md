# RustTerminal

## Resumo Geral:
Este código é uma implementação de um terminal rústico, tendo funções básicas de redirecionamento de saida de programas para arquivos ou outros programas.
O código espera por um input do usuário, sendo este limitado a 4096 bytes, que será quebrado de acordo com os espaços e será dividido, para isso, utilizando um ponteiro triplo, para acomodar os ponteiros duplos de cada possível argumento. A alocação se dá de acordo com o tamanho do input;

### Detalhes:
O principal do código foi separado da seguinte forma:
De acordo com o input do usuário, é verificado a quantia de separadores (>, | ou <), segundo essa quantia, é calculado a quantidade de programas/tarefas a serem executados(as)
Segundo o valor obtido com a verificação, o código é redirecionado para alguma das três funções principais:
1. Realiza a execução quando há um único programa, basicamente executando-o junto a seus argumentos.
2. Realiza a execução quando há dois programas, verificando o tipo de separador executando o(s) programa(s), salvando a saída de um programa no arquivo ou lendo de um arquivo já existente e passando o conteúdo para um programa.
3. É a função mais complexa das 3, ela foi criada com a proposta de ser o mais adaptável de acordo com o input do usuário, desde que este tenha uma quantia maior ou igual a 2 separadores ou 3 tarefas. A função funciona de forma a salvar em um ponteiro alocado dinâmicamente os PID's dos processos filhos; Cria um ponteiro duplo e aloca ele de acordo com a quantia de pipes para salvar os pipes e os passar para cada filho; Aloca um ponteiro simples para guardar um valor referente ao tipo de separador ao qual o programa/tarefa estará vinculado. Após a execução, ele desaloca esses ponteiros que foram alocados durante a função.
Ademais temos funções de abertura de programas/arquivos, leitura de arquivos e funções secundárias que auxiliam as funções principais

### Bugs conhecidos:
Ao se utilizar comandos que possuem parâmetros inteiriços mas separados por espaços, como echo "text text", o programa não realiza um subtratamento de string, visto que ele quebra a string de acordo com os espaços, ao se colocar um parâmetro semelhante ao exemplo, o programa retornará um erro.

### Possíveis bugs:
É possível que se ocorram bugs ao realizar comando do tipo ls | grep x < archive.txt ou semelhantes.
