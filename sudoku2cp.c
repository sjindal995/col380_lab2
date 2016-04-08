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
	// #pragma omp critical
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

	grid output = q->list[q->start];
	q->start = (q->start + 1)%q->capacity;
	q->size--;
	return output;
}

grid frontQueue(struct queue* q){
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
	pgrid output;
	ll pv=0;
	for(r_num = 0; r_num < SIZE; r_num++){
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

int elimination(grid* input,pgrid* possibleGrid){
	int r_num, c_num, r;
	int changed = 1;
	while(changed){
		changed = 0;
		for(r_num = 0; r_num < SIZE; r_num++){
			for(c_num = 0; c_num < SIZE; c_num++){
				if(input->g[r_num][c_num] == 0){
					ll possible_vals = possibleGrid->pg[r_num][c_num];
					// ll possible_vals = getPossibleValues(input,r_num,c_num);
					if(possible_vals == 0){
						return -1;
					}
					int p2 = isPower2(possible_vals);
					if(p2>0){
						changed = 1;
						input->g[r_num][c_num] = p2;
						if(updatePossibleGrid(input, r_num,c_num,possibleGrid)<0){
							return -1;
						}
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

int lr2(grid* input){
	int r,c, changed=1, res,resp, ii, jj, mr, mc;
	ll pv1,pv2;
	pgrid possibleGrid = getPossibleGrid(input);
	while(changed){
		changed=0;
		res = elimination(input,&possibleGrid);
		// pgrid possibleGrid=getPossibleGrid(input);
		if(res<0) return -1;	
		for(r=0; r<SIZE; r++){
			for(c=0; c<SIZE; c++){
				if (input->g[r][c]==0){
					pv1 = possibleGrid.pg[r][c];
					if(pv1==0){
						return -1;
					}
					resp = isPower2(pv1);
					if(resp==-1){
						for(ii=0; ii<SIZE; ii++){
							//row
							if(ii!=c && (input->g[r][ii]==0)){
								pv2 = possibleGrid.pg[r][ii];
								if(pv2==0){
									return -1;
								}
								else{
									pv1 &= ~pv2;
								}
							}
						}
						resp = isPower2(pv1);
						if(resp>0){
							input->g[r][c] = resp;
							changed=1;
							res = updatePossibleGrid(input, r, c, &possibleGrid);
							if(res<0) return -1;
						}
						else if (resp==-1){
							return -1;
						}
					}
				}
			}
		}
		for(r=0; r<SIZE; r++){
			for(c=0; c<SIZE; c++){
				if (input->g[r][c]==0){
					pv1 = possibleGrid.pg[r][c];
					if(pv1==0){
						return -1;
					}
					resp = isPower2(pv1);
					if(resp==-1){
						for(ii=0; ii<SIZE; ii++){
							if(ii!=r && (input->g[ii][c]==0)){
								pv2 = possibleGrid.pg[ii][c];
								if(pv2==0){
									return -1;
								}
								else{
									pv1 &= ~pv2;
								}
							}
						}
						resp = isPower2(pv1);
						if(resp>0){
							input->g[r][c] = resp;
							changed=1;
							res = updatePossibleGrid(input, r, c, &possibleGrid);
							if(res<0) return -1;
						}
						else if (resp==-1){
							return -1;
						}
					}
				}
			}
		}
		for(r=0; r<SIZE; r++){
			for(c=0; c<SIZE; c++){
				if (input->g[r][c]==0){
					pv1 = possibleGrid.pg[r][c];
					if(pv1==0){
						return -1;
					}
					resp = isPower2(pv1);
					if(resp==-1){
						mr = (r/MINIGRIDSIZE)*MINIGRIDSIZE;
						mc = (c/MINIGRIDSIZE)*MINIGRIDSIZE;
						for(ii=0; ii<MINIGRIDSIZE; ii++){
							for(jj=0;jj<MINIGRIDSIZE;jj++){
								if( ((mr+ii)!=r || (mc+jj)!=c) && (input->g[mr+ii][mc+jj]==0)){
									pv2 = possibleGrid.pg[mr+ii][mc+jj];
									if(pv2==0){
										return -1;
									}
									else{
										pv1 &= ~pv2;
									}
								}
							}
						}
						resp = isPower2(pv1);
						if(resp>0){
							input->g[r][c] = resp;
							changed=1;
							res = updatePossibleGrid(input, r, c, &possibleGrid);
							if(res<0) return -1;
						}
						else if (resp==-1){
							return -1;
						}
					}
				}
			}
		}		
	}
	return 0;
}


grid solveSudokuRec(grid input){
	if(isValid2(&input,&input)) return input;
	int r_num;
	int c_num;
	int r;
	grid input1=input;
	// r = elimination(&input1);
	r = lr2(&input1);
	int fc;
	if(r < 0){
		return input;
	}
	else{
		if(isValid2(&input1,&input1)){
			return input1;
		}
	}
	for(r_num = 0; r_num < SIZE; r_num++){
		for(c_num = 0; c_num < SIZE; c_num++){
			if(input1.g[r_num][c_num] == 0){
				ll possible_vals =getPossibleValues(&input1,r_num,c_num);
				int i;
				grid output;
				for(i=0; i<SIZE; i++){
					#pragma omp critical
						fc = found_correct;
					if(fc){ 
						return input;
					}
					if(possible_vals&((offset)<<i)){
						input1.g[r_num][c_num] = i+1;
						output = solveSudokuRec(input1);
						if(isValid2(&input, &output)){
							return output;
						}
						else{
							input1.g[r_num][c_num] = 0;
						}
					}
				}
				return input;
			}
		}
	}
	return input;
}


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
	pushQueue(q,&i2);
	int r_num, c_num, i;
	if(SIZE<=9 && thread_count>2) thread_count=2;
	else if (SIZE<=16 && thread_count>4) thread_count=4;
	

	grid output;
	while(q->size < 4*thread_count && !isEmptyQueue(q)){
		grid curr = popQueue(q);
		int break1=0;
		for(r_num = 0; r_num < SIZE; r_num++){
			for(c_num = 0; c_num < SIZE; c_num++){
				if(curr.g[r_num][c_num] == 0){
					ll possible_vals = getPossibleValues(&curr, r_num, c_num);
					for(i = 0; i < SIZE; i++){
						if(possible_vals&((1LL)<<i)){
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
				temp = solveSudokuRec(q->list[(i + q->start)%q->capacity]);
				#pragma omp critical
					if(isValid2(&input, &temp)) {
						output = temp;
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
