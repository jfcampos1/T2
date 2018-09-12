#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include  <signal.h>

typedef struct process{
	int pid;
	char name[256];
	char status[20];
	int tiempo;
	int length;
	int **array_rafagas;
} Process;

typedef struct procesos{
	Process** oneprocess;
	int length;
} Procesos;

typedef struct board{
	int n_a;
	int n_b;
	int n_c;
	int n_d;
	int length;
	int **array_board;
	int **array_first;
	int **array_around;
	int ***array_past;
} Board;

typedef struct boards{
	Board** oneboard;
	int length;
} Boards;

void free_all_mem(Boards* todos_tableros, Procesos* todos_procesos);
void PrintUsage(); // Print the correct usage
void printboard(Board* tablero, int turn);
void printorder(Board* tablero, int turn, int i, int x);
void printfinalprocess(Boards* todos_tableros, Procesos* todos_procesos);
int countcells(Board* tablero);
void writeoutput(Procesos* todos_procesos, Boards* todos_tableros);

int readlines(char * filename, Procesos* todos_procesos, Boards* todos_tableros);
void initialice_mem(Board* tablero);
void free_mem(Board* tablero);
void free_mem_rafagas(Process* proceso);

int simulation(Procesos* todos_procesos, Boards* todos_tableros, int time);
void un_process_loop(Procesos* todos_procesos, Boards* todos_tableros, int timer, int i);
void loadprocess(Board* tablero, Process* un_proceso);
// Check surroundings
void reviewaround(Board* tablero);
int sum_down(Board* tablero, int i, int x);
int sum_center(Board* tablero, int i, int x);
int sum_up(Board* tablero, int i, int x);

void lifeordeath(Board* tablero);
int checkloop(Board* tablero);
int checkempty(Board* tablero);
void reorderpast(Board* tablero);
void statesave(Procesos* todos_procesos, int out_empty,int out_loop, int count);

void INThandler(int);

Procesos* todos_procesos;
Boards* todos_tableros;
int number_process;
int count;
int out_empty;
int out_loop;
int timer;
int withsubprocess = 0;  // --------- Edit here to 1 to don't use subprocess and test valgrind---

int main(int argc, char *argv[]) {

	// Number of arguments less than 3, exit
	if (argc != 3){
		PrintUsage();
		return 1;
	}
	signal(SIGINT, INThandler);
	todos_procesos = malloc(sizeof(Procesos));
	todos_tableros = malloc(sizeof(Boards));
	readlines(argv[1], todos_procesos, todos_tableros);
	timer = atoi(argv[2]);
	simulation(todos_procesos, todos_tableros, atoi(argv[2]));
	printfinalprocess(todos_tableros ,todos_procesos);
	writeoutput(todos_procesos, todos_tableros);
	free_all_mem(todos_tableros,todos_procesos);
  return 0;
}

void free_all_mem(Boards* todos_tableros, Procesos* todos_procesos){
	for(int i = 0; i < todos_procesos -> length; i++){
		free_mem_rafagas(todos_procesos -> oneprocess[i]);
		free(todos_procesos -> oneprocess[i]);
	}
	free(todos_procesos -> oneprocess);
	free(todos_procesos);
	for(int i = 0; i < todos_tableros -> length; i++){
		free_mem(todos_tableros -> oneboard[i]);
		free(todos_tableros -> oneboard[i]);
	}
	free(todos_tableros -> oneboard);
	free(todos_tableros);
}
// Print the correct usage
void PrintUsage(){
	printf("Usage: ./life <file> <t> \n");
	printf("<file> nombre de archivo input.\n");
	printf("<t> tiempo maximo de ejecución por tablero.\n");
}

void printboard(Board* tablero, int turn){
	int num_d = tablero -> n_d;
	printf("  | ");
	for(int i = 0; i < num_d; i++){
		if(i+1 > 9){ // i +1 if start in 1,1
			printf("%d| ", i);
		}
		else{// i +1 if start in 1,1
			printf("%d| ", i);
		}
	}
	printf("\n");
	for(int i = 0; i < num_d; i++){
		if(i > 9){ // i +1 if start in 1,1
			printf("%d|", i);
		}
		else{ // i +1 if start in 1,1
			printf("%d|", i);
		}
		for(int x = 0; x < num_d; x++){
			printorder(tablero, turn, i , x);
		}
		printf("\n");
	}
	printf("\n");
}

void printorder(Board* tablero, int turn, int i, int x){
	if(turn == 0){
		if(tablero -> array_board[i][x] == 0){
			printf(" \u25A1 ");
		}
		else{
			printf(" \u25A0 ");
		}
	}
	else{
		if(tablero -> array_first[i][x] == 0){
			printf(" \u25A1 ");
		}
		else{
			printf(" \u25A0 ");
		}
	}
}

void printfinalprocess(Boards* todos_tableros, Procesos* todos_procesos){
	for(int i = 0; i < todos_procesos -> length; i++){
		printf("%s ", todos_procesos -> oneprocess[i] -> name);
		printf("Termino por %s. ",todos_procesos -> oneprocess[i] -> status);
		printf("Tiempo de simulación: %d. ", todos_procesos -> oneprocess[i] -> tiempo);
		printf("%d Células\n", countcells(todos_tableros -> oneboard[i]));
		printf("Tablero inicio\n");
		printboard(todos_tableros -> oneboard[i],1);
		printf("Tablero final\n");
		printboard(todos_tableros -> oneboard[i],0);
	}
}

int countcells(Board* tablero){
	int num_d = tablero -> n_d;
	int count = 0;
	for(int i = 0; i < num_d; i++){
		for(int x = 0; x < num_d; x++){
			if(tablero -> array_board[i][x] == 1){
				count++;
			}
		}
	}
	return count;
}

void writeoutput(Procesos* todos_procesos, Boards* todos_tableros){
	FILE *fp;
	fp = fopen("outcsv.csv", "w+");
	for(int i = 0; i < todos_procesos -> length; i++){
		fprintf(fp,"%s,", todos_procesos -> oneprocess[i] -> name);
		fprintf(fp,"%d,", todos_procesos -> oneprocess[i] -> tiempo);
		fprintf(fp,"%d,", countcells(todos_tableros -> oneboard[i]));
		fprintf(fp,"%s\n",todos_procesos -> oneprocess[i] -> status);
	}
	fclose(fp);
}

int readlines(char * filename, Procesos* todos_procesos, Boards* todos_tableros){
	FILE *fp = fopen(filename,"r");
	if(!fp){
		printf("Not file in directory.\n");
		return 1;
	}
	int num_tables;	int num_a; int num_b;	int num_c;
	int num_d; char name[256];
	int num_cel; int cel_x; int cel_y;
	fscanf(fp,"%i", &num_tables);
	fscanf(fp,"%i", &num_a); fscanf(fp,"%i", &num_b);
	fscanf(fp,"%i", &num_c); fscanf(fp,"%i", &num_d);
	todos_tableros -> length = num_tables;
	todos_tableros -> oneboard = malloc(sizeof(Board*)*(todos_tableros -> length));
	for (int i = 0; i < num_tables;i++){
		Board* nuevo_tablero = malloc(sizeof(Board));
		nuevo_tablero -> n_a = num_a;	nuevo_tablero -> n_b = num_b;
		nuevo_tablero -> n_c = num_c;	nuevo_tablero -> n_d = num_d;
		initialice_mem(nuevo_tablero);
		todos_tableros -> oneboard[i] = nuevo_tablero;
	}
	todos_procesos -> length = num_tables;
	todos_procesos -> oneprocess = malloc(sizeof(Process*)*(todos_procesos -> length));
	for(int i = 0; i < num_tables; i ++){
		Process* nuevo_proceso = malloc(sizeof(Process));
		fscanf(fp,"%s", name);
		strcpy(nuevo_proceso -> name, name);
		fscanf(fp,"%i", &num_cel);
		nuevo_proceso -> array_rafagas = malloc(num_cel*sizeof(int*));
		for (int x = 0; x < num_cel; x++){
			nuevo_proceso -> array_rafagas[x] = malloc(2*sizeof(int));
			fscanf(fp,"%i", &cel_x);
			fscanf(fp,"%i", &cel_y);
			nuevo_proceso -> array_rafagas[x][0] = cel_x;
			nuevo_proceso -> array_rafagas[x][1] = cel_y;
		}
		nuevo_proceso -> length = num_cel;
		todos_procesos -> oneprocess[i] = nuevo_proceso;
	}
	fclose(fp);
	return 2;
}
void initialice_mem(Board* tablero){
	int num_d = tablero -> n_d;
	tablero -> array_board = malloc(num_d*sizeof(int*));
	tablero -> array_around = malloc(num_d*sizeof(int*));
	tablero -> array_first = malloc(num_d*sizeof(int*));
	tablero -> array_past = malloc(4*sizeof(int**));
	for(int z = 0; z < 4; z++){
		tablero -> array_past[z] = malloc(num_d*sizeof(int*));
		for(int i = 0; i < num_d; i++){
			tablero -> array_past[z][i] = malloc(num_d*sizeof(int));
			for(int x = 0; x < num_d; x++){
				tablero -> array_past[z][i][x] = 0;
			}
		}
	}
	for(int i = 0; i < num_d; i++){
		tablero -> array_first[i] = malloc(num_d*sizeof(int));
		tablero -> array_board[i] = malloc(num_d*sizeof(int));
		tablero -> array_around[i] = malloc(num_d*sizeof(int));
		for(int x = 0; x < num_d; x++){
			tablero -> array_first[i][x] = 0;
			tablero -> array_board[i][x] = 0;
			tablero -> array_around[i][x] = 0;
		}
	}
}

void free_mem(Board* tablero){
	int num_d = tablero -> n_d;
	for(int z = 0; z < 4; z++){
		for(int i = 0; i < num_d; i++){
			free(tablero -> array_past[z][i]);
		}
		free(tablero -> array_past[z]);
	}
	for(int i = 0; i < num_d; i++){
		free(tablero -> array_first[i]);
		free(tablero -> array_board[i]);
		free(tablero -> array_around[i]);
	}
	free(tablero -> array_first);
	free(tablero -> array_board);
	free(tablero -> array_around);
	free(tablero -> array_past);
}

void free_mem_rafagas(Process* proceso){
	int num_d = proceso -> length;
	for(int i = 0; i < num_d; i++){
		free(proceso -> array_rafagas[i]);
	}
	free(proceso -> array_rafagas);
}


int simulation(Procesos* todos_procesos, Boards* todos_tableros, int timer){
	if(!withsubprocess){
		// ----------------------- With subprocess ------------------------------------
		number_process = todos_tableros -> length - 1;
		pid_t childpid;
		int status;

		// now create new process
		int flag = 0;
		for(int i = 0 ; i < todos_tableros -> length; i++){
			if (flag == 0){
				childpid = vfork();
				if(childpid >= 0){
					if(childpid != 0){
						flag = 1;
					}
				}
			}
		}

		if (childpid >= 0){
			if (childpid == 0){
				un_process_loop(todos_procesos,todos_tableros,timer, number_process);
				number_process --;
				_exit(0); // child exits with user-provided return code
			}
			else{ // fork() returns new pid to the parent process
				if(number_process < 0){
					wait(&status);
				}
				else{
					wait(&status); // wait for child to exit, and store its status
					un_process_loop(todos_procesos,todos_tableros,timer, number_process);
					number_process --;
					_exit(0);  // parent exits
				}
			}
		}
		else{ // fork returns -1 on failure
			perror("fork"); // display error message
			exit(0);
		}
		// ----------------------- End subprocess --------------------------------
	}
	else{
		// ------------------------ Without subprocess --------------------------------------------------

		number_process = 0;
		for(int i = 0; i < todos_tableros -> length; i++){
			un_process_loop(todos_procesos,todos_tableros,timer, i);
			number_process ++;
		}
	}
	return 0;
}

void un_process_loop(Procesos* todos_procesos, Boards* todos_tableros, int timer, int i){
	loadprocess(todos_tableros -> oneboard[i], todos_procesos -> oneprocess[i]);
/*	printf("Start Board\n");
	printf("name: %s\n", todos_procesos -> oneprocess[i] -> name);
	printboard(todos_tableros -> oneboard[i], 0);
*/count = 0;
	out_empty = 0;
	out_loop = 0;
	while(count < timer && !out_loop && !out_empty){
		reviewaround(todos_tableros -> oneboard[i]);
		lifeordeath(todos_tableros -> oneboard[i]);
		out_empty = checkempty(todos_tableros -> oneboard[i]);
		out_loop = checkloop(todos_tableros -> oneboard[i]);
		reorderpast(todos_tableros -> oneboard[i]);
		count ++;
	}
	statesave(todos_procesos,out_empty,out_loop,count);
}

void loadprocess(Board* tablero, Process* un_proceso){
	for(int i = 0; i < un_proceso -> length; i ++){
		int x = un_proceso -> array_rafagas[i][1];
		int y = un_proceso -> array_rafagas[i][0];
		tablero -> array_board[x][y] = 1;
		tablero -> array_first[x][y] = 1;
	}
}
// Check surroundings
void reviewaround(Board* tablero){
	int num_d = tablero -> n_d;
	for (int i = 0; i < num_d; i ++){
		for(int x = 0; x < num_d; x ++){
			int lado = 0;
			int suma = 0;
			if(!x){  // Rigth
				lado = 1;
			}
			else if(x == num_d-1){ // Left
				lado = -1;
			}
			if (!i){
				suma += tablero -> array_board[i+1][x];
				if (lado != 0){
					suma += tablero -> array_board[i][x + lado];
					suma += tablero -> array_board[i+1][x + lado];
				}
				else{ // abajo, izq y derecha
					suma += sum_center(tablero, i, x);
					suma += sum_down(tablero, i, x);
				}
			}
			else if(i == num_d-1){
				suma += tablero -> array_board[i-1][x];
				if (lado != 0){ //arriba y lados
					suma += tablero -> array_board[i-1][x + lado];
					suma += tablero -> array_board[i][x + lado];
				}
				else{ // arriba
					suma += sum_up(tablero, i, x);
					suma += sum_center(tablero, i, x);
				}
			}
			else{
				suma += tablero -> array_board[i-1][x];
				suma += tablero -> array_board[i+1][x];
				if(lado){ //arriba y abajo y lados
					suma += tablero -> array_board[i-1][x + lado];
					suma += tablero -> array_board[i][x + lado];
					suma += tablero -> array_board[i+1][x + lado];
				}
				else{ //Todos los lados
					suma += sum_up(tablero, i, x);
					suma += sum_center(tablero, i, x);
					suma += sum_down(tablero, i, x);
				}
			}
			tablero -> array_around[i][x] = suma;
		}
	}
}

int sum_down(Board* tablero, int i, int x){
	int suma = 0;
	suma += tablero -> array_board[i+1][x + 1];
	suma += tablero -> array_board[i+1][x -1];
	return suma;
}

int sum_center(Board* tablero, int i, int x){
	int suma = 0;
	suma += tablero -> array_board[i][x + 1];
	suma += tablero -> array_board[i][x - 1];
	return suma;
}

int sum_up(Board* tablero, int i, int x){
	int suma = 0;
	suma += tablero -> array_board[i-1][x + 1];
	suma += tablero -> array_board[i-1][x -1];
	return suma;
}

void lifeordeath(Board* tablero){
	int num_a = tablero -> n_a;
	int num_b = tablero -> n_b;
	int num_c = tablero -> n_c;
	int num_d = tablero -> n_d;
	for (int i = 0; i < num_d; i ++){
		for(int x = 0; x < num_d; x ++){
			int num =	tablero -> array_around[i][x];
			if(tablero -> array_board[i][x] == 1){
				if (num_b > num || num > num_c){
					tablero -> array_board[i][x] = 0;
				}
			}
			else if(num == num_a){
					tablero -> array_board[i][x] = 1;
			}
		}
	}
}

int checkloop(Board* tablero){
	int num_d = tablero -> n_d;
	int loops = 0; // loops in 1, board in loop
	for(int z = 0; z < 4; z++){
		int loop = 0; // if loop = 0, board in loop
		for(int i = 0; i < num_d; i++){
			for(int x = 0; x < num_d; x++){
				int board = tablero -> array_board[i][x];
				int past = tablero -> array_past[z][i][x];
				if( board != past){
					loop = 1;
				}
			}
		}
		if(!loop){
			loops = 1;
		}
	}
	return loops;
}

int checkempty(Board* tablero){
	int num_d = tablero -> n_d;
	int empty = 1;
	for(int i = 0; i < num_d; i++){
		for(int x = 0; x < num_d; x++){
			int board = tablero -> array_board[i][x];
			if(board){
				empty = 0;
			}
		}
	}
	return empty;
}

void reorderpast(Board* tablero){
	int num_d = tablero -> n_d;
	for(int z = 0; z < 3; z++){
		for(int i = 0; i < num_d; i++){
			for(int x = 0; x < num_d; x++){
				tablero -> array_past[z][i][x] = tablero -> array_past[z + 1][i][x];
			}
		}
	}
	for(int i = 0; i < num_d; i++){
		for(int x = 0; x < num_d; x++){
			tablero -> array_past[3][i][x] = tablero -> array_board[i][x];
		}
	}
}

void statesave(Procesos* todos_procesos, int out_empty,int out_loop, int count){
	char loops[20] = "LOOP";
	char nocells[20] = "NOCELLS";
	char notime[20] = "NOTIME";
	if(out_empty){
		strcpy(todos_procesos -> oneprocess[number_process] -> status, nocells);
	}
	else if(out_loop){
		strcpy(todos_procesos -> oneprocess[number_process] -> status, loops);
	}
	if(count == timer){
		strcpy(todos_procesos -> oneprocess[number_process] -> status, notime);
	}
	todos_procesos -> oneprocess[number_process] -> tiempo = count -1;
}

void  INThandler(int sig){
	char end_signal[20] = "SIGNAL";
	signal(sig, SIG_IGN);
	if(!withsubprocess){
		if(number_process < 0){
			printfinalprocess(todos_tableros ,todos_procesos);
			writeoutput(todos_procesos, todos_tableros);
			free_all_mem(todos_tableros,todos_procesos);
			exit(0);
		}
		else{
			statesave(todos_procesos, out_empty, out_loop, count);
			strcpy(todos_procesos -> oneprocess[number_process] -> status, end_signal);
			count = 0;
			number_process--;
			exit(0);
		}
	}
	else{
		printf("aquiiiiiiii n_p: %d n_t: %d \n", number_process,number_process - todos_tableros -> length);
		if(number_process - todos_tableros -> length < 0){
			int number = todos_tableros -> length - number_process;
			int inicio = number_process;
			for(int i = inicio; i < number; i++){
				statesave(todos_procesos, out_empty, out_loop, count);
				strcpy(todos_procesos -> oneprocess[i] -> status, end_signal);
				count = 1;
				number_process ++;
			}
			printf("aquiiiiiiii %d \n", todos_tableros -> length - number_process);
			printfinalprocess(todos_tableros ,todos_procesos);
			writeoutput(todos_procesos, todos_tableros);
			free_all_mem(todos_tableros,todos_procesos);
			exit(0);
		}
	}
}
