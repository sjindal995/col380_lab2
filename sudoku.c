#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool checkPartial(int** input, int row, int column){
	int i;
	int val = input[row][column];
	for(i = 0; i<SIZE; i++){
		if((input[row][i] == val) && (i != column)) return false;
		if((input[i][column] == val) && (i != row)) return false;
	}
	int j;
	int mini_row = row/MINIGRIDSIZE;
	int mini_column = column/MINIGRIDSIZE;
	for(i = 0; i < MINIGRIDSIZE; i++){
		for(j = 0; j < MINIGRIDSIZE; j++){
			if((input[mini_row + i][mini_column + j] == val) && ((mini_row + i != row) || (mini_column + j != column))) return false;
		}
	}
	return true;
}

bool* getPossibleValues(int** input, int row, int column){
	int val;
	bool* possible_vals = malloc(SIZE*sizeof(bool));
	for(val = 1; val < SIZE; val++){
		input[row][column] = val;
		if(checkPartial(input, row, column)) possible_vals[val] = true;
	}
}


int** solvesudoku(int** input){
	int r_num;
	int c_num;
	for(r_num = 0; r_num < SIZE, r_num++){
		for(c_num = 0; c_num < SIZE; c_num++){
			if(input[r_num][c_num] == 0){
				bool* possible_vals = getPossibleValues(input, row, column);
			}
		}
	}
}
