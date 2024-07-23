// Bibliotecas usadas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define buffTAM 4096

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
void alocaMeVETArgs(char ***point, int qntProgs) {
	int i = 0;

	*point = malloc(1*qntProgs);
	if (point == NULL) {
		perror("Erro de realocação de memória! ");
		exit(1);
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

//quebra a string e coloca cada comando + argumento em um ponteiro duplo, os separadores ficam em ponteiros duplos próprios
void quebraSTR(char ***pointStr, char *str, int *qntToken, int *qntAUX) {
	char *tok, **saveP = NULL;
	int aux = 0, tam, i, qntArgs = 0, retPrim = 0, qnt = 0;

	tok = strtok(str, " ");

	while (tok != NULL)
	{
		if (verifSeparador(tok) == 0) {
			if (qntArgs > 0) {

				i = 0;
				while (i < qntArgs) //volta pra primeira posição pra realocar
				{
					(*pointStr)--;
					i++;
				}
				
				if (realloc(*pointStr, (qntArgs + 1)) == NULL) { //realoca variante a quantia de argumentos
					perror("Erro de alocação de memória! ");
					exit(1);
				}

				i = 0;
				while (i < qntArgs) //volta pra posição antes do realoc
				{
					(*pointStr)++;
					i++;
				}
			}
			else if (qntArgs == 0) {
				alocaMeVETArgs(pointStr, 1);
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
			qntAUX[qnt++] = qntArgs;
			qntArgs = 0;
			retPrim++;

			pointStr++;

			alocaMeVETArgs(pointStr, 1);

			tam = strlen(tok);
			alocaMemArgs(pointStr, tam); 
			memcpy(**pointStr ,tok, tam);
			aux++;
			qntAUX[qnt++] = 1;

			retPrim++;
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
	qntAUX[qnt] = qntArgs;
	qntArgs = 0;

	i = 0;

	while (i < retPrim) //retorna para a primeira posição
	{
		pointStr--;
		i++;
	}

	*qntToken = aux;
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

//verifica separador e retorna -> 0 = PIPE | 1 = > (maior que) | 2 = < (menor que)	|| Retorna em qualquer outro caso
int verifSEP(char *args) {
 	int i = 0, aux;

 	if (strcmp(args, "|") == 0) {
		aux = 0;
	}
	else if (strcmp(args, ">") == 0) {
		aux = 1;
	}
	else if (strcmp(args, "<") == 0) {
		aux = 2;
	}
	else {
		aux = -1;
	}
	
	return aux;
}

//passa args(nome do arquivo sem espaços) e um vetor de descritor de arquivo pipe onde lê do STDIN e escreve o conteúdo lido para um arquivo
void criaArqv(char *args, int *pipeFD) {
	FILE *arq;
	char buff[buffTAM];

	memset(buff, 0, buffTAM); //zerar o vetor buffer

	dup2(pipeFD[ReadP], STDIN_FILENO);

	//fecha no geral
	close(pipeFD[WriteP]);
	close(pipeFD[ReadP]);


	arq = fopen(args, "w");
	if (arq == NULL) {
		perror("Erro de criação de arquivo");
		exit(1);
	}
	
	read(STDIN_FILENO, buff, 4096);
	fwrite(buff, 1, 4096, arq);

	fclose(arq);

	exit(1);

	//liberando fd
	//close(STDIN_FILENO);
}

//passa args, o vetor do pipe, a ponta e em qual descritor vincular para executar e ler/escrever em/de algum programa
void execProg(char **args, int *pipeFD, int ponta, int vinculo) {
	dup2(pipeFD[ponta], vinculo);

	//fecha no geral
	close(pipeFD[WriteP]);
	close(pipeFD[ReadP]);

	if (execvp(args[0], args) == -1) {
		perror("Erro de execução de programa! ");
		exit(1);
	}

	//liberando vinculo
	//close(vinculo);
}

void leArqv(char *args, int *pipeFD) {
	FILE *arq;
	char buff[buffTAM];

	memset(buff, 0, buffTAM); //zerar o vetor buffer

	dup2(pipeFD[WriteP], STDOUT_FILENO);

	//fecha no geral
	close(pipeFD[WriteP]);
	close(pipeFD[ReadP]);


	arq = fopen(args, "r");
	if (arq == NULL) {
		perror("Erro de abertura de arquivo");
		exit(1);
	}
	
	fread(buff, 1, buffTAM, arq);
	write(STDOUT_FILENO, buff, buffTAM);

	fclose(arq);

	exit(1);

	//liberando fd
	//close(STDIN_FILENO);
}

//função correta
//abre dois programas
void doisProgs(char ***args) {
	int pidF1 = -1, pidF2 = 0, pipeFD[2], sep;

	/*
	########  teste hardcode
	char *abc[] = {"ls", "-la", NULL};
	char *abcd[] = {"grep" , "R", NULL};
	*/

	if (pipe(pipeFD) < 0) {
		perror("Erro de criação de pipe! ");
		exit(1);
	}

	//verifica o separador
	sep = verifSEP(*(args[1]));

	pidF1 = fork();
	if (pidF1 > 0) {
		pidF2 = fork();
	}

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
		waitpid(pidF1, NULL, NULL);
		waitpid(pidF2, NULL, NULL);
	}
	else if (pidF1 == 0) {
		if (sep == 0 || sep == 1) {
			execProg(*args, pipeFD, WriteP, STDOUT_FILENO);
		}
		else if (sep == 2)
		{
			execProg(*args, pipeFD, ReadP, STDIN_FILENO);
		}
	}
	else if (pidF2 == 0) {
		args += 2;

		if (sep == 0) {
			execProg(*args, pipeFD, ReadP, STDIN_FILENO);
		}
		else if (sep == 1)
		{
			criaArqv(**args, pipeFD);
		}
		else if (sep == 2)
		{
			leArqv(**args, pipeFD);
		}
	}
	else {
		printf("Erro de argumentos! \n");
		exit(1);
	}
	
}

void alocaMEMint(int **point, int qnt) {
	*point = malloc(1*sizeof(int));
	if (point == NULL) {
		perror("Erro de memória! ");
		exit(1);
	}
}

//FUNÇÕES ADAPTADAS PARA 3 OU MAIS PARAMETROS
//passa args(nome do arquivo sem espaços) e um vetor de descritor de arquivo pipe onde lê do STDIN e escreve o conteúdo lido para um arquivo
void criaArqv3Args(char *args, int *pipeFD, int *pipeFD2) {
	FILE *arq;
	char buff[buffTAM];

	memset(buff, 0, buffTAM); //zerar o vetor buffer

	dup2(pipeFD[ReadP], STDIN_FILENO);

	if (pipeFD2 != NULL) {
		dup2(pipeFD2[WriteP], STDOUT_FILENO);

		close(pipeFD2[WriteP]);
		close(pipeFD2[ReadP]);
	}

	//fecha no geral
	close(pipeFD[WriteP]);
	close(pipeFD[ReadP]);


	arq = fopen(args, "w");
	if (arq == NULL) {
		perror("Erro de criação de arquivo");
		exit(1);
	}
	
	read(STDIN_FILENO, buff, 4096);
	fwrite(buff, 1, 4096, arq);

	fclose(arq);

	arq = fopen(args, "r");
	// le do arquivo e escreve na saida
	fread(buff, 1, buffTAM, arq);

	fclose(arq);

	write(STDOUT_FILENO, buff, buffTAM);

	//após escrever, ele sai, trabalho concluido
	exit(1);

	//liberando fd
	//close(STDIN_FILENO);
}

//passa args, o vetor do pipe, a ponta e em qual descritor vincular para executar e ler/escrever em/de algum programa
void execProg3Args(char **args, int *pipeFD, int *pipeFD2, int ponta1, int vinculo1, int ponta2, int vinculo2) {
	dup2(pipeFD[ponta1], vinculo1);

	
	if (pipeFD2 != NULL) {
		dup2(pipeFD2[ponta2], vinculo2);

		close(pipeFD2[WriteP]);
		close(pipeFD2[ReadP]);
	}

	//fecha no geral
	close(pipeFD[WriteP]);
	close(pipeFD[ReadP]);
	if (execvp(args[0], args) == -1) {
		perror("Erro de execução de programa! ");
		exit(1);
	}

	//liberando vinculo
	//close(vinculo);
}



void forkOPT(char **args, int sep, int *pipeFD, int *pipeFD2, int control, int extrem) {
	if (extrem == 1) {//está no inicio
		if (sep == 0) {
			execProg(args, pipeFD, WriteP, STDOUT_FILENO);
		}
		else if (sep == 1) {

			criaArqv(*args, pipeFD);
		}
		else if (sep == 2) {

			criaArqv3Args(*args, pipeFD2, pipeFD);
		}
	}
	else if (extrem == 2)// está no fim
	{
		if (sep == 0) {
			execProg(args, pipeFD, ReadP, STDIN_FILENO);
		}
		else if (sep == 1) {

			criaArqv(*args, pipeFD);
		}
		else if (sep == 2) {

			leArqv(*args, pipeFD);
		}
	}
	else {
		if (sep == 0) {
			execProg3Args(args, pipeFD, pipeFD2, ReadP, STDIN_FILENO, WriteP, STDOUT_FILENO);
		}
		else if (sep == 1) {

			criaArqv3Args(*args, pipeFD, pipeFD2);
		}
		else if (sep == 2) {

			criaArqv3Args(*args, pipeFD2, pipeFD);
		}
	}
}

void fechaPipes(int **pipeFD, int qntProgs) {
	int i;

	i = 0;
	while (i < (qntProgs - 1))
	{
		//fecha no geral
		close(*pipeFD[WriteP]);
		close(*pipeFD[ReadP]);
		pipeFD++;
		i++;
	}
}

void tresMaisProg(char ***args, int qntProgs) {
	int *pidFG = NULL, **pipeFD = NULL, *sep = NULL, control = 0, i = 0, aux;
	int **pipeFD2 = NULL, **pipeFD_base = NULL, *sep_base = NULL, savePipe1[2], savePipe2[2];

	pidFG = malloc(1 * (qntProgs));
	if (pidFG == NULL) {
		perror("Erro de memória! ");
		exit(1);
	}

	pipeFD = malloc(1 * (qntProgs - 1));
	if (pipeFD == NULL) {
		perror("Erro de memória! ");
		exit(1);
	}

	i = 0;
	while (i < (qntProgs - 1))//cria todos pipes necessários
	{
		alocaMEMint(pipeFD, 2); //aloca memória para pipes
		if (pipe(*pipeFD) < 0) {
			perror("Erro na criação do pipe ");
		}
		pipeFD++;
		i++;
	}

	i = 0;
	while (i < (qntProgs - 1))//retorna pipes pro inicio
	{
		pipeFD--;
		i++;
	}

	//aloca memoria pro separador:
	sep = malloc(1*(qntProgs - 1));
	if (sep == NULL) {
		perror("Erro de memória! ");
		exit(1);
	}

	i = 0;
	while (i < (qntProgs - 1))
	{
		args++;
		*sep = verifSEP(**args);
		sep++;
		i++;
		args++;
	}
	i = 0;
	while (i < (qntProgs - 1))
	{
		sep--;
		args--;
		args--;
		i++;
	}

	sep_base = sep;
	pipeFD_base = pipeFD;

	i = 0;
	while (i < qntProgs) { //cria todos fork necessários ps: +1 pro pai
		(*pidFG) = fork();
		control++;
		if (*pidFG == 0 || i == ((qntProgs - 1))) {
			break; //se estiver no filho quebre ou caso esteja no ultimo filho, quebre tambem
		}
		(pidFG)++;
		(args) += 2;// pula pro separador
		i++;
	}

	if (*pidFG == 0) {
		//filhos
		if (control == 1) { //está no primeiro filho
			savePipe1[0] = ((pipeFD[0])[0]);
			savePipe1[1] = ((pipeFD[0])[1]);

			i = 0;
			while (i < (qntProgs - 1)) {
				if (((*pipeFD_base)[0]) != savePipe1[0] && ((*pipeFD_base)[0]) != savePipe1[1]) {
					//fecha no geral
					close((*pipeFD_base)[WriteP]);
					close((*pipeFD_base)[ReadP]);
					pipeFD_base++;
					i++;
				}
				else {
					pipeFD_base++;
					i++;
				}
			}

			forkOPT(*args, (sep[control - 1]), savePipe1, NULL, control, 1);
		}
		else if (control == qntProgs) { //está no ultimo filho
			savePipe1[0] = ((pipeFD[control - 2])[0]);
			savePipe1[1] = ((pipeFD[control - 2])[1]);

			i = 0;
			while (i < (qntProgs - 1)) {
				if (((*pipeFD_base)[0]) != savePipe1[0] && ((*pipeFD_base)[1]) != savePipe1[1]) {
					//fecha no geral
					close((*pipeFD_base)[WriteP]);
					close((*pipeFD_base)[ReadP]);
					pipeFD_base++;
					i++;
				}
				else {
					pipeFD_base++;
					i++;
				}
			}

			forkOPT(*args, (sep[control - 2]), savePipe1, NULL, control, 2);
		}
		else {
			savePipe1[0] = ((pipeFD[control - 2])[0]);
			savePipe1[1] = ((pipeFD[control - 2])[1]);
			savePipe2[0] = ((pipeFD[control - 1])[0]);
			savePipe2[1] = ((pipeFD[control - 1])[1]);

			i = 0;
			while (i < (qntProgs - 1)) {
				if (((*pipeFD_base)[0]) == savePipe1[0] && ((*pipeFD_base)[1]) == savePipe1[1]) {
					pipeFD_base++;
					i++;
				}
				else if (((*pipeFD_base)[0]) == savePipe2[0] && ((*pipeFD_base)[1]) == savePipe2[1]) {
					pipeFD_base++;
					i++;
				}
				else {
					//fecha no geral
					close((*pipeFD_base)[WriteP]);
					close((*pipeFD_base)[ReadP]);
					pipeFD_base++;
					i++;
				}
			}

			forkOPT(*args, (sep[control - 2]), savePipe1, savePipe2, control, 0);
		}
	}
	else { //loop para o pai
		//fecha todos fd
		i = 0;
		while (i < (qntProgs - 1))
		{
			//fecha no geral
			close((*pipeFD)[WriteP]);
			close((*pipeFD)[ReadP]);
			(pipeFD)++;
			i++;
		}

		i = 0;
		while (i < (qntProgs - 1))
		{
			(pipeFD)--;
			i++;
		}

		i = 0;
		while (i < (control))
		{
			waitpid(*pidFG, NULL, NULL);
			(pidFG)--;
			i++;
		}
	}

	//libera memoria:
	i = 0;
	while (i < (qntProgs - 1)) {
		pipeFD++;
		i++;
	}

	i = 0;
	while (i < (qntProgs - 1))
	{
		(pipeFD)--;
		free(*pipeFD);
		i++;
	}

	free(sep);
	free(pipeFD);
	pidFG++; //avança pra primeira posição do ponteiro alocado
	free(pidFG);
}

int qntProgramas(char *comline, int *qntSeparador) {
	int i = 0, qntprogs = 1;
	while (comline[i] != '\0') {
		if (comline[i] == '|' || comline[i] == '>' || comline[i] == '<') {
			qntprogs++;
			(*qntSeparador)++;
		}
		i++;
	}
	return qntprogs;
}

void LidaComPrograma (char *comline) {
	char ***args = NULL;
	int qntTokens, i, j, qntProgs, separador = 0, auxposi;

	qntProgs = qntProgramas(comline, &separador);

	//vetor auxiliar quantia de args de cada arg
	int qntAUX[separador + qntProgs];
	auxposi = (separador + qntProgs) - 1;

	args = malloc(1 * strlen(comline));
	if (args == NULL) {
		perror("Erro de alocação de memória! ");
		exit(1);
	}

	quebraSTR(args, comline, &qntTokens, qntAUX);// passa a posição da memória do ponteiro duplo pra modificação

	if (qntProgs == 1) {
		umProg(*args);
	}
	else if (qntProgs == 2) {
		doisProgs(args);
	}
	else if (qntProgs >= 3) {
		tresMaisProg(args, qntProgs);
	}
	else {
		printf("!! Erro !!");
	}

	//ao final da execução, desaloca a memória:
	i = 0;
	int aux;

	while (i < (qntProgs + separador))
	{
		args++; //coloca ponteiro tripo na ultima posição
		i++;
	}

	while (i > 0)
	{
		args--; 
		aux = 0;
		while (qntAUX[auxposi] > 0) //coloca o ponteiro duplo na ultima posição
		{
			(*args)++;
			qntAUX[auxposi]--;
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
		auxposi--;
	}
	free(args);
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
