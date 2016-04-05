#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sudoku.h"


struct possibleVals{
	int* vals;
	int size;
};
struct possibleVals** possibleGrid;

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
			possible_vals.vals = calloc(SIZE, sizeof(int));
			possible_vals.size = 0;
			if(input[r_num][c_num]  == 0){
				possible_vals = getPossibleValues(input, r_num, c_num);
			}
			else{
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


void updatePossibleGrid(int** input, int r_num, int c_num){
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
			if(i != r_num && j != c_num){
				for(k = 0; k<possibleGrid[i][j].size; k++){
					if(input[r_num][c_num] != possibleGrid[i][j].vals[k]){
						possibleGrid[i][j].vals[up_size0] = possibleGrid[i][j].vals[k];
						up_size0++;
					}
				}
				// possibleGrid[i][j].vals = up_vals0;
				possibleGrid[i][j].size = up_size0;
			}
		}
	}
}

int elimination(int** input){
	int r_num, c_num;
	for(r_num = 0; r_num < SIZE; r_num++){
		for(c_num = 0; c_num < SIZE; c_num++){
			if(input[r_num][c_num] == 0){
				if(possibleGrid[r_num][c_num].size == 0){
					return -1;
				}
				else if(possibleGrid[r_num][c_num].size == 1){
					input[r_num][c_num] = possibleGrid[r_num][c_num].vals[0];
					// printf("\npossible grid\n");
					// printPossibleGrid(possibleGrid);
					updatePossibleGrid(input, r_num, c_num);
					// printf("\nupdated possible grid : %d, %d : val: %d\n", r_num, c_num, input[r_num][c_num]);
					// printPossibleGrid(possibleGrid);
				}
			}
		}
	}
	return 0;
}


int** solveSudoku1(int** input){
	int r_num;
	int c_num;
	for(r_num = 0; r_num < SIZE; r_num++){
		for(c_num = 0; c_num < SIZE; c_num++){
			if(input[r_num][c_num] == 0){
				struct possibleVals possible_vals = getPossibleValues(input, r_num, c_num);
				int i;
				int** output;
				// for(i = 0; i< SIZE; i++){
				// 	if(possible_vals[i])printf("%d, ",i+1);
				// 	else printf("%d, ", possible_vals[i]);
				// }
				// printf("\n");
				for(i = 0; i < possible_vals.size; i++){
					input[r_num][c_num] = possible_vals.vals[i];
					// printf("---------------------------------: %d, %d\n", r_num, c_num);
					// printGrid(input);
					// printf("---------------------------------\n");
					output = solveSudoku1(input);
					if(isValid(input, output)){
						return output;
					}
					else{
						input[r_num][c_num] = 0;
					}
				}
				// printf("return ---------------------------------: %d, %d\n", r_num, c_num);
				// 		printGrid(input);
				// 		printf("---------------------------------\n");
				return input;
			}
		}
	}
	// exit(0);
	return input;
}

int** solveSudoku(int** input){
	possibleGrid = getPossibleGrid(input);
	int r;
	if((r = elimination(input)) < 0) return input;
	return solveSudoku1(input);
}
