#include "minimax.h"
#include <stdbool.h>
#include<stdio.h>
#include "src/Lab3/intervalTimer.h"
#define MAX_TABLE_SIZE 9 // Max array size of move score table
#define R_0 0  // row 0
#define R_1 1  // row 1
#define R_2 2  // row 2
#define C_0 0 // column 0
#define C_1 1 // column 1
#define C_2 2 // column 2
#define STARTING_MIN_SCORE -15 //Arbitrary value that is lower than -10 to record highest score
#define STARTING_MAX_SCORE 20 // Arbitrary value that is higher than 10 to record lowest score

minimax_move_t choice; // Global choice variable to get the final choice after calling minimax

void minimax_initBoard(minimax_board_t* board)
{
    // for all rows and columns
    for (uint8_t i = 0; i < MINIMAX_BOARD_ROWS; i++)
    {
        for (uint8_t j = 0; j < MINIMAX_BOARD_COLUMNS; j++)
        {
            // init all squares in board to empty squares
            board->squares[i][j] = MINIMAX_EMPTY_SQUARE;
        }
    }

}

bool minimax_isGameOver(minimax_score_t score)
{
	// If minimax score = not endgame, then game is not over
    if (score == MINIMAX_NOT_ENDGAME)
    {
        return false;
    }

	// any other score, game is over
    return true;
}

minimax_score_t minimax_computeBoardScore(minimax_board_t* board, bool player_is_x)
{
	// If player is X, Search every possible winning sequence and return X winning score if true
    if (player_is_x)
    {
        // search for any row sequence
        if ((board->squares[R_0][C_0] == MINIMAX_X_SQUARE && board->squares[R_0][C_1] == MINIMAX_X_SQUARE && board->squares[R_0][C_2] == MINIMAX_X_SQUARE) ||
            (board->squares[R_1][C_0] == MINIMAX_X_SQUARE && board->squares[R_1][C_1] == MINIMAX_X_SQUARE && board->squares[R_1][C_2] == MINIMAX_X_SQUARE) ||
            (board->squares[R_2][C_0] == MINIMAX_X_SQUARE && board->squares[R_2][C_1] == MINIMAX_X_SQUARE && board->squares[R_2][C_2] == MINIMAX_X_SQUARE) ||
            //search for any column sequence
            (board->squares[R_0][C_0] == MINIMAX_X_SQUARE && board->squares[R_1][C_0] == MINIMAX_X_SQUARE && board->squares[R_2][C_0] == MINIMAX_X_SQUARE) ||
            (board->squares[R_0][C_1] == MINIMAX_X_SQUARE && board->squares[R_1][C_1] == MINIMAX_X_SQUARE && board->squares[R_2][C_1] == MINIMAX_X_SQUARE) ||
            (board->squares[R_0][C_2] == MINIMAX_X_SQUARE && board->squares[R_1][C_2] == MINIMAX_X_SQUARE && board->squares[R_2][C_2] == MINIMAX_X_SQUARE) ||
            // search diagonal sequences
            (board->squares[R_0][C_0] == MINIMAX_X_SQUARE && board->squares[R_1][C_1] == MINIMAX_X_SQUARE && board->squares[R_2][C_2] == MINIMAX_X_SQUARE) ||
            (board->squares[R_2][C_0] == MINIMAX_X_SQUARE && board->squares[R_1][C_1] == MINIMAX_X_SQUARE && board->squares[R_0][C_2] == MINIMAX_X_SQUARE))
            return MINIMAX_X_WINNING_SCORE; // X wins

    }

    else // player is O
    {
        // search for any row sequence
        if ((board->squares[R_0][C_0] == MINIMAX_O_SQUARE && board->squares[R_0][C_1] == MINIMAX_O_SQUARE && board->squares[R_0][C_2] == MINIMAX_O_SQUARE) ||
			(board->squares[R_1][C_0] == MINIMAX_O_SQUARE && board->squares[R_1][C_1] == MINIMAX_O_SQUARE && board->squares[R_1][C_2] == MINIMAX_O_SQUARE) ||
			(board->squares[R_2][C_0] == MINIMAX_O_SQUARE && board->squares[R_2][C_1] == MINIMAX_O_SQUARE && board->squares[R_2][C_2] == MINIMAX_O_SQUARE) ||
			//search for any column sequence
			(board->squares[R_0][C_0] == MINIMAX_O_SQUARE && board->squares[R_1][C_0] == MINIMAX_O_SQUARE && board->squares[R_2][C_0] == MINIMAX_O_SQUARE) ||
			(board->squares[R_0][C_1] == MINIMAX_O_SQUARE && board->squares[R_1][C_1] == MINIMAX_O_SQUARE && board->squares[R_2][C_1] == MINIMAX_O_SQUARE) ||
			(board->squares[R_0][C_2] == MINIMAX_O_SQUARE && board->squares[R_1][C_2] == MINIMAX_O_SQUARE && board->squares[R_2][C_2] == MINIMAX_O_SQUARE) ||
			// search diagonal sequences
			(board->squares[R_0][C_0] == MINIMAX_O_SQUARE && board->squares[R_1][C_1] == MINIMAX_O_SQUARE && board->squares[R_2][C_2] == MINIMAX_O_SQUARE) ||
			(board->squares[R_2][C_0] == MINIMAX_O_SQUARE && board->squares[R_1][C_1] == MINIMAX_O_SQUARE && board->squares[R_0][C_2] == MINIMAX_O_SQUARE))
			return MINIMAX_O_WINNING_SCORE; // O wins
    }

    // check to see if board is full:
    for (uint8_t i = 0; i < MINIMAX_BOARD_ROWS; i++)
    {
        for (uint8_t k = 0; k < MINIMAX_BOARD_COLUMNS; k++)
        {
            if (board->squares[i][k] == MINIMAX_EMPTY_SQUARE)
                // game is not over
                return MINIMAX_NOT_ENDGAME;
        }
    }

    // game is a draw
    return MINIMAX_DRAW_SCORE;


}

minimax_score_t minimax(minimax_board_t* board, bool current_player_is_x)
{
    

    // check score of current board based on who played last
    minimax_score_t current_score = minimax_computeBoardScore(board, !current_player_is_x);
    // if game is over return to parent caller the score of the player who originally made the move
    if (minimax_isGameOver(current_score))
        return current_score;

    //game is not over, recurse:
    // initialize move-score table with two arrays
    //move array
    minimax_move_t moves[MAX_TABLE_SIZE];
    //score array
    minimax_score_t scores[MAX_TABLE_SIZE];
    int16_t move_score_table_size = 0;
	// For all rows and columns:
    for (uint8_t i = 0; i < MINIMAX_BOARD_ROWS; i++)
    {
        for (uint8_t k = 0; k < MINIMAX_BOARD_COLUMNS; k++)
        {
            if (board->squares[i][k] == MINIMAX_EMPTY_SQUARE)
            {
                // make move based on who's playing
                if (current_player_is_x)
                    board->squares[i][k] = MINIMAX_X_SQUARE;
                else
                    board->squares[i][k] = MINIMAX_O_SQUARE;
				// recursively call minimax from the opponents' possible moves
                current_score = minimax(board, !current_player_is_x);
                //add score to scores table
                scores[move_score_table_size] = current_score;
                //add move to moves table
                moves[move_score_table_size].row = i;
                moves[move_score_table_size].column = k;
                //increment table size variable:
                move_score_table_size++;
                //return square to empty
                board->squares[i][k] = MINIMAX_EMPTY_SQUARE;
            }

        }
    }

    // return final score depending on which player is playing:
    //declare two variable to keep track of largest score in move-score table
    // x_score is initialized to -15 to keep track of largest value
    // y_score is initialized to 20 to keep track of smallest value
    minimax_score_t x_score = STARTING_MIN_SCORE;
    minimax_score_t y_score = STARTING_MAX_SCORE;
    if (current_player_is_x)
    {
        for (uint8_t j = 0; j < move_score_table_size; j++)
        {
            if (scores[j] > x_score)
            {
                // record highest score
                x_score = scores[j];
                //update choice variable
                choice.row = moves[j].row;
                choice.column = moves[j].column;
                // update current score
                current_score = x_score;
            }

        }

    }

    else
    {
        for (uint8_t n = 0; n < move_score_table_size; n++)
        {
            if (scores[n] < y_score)
            {
                // record lowest score
                y_score = scores[n];
                //update choice variable
                choice.row = moves[n].row;
                choice.column = moves[n].column;
                //update current score
                current_score = y_score;
            }

        }

    }

    return current_score;

}

void minimax_computeNextMove(minimax_board_t* board, bool current_player_is_x, uint8_t* row, uint8_t* column)
{

	// If board is empty, minimax should not run to save time.
	
	//Check for an occupied square before calling minimax:
    for (uint8_t i = 0; i < MINIMAX_BOARD_ROWS; i++)
        {
            for (uint8_t j = 0; j < MINIMAX_BOARD_COLUMNS; j++)
            {
               if (board->squares[i][j] != MINIMAX_EMPTY_SQUARE)
               {
                   minimax(board, current_player_is_x);
                   *row = choice.row;
                   *column = choice.column;
                  return;
               }

            }

        }

    // board is empty, choose top left as move.
    *row = 0;
    *column = 0;


}

//Print board routine
void printboard(minimax_board_t* board)
{
	//For all rows and columns
    for (uint8_t i = 0; i < MINIMAX_BOARD_ROWS; i++)
    {
        for (uint8_t j = 0; j < MINIMAX_BOARD_COLUMNS; j++)
        {
			//Print an X or an O to the console
            if (board->squares[i][j] == MINIMAX_X_SQUARE)
                printf("X");
            else if (board->squares[i][j] == MINIMAX_O_SQUARE)
                printf("O");
            else
                printf(" "); 
        }
        printf("\n");
    }

}
