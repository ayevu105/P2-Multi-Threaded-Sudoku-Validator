/* @file sudoku.c
 * @brief The following code fufills the requirments of the Multi-Threaded Sudoku Validator project
 * where the main driver verifies a sudoku puzzle of any size and whether there are 0's that exists within
 * the puzzle instead of numbers. 
 * @date 01/29/2023
 * compile: gcc -o sudoku -lm -pthread sudoku.c
 * run (verify complete puzzle and valid puzzle): ./sudoku puzzle.txt
 */

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// structure for passing data to the threads
  typedef struct {
      int row;
      int column;
      int** puzzle;
      int size;
      int* validity;
  } Parameters;

void* checkRow(void* parameters);
void* checkCol(void* parameters);
void* checkBox(void* parameters);
bool verifyPuzzleComplete(int** puzzle, int size);

// stores current box index
int boxCount = 1;

// checkRow determines whether a row is valid
void* checkRow(void* parameters) {

  Parameters* params = (Parameters*) parameters;

  // 0 means not found and 1 means found
  int foundVals[params->size + 1];

  // initialize all values to zero
  for (int i = 1; i <= params->size; i++) {
    foundVals[i] = 0;
  }

  // debugging test
  for (int i = 1; i <= params->size; i++) {
    foundVals[params->puzzle[params->row][i]] = 1;
  }

  // check if there are any 0's
  bool valid = true;
  for (int i = 1; i <= params->size; i++) {
    if (foundVals[i] == 0) {
      valid = false;
      break;
    }
  }
  if (!valid) {
    params->validity[params->row] = 0;
  }
  else {
    params->validity[params->row] = 1;
  }

}

//checkCol determines whether a certain column in the puzzle is valid
void* checkCol(void* parameters) {

  Parameters* params = (Parameters*) parameters;

  // 0 means not found and 1 means found
  int foundVals[params->size + 1];

  // initialize all values to zero
  for (int i = 1; i <= params->size; i++) {
    foundVals[i] = 0;
  }

  for (int i = 1; i <= params->size; i++) {
    foundVals[params->puzzle[i][params->column]] = 1;
  }

 // check if there are any 0's
  bool valid = true;
  for (int i = 1; i <= params->size; i++) {
    if (foundVals[i] == 0) {
      valid = false;
      break;
    }
  }
  if (!valid) {
    params->validity[params->column] = 0;
  }
  else {
    params->validity[params->column] = 1;
  }

}

//checkBox determines wether a certain box in the puzzle is valid
void* checkBox(void* parameters) {

  Parameters* params = (Parameters*) parameters;

  // 0 means not found and 1 means found
  int foundVals[params->size + 1];

  // initialize all values to zero
  for (int i = 1; i <= params->size; i++) {
    foundVals[i] = 0;
  }

  int length = sqrt(params->size);

  // loop through the indexes of the box and tells if value is found
  for (int row = params->row; row <= params->row + length - 1; row++) {
    for (int col = params->column; col <= params->column + length - 1; col++) {
      foundVals[params->puzzle[row][col]] = 1;
    }
  }

  // check if there are any 0's
  bool valid = true;
  for (int i = 1; i <= params->size; i++) {
    if (foundVals[i] == 0) {
      valid = false;
      break;
    }
  }
  
  if (!valid) {
    params->validity[boxCount] = 0;
  }
  else {
    params->validity[boxCount] = 1;
  }
  boxCount = boxCount + 1;
}

// verifyPuzzleComplete determines if the puzzle is complete and no 0's exist
bool verifyPuzzleComplete(int** puzzle, int size) {
  bool complete = true;
  for (int row = 1; row <= size; row++) {
    for (int col = 1; col <= size; col++) {
      if (puzzle[row][col] == 0) {
        complete = false;
        break;
      }
    }
  }
  return complete;
}

// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {

  *complete = verifyPuzzleComplete(grid, psize);
  if (*complete == false) {
    return;
  }

  // integer arrays that determines the validity of each row, column and box in the puzzle
  int* rowValidity = malloc((psize + 1) * sizeof(*rowValidity));
  int* colValidity = malloc((psize + 1) * sizeof(*colValidity));
  int* boxValidity = malloc((psize + 1) * sizeof(*boxValidity));

  for (int i = 1; i <= psize; i++) {
    rowValidity[i] = -1;
    colValidity[i] = -1;
    boxValidity[i] = -1;
  }

  // allocate memory for threads
  pthread_t* rowThreads = malloc(sizeof(pthread_t) * (psize + 1));
  pthread_t* colThreads = malloc(sizeof(pthread_t) * (psize + 1));
  pthread_t* boxThreads = malloc(sizeof(pthread_t) * (psize + 1));

  if (rowThreads == NULL) {
    printf("ERROR: out of memory for row threads\n");
    exit(EXIT_FAILURE);
  }

  if (colThreads == NULL) {
    printf("ERROR: out of memory for column threads\n");
    exit(EXIT_FAILURE);
  }

  if (boxThreads == NULL) {
    printf("ERROR: out of memory for box threads\n");
    exit(EXIT_FAILURE);
  }

  // loops to call helper functions to determine validity of each row, column and box and then 
  // creates threads
  for (int row = 1; row <= psize; row++) {
    Parameters* params = (Parameters*) malloc(sizeof(Parameters));
    params->row = row;
    params->column = -1;
    params->puzzle = grid;
    params->size = psize;
    params->validity = rowValidity;

    if (pthread_create(&rowThreads[row], NULL, checkRow, (void*) params)) {
      printf("ERROR: create row threads failed");
      exit(EXIT_FAILURE);
    }
  }

  for (int col = 1; col <= psize; col++) {
    Parameters* params = (Parameters*) malloc(sizeof(Parameters));
    params->row = -1;
    params->column = col;
    params->puzzle = grid;
    params->size = psize;
    params->validity = colValidity;

    if (pthread_create(&colThreads[col], NULL, checkCol, (void*) params)) {
      printf("ERROR: create column threads failed");
      exit(EXIT_FAILURE);
    }
  }

  int boxSize = sqrt(psize);

  int count = 1;
  for (int row = 1; row <= psize; row += boxSize) {
    for (int col = 1; col <= psize; col += boxSize) {
      Parameters* params = (Parameters*) malloc(sizeof(Parameters));
      params->row = row;
      params->column = col;
      params->puzzle = grid;
      params->size = psize;
      params->validity = boxValidity;

      if (pthread_create(&boxThreads[count], NULL, checkBox, (void*) params)) {
        printf("ERROR: create row threads failed");
        exit(EXIT_FAILURE);
      }
      count++;
    }
  }

  // join threads
  for (int i = 1; i <= psize; i++) {
    pthread_join(rowThreads[i], NULL);
    pthread_join(colThreads[i], NULL);
    pthread_join(boxThreads[i], NULL);
  }

  // checks if puzzle is valid
  *valid = true;

  for (int i = 1; i <= psize; i++) {

    // error checking for if an index was never checked
    if (rowValidity[i] == -1) {
      printf("ERROR: not all rows were checked\n");
      *valid = false;
      return;
    }
    if (colValidity[i] == -1) {
      printf("ERROR: not all columns were checked\n");
      *valid = false;
      return;
    }
    if (boxValidity[i] == -1) {
      printf("ERROR: not all boxes were checked\n");
      *valid = false;
      return;
    }

    if (rowValidity[i] == 0 || colValidity[i] == 0 || boxValidity[i] == 0) {
      *valid = false;
      return;
    }
  }
  
  // free validity and thread arrays
  free(rowValidity);
  free(colValidity);
  free(boxValidity);

  free(rowThreads);
  free(colThreads);
  free(boxThreads);
}

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
// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D array
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
