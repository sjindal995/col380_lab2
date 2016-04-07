/*2013CS10253_2013CS10245*/
#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sudoku.h"

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

struct possibleVals getPossibleValues(int** input, int row, int column){
	int i;
	int* bool_vals = malloc(SIZE*sizeof(int));
	// memset(possible_vals, 1, SIZE*sizeof(int));
	for(i = 0; i < SIZE;i++){
		bool_vals[i] = 1;
	}
	// printf("discarded: ");
	for(i = 0; i < SIZE; i++){
		if(input[row][i] != 0){
			// printf("%d, ", input[row][i]);
			bool_vals[input[row][i] - 1] = 0;
		}
		if(input[i][column] != 0){
			// printf("%d, ", input[i][column]);
			bool_vals[input[i][column] - 1] = 0;
		}
	}
	int j;
	int mini_row = (row/MINIGRIDSIZE)*MINIGRIDSIZE;
	int mini_column = (column/MINIGRIDSIZE)*MINIGRIDSIZE;
	for(i = 0; i < MINIGRIDSIZE; i++){
		for(j = 0; j < MINIGRIDSIZE; j++){
			if(input[mini_row + i][mini_column + j] != 0){
				// printf("%d, ", input[mini_row + i][mini_column + j]);
				bool_vals[input[mini_row+i][mini_column+j] - 1] = 0;
			}
		}
	}
	// printf("\n");
	struct possibleVals possible_vals;
	possible_vals.vals = calloc(SIZE, sizeof(int));
	possible_vals.size = 0;
	for(i = 0;i < SIZE; i++){
		if(bool_vals[i] == 1){
			possible_vals.vals[possible_vals.size] = i+1;
			possible_vals.size++;
		}
	}
	free(bool_vals);
	return possible_vals;
}

struct possibleVals** getPossibleGrid(int** input){
	int r_num, c_num, i;
	struct possibleVals** output = malloc(sizeof(struct possibleVals*)*SIZE);
	for(r_num = 0; r_num < SIZE; r_num++){
		// struct possibleVals pv;
		// pv.vals = calloc(SIZE, sizeof(int));
		// pv.size = 0;
		output[r_num] = malloc(SIZE*sizeof(struct possibleVals));
		for(c_num = 0; c_num < SIZE; c_num++){
			struct possibleVals possible_vals;
			if(input[r_num][c_num]  == 0){
				possible_vals = getPossibleValues(input, r_num, c_num);
			}
			else{
				possible_vals.vals = calloc(SIZE, sizeof(int));
				possible_vals.size = 0;
				possible_vals.vals[0] = input[r_num][c_num];
				possible_vals.size++;
			}
			output[r_num][c_num] = possible_vals;
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

void printPossibleGrid(struct possibleVals** possible_vals){
	int i,j,k;
	for (i=0;i<SIZE;i++){
		for (j=0;j<SIZE;j++){
			printf(":: [");
			for(k = 0; k<possible_vals[i][j].size; k++){
				printf("%d ",possible_vals[i][j].vals[k]);
			}
			printf("] ");
		}
		printf("\n");
	}
}


int updatePossibleGrid(int** input, int r_num, int c_num, struct possibleVals** possibleGrid){
	int i,j;
	for(i = 0; i<SIZE; i++){
		// int* up_vals1 = malloc(SIZE*sizeof(int));
		int up_size1 = 0;
		if(i != c_num){
			for(j = 0; j<possibleGrid[r_num][i].size; j++){
				if(input[r_num][c_num] != possibleGrid[r_num][i].vals[j]){
					possibleGrid[r_num][i].vals[up_size1] = possibleGrid[r_num][i].vals[j];
					up_size1++;
				}
			}
			// possibleGrid[r_num][i].vals = up_vals1;
			possibleGrid[r_num][i].size = up_size1;
			if(up_size1 == 0){
				// printf("1\n");
				return -1;
			}
		}
		// int* up_vals2 = malloc(SIZE*sizeof(int));
		int up_size2 = 0;
		if(i != r_num){
			for(j = 0; j<possibleGrid[i][c_num].size; j++){
				if(input[r_num][c_num] != possibleGrid[i][c_num].vals[j]){
					possibleGrid[i][c_num].vals[up_size2] = possibleGrid[i][c_num].vals[j];
					up_size2++;
				}
			}
			// possibleGrid[i][c_num].vals = up_vals2;
			possibleGrid[i][c_num].size = up_size2;
			if(up_size2 == 0){
				// printf("2\n");
				return -1;
			}
		}
	}
	// printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	// printPossibleGrid(possibleGrid);
	// printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	int mini_row = (r_num/MINIGRIDSIZE)*MINIGRIDSIZE;
	int mini_column = (c_num/MINIGRIDSIZE)*MINIGRIDSIZE;
	int k;
	for(i = mini_row; i < MINIGRIDSIZE+mini_row; i++){
		for(j = mini_column; j < MINIGRIDSIZE+mini_column; j++){
			// int* up_vals0 = malloc(SIZE*sizeof(int));
			int up_size0 = 0;
			if((i != r_num) && (j != c_num)){
				for(k = 0; k<possibleGrid[i][j].size; k++){
					if(input[r_num][c_num] != possibleGrid[i][j].vals[k]){
						possibleGrid[i][j].vals[up_size0] = possibleGrid[i][j].vals[k];
						up_size0++;
					}
				}
				// possibleGrid[i][j].vals = up_vals0;
				possibleGrid[i][j].size = up_size0;
				if(up_size0 == 0){
					// printf("3\n");
					return -1;
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

void freePossibleGrid(struct possibleVals** possibleGrid){
	int r_num, c_num, i;
	for(r_num = 0; r_num < SIZE; r_num++){
		for(c_num = 0; c_num < SIZE; c_num++){
			free(possibleGrid[r_num][c_num].vals);
		}
		free(possibleGrid[r_num]);
	}
	free(possibleGrid);
}

void freeGrid(int** grid){
	int r_num, c_num, i;
	for(r_num = 0; r_num < SIZE; r_num++){
		free(grid[r_num]);
	}
	free(grid);
}

int elimination(int** input){
	int r_num, c_num, r;
	int changed = 1;
	while(changed){
		changed = 0;
		for(r_num = 0; r_num < SIZE; r_num++){
			for(c_num = 0; c_num < SIZE; c_num++){
				if(input[r_num][c_num] == 0){
					struct possibleVals possible_vals = getPossibleValues(input, r_num, c_num);
					if(possible_vals.size == 0){
						// printf("4\n");
						return -1;
					}
					else if(possible_vals.size == 1){
						changed = 1;
						input[r_num][c_num] = possible_vals.vals[0];
						// printf("\npossible grid : %d, %d : val: %d\n", r_num, c_num, input[r_num][c_num]);
						// printPossibleGrid(possibleGrid);
						// if((r = updatePossibleGrid(input, r_num, c_num, possibleGrid)) < 0){
						// 	// printf("5\n");
						// 	return -1;
						// }
						// printf("\nupdated possible grid : %d, %d : val: %d\n", r_num, c_num, input[r_num][c_num]);
						// printPossibleGrid(possibleGrid);
					}
					free(possible_vals.vals);
				}
			}
		}
	}
	return 0;
}

int loneRanger(int** input){
	int r, c,i, j, mini_row, mini_column, k, noset, set,changed=1;
	struct possibleVals pv1,pv2;
	int *bool_vals;
	struct possibleVals** possibleGrid = getPossibleGrid(input);
	while(changed){
		changed=0;
		for(r=0; r<SIZE; r++){
			for(c=0; c<SIZE; c++){
				if(input[r][c]==0){
					// pv1 = getPossibleValues(input,r,c);
					pv1 = possibleGrid[r][c];
					if(pv1.size==0){
						//incorrect input
						freePossibleGrid(possibleGrid);
						return -1;
					}
					else if (pv1.size==1){
						input[r][c]=pv1.vals[0];
						changed=1;
						if(updatePossibleGrid(input,r,c,possibleGrid)<0){
							freePossibleGrid(possibleGrid);
							return -1;
						}
					}
					else if (pv1.size>1){
						bool_vals = calloc(SIZE,sizeof(int));
						// memset(bool_vals, 0, SIZE*sizeof(int));
						for (i=0; i<pv1.size; i++){
							bool_vals[pv1.vals[i]-1]=1;
						}
						for(i=0;i<SIZE;i++){
							//row
							if(i!=c){
								if(input[r][i]==0){
									// pv2 = getPossibleValues(input,r,i);
									pv2 = possibleGrid[r][i];
									if(pv2.size == 0){
										freePossibleGrid(possibleGrid);
										return -1;
									}
									else if(pv2.size>0){
										for(j=0; j<pv2.size;j++){
											bool_vals[pv2.vals[j]-1]=0;
										}
									}
									// free(pv2.vals);
								}	
							}
							//column
							if(i!=r){
								if(input[i][c]==0){
									// pv2 = getPossibleValues(input,i, c);
									pv2 = possibleGrid[i][c];
									if(pv2.size == 0){
										freePossibleGrid(possibleGrid);
										return -1;
									}
									else if(pv2.size>0){
										for(j=0;j<pv2.size;j++){
											bool_vals[pv2.vals[j]-1]=0;
										}
									}
									// free(pv2.vals);
								}
							}
						}
						mini_row = (r/MINIGRIDSIZE)*MINIGRIDSIZE;
						mini_column = (c/MINIGRIDSIZE)*MINIGRIDSIZE;
						for(i=0; i<MINIGRIDSIZE; i++){
							for(j=0; j<MINIGRIDSIZE; j++){
								if(i!=r && j!=c){
									if(input[mini_row+i][mini_column+j]!=0){
										// pv2 = getPossibleValues(input, mini_row+i, mini_column+j);
										pv2 = possibleGrid[mini_row+i][mini_column+j];
										if(pv2.size == 0){
											freePossibleGrid(possibleGrid);
											return -1;
										}
										else if(pv2.size>0){
											for(k=0;k<pv2.size; k++){
												bool_vals[pv2.vals[k]-1]=0;
											}
										}
										// free(pv2.vals);
									}
								}
							}
						}
						noset=0;
						for(i=0; i<SIZE; i++){
							if(bool_vals[i]==1){
								noset++;
								set = i+1;
							}
						}
						if(noset==1){
							input[r][c]=set;
							changed=1;
							if(updatePossibleGrid(input,r,c,possibleGrid)<0){
								freePossibleGrid(possibleGrid);
								return -1;
							}
							// possibleGrid[r][c].vals[0]=set;
							// possibleGrid[r][c].size=1;
						}
						else if(noset > 1){
							freePossibleGrid(possibleGrid);
							return -1;
						}
						// if(updatePossibleGrid(input, r, c, possibleGrid)<0){
						// return -1;
						// }
						free(bool_vals);
					}
					// free(pv1.vals);
				}
			}
		}
	}
	freePossibleGrid(possibleGrid);
	return 0;
}


int** solveSudokuRec(int** input){
	if(isComplete(input)) return input;
	int r_num;
	int c_num;
	// struct possibleVals** possibleGrid = getPossibleGrid(input);
	// printf("\npossible grid\n");
	// printPossibleGrid(possibleGrid);
	int r;
	int** input1 = malloc(SIZE*sizeof(int*));
	for(r_num = 0; r_num < SIZE; r_num++){
		input1[r_num] = malloc(SIZE*sizeof(int));
		for(c_num = 0; c_num < SIZE; c_num++){
			input1[r_num][c_num] = input[r_num][c_num];
		}
	}
	// r = elimination(input1);
	r = loneRanger(input1);
	if(r < 0){
		// freePossibleGrid(possibleGrid);
		freeGrid(input1);
		return input;
	}
	else{
		if(isComplete(input1)){
			// freePossibleGrid(possibleGrid);
			return input1;
		}
		// for(r_num = 0; r_num < SIZE; r_num++){
		// 	for(c_num = 0; c_num < SIZE; c_num++){
		// 		input[r_num][c_num] = input1[r_num][c_num];
		// 	}
		// }
		// free(input1);
	}
	// printf("\nupdated possible grid\n");
	// printPossibleGrid(possibleGrid);
	for(r_num = 0; r_num < SIZE; r_num++){
		for(c_num = 0; c_num < SIZE; c_num++){
			if(input1[r_num][c_num] == 0){
				struct possibleVals possible_vals = getPossibleValues(input1, r_num, c_num);
				int i;
				int** output;
				// for(i = 0; i< SIZE; i++){
				// 	if(possible_vals[i])printf("%d, ",i+1);
				// 	else printf("%d, ", possible_vals[i]);
				// }
				// printf("\n");
				for(i = 0; i < possible_vals.size; i++){
					input1[r_num][c_num] = possible_vals.vals[i];
					// printf("---------------------------------: %d, %d\n", r_num, c_num);
					// printGrid(input1);
					// printf("---------------------------------\n");
					output = solveSudokuRec(input1);
					if(isValid(input, output)){
						// freePossibleGrid(possibleGrid);
						// freeGrid(input1);
						return output;
					}
					else{
						input1[r_num][c_num] = 0;
					}
				}
				// printf("return ---------------------------------: %d, %d\n", r_num, c_num);
				// 		printGrid(input1);
				// 		printf("---------------------------------\n");
				// freePossibleGrid(possibleGrid);
				freeGrid(input1);
				return input;
			}
		}
	}
	// exit(0);
	// freePossibleGrid(possibleGrid);
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
	// struct possibleVals** possibleGrid = getPossibleGrid(input);
	// int r;
	// if((r = elimination(input, possibleGrid)) < 0) return input;
	struct queue* q = malloc(sizeof(struct queue));
	initQueue(q);
	pushQueue(q,makeCopy(input));
	int r_num, c_num, i;
	if(thread_count > 4) thread_count = 4;
	while(q->size < thread_count && !isEmptyQueue(q)){
		int** curr = popQueue(q);
		int break1=0;
		for(r_num = 0; r_num < SIZE; r_num++){
			for(c_num = 0; c_num < SIZE; c_num++){
				if(curr[r_num][c_num] == 0){
					struct possibleVals possible_vals = getPossibleValues(curr, r_num, c_num);
					for(i = 0; i < possible_vals.size; i++){
						int** curr_child = makeCopy(curr);
						curr_child[r_num][c_num] = possible_vals.vals[i];
						pushQueue(q, curr_child);
					}
					break1 = 1;
					break;
				}
			}
			if(break1) break;
		}
	}
	int** output;
	omp_set_num_threads(thread_count);
	#pragma omp parallel for 
		for(i = 0; i < q->size; i++){
			int** temp;
			if(!output){
				temp = solveSudokuRec(q->list[(i + q->start)%q->capacity]);
				#pragma omp critical
					if(isValid(input, temp)) output = temp;
			}
		}
	if(!output) output = input;
	return output;
	// return solveSudokuRec(input);
}
