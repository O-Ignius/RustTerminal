// Bibliotecas usadas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define pipes 0
#define MaiQ 1
#define MenQ 2

#define ReadP 0
#define WriteP 1

// Declaração das funções para o .h

/*
Retornar quantia de argumentos de um determinado programa
retorna >= 1 pois arg[0] é o programa
*/
int QntArgs(char *ProgramPlsArgs);

/*
Tokeniza a string base usando pipes e separa programa a programa junto de seus argumentos
Usa memória dinâmicamente alocada e a libera ao final da função
*/
void LidaComPrograma (char *comline);

// Funções

void Retira1Espaco(char* prog) {
    int i = 0;
    if (prog[0] == ' ') {
        while (prog[i + 1] != '\0') {
            prog[i] = prog[i+1];
            i++;
        }
        prog[i] = '\0';
        if (prog[i - 1] == ' ') {
            prog[i - 1] = '\0';
        }
    }
    
}

int QntArgs(char *ProgramPlsArgs) {
	int i = 0, space = 1;

	while (ProgramPlsArgs[i] != '\0')
	{
		//retorna a quantia de espaços caso o caractere pós espaço seja o pipe (|)
		if (ProgramPlsArgs[i] == ' ' && ProgramPlsArgs[i + 1] == '\0') {
			return space;
		}
		//caso não seja, adiciona a quantia de espaços se o caractere atual for espaço
		else if (ProgramPlsArgs[i] == ' ')
		{
			space++;
		}

		i++;
	}
	return space;
}

void ExecutaSalvaARQ(char* programa, char** args, int* pipeP, int flag1, int typ, char* FName, int Fim) {
	char txt[4096];
	int save, verif;
	int filho1, filho2;

    //flag1 0 = leitura -- flag1 1 = escrita ;
    //typ == 0 -> | -- typ == 1 -> > -- typ == 2 -> < ;

	//programa filho (o que o shell irá rodar) o que escreve
	if ((filho1 = fork()) == 0) {
		if (flag1 == ReadP) {
			dup2(pipeP[ReadP], STDIN_FILENO);
		}
		else {
			dup2(pipeP[WriteP], STDOUT_FILENO);
		}
        
		//fecha para todos
		close(pipeP[WriteP]);
        close(pipeP[ReadP]);

        if (typ == MenQ) {
            save = open(FName, O_RDWR);
            if (save == -1) {
                perror("Erro de abertura/criação de arquivo! ");
                exit(1);
            }
            //enquanto não chegar ao EOF
            verif = read(save, txt, 4096);
            while(verif != 0) {
                if (verif == -1) {
                    perror("Erro de leitura de saída! ");
                }

                write(STDOUT_FILENO, txt, strlen(txt));
                verif = read(save, txt, 4096);
            }
            

            //devolve descritor arquivo:
			if (flag1 == WriteP) {
				close(STDIN_FILENO);
			}
			else {
				close(STDOUT_FILENO);
			}
        }
        else {
			if (execvp(programa, args) == -1) {
				perror("Erro de execução do programa! ");
			}
        }
	}
    //erro
	else if (filho1 < 0) {
		perror("Erro de criação de processo filho! ");
		exit(1);
	}
    if (typ != pipes) {
        if ((filho2 = fork()) == 0) {
			//flag aqui será o contrário do fork acima
			if (flag1 == WriteP) {
				dup2(pipeP[ReadP], STDIN_FILENO);
			}
			else {
				dup2(pipeP[WriteP], STDOUT_FILENO);
			}

			//fecha no geral
			close(pipeP[WriteP]);
			close(pipeP[ReadP]);

			//aqui só pode ser do tipo maior que ou menor que
			if (typ == MaiQ) {
				creat(FName, 00700 | 00070 | 00007);
				save = open(FName, O_RDWR);
				if (save == -1) {
					perror("Erro de abertura/criação de arquivo! ");
					exit(1);
				}
				//enquanto ler, escreva
				while ((verif = read(STDIN_FILENO, txt, 4096)) != 0)
				{
					if (verif == -1) {
						perror("Erro de leitura de saída! ");
					}
					write(save, txt, strlen(txt));
				}

				//devolve descritor arquivo:
				if (flag1 == WriteP) {
					close(STDIN_FILENO);
				}
				else {
					close(STDOUT_FILENO);
				}
				
			}
			else {
				dup2(pipeP[ReadP], STDIN_FILENO);

				close(pipeP[WriteP]);
				close(pipeP[ReadP]);

				if (execvp(programa, args) == -1) {
					perror("Erro de execução do programa! ");
				}
			}
	    }
	    //erro
        else if (filho2 < 0) {
            perror("Erro de criação de processo filho! ");
            exit(1);
        }
    }
	//programa pai (SHELL)
	else {
		if (Fim == 1) {
			char lido[4096];
			int ler = 0;

			dup2(pipeP[ReadP], STDIN_FILENO);

			//fecha para todos
			close(pipeP[WriteP]);
			close(pipeP[ReadP]);

			//3° parametro: WUNTRACED = executa 1 por vez | WNOHANG = executa mesmo que o outro não tenha finalizado
			waitpid(filho1, NULL, WUNTRACED);
			if (typ != pipes) {
				waitpid(filho2, NULL, WUNTRACED);
			}

			//enquanto escrita, leia
			ler = read(STDIN_FILENO, lido, 4096);
			while (ler != 0)
			{
				if (ler == -1) {
					perror("Erro de leitura! ");
					break;
				}
				write(STDOUT_FILENO, lido, strlen(lido));
				fflush(stdout);
				ler = read(STDIN_FILENO, lido, 4096);
			}

			close(STDIN_FILENO);
		}
		else {
			close(pipeP[WriteP]);
			close(pipeP[ReadP]);

			//3° parametro: WUNTRACED = executa 1 por vez | WNOHANG = executa mesmo que o outro não tenha finalizado
			waitpid(filho1, NULL, WUNTRACED);
			if (typ != pipes) {
				waitpid(filho2, NULL, WUNTRACED);
			}
		}
	}
}

void RetiraAspas (char* str) {
	int i = 0, aux = 0;

	if (i == 0 && str[i] == '\"') {
		str[i] = str[i + 1];
		i++;
		while(str[i + 1] != '\"') {
			str[i] = str[i + 1];
			i++;
		}
		str[i] = '\0';
	}
}

void TokenizacaoPIPE (char* string, int* pipeP, int* qualpipe, int Fim) {
	char *tokenargs, **prog, token[strlen(string) + 1];
	int qntargs, i = 0;

	//copia string recebida para uma auxiliar
	strcpy(token, string);

	//aloca memória para receber o nome de chamada do programa + argumentos
	prog = (char** ) malloc(sizeof(char));

	if (prog == NULL) {
		perror("Erro na alocação de memória! ");
		exit(1);
	}

	qntargs = QntArgs(token);

	i = 0;
	//aloca cada espaço do ponteiro duplo para armazenar argumentos
	while (i < qntargs)
	{
		if (i == 0) {
			prog[i] = (char* )malloc(sizeof(char) * 200);
		}
		else{
			prog[i] = (char* )malloc(sizeof(char) * 50);
		}
		i++;
	}
	prog[i] = (char* )malloc(1);
	prog[i] = NULL;

	tokenargs = strtok(token, " ");
	//Se não há tokens iniciais, há somente o programa
	if (tokenargs == NULL) {
		ExecutaSalvaARQ(token, NULL, pipeP, *qualpipe, pipes, NULL, 0);
	}
	//se há argumentos separemos o executável de seus argumentos
	else {
		strcpy(prog[0], tokenargs);
		i = 1;

		if (qntargs > 1) {
			tokenargs = strtok(NULL, " ");

			while (tokenargs != NULL)
			{
				strcpy(prog[i], tokenargs);
				i++;
				tokenargs = strtok(NULL, " ");
			}
		}
		else{
			prog[i] = NULL;
		}

		//executa o programa junto de seus argumentos
		ExecutaSalvaARQ(prog[0], prog, pipeP, *qualpipe, pipes, NULL, Fim);

		if (*qualpipe == 0) {
			*qualpipe = 1;
		}
		else if (*qualpipe == 1) {
			*qualpipe = 0;
		}
	}

	//libera memória alocada dos args
	i = qntargs;

	while (i >= 0)
	{
		free(prog[i]);
		i--;
	}
	free(prog);
}

void TokenizacaoMaQ (char* string, int* pipeP, int* qualpipe) {
	char *tokenargs, **prog, token[strlen(string) + 1], FName[200];
	int qntargs, i = 0, controle = 0;

	//copia string recebida para uma auxiliar
	strcpy(token, string);

	//aloca memória para receber o nome de chamada do programa + argumentos
	prog = (char** ) malloc(sizeof(char));

	if (prog == NULL) {
		perror("Erro na alocação de memória! ");
		exit(1);
	}

	qntargs = QntArgs(token);

	i = 0;
	//aloca cada espaço do ponteiro duplo para armazenar argumentos
	//-2 pois não quero salvar o ">" nem a string com nome do arquivo
	while (i < qntargs - 2)
	{
		if (i == 0) {
			prog[i] = (char* )malloc(sizeof(char) * 200);
		}
		else if (i == qntargs - 3) {
			prog[i] = (char* )malloc(sizeof(char) * 4096);
		}
		else {
			prog[i] = (char* )malloc(sizeof(char) * 50);
		}
		i++;
	}

	tokenargs = strtok(token, " ");
	//Se não há tokens iniciais, há somente o programa
	if (tokenargs == NULL) {
		ExecutaSalvaARQ(prog[0], prog, pipeP, *qualpipe, MaiQ, FName, 0);
	}
	//se há argumentos separemos o executável de seus argumentos
	else {
		strcpy(prog[0], tokenargs);
		i = 1;

		if (qntargs > 1) {
			tokenargs = strtok(NULL, " ");

			while (tokenargs != NULL)
			{
				if (strstr(tokenargs, "\"")) {
					RetiraAspas (tokenargs);
					strcpy(prog[i], tokenargs);
					i++;
				}
				else if (strstr(tokenargs, ">") != NULL) {
					controle = 1;
				}
				else if (controle == 0) {
					strcpy(prog[i], tokenargs);
					i++;
				}
				else if (controle == 1){
					strcpy(FName, tokenargs);
				}
				
				tokenargs = strtok(NULL, " ");
			}
		}
		else{
			prog[i] = NULL;
		}

		ExecutaSalvaARQ(prog[0], prog, pipeP, *qualpipe, MaiQ, FName, 0);
	}

	//libera memória alocada dos args
	i = qntargs - 3;

	while (i >= 0)
	{
		free(prog[i]);
		i--;
	}
	free(prog);

	if (*qualpipe == 0) {
		*qualpipe = 1;
	}
	else if (*qualpipe == 1) {
		*qualpipe = 0;
	}
}

void TokenizacaoMeQ (char* string, int* pipeP, int* qualpipe) {
	char *tokenargs, **prog, token[strlen(string) + 1], FName[200];
	int qntargs, i = 0, controle = 0;

	//copia string recebida para uma auxiliar
	strcpy(token, string);

	//aloca memória para receber o nome de chamada do programa + argumentos
	prog = (char** ) malloc(sizeof(char));

	if (prog == NULL) {
		perror("Erro na alocação de memória! ");
		exit(1);
	}

	qntargs = QntArgs(token);

	i = 0;
	//aloca cada espaço do ponteiro duplo para armazenar argumentos
	//-2 pois não quero salvar o ">" nem a string com nome do arquivo
	while (i < qntargs - 2)
	{
		//programa
		if (i == 0) {
			prog[i] = (char* )malloc(sizeof(char) * 200);
		}
		//argumentos | nome arquivo
		else {
			prog[i] = (char* )malloc(sizeof(char) * 100);
		}
		i++;
	}

	tokenargs = strtok(token, " ");
	//Se não há tokens iniciais, há somente o programa
	if (tokenargs == NULL) {
		ExecutaSalvaARQ(prog[0], prog, pipeP, *qualpipe, MenQ, FName, 0);
	}
	//se há argumentos separemos o executável de seus argumentos
	else {
		strcpy(prog[0], tokenargs);
		i = 1;

		if (qntargs > 1) {
			tokenargs = strtok(NULL, " ");

			while (tokenargs != NULL)
			{
				if (strstr(tokenargs, "\"")) {
					RetiraAspas (tokenargs);
					strcpy(prog[i], tokenargs);
					i++;
				}
				else if (strstr(tokenargs, "<") != NULL) {
					controle = 1;
				}
				else if (controle == 0) {
					strcpy(prog[i], tokenargs);
					i++;
				}
				else if (controle == 1){
					strcpy(FName, tokenargs);
				}
				
				tokenargs = strtok(NULL, " ");
			}
		}
		else{
			prog[i] = NULL;
		}

		ExecutaSalvaARQ(prog[0], prog, pipeP, *qualpipe, MenQ, FName, 0);
	}

	//libera memória alocada dos args
	i = qntargs - 3;

	while (i >= 0)
	{
		free(prog[i]);
		i--;
	}
	free(prog);

	if (*qualpipe == 0) {
		*qualpipe = 1;
	}
	else if (*qualpipe == 1) {
		*qualpipe = 0;
	}
}

void Processos () {

}

void LidaComPrograma (char *comline) {
	char *tokenprog, *saveP, *verifFim;
	int i = 0;
    int pipe1[2], pipe2[2], qualpipe = 1, Fim = 0;

	// Usa strtok para quebrar a string no 1° programa + argumento ou semelhante
	tokenprog = strtok_r(comline, "|", &saveP);

	while (tokenprog != NULL) {
		verifFim = saveP;

		if (strtok_r(NULL, "|", &verifFim) == NULL) {
			Fim = 1;
		}

		if (strstr(tokenprog, ">") != NULL) {
			TokenizacaoMaQ (tokenprog, pipeP, &qualpipe);
		}
		else if (strstr(tokenprog, "<") != NULL) {
			TokenizacaoMeQ (tokenprog, pipeP, &qualpipe);
		}
		else {
			TokenizacaoPIPE (tokenprog, pipeP, &qualpipe, Fim);
		}
		tokenprog = strtok_r(NULL, "|", &saveP);
		// so pra retirar os espaços iniciais e finais caso estes existam pra evitar bugs
		if (tokenprog != NULL) {
			Retira1Espaco(tokenprog);
		}
	}
	tokenprog = strtok_r(NULL, "\0", &saveP);

	if (tokenprog != NULL) {
		if (strstr(tokenprog, ">") != NULL) {
		TokenizacaoMaQ (tokenprog, pipeP, &qualpipe);
		}
		else if (strstr(tokenprog, "<") != NULL) {
			TokenizacaoMeQ (tokenprog, pipeP, &qualpipe);
		}
		else {
			TokenizacaoPIPE (tokenprog, pipeP, &qualpipe, Fim);
		}
	}
}

//func teste
void tiraon(char *txt) {
	for (int i = 0; i < 4096; i++) {
		if (txt[i] == '\n') {
			txt[i] = '\0';
			break;
		}
	}
}

void alocaMem(char *point, int tam) {
	point = malloc(1 * tam);
}

void quebraSTR(char **pointStr, char *str, int *qntToken) {
	char *tok;
	int aux = 0;

	tok = strtok(str, " ");
	alocaMem(*pointStr, strlen(tok));
	pointStr++;
	aux++;
	while (tok != NULL)
	{
		tok = strtok(NULL, " ");
		alocaMem(*pointStr, strlen(tok));
		pointStr++;
		aux++;
	}
	
	*qntToken = aux;
	//retorna o ponteiro duplo pra primeira posição
	for (int i = 0; i <= aux; i ++) {
		pointStr--;
	}
}

int main (void) {
	char comline[4096];

	while(1) {
		//zerar string base
		memset(comline, 0, strlen(comline));

		printf("$ ");
		setbuf(stdout, NULL);
		setbuf(stdin, NULL);

		int leu = read(STDIN_FILENO, comline, 4096);
		if (leu == -1) {
			perror("Erro de leitura de teclado! ");
		}
		tiraon(comline);

		//QUEBRA O LOOP INFINITO
		if (strstr(comline, "exit") != NULL) {
			return 0;
		}

		LidaComPrograma (comline);
	}

	return 0;
}
