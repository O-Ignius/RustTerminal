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
//tira o \n :P
void tiraon(char *txt) {
	for (int i = 0; i < 4096; i++) {
		if (txt[i] == '\n') {
			txt[i] = '\0';
			break;
		}
	}
}

//pensando que temos um ponteiro triplo (só pra apontar para os ponteiros duplos)
//Está função realoca o ponteiro duplo de acordo com a quantia de argumentos que um determinado programa tem
void alocaMeVETArgs(char **point, int qntProgs) {
	int i = 0;

	//retorna ponteiro pro inicio
	while (i < qntProgs)
	{
		point--;
		i++;
	}

	if (realloc(point, 1 * qntProgs) == NULL) {
		perror("Erro de realocação de memória! ");
		exit(1);
	}

	i = 0;
	//retorna ponteiro para posição que estava antes do realloc
	while (i < qntProgs)
	{
		point++;
		i++;
	}

}

//aloca um vetor para receber uma string que contenha um argumento ou nome do programa
void alocaMemArgs(char ***point, int tam) {
	**point = malloc(1 * tam);
	if (**point == NULL) {
		perror("Erro de alocação de memória! ");
		exit(1);
	}
}
	
//Uma função que retorna 1 caso tok seja pipe, maior que ou menor que, senão, retorna 0
int verifSeparador(char *tok) {
	if (strcmp(tok, "<") == 0) {
		return (1);
	}
	else if (strcmp(tok, "|") == 0)
	{
		return (1);
	}
	else if (strcmp(tok, ">") == 0)
	{
		return (1);
	}
	else {
		return (0);
	}
}

//retorna tambem um inteiro referente a quantia de programas a serem abertos
//############Função Correta!
int quebraSTR(char ***pointStr, char *str, int *qntToken) {
	char *tok, **saveP, ***saveppp;
	int aux = 0, tam, qntProgs = 1, i, qntArgs = 0;

	tok = strtok(str, " ");

	while (tok != NULL)
	{
		if (verifSeparador(tok) == 0) {
			if (qntArgs > 0) {
				alocaMeVETArgs(*pointStr, qntArgs);
			}
			else {
				*pointStr = malloc(1);
				if (pointStr == NULL) {
					perror("Erro de alocação de memória! ");
					exit(1);
				}
			}

			tam = strlen(tok);
			alocaMemArgs(pointStr, tam); 
			memcpy(**pointStr ,tok, tam);
			saveP = (*pointStr);	//salva posição do ponteiro para apontá-lo pra NULL
			(*pointStr)++;
			aux++;
			qntArgs++;
		}
		else {
			//volta ponteiro pra posição inicial
			i = 0;
			(*pointStr) = NULL;
			(*pointStr) = saveP; //volta o ponteiro para antes de NULL

			while (i < (qntArgs - 1)) //-1 pois não está levando em conta o NULL do final
			{	
				(*pointStr)--;
				i++;
			}
			qntArgs = 0;

			qntProgs++;
			pointStr++;

			*pointStr = malloc(1);
			if (pointStr == NULL) {
				perror("Erro de alocação de memória! ");
				exit(1);
			}

			tam = strlen(tok);
			alocaMemArgs(pointStr, tam); 
			memcpy(**pointStr ,tok, tam);
			aux++;

			pointStr++;
		}

		tok = strtok(NULL, " ");
	}

	i = 0;
	(*pointStr) = NULL;
	(*pointStr) = saveP; //volta o ponteiro para antes de NULL

	while (i < (qntArgs - 1)) //-1 pois não está levando em conta o NULL do final
	{	
		(*pointStr)--;
		i++;
	}
	qntArgs = 0;

	i = 0;

	while (i < qntProgs) //retorna para a primeira posição
	{
		pointStr--;
		i++;
	}

	*qntToken = aux;

	return qntProgs;
}

//função correta
//abre um único programa
void umProg(char **args) {
	int pidF;

	pidF = fork();

	if (pidF < 0) {
		perror("Erro de fork! ");
		exit(1);
	}
	
	//pai
	if (pidF > 0) {
		//3° parametro: WUNTRACED = executa 1 por vez | WNOHANG = executa mesmo que o outro não tenha finalizado
		waitpid(pidF, NULL, WUNTRACED);
	}
	//filho
	else {
		if (execvp(args[0], args) == -1) {
			perror("Erro de execução de programa! ");
			exit(1);
		}
	}
}

int verifMenQ(char **args, int qntTokens) {
	int i = 0, aux = 0;;

	while (i < qntTokens)
	{
		if (strcmp(args[i], "<") == 0) {
			aux++;
		}

		i++;
	}
	
	return aux;
}

//função correta
//abre dois programas
void doisProgs(char ***args, int qntTokens) {
	int pidF1, pidF2, pipeFD[2];

	/*
	########  teste hardcode
	char *abc[] = {"ls", "-la", NULL};
	char *abcd[] = {"grep" , "R", NULL};
	*/

	if (pipe(pipeFD) < 0) {
		perror("Erro de criação de pipe! ");
		exit(1);
	}


	pidF1 = fork();
	pidF2 = fork();

	if (pidF1 < 0) {
		perror("Erro de fork! ");
		exit(1);
	}
	if (pidF2 < 0) {
		perror("Erro de fork! ");
		exit(1);
	}
	
	//pai
	if (pidF1 > 0 && pidF2 > 0) {
		//fecha no geral
		close(pipeFD[WriteP]);
		close(pipeFD[ReadP]);

		//3° parametro: WUNTRACED = executa 1 por vez | WNOHANG = executa mesmo que o outro não tenha finalizado
		waitpid(pidF1, NULL, WUNTRACED);
		waitpid(pidF2, NULL, WUNTRACED);
	}
	//filho1
	else if (pidF1 == 0) {
		dup2(pipeFD[WriteP], STDOUT_FILENO);

		//fecha no geral
		close(pipeFD[WriteP]);
		close(pipeFD[ReadP]);

		if (execvp(*args[0], *args) == -1) {
			perror("Erro de execução de programa! ");
			exit(1);
		}
	}
	else if (pidF2 == 0) {
		args+=2;
		dup2(pipeFD[ReadP], STDIN_FILENO);

		//fecha no geral
		close(pipeFD[WriteP]);
		close(pipeFD[ReadP]);

		if (execvp(*args[0], *args) == -1) {
			perror("Erro de execução de programa! ");
			exit(1);
		}
	}
}

void tresMaisProg() {
;
}

void LidaComPrograma (char *comline) {
	char ***args;
	int qntTokens, i, j, qntProgs;

	qntProgs = quebraSTR(&(*args), comline, &qntTokens);// passa a posição da memória do ponteiro duplo pra modificação

	if (qntProgs == 1) {
		umProg(*args);
	}
	else if (qntProgs == 2) {
		doisProgs(args, qntTokens);
	}
	else if (qntProgs >= 3) {

	}
	else {
		printf("!! Erro !!");
	}

	//ao final da execução, desaloca a memória:
	i = 0;
	int aux;

	while (i < (qntProgs + (qntProgs - 1)))
	{
		args++; //coloca ponteiro tripo na ultima posição
		i++;
	}

	while (i > 0)
	{
		args--; 
		aux = 0;
		while (**args != NULL) //coloca o ponteiro duplo na ultima posição
		{
			(*args)++;
			aux++;
		}
		while (aux > 0) //desaloca a partir da ultima posição
		{
			(*args)--;
			free(**args);
			aux--;
		}
		free(*args);//libera memoria do ponteiro triplo a partir da ultima posição
		i--;
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
