#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sudoku.h"

int* getPossibleValues(int** input, int row, int column){
	int i;
	int* possible_vals = malloc(SIZE*sizeof(int));
	// memset(possible_vals, 1, SIZE*sizeof(int));
	for(i = 0; i < SIZE;i++){
		possible_vals[i] = 1;
	}
	printf("discarded: ");
	for(i = 0; i < SIZE; i++){
		if(input[row][i] != 0){
			printf("%d, ", input[row][i]);
			possible_vals[input[row][i] - 1] = 0;
		}
		if(input[i][column] != 0){
			printf("%d, ", input[i][column]);
			possible_vals[input[i][column] - 1] = 0;
		}
	}
	int j;
	int mini_row = (row/MINIGRIDSIZE)*MINIGRIDSIZE;
	int mini_column = (column/MINIGRIDSIZE)*MINIGRIDSIZE;
	for(i = 0; i < MINIGRIDSIZE; i++){
		for(j = 0; j < MINIGRIDSIZE; j++){
			if(input[mini_row + i][mini_column + j] != 0){
				printf("%d, ", input[mini_row + i][mini_column + j]);
				possible_vals[input[mini_row+i][mini_column+j] - 1] = 0;
			}
		}
	}
	printf("\n");
	return possible_vals;
}

void printGrid(int** outputGrid){
	int i,j;
	for (i=0;i<SIZE;i++){
		for (j=0;j<SIZE;j++)
			printf("%d ",outputGrid[i][j]);
		printf("\n");
	}
}

int** solveSudoku(int** input){
	int r_num;
	int c_num;
	for(r_num = 0; r_num < SIZE; r_num++){
		for(c_num = 0; c_num < SIZE; c_num++){
			if(input[r_num][c_num] == 0){
				int* possible_vals = getPossibleValues(input, r_num, c_num);
				int i;
				int** output;
				for(i = 0; i< SIZE; i++){
					if(possible_vals[i])printf("%d, ",i+1);
					else printf("%d, ", possible_vals[i]);
				}
				printf("\n");
				for(i = 0; i<SIZE; i++){
					if(possible_vals[i]){
						input[r_num][c_num] = i+1;
						printf("---------------------------------: %d, %d\n", r_num, c_num);
						printGrid(input);
						printf("---------------------------------\n");
						output = solveSudoku(input);
						if(isValid(input, output)){
							return output;
						}
						else{
							input[r_num][c_num] = 0;
						}
					}
				}
				printf("return ---------------------------------: %d, %d\n", r_num, c_num);
						printGrid(input);
						printf("---------------------------------\n");
				return input;
			}
		}
	}
	// exit(0);
	return input;
}
