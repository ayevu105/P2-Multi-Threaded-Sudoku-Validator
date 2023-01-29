

//compile: gcc -pthread -lm -std=c99 sudoku.c -o sudoku
// run (verify): ./sudoku puzzle-easy.txt

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// structure for passing data to threads
typedef struct {
    int row;
    int column;
    int** grid;
    int psize;
} parameters;

// this array holds status for complete thread, row thread, and column thread
// index 0 - completnes, 1 - rows validation, 2 - columns validation
int status[3] = {0};

// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}

// Checks whether the grid is complete (does not contain 0's) 
void *completeCheck(void *data) {
  parameters *prmtrs = (parameters *)data;
  for (int i = 1; i <= prmtrs->psize; i++) {
    for (int j = 1; j <= prmtrs->psize; j++){
      if(prmtrs->grid[i][j] == 0) {
        status[0] = 2; // 2 for incomplete
        pthread_exit(0);
      }
    }
  }
  status[0] = 1; // valid
  pthread_exit(0);
}

// Checks whether the rows of the sudoku grid are valid.
void *validateRows(void *data) {
  parameters *prmtrs = (parameters *)data;
  int digit_check[prmtrs->psize];
  for (int i = 1; i <= prmtrs->psize; i++) {
    for (int j = 1; j <= prmtrs->psize; j++){
      if (digit_check[prmtrs->grid[i][j] - 1] == 1) {
        pthread_exit(0); // invalid
     }
    digit_check[prmtrs->grid[i][j] - 1] = 1;
    }
    memset(digit_check, 0, sizeof(digit_check));
  }
  status[1] = 1; // valid
  pthread_exit(0);
}

// Checks whether the columns of the sudoku grid are valid.
void *validateColumns(void *data) {
  parameters *prmtrs = (parameters *)data;
  int digit_check[prmtrs->psize];
  for (int i = 1; i <= prmtrs->psize; i++) {
    for (int j = 1; j <= prmtrs->psize; j++){
      if (digit_check[prmtrs->grid[j][i] - 1] == 1) {
        pthread_exit(0); // invalid
      }
      digit_check[prmtrs->grid[j][i] - 1] = 1;
    }
    memset(digit_check, 0, sizeof(digit_check));
  }
  status[2] = 1; // valid
  pthread_exit(0);
}

// Checks if the subgrid of the sudoku grid is valid
void *validateSubgrid(void *data) {
  parameters *prmtrs = (parameters *)data;
  int digit_check[prmtrs->psize];
  for (int i = prmtrs->row; i < prmtrs->row + sqrt(prmtrs->psize); i++) {
    for (int j = prmtrs->column; j < prmtrs->column + sqrt(prmtrs->psize); j++) {
      if (digit_check[prmtrs->grid[i][j]-1] == 1) {
        return (void *)0; // Invalid sudoku subgrid
      }
      digit_check[prmtrs->grid[i][j]-1] = 1;
    }
    memset(digit_check, 0, sizeof(digit_check));
  }
  return (void *)1; // Valid sudoku subgrid
}

// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
// check if the grid is complete, status[0]
  pthread_t tid_complete;
  parameters *data_complete = (parameters *)malloc(sizeof(parameters));
  data_complete->row = 1;
  data_complete->column = 1;
  data_complete->psize = psize;
  data_complete->grid = grid;
  pthread_create(&tid_complete, NULL, completeCheck, (void *) data_complete);
  pthread_join(tid_complete, NULL);
  if (status[0] == 1) {
    *complete = true;
  }
  if (complete) {
// check if rows are valid, status[1]
    pthread_t tid_rows;
    parameters *data_rows = (parameters *)malloc(sizeof(parameters));
    data_rows->row = 1;
    data_rows->column = 1;
    data_rows->psize = psize;
    data_rows->grid = grid;
    pthread_create(&tid_rows, NULL, validateRows, (void *)data_rows);
    pthread_join(tid_rows, NULL);
// check if columns are valid, status[2]
    pthread_t tid_columns;
    parameters *data_columns = (parameters *)malloc(sizeof(parameters));
    data_columns->row = 1;
    data_columns->column = 1;
    data_columns->psize = psize;
    data_columns->grid = grid;
    pthread_create(&tid_columns, NULL, validateColumns, (void *)data_columns);
    pthread_join(tid_columns, NULL);
// check if subgrids are valid
    pthread_t tid_subgrid[psize];
    int tid_subgrid_status[psize];
    int i = 0;
    double s = sqrt(psize);
    parameters *data_subgrid[psize];
    for (int r = 1; r <= psize; r += s) {
      for (int c = 1; c <= psize; c += s, i++) {
        data_subgrid[i] = (parameters *)malloc(sizeof(parameters));
        data_subgrid[i]->row = r;
        data_subgrid[i]->column = c;
        data_subgrid[i]->psize = psize;
        data_subgrid[i]->grid = grid;
      }
    }
    // if subgrid is not valid - exit
    int return_code;
    for (int i = 0; i < psize; i++) {
      pthread_create(&tid_subgrid[i], NULL, validateSubgrid, (void *)data_subgrid[i]);
    }
    for (int i = 0; i < psize; i++) {
      if (pthread_join(tid_subgrid[i], NULL) == return_code) {
        *valid = false;
        exit(EXIT_FAILURE);
      }
    }
    // if threads for cheking subgrids didn't exit with failure, check if
    // rows validation status and columns validation status equals to 1
    // if so - set *valid to true
    if (status[1] == 1 && status[2] == 1) {
      *valid = true;
    }
  }
}

// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D statusay
  int **grid = NULL;
  // find grid size and fill grid
  int sudokuSize = readSudokuPuzzle(argv[1], &grid);
  bool valid = false;
  bool complete = false;
  checkPuzzle(sudokuSize, grid, &complete, &valid);
  printf("Complete puzzle? ");
  printf(complete ? "true\n" : "false\n");
  if (complete) {
    printf("Valid puzzle? ");
    printf(valid ? "true\n" : "false\n");
  }
  printSudokuPuzzle(sudokuSize, grid);
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}
