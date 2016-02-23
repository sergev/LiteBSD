/*
 * Eight Queens puzzle
 *
 * (C) 2010 by Mark Sproul
 * Open source as per standard Arduino code
 * Modified by Pito 12/2012 for SmallC and then by Alexey Frunze for Smaller C
 */
#include <stdio.h>

#define TRUE    1
#define FALSE   0

unsigned int board[8];
unsigned int nloops;
int nsolutions;

int check_current_board()
{
    int ii;
    int row;
    int long_row;
    int long_columns;
    int nbits;

    // we know we have 1 in each row,
    // Check for 1 in each column
    row = 0;
    for (ii=0; ii<8; ii++) {
        row |= board[ii];
    }
    if (row != 0x0ff) {
        return FALSE;
    }

    // we have 1 in each column, now check the diagonals
    long_columns = 0;
    for (ii=0; ii<8; ii++) {
        long_row = board[ii] & 0x0ff;
        long_row = long_row << ii;

        long_columns |= long_row;
    }

    // now count the bits
    nbits = 0;
    for (ii=0; ii<16; ii++) {
        if ((long_columns & 0x01) == 0x01) {
            nbits++;
        }
        long_columns = long_columns >> 1;
    }

    if (nbits != 8) {
        return FALSE;
    }

    // we now have to check the other diagonal
    long_columns = 0;
    for (ii=0; ii<8; ii++) {
        long_row = board[ii] & 0x0ff;
        long_row = long_row << 8;
        long_row = long_row >> ii;

        long_columns |= long_row;
    }

    // now count the bits
    nbits = 0;
    for (ii=0; ii<16; ii++) {
        if ((long_columns & 0x01) == 0x01) {
            nbits++;
        }
        long_columns = long_columns >> 1;
    }

    if (nbits != 8) {
        return FALSE;
    }
    return TRUE;
}

int check_for_done()
{
    int ii;
    int done;
    int row;

    done = FALSE;

    // we know we have 1 in each row,
    // Check for 1 in each column
    row = 0;
    for (ii=0; ii<8; ii++) {
        row |= board[ii];
    }

    if (row == 0x01) {
        done = TRUE;
    }
    return done;
}

void rotate_queens()
{
    int ii;
    int keep_going;
    int row;

    ii = 0;
    keep_going = TRUE;
    while (keep_going && (ii < 8)) {
        row = board[ii] & 0x0ff;
        row = (row >> 1) & 0x0ff;
        if (row != 0) {
            board[ii] = row;
            keep_going = FALSE;
        } else {
            board[ii] = 0x080;
        }
        ii++;
    }
}

void print_board()
{
    int ii;
    int jj;
    int row;

    printf("\nLoop= %d\n", nloops);
    printf("Solution count= %d\n", nsolutions);

    printf("+----------------+\n");
    for (ii=0; ii<8; ii++) {
        row = board[ii];

        printf("|");
        for (jj=0; jj<8; jj++) {
            if (row & 0x080) {
                printf("Q ");
            } else {
                printf(". ");
            }
            row = row << 1;
        }
        printf("|\n");
    }
    printf("+----------------+\n");
}

int main()
{
    int ii;

    // put the 8 queens on the board, 1 in each row
    printf("\nEight Queens brute force");
    printf("\n************************\n");

    for (ii=0; ii<8; ii++) {
        board[ii] = 0x080;
    }
    print_board();

    nloops = 0;
    nsolutions = 0;

    while (1) {
        nloops++;

        if (check_current_board()) {
            nsolutions++;
            print_board();
        }

        rotate_queens();
        if (check_for_done()) {
            printf("All done\n");
            return (1);
        }
    }
}
