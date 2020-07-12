/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    // no more than 12 local variables !
    if ((M == 32) && (N == 32)) {
        // case 1
        int stride = 8;
        for (int i = 0; i < M; i += stride) {
            for (int j = 0; j < N; j += stride) {
                if (i == j) {
                   for (int smallI = 0; smallI < stride; smallI++) {
                       for (int smallJ = 0; smallJ < stride; smallJ++) {
                           B[i + (smallI + 1) % stride][j + smallJ] = A[i + smallI][j + smallJ];
                       }
                   }
                   for (int smallI = 1; smallI < stride; smallI++) {
                       for (int smallJ = 0; smallJ < stride; smallJ++) {
                           int temp = B[i + smallI - 1][j + smallJ];
                           B[i + smallI - 1][j + smallJ] = B[i + smallI][j + smallJ];
                           B[i + smallI][j + smallJ] = temp;
                       }
                   }
                   for (int smallI = 0; smallI < stride; smallI++) {
                       for (int smallJ = smallI; smallJ < stride; smallJ++) {
                           int row = i + smallI;
                           int col = j + smallJ;
                           int temp = B[row][col];
                           B[row][col] = B[col][row];
                           B[col][row] = temp;
                       }
                   }
                }
                else {
                    for (int smallI = 0; smallI < stride; smallI++)
                    {
                        for (int smallJ = 0; smallJ < stride; smallJ++) {
                            int row = i + smallI;
                            int col = j + smallJ;
                            int temp = A[row][col];
                            B[col][row] = temp;
                        }
                        /* code */
                    }
                }
                
            }
        }
    }

    if ((M == 61) && (N == 67)) {
        int stride = 16;
        int rowLength = (N/stride)*stride;
        int colLength = (M/stride)*stride;
        for (int i = 0; i < rowLength; i += stride) {
            for (int j = 0; j < colLength; j += stride) {
                for (int smallI = 0; smallI < stride; smallI++){
                    for (int smallJ = 0; smallJ < stride; smallJ++) {
                        int row = i + smallI;
                        int col = j + smallJ;
                        int temp = A[row][col];
                        B[col][row] = temp;
                    }
                }
            }
        }

        for (int i = 0; i < rowLength; i++) {
            for (int j = colLength; j < M; j++) {
                B[j][i] = A[i][j];
            }
        }
        
        

        int smallStride = 8;
        for (int j = 0; j < M; j+=smallStride) {
            for (int i = rowLength; i < N; i++) {
                for (int smallJ = 0; smallJ < smallStride; smallJ++) {
                    int row = i;
                    int col = j + smallJ;
                    B[col][row] = A[row][col];
                }
            }
        }
        for (int i = rowLength; i < N; i++) {
            for (int j = (M/smallStride)*smallStride; j < M; j++) {
                B[j][i] = A[i][j];
            }
        }

        
        
    }
    // case 2 M = N = 64
    if ((M == 64) && (N == 64)) {
        int stride = 8;
        for (int i = 0; i < N; i += (stride)) {
            for (int j = 0; j < M; j += stride) {
                for (int innerX = 0; innerX < (stride/2); innerX++) {
                    int A1 = A[i + innerX][j];
                    int A2 = A[i + innerX][j + 1];
                    int A3 = A[i + innerX][j + 2];
                    int A4 = A[i + innerX][j + 3];
                    int A5 = A[i + innerX][j + 4];
                    int A6 = A[i + innerX][j + 5];
                    int A7 = A[i + innerX][j + 6];
                    int A8 = A[i + innerX][j + 7];

                    B[j][i + innerX] = A1;
                    B[j + 1][i + innerX] = A2;
                    B[j + 2][i + innerX] = A3;
                    B[j + 3][i + innerX] = A4;
                    B[j][i + innerX + (stride/2)] = A5;
                    B[j + 1][i + innerX + (stride/2)] = A6;
                    B[j + 2][i + innerX + (stride/2)] = A7;
                    B[j + 3][i + innerX + (stride/2)] = A8;


                }
                
                for (int innerY = 0; innerY < (stride/2); innerY++) {
                    int A1 = A[stride/2 + i][j + innerY];
                    int A2 = A[stride/2 + i + 1][j + innerY];
                    int A3 = A[stride/2 + i + 2][j + innerY];
                    int A4 = A[stride/2 + i + 3][j + innerY];
                    int B1 = B[j + innerY][i + stride/2];
                    int B2 = B[j + innerY][i + stride/2 + 1];
                    int B3 = B[j + innerY][i + stride/2 + 2];
                    int B4 = B[j + innerY][i + stride/2 + 3];

                    B[j + innerY][i + stride/2] = A1;
                    B[j + innerY][i + stride/2 + 1] = A2;
                    B[j + innerY][i + stride/2 + 2] = A3;
                    B[j + innerY][i + stride/2 + 3] = A4;
                    
                    B[j + innerY + (stride/2)][i] = B1;
                    B[j + innerY + (stride/2)][i + 1] = B2;
                    B[j + innerY + (stride/2)][i + 2] = B3;
                    B[j + innerY + (stride/2)][i + 3] = B4;

                }

                for (int innerX = (stride/2); innerX < (stride); innerX++) {
                    for (int innerY = (stride/2); innerY < (stride); innerY++) {
                        int row = i + innerX;
                        int col = j + innerY;
                        B[col][row] = A[row][col];
                    }
                }
                
            }
        }


    }

}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

