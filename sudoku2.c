/*2013CS10253_2013CS10245*/
#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sudoku.h"

#define ll long long

int found_correct=0;
 
typedef int G[SIZE][SIZE];

typedef struct Grid{
	G g;
} grid;



typedef ll PG[SIZE][SIZE];

typedef struct Pgrid{
	PG pg;
} pgrid;

ll offset;
//ith bit is set, means (i+1) is a possible value at that position

extern int thread_count;
// struct possibleVals{
// 	int* vals;
// 	int size;
// };

// struct stack{
// 	int*** list;
// 	int size;
// 	int capacity;
// };

// void initStack(struct stack* s){
// 	s->size = 0;
// 	s->capacity = 1;
// 	s->list = malloc(s->capacity*sizeof(int**));
// }

// void push(struct stack* s, int** val){
// 	if(s->size < s->capacity){
// 		s->list[s->size] = val;
// 		s->size++;
// 	}
// 	else{
// 		int*** new_list = malloc(2*s->capacity*sizeof(int**));
// 		int i;
// 		for(i = 0; i<s->size; i++){
// 			new_list[i] = s->list[i];
// 		}
// 		s->list = new_list;
// 		s->list[s->size] = val;
// 		s->size++;
// 		s->capacity *= 2;
// 	}
// }

// int isEmptyStack(struct stack* s){
// 	return (s->size == 0);
// }




// int** pop(struct stack* s){
// 	if(isEmptyStack(s)) return NULL;
// 	else{
// 		s->size--;
// 		return s->list[s->size];
// 	}
// }

// int** top(struct stack* s){
// 	if(isEmptyStack(s)) return NULL;
// 	return s->list[s->size - 1];
// }

int isValid2(grid* original, grid* solution){
	int valuesSeen[SIZE],i,j,k;

	//check all rows
	for (i=0;i<SIZE;i++){
		for (k=0;k<SIZE;k++) valuesSeen[k] = 0;
		for (j=0;j<SIZE;j++){
			if (solution->g[i][j]==0) return 0;
			if ((original->g[i][j])&&(solution->g[i][j] != original->g[i][j])) return 0;
			int v = solution->g[i][j];
			if (valuesSeen[v-1]==1){
				return 0;
			}
			valuesSeen[v-1] = 1;
		}
	}

	//check all columns
	for (i=0;i<SIZE;i++){
		for (k=0;k<SIZE;k++) valuesSeen[k] = 0;
		for (j=0;j<SIZE;j++){
			int v = solution->g[j][i];
			if (valuesSeen[v-1]==1){
				return 0;
			}
			valuesSeen[v-1] = 1;
		}
	}

	//check all minigrids
	//check all rows
	for (i=0;i<SIZE;i=i+MINIGRIDSIZE){
		for (j=0;j<SIZE;j=j+MINIGRIDSIZE){
			for (k=0;k<SIZE;k++) valuesSeen[k] = 0;
			int r,c;
			for (r=i;r<i+MINIGRIDSIZE;r++)
				for (c=j;c<j+MINIGRIDSIZE;c++){
					int v = solution->g[r][c];
					if (valuesSeen[v-1]==1) {
						return 0;
					}
					valuesSeen[v-1] = 1;
				}
		}
	}
	#pragma omp critical
		found_correct=1;
	return 1;
}


int isPower2 (ll inp){
	if(inp==0) return -2;
	else if (inp == 1) return 1;
	else if ( !(inp&(inp-1))){
		int j=0;
		while(inp!=1){
			inp /=2;
			j++;
		}
		return j+1;
	}
	else return -1;
}

struct queue{
	// grid* list;
	grid* list;
	int start;
	int size;
	int capacity;
};

void initQueue(struct queue* q){
	q->start = 0;
	q->size = 0;
	q->capacity = 5;
	q->list = (grid*)malloc(q->capacity*sizeof(grid));
}

void pushQueue(struct queue* q, grid* val){
	if(q->size < q->capacity){
		q->list[(q->start + q->size)%q->capacity] = *val;
		q->size++;
	}
	else{
		// int*** new_list = malloc(2*q->capacity*sizeof(int**));
		grid* new_list = malloc(2*q->capacity*sizeof(grid));
		int i;
		for(i = 0; i<q->size; i++){
			new_list[i] = q->list[(i + q->start)%q->capacity];
		}
		q->start = 0;
		q->list = new_list;
		q->capacity *= 2;
		q->list[(q->start + q->size)%q->capacity] = *val;
		q->size++;
	}
}

int isEmptyQueue(struct queue* q){
	return (q->size == 0);
}

grid popQueue(struct queue* q){
	if(isEmptyQueue(q)){
		printf("pop from empty queue\n");
		exit(1);	
		// return NULL;
	}

	grid output = q->list[q->start];
	q->start = (q->start + 1)%q->capacity;
	q->size--;
	return output;
}

grid frontQueue(struct queue* q){
	// if(isEmptyQueue(q)) return NULL;
	return (q->list[q->start]);	
}

ll getPossibleValues(grid* input, int row, int column){
	int i;
	ll res = 0;
	for(i = 0; i < SIZE;i++){
		res |= ((offset)<<i);
	}
	for(i = 0; i < SIZE; i++){
		if(input->g[row][i] != 0){
			res &= ~((offset)<<(input->g[row][i]-1));
		}
		if(input->g[i][column] != 0){
			res &= ~((offset)<<(input->g[i][column] - 1));
		}
	}
	int j;
	int mini_row = (row/MINIGRIDSIZE)*MINIGRIDSIZE;
	int mini_column = (column/MINIGRIDSIZE)*MINIGRIDSIZE;
	for(i = 0; i < MINIGRIDSIZE; i++){
		for(j = 0; j < MINIGRIDSIZE; j++){
			if(input->g[mini_row + i][mini_column + j] != 0){
				res &= ~((offset)<<(input->g[mini_row+i][mini_column+j] - 1));
			}
		}
	}
	return res;
}

pgrid getPossibleGrid(grid* input){
	int r_num, c_num, i;
	// ll** output = malloc(sizeof(ll*)*SIZE);
	pgrid output;
	ll pv=0;
	for(r_num = 0; r_num < SIZE; r_num++){
		// output[r_num] = malloc(SIZE*sizeof(ll));
		for(c_num = 0; c_num < SIZE; c_num++){
			pv=0;
			if(input->g[r_num][c_num]  == 0){
				pv = getPossibleValues(input, r_num, c_num);
			}
			else{
				pv |= ((offset)<<(input->g[r_num][c_num] - 1));
			}
			output.pg[r_num][c_num] = pv;
		}
	}
	return output;
}

void printGrid(grid* outputGrid){
	int i,j;
	for (i=0;i<SIZE;i++){
		for (j=0;j<SIZE;j++)
			printf("%d ",outputGrid->g[i][j]);
		printf("\n");
	}
}

void printPossibleGrid(pgrid* possible_vals){
	int i,j,k;
	for (i=0;i<SIZE;i++){
		for (j=0;j<SIZE;j++){
			printf(":: [");
			for(k = 0; k<SIZE; k++){
				if(possible_vals->pg[i][j]&((offset)<<k))
					printf("%d ",k+1);
			}
			printf("] ");
		}
		printf("\n");
	}
}


int updatePossibleGrid(grid* input, int r_num, int c_num, pgrid* possibleGrid){
	int i,j;
	for(i = 0; i<SIZE; i++){
		if(i != c_num){
			possibleGrid->pg[r_num][i] &= ~((offset)<<(input->g[r_num][c_num]-1));  
			if(possibleGrid->pg[r_num][i] == 0){
				return -1;
			}
		}
		if(i != r_num){
			possibleGrid->pg[i][c_num] &= ~((offset)<<(input->g[r_num][c_num] - 1));
			if(possibleGrid->pg[i][c_num] == 0){
				return -1;
			}
		}
	}
	int mini_row = (r_num/MINIGRIDSIZE)*MINIGRIDSIZE;
	int mini_column = (c_num/MINIGRIDSIZE)*MINIGRIDSIZE;
	int k;
	for(i = mini_row; i < MINIGRIDSIZE+mini_row; i++){
		for(j = mini_column; j < MINIGRIDSIZE+mini_column; j++){
			int up_size0 = 0;
			if((i != r_num) && (j != c_num)){
				possibleGrid->pg[i][j] &= ~((offset)<<(input->g[r_num][c_num] - 1));
				if(possibleGrid->pg[i][j] == 0){
					return -1;
				}
			}
		}
	}
	return 0;
}

// int elimination(grid* input, pgrid* possibleGrid){
int elimination(grid* input){
	int r_num, c_num, r;
	int changed = 1;
	while(changed){
		changed = 0;
		for(r_num = 0; r_num < SIZE; r_num++){
			for(c_num = 0; c_num < SIZE; c_num++){
				if(input->g[r_num][c_num] == 0){
					// ll possible_vals = possibleGrid->pg[r_num][c_num];
					ll possible_vals = getPossibleValues(input,r_num,c_num);
					if(possible_vals == 0){
						return -1;
					}
					int p2 = isPower2(possible_vals);
					if(p2>0){
						changed = 1;
						input->g[r_num][c_num] = p2;
						// if(updatePossibleGrid(input,r_num,c_num,possibleGrid)<0){
						// 	return -1;
						// }
					}
				}
			}
		}
	}
	return 0;
}


int isComplete(grid* input){
	int r_num, c_num;
	for(r_num = 0; r_num < SIZE; r_num++){
		for(c_num = 0; c_num < SIZE; c_num++){
			if(input->g[r_num][c_num] == 0) return 0;
		}
	}
	return 1;
}

// void freePossibleGrid(pgrid grid){
// 	int r_num, c_num, i;
// 	for(r_num = 0; r_num < SIZE; r_num++){
// 		free(grid[r_num]);
// 	}
// 	free(grid);
// }

// void freeGrid(int** grid){
// 	int r_num, c_num, i;
// 	for(r_num = 0; r_num < SIZE; r_num++){
// 		free(grid[r_num]);
// 	}
// 	free(grid);
// }

// int loneRanger(grid* input, pgrid* possibleGrid){
int loneRanger(grid* input){
	int r, c,i, j, mini_row, mini_column, k, noset, set,changed=1;
	int res;
	ll pv1,pv2;
	// int *bool_vals;
	pgrid possibleGrid = getPossibleGrid(input);
	// ll** possibleGrid = getPossibleGrid(input);
	while(changed){
		changed=0;
		for(r=0; r<SIZE; r++){
			for(c=0; c<SIZE; c++){
				if(input->g[r][c]==0){
					pv1 = possibleGrid.pg[r][c];
					// pv1 = getPossibleValues(input,r,c);
					res = isPower2(pv1);
					if(pv1==0){
						return -1;
					}
					else if (res > 0){
						input->g[r][c]=res;
						changed=1;
						if(updatePossibleGrid(input,r,c,&possibleGrid)<0){
							return -1;
						}
					}
					else{
						// bool_vals = calloc(SIZE,sizeof(int));
						// for(i = 0; i<SIZE; i++){
						// 	if(pv1&((offset)<<i)) bool_vals[i]=1;
						// }
						for(i=0;i<SIZE;i++){
							//row
							if(i!=c){
								if(input->g[r][i]==0){
									pv2 = possibleGrid.pg[r][i];
									// pv2 = getPossibleValues(input,r,i);
									if(pv2 == 0){
										return -1;
									}
									else{
										pv1 &= ~pv2;
									}
								}	
							}	
							//column
							if(i!=r){
								if(input->g[i][c]==0){
									pv2 = possibleGrid.pg[i][c];
									// pv2 = getPossibleValues(input,i,c);
									if(pv2 == 0){
										return -1;
									}
									else{
										pv1 &= ~pv2;
									}
								}
							}
						}
						mini_row = (r/MINIGRIDSIZE)*MINIGRIDSIZE;
						mini_column = (c/MINIGRIDSIZE)*MINIGRIDSIZE;
						for(i=0; i<MINIGRIDSIZE; i++){
							for(j=0; j<MINIGRIDSIZE; j++){
								if(i!=r && j!=c){
									if(input->g[mini_row+i][mini_column+j]==0){
										pv2 = possibleGrid.pg[mini_row+i][mini_column+j];
										// pv2 = getPossibleValues(input,mini_row+i,mini_column+j);
										if(pv2 == 0){
											return -1;
										}
										else{
											pv1 &= ~pv2;
										}
									}
								}
							}
						}
						int res_dummy = isPower2(pv1);
						if(res_dummy == -1){
							return -1;
						}
						else if(res_dummy > 0){
							input->g[r][c] = res_dummy;
							if(updatePossibleGrid(input,r,c,&possibleGrid)<0){
								return -1;
							}
						}
						// free(bool_vals);
					}
				}
			}
		}
	}
	return 0;
}


// grid solveSudokuRec(grid input, pgrid possibleGrid){
grid solveSudokuRec(grid input){
	if(isValid2(&input,&input)) return input;
	int r_num;
	int c_num;
	int r;
	// int** input1 = malloc(SIZE*sizeof(int*));
	grid input1=input;
	// for(r_num = 0; r_num < SIZE; r_num++){
	// 	// input1[r_num] = malloc(SIZE*sizeof(int));
	// 	for(c_num = 0; c_num < SIZE; c_num++){
	// 		input1.g[r_num][c_num] = input.g[r_num][c_num];
	// 	}
	// }
	// ll** possibleGrid1 = malloc(SIZE*sizeof(ll*));
	// pgrid possibleGrid1 = possibleGrid;
	// for(r_num = 0; r_num < SIZE; r_num++){
	// 	// possibleGrid1[r_num] = malloc(SIZE*sizeof(ll));
	// 	for(c_num = 0; c_num < SIZE; c_num++){
	// 		possibleGrid1.pg[r_num][c_num] = possibleGrid.pg[r_num][c_num];
	// 	}
	// }
	// r = loneRanger(input1, possibleGrid1);
	// r = elimination(&input1, &possibleGrid1);
	r = elimination(&input1);
	// r = loneRanger(&input1);
	int fc;
	if(r < 0){
		// freeGrid(input1);
		// freePossibleGrid(possibleGrid1);
		return input;
	}
	else{
		if(isValid2(&input1,&input1)){
			// freePossibleGrid(possibleGrid1);
			return input1;
		}
	}
	for(r_num = 0; r_num < SIZE; r_num++){
		for(c_num = 0; c_num < SIZE; c_num++){
			if(input1.g[r_num][c_num] == 0){
				// ll possible_vals = getPossibleValues(input1, r_num, c_num);
				// ll possible_vals = possibleGrid1.pg[r_num][c_num];
				ll possible_vals =getPossibleValues(&input1,r_num,c_num);
				int i;
				// int** output;
				grid output;
				for(i=0; i<SIZE; i++){
					#pragma omp critical
						fc = found_correct;
					if(fc){ 
						return input;
					}
					if(possible_vals&((offset)<<i)){
						input1.g[r_num][c_num] = i+1;
						// ll** possibleGrid2 = malloc(SIZE*sizeof(ll*));
						// pgrid possibleGrid2=possibleGrid1;
						// int r_num0, c_num0;
						// for(r_num0 = 0; r_num0 < SIZE; r_num0++){
						// 	// possibleGrid2[r_num0] = malloc(SIZE*sizeof(ll));
						// 	for(c_num0 = 0; c_num0 < SIZE; c_num0++){
						// 		possibleGrid2[r_num0][c_num0] = possibleGrid2[r_num0][c_num0];
						// 	}
						// }
						// if(updatePossibleGrid(&input1, r_num, c_num, &possibleGrid2) < 0){
						// 	// freePossibleGrid(possibleGrid1);
						// 	// freePossibleGrid(possibleGrid2);
						// 	return input;
						// }
						// output = solveSudokuRec(input1, possibleGrid2);
						output = solveSudokuRec(input1);
						// freePossibleGrid(possibleGrid2);
						if(isValid2(&input, &output)){
							// freePossibleGrid(possibleGrid1);
							return output;
						}
						else{
							input1.g[r_num][c_num] = 0;
						}
					}
				}
				// freePossibleGrid(possibleGrid1);
				// freeGrid(input1);
				return input;
			}
		}
	}
	// freePossibleGrid(possibleGrid1);
	// freeGrid(input1);
	return input;
}

// int** solveSudokuIt(int** input){
// 	struct stack st;
// 	initStack(st);
// 	push(st, input);
// 	while(!isEmptyStack(st)){
// 		int** curr = pop(st);
// 		int r_num, c_num;
// 		for(r_num = 0; r_num < SIZE; r_num++){
// 			for(c_num = 0; c_num < SIZE; c_num++){
				
// 			}
// 		}
// 	}
// }

// grid makeCopy(grid src){
// 	int r_num, c_num;
// 	// int** dst = malloc(SIZE*sizeof(int*));
// 	grid dst;
// 	for(r_num = 0; r_num < SIZE; r_num++){
// 		// dst[r_num] = malloc(SIZE*sizeof(int));
// 		for(c_num = 0; c_num < SIZE; c_num++){
// 			dst[r_num][c_num] = src[r_num][c_num];
// 		}
// 	}
// 	return dst;
// }

int** solveSudoku(int** inp){
	if(SIZE<32) offset=1;
	else offset=1LL;
	struct queue* q = (struct queue*)malloc(sizeof(struct queue));
	initQueue(q);
	grid input;
	int ii,jj;
	for(ii=0; ii<SIZE; ii++){
		for(jj=0;jj<SIZE; jj++){
			input.g[ii][jj]=inp[ii][jj];
		}
	}
	grid i2=input;
	// pushQueue(q,makeCopy(input));
	pushQueue(q,&i2);
	int r_num, c_num, i;
	if(thread_count > 4) thread_count = 4;
	// thread_count = 1;
	// while(q->size < thread_count*SIZE && !isEmptyQueue(q)){
	grid output;
	while(q->size < 7 && !isEmptyQueue(q)){
		// int** curr = popQueue(q);
		grid curr = popQueue(q);
		int break1=0;
		for(r_num = 0; r_num < SIZE; r_num++){
			for(c_num = 0; c_num < SIZE; c_num++){
				if(curr.g[r_num][c_num] == 0){
					ll possible_vals = getPossibleValues(&curr, r_num, c_num);
					for(i = 0; i < SIZE; i++){
						if(possible_vals&((1LL)<<i)){
							// int** curr_child = makeCopy(curr);
							grid curr_child = curr;
							curr_child.g[r_num][c_num] = i+1;
							pushQueue(q, &curr_child);
						}
					}
					break1 = 1;
					break;
				}
			}
			if(break1) break;
		}
		if(!break1 && isValid2(&input,&curr)){
			output= curr;
			q->size=0;
			break;
		}
	}
	// int** output;
	omp_set_num_threads(thread_count);
	#pragma omp parallel for 
		for(i = 0; i < q->size; i++){
			grid temp;
			int fc;
			#pragma omp critical
			{
				 fc = found_correct;
			}
			if(!fc){
				// pgrid possibleGrid = getPossibleGrid(&(q->list[(i + q->start)%q->capacity]));
				temp = solveSudokuRec(q->list[(i + q->start)%q->capacity]);
				#pragma omp critical
					if(isValid2(&input, &temp)) {
						output = temp;
						// found_correct=1;
					}
			}
		}
	if(!found_correct) output = input;
	int** final = malloc(SIZE*sizeof(int*));
	for(ii=0; ii<SIZE; ii++){
		final[ii] = malloc(SIZE*sizeof(int));
		for(jj=0;jj<SIZE;jj++){
			final[ii][jj]= output.g[ii][jj];
		}
	}
	return final;
}
