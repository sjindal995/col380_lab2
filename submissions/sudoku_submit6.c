/*2013CS10253_2013CS10245*/
#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sudoku.h"

#define ll long long

int** global_output;
//ith bit is set, means (i+1) is a possible value at that position

extern int thread_count;
struct possibleVals{
	int* vals;
	int size;
};

struct stack{
	int*** list;
	int size;
	int capacity;
};

void initStack(struct stack* s){
	s->size = 0;
	s->capacity = 1;
	s->list = malloc(s->capacity*sizeof(int**));
}

void push(struct stack* s, int** val){
	if(s->size < s->capacity){
		s->list[s->size] = val;
		s->size++;
	}
	else{
		int*** new_list = malloc(2*s->capacity*sizeof(int**));
		int i;
		for(i = 0; i<s->size; i++){
			new_list[i] = s->list[i];
		}
		s->list = new_list;
		s->list[s->size] = val;
		s->size++;
		s->capacity *= 2;
	}
}

int isEmptyStack(struct stack* s){
	return (s->size == 0);
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


int** pop(struct stack* s){
	if(isEmptyStack(s)) return NULL;
	else{
		s->size--;
		return s->list[s->size];
	}
}

int** top(struct stack* s){
	if(isEmptyStack(s)) return NULL;
	return s->list[s->size - 1];
}

struct queue{
	int*** list;
	int start;
	int size;
	int capacity;
};

void initQueue(struct queue* q){
	q->start = 0;
	q->size = 0;
	q->capacity = 1;
	q->list = malloc(q->capacity*sizeof(int**));
}

void pushQueue(struct queue* q, int** val){
	if(q->size < q->capacity){
		q->list[(q->start + q->size)%q->capacity] = val;
		q->size++;
	}
	else{
		int*** new_list = malloc(2*q->capacity*sizeof(int**));
		int i;
		for(i = 0; i<q->size; i++){
			new_list[i] = q->list[(i + q->start)%q->capacity];
		}
		q->start = 0;
		q->list = new_list;
		q->capacity *= 2;
		q->list[(q->start + q->size)%q->capacity] = val;
		q->size++;
	}
}

int isEmptyQueue(struct queue* q){
	return (q->size == 0);
}

int** popQueue(struct queue* q){
	if(isEmptyQueue(q)){
		return NULL;
	}
	int** output = q->list[q->start];
	q->start = (q->start + 1)%q->capacity;
	q->size--;
	return output;
}

int** frontQueue(struct queue* q){
	if(isEmptyQueue(q)) return NULL;
	return (q->list[q->start]);	
}

ll getPossibleValues(int** input, int row, int column){
	int i;
	ll res = 0;
	for(i = 0; i < SIZE;i++){
		res |= (1<<i);
	}
	for(i = 0; i < SIZE; i++){
		if(input[row][i] != 0){
			res &= ~(1<<(input[row][i]-1));
		}
		if(input[i][column] != 0){
			res &= ~(1<<(input[i][column] - 1));
		}
	}
	int j;
	int mini_row = (row/MINIGRIDSIZE)*MINIGRIDSIZE;
	int mini_column = (column/MINIGRIDSIZE)*MINIGRIDSIZE;
	for(i = 0; i < MINIGRIDSIZE; i++){
		for(j = 0; j < MINIGRIDSIZE; j++){
			if(input[mini_row + i][mini_column + j] != 0){
				res &= ~(1<<(input[mini_row+i][mini_column+j] - 1));
			}
		}
	}
	return res;
}

ll** getPossibleGrid(int** input){
	int r_num, c_num, i;
	ll** output = malloc(sizeof(ll*)*SIZE);
	ll pv=0;
	for(r_num = 0; r_num < SIZE; r_num++){
		output[r_num] = malloc(SIZE*sizeof(ll));
		for(c_num = 0; c_num < SIZE; c_num++){
			pv=0;
			if(input[r_num][c_num]  == 0){
				pv = getPossibleValues(input, r_num, c_num);
			}
			else{
				pv = (1<<(input[r_num][c_num] - 1));
			}
			output[r_num][c_num] = pv;
		}
	}
	return output;
}

void printGrid(int** outputGrid){
	int i,j;
	for (i=0;i<SIZE;i++){
		for (j=0;j<SIZE;j++)
			printf("%d ",outputGrid[i][j]);
		printf("\n");
	}
}

void printPossibleGrid(ll** possible_vals){
	int i,j,k;
	for (i=0;i<SIZE;i++){
		for (j=0;j<SIZE;j++){
			printf(":: [");
			for(k = 0; k<SIZE; k++){
				if(possible_vals[i][j]&(1<<k))
					printf("%d ",k+1);
			}
			printf("] ");
		}
		printf("\n");
	}
}


int updatePossibleGrid(int** input, int r_num, int c_num, ll** possibleGrid){
	int i,j;
	for(i = 0; i<SIZE; i++){
		if(i != c_num){
			possibleGrid[r_num][i] &= ~(1<<(input[r_num][c_num]-1));  
			if(possibleGrid[r_num][i] == 0){
				return -1;
			}
		}
		if(i != r_num){
			possibleGrid[i][c_num] &= ~(1<<(input[r_num][c_num] - 1));
			if(possibleGrid[i][c_num] == 0){
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
				possibleGrid[i][j] &= ~(1<<(input[r_num][c_num] - 1));
				if(possibleGrid[i][j] == 0){
					return -1;
				}
			}
		}
	}
	return 0;
}

int elimination(int** input){
	int r_num, c_num, r;
	int changed = 1;
	while(changed){
		changed = 0;
		for(r_num = 0; r_num < SIZE; r_num++){
			for(c_num = 0; c_num < SIZE; c_num++){
				if(input[r_num][c_num] == 0){
					// ll possible_vals = possibleGrid[r_num][c_num];
					ll possible_vals = getPossibleValues(input, r_num, c_num);
					if(possible_vals == 0){
						return -1;
					}
					int p2 = isPower2(possible_vals);
					if(p2>0){
						changed = 1;
						input[r_num][c_num] = p2;
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


int isComplete(int** input){
	int r_num, c_num;
	for(r_num = 0; r_num < SIZE; r_num++){
		for(c_num = 0; c_num < SIZE; c_num++){
			if(input[r_num][c_num] == 0) return 0;
		}
	}
	return 1;
}

void freePossibleGrid(ll** grid){
	int r_num, c_num, i;
	for(r_num = 0; r_num < SIZE; r_num++){
		free(grid[r_num]);
	}
	free(grid);
}

void freeGrid(int** grid){
	int r_num, c_num, i;
	for(r_num = 0; r_num < SIZE; r_num++){
		free(grid[r_num]);
	}
	free(grid);
}

int loneRanger(int** input){
	int r, c,i, j, mini_row, mini_column, k, noset, set,changed=1;
	int res;
	ll pv1,pv2;
	int *bool_vals;
	// ll** possibleGrid = getPossibleGrid(input);
	while(changed){
		changed=0;
		for(r=0; r<SIZE; r++){
			for(c=0; c<SIZE; c++){
				if(input[r][c]==0){
					// pv1 = possibleGrid[r][c];
					pv2 = getPossibleValues(input, r, c);
					res = isPower2(pv1);
					if(pv1==0){
						return -1;
					}
					else if (res > 0){
						input[r][c]=res;
						changed=1;
						// if(updatePossibleGrid(input,r,c,possibleGrid)<0){
						// 	return -1;
						// }
					}
					else{
						bool_vals = calloc(SIZE,sizeof(int));
						for(i = 0; i<SIZE; i++){
							if(pv1&(1<<i)) bool_vals[i]=1;
						}
						for(i=0;i<SIZE;i++){
							//row
							if(i!=c){
								if(input[r][i]==0){
									// pv2 = possibleGrid[r][i];
									pv2 = getPossibleValues(input, r, i);
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
								if(input[i][c]==0){
									// pv2 = possibleGrid[i][c];
									pv2 = getPossibleValues(input, i, c);
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
									if(input[mini_row+i][mini_column+j]==0){
										// pv2 = possibleGrid[mini_row+i][mini_column+j];
										pv2 = getPossibleValues(input, mini_row+i, mini_column+j);
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
							input[r][c] = res_dummy;
							// if(updatePossibleGrid(input,r,c,possibleGrid)<0){
							// 	return -1;
							// }
						}
						free(bool_vals);
					}
				}
			}
		}
	}
	return 0;
}


int** solveSudokuRec(int** input){
	if(isComplete(input)) return input;
	int r_num;
	int c_num;
	int r;
	int** input1 = malloc(SIZE*sizeof(int*));
	for(r_num = 0; r_num < SIZE; r_num++){
		input1[r_num] = malloc(SIZE*sizeof(int));
		for(c_num = 0; c_num < SIZE; c_num++){
			input1[r_num][c_num] = input[r_num][c_num];
		}
	}
	// r = loneRanger(input1);
	r = elimination(input1);
	if(r < 0){
		freeGrid(input1);
		return input;
	}
	else{
		if(isComplete(input1)){
			return input1;
		}
	}
	for(r_num = 0; r_num < SIZE; r_num++){
		for(c_num = 0; c_num < SIZE; c_num++){
			if(input1[r_num][c_num] == 0){
				ll possible_vals = getPossibleValues(input1, r_num, c_num);
				int i;
				int** output;
				for(i=0; i<SIZE; i++){
					if(possible_vals&(1<<i)){
						input1[r_num][c_num] = i+1;
						if(global_output){
							return input;
						}
						output = solveSudokuRec(input1);
						if(isValid(input, output)){
							return output;
						}
						else{
							input1[r_num][c_num] = 0;
						}
					}
				}
				freeGrid(input1);
				return input;
			}
		}
	}
	freeGrid(input1);
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

int** makeCopy(int** src){
	int r_num, c_num;
	int** dst = malloc(SIZE*sizeof(int*));
	for(r_num = 0; r_num < SIZE; r_num++){
		dst[r_num] = malloc(SIZE*sizeof(int));
		for(c_num = 0; c_num < SIZE; c_num++){
			dst[r_num][c_num] = src[r_num][c_num];
		}
	}
	return dst;
}

int** solveSudoku(int** input){
	struct queue* q = malloc(sizeof(struct queue));
	initQueue(q);
	pushQueue(q,makeCopy(input));
	int r_num, c_num, i;
	if(SIZE <= 9) thread_count = 1;
	else if((SIZE <= 16) && (thread_count >= 2)) thread_count = 2;
	else if((SIZE > 16) && (thread_count >= 4)) thread_count = 4;
	// if(thread_count > 4) thread_count = 4;
	// thread_count = 1;
	while(q->size < thread_count && !isEmptyQueue(q)){
		int** curr = popQueue(q);
		int break1=0;
		for(r_num = 0; r_num < SIZE; r_num++){
			for(c_num = 0; c_num < SIZE; c_num++){
				if(curr[r_num][c_num] == 0){
					ll possible_vals = getPossibleValues(curr, r_num, c_num);
					for(i = 0; i < SIZE; i++){
						if(possible_vals&(1<<i)){
							int** curr_child = makeCopy(curr);
							curr_child[r_num][c_num] = i+1;
							pushQueue(q, curr_child);
						}
					}
					break1 = 1;
					break;
				}
			}
			if(break1) break;
		}
	}
	omp_set_num_threads(thread_count);
	#pragma omp parallel for 
		for(i = 0; i < q->size; i++){
			int** temp;
			int found=0;
			#pragma omp critical
			{
				if(global_output){
					found=1;
				}
			}
			if(!found){
				temp = solveSudokuRec(q->list[(i + q->start)%q->capacity]);
				#pragma omp critical
					if(isValid(input, temp)) global_output = temp;
			}
		}
	if(!global_output) global_output = input;
	return global_output;
}
