#include "ticTacToeControl.h"
#include "ticTacToeDisplay.h"
#include "src/Lab3/intervalTimer.h"
#include "src/Lab2/buttons.h"
#include <stdio.h>
#include "supportFiles/display.h"
#include "minimax.h"

#define WELCOME_SCRREN_ERASE_X 20 // Starting x-coordinate for the welcome screen message
#define WELCOME_SCRREN_ERASE_Y 100 // Starting x-coordinate for the welcome screen message
#define WELCOME_SCREEN_ERASE_SIZE 2 // Text size of Welcome Screen
#define MAXIMUM_MOVES 9 // Maximum amount of moves in any given game. Used to determine a tie
#define ADC_COUNTER_MAX_VALUE 100/100 // 50 ms for the adc: divided by Timer Period in ms
#define AUTOPLAY_CNTR_MAX_VALUE 4000/100// 4 second delay before computer begins game automatically: 4000/ Timer Period in ms
#define WELCOME_SCREEN_CNTR_MAX_VALUE 5000/100 // 5 second delay before welcome screen disappears 5000/ Timer Period in ms
#define RESET_GAME_SEQUENCE 0x1 // Sequence used to read button 0 when player wants to reset game
// States for the controller state machine.
enum ticTacToeControl_st_t {
    init_st,                 // Start here, transition out of this state on the first tick
    never_touched_st,        // Wait here for a while to give user a chance to play first
    computer_move_st,       // Computer takes its move in this state
    waiting_for_move_st,    // waiting for move of human player
    adc_Counter_running_st,     // waiting for the touch-controller ADC to settle
    game_over_st,            // Total number of moves has reached 9, or a player has won. Remain in this state until reset button is pressed
    waiting_release_st      // State waiting for player to release finger before drawing move

} currentState;

// Init function. Sets the currentState variable in State Machine to init_st 
void ticTacToeControl_init() 
{

    currentState = init_st;
}

void ticTacToeControl_tick()
{

	// variable to store total moves in game
    static uint8_t move_count; 
    // Counters
    static uint adcCounter; // counter for adc Touchpad
    static uint8_t autoplay_counter; // counter that when expired, computer plays first
    static uint8_t welcomescreen_counter; // counter for welcome screen delay
    static bool player_is_X; // True means human player is X
    static minimax_score_t game_score; // Current game board score

    //Current game board
    static minimax_board_t game_board;

       // Perform state update first.
        switch (currentState) {
        case init_st:
			// Stay in init state (welcome screen on) until welcome screen counter expires
            if(welcomescreen_counter < WELCOME_SCREEN_CNTR_MAX_VALUE)
            {

                currentState = init_st;
            }

			//Otherwise, erase welcome message and begin game by going to never_touched_st
            else
            {

                // erase welcome message
                    display_setCursor(WELCOME_SCRREN_ERASE_X, WELCOME_SCRREN_ERASE_Y);            // Start somewhere in the middle of the LCD screen.
                    display_setTextColor(DISPLAY_BLACK);  // Make the text black to erase
                    display_setTextSize(WELCOME_SCREEN_ERASE_SIZE);
                    display_println("Touch Board to play as X\n        --or--\n wait for the computer\n      and play as O"); // welcome message with proper spacing
                    ticTacToeDisplay_drawBoardLines(); // draw the game board

                 //initialize game board values
                 minimax_initBoard(&game_board);

                 //initialize total moves drawn on board
                 move_count =0;

                 //initialize Counters
                 autoplay_counter=0;
                 adcCounter =0;
                 welcomescreen_counter = 0;

				 // update state to first state of game
				 currentState = never_touched_st;   

                 //initialize game board score
                 game_score = MINIMAX_NOT_ENDGAME;
            }
            break;

			//state at beginning of game
        case never_touched_st:

            // Cover rare case where player touches screen at same time autoplay counter expires
            if (display_isTouched() && autoplay_counter == AUTOPLAY_CNTR_MAX_VALUE)
            {
                //Go ahead and play as if counter didn't expire (play as X):
                
                player_is_X = true;
                
                //Start adcCounter
                display_clearOldTouchData();
                currentState = adc_Counter_running_st;

            }
            
            // if player touches screen before autoplay counter expires
            else if (display_isTouched() && autoplay_counter < AUTOPLAY_CNTR_MAX_VALUE)
            {
                // Play as X
                player_is_X = true;
                
                //Start adcCounter
                display_clearOldTouchData();
                currentState = adc_Counter_running_st;
            }
            
            // If autoplay counter expires before screen is touched, computer is X, and go to computer move st
            else if (!display_isTouched() && autoplay_counter == AUTOPLAY_CNTR_MAX_VALUE)
            {
				// Play as O
                player_is_X = false;
                currentState = computer_move_st;
            }
            
            else
                // remain in state
                currentState = never_touched_st;
                
            break;

			// state when computer is waiting for player to make a move
        case waiting_for_move_st:

			//if total moves has reached maximum moves, the game is over
            if (move_count == MAXIMUM_MOVES)
              currentState = game_over_st;
            // if display is touched, transition to adc_Counter_running_st
            else if (display_isTouched())
            {
                display_clearOldTouchData();
                currentState = adc_Counter_running_st;
            }

            else
                // remain in current state
                currentState = waiting_for_move_st;
            break;

			// state when adc Converter is settling and registering a touch
        case adc_Counter_running_st:

			// if the adc Converter has settled, read touch coordinates and register selection if valid
            if (adcCounter == ADC_COUNTER_MAX_VALUE)
            {
                currentState = waiting_release_st;
                //reset adcCounter
                adcCounter = 0;
   
            }

            else // adcCounter is not expired
                // return to current state to increment adcCounter
                currentState = adc_Counter_running_st;
            break;
			//state where computer takes it's turn
        case computer_move_st:
			// go to computer move state if game is not over:

			// check if game is over:
			// if total moves has reached maximum moves or computer has won, go to game_over_st
			if (move_count == MAXIMUM_MOVES || minimax_isGameOver(game_score))
				currentState = game_over_st;
			// game is not over, it's player's turn
			else
              currentState = waiting_for_move_st;
            break;
			//state where waiting release of player finger
        case waiting_release_st:
			// when it's released play the player's move
            if (!display_isTouched())
            {
				//Temporary variables to store the square selection
                uint8_t row;
                uint8_t column;
                  // decide which square was touched
                  ticTacToeDisplay_touchScreenComputeBoardRowColumn(&row, &column);

                   //if player selected an empty square, the player can play. If not, ignore it:
                    if (game_board.squares[row][column] == MINIMAX_EMPTY_SQUARE)
                    {

                     //update board with move:
                       player_is_X ? game_board.squares[row][column] = MINIMAX_X_SQUARE : game_board.squares[row][column] = MINIMAX_O_SQUARE;
                     // update display with move
                       player_is_X ? ticTacToeDisplay_drawX(row, column, false) : ticTacToeDisplay_drawO(row, column, false); // false means do not erase.

                      //increase move counter:
                       move_count++;
					   // Compute current game score
                       game_score = minimax_computeBoardScore(&game_board, player_is_X);

                       // go to computer move state if game is not over:

                        // check if game is over:
                          // if total moves has reached maximum moves or player has won (which should never happen), go to game_over_st
                          if (move_count == MAXIMUM_MOVES || minimax_isGameOver(game_score))
                          currentState = game_over_st;
                         // game is not over, it's the computer's turn
                          else
                            currentState = computer_move_st;
                       }

                        else // player selected occupied square (invalid selection), go back to waiting_for_move_st
                       currentState = waiting_for_move_st;
            }
			//If finger is still held down, remain in state
            else
                currentState = waiting_release_st;

               break;

        //state where game is over
        case game_over_st:
			//if button 0 is pressed then reset the game
            if ((buttons_read() & BUTTONS_BTN0_MASK) == RESET_GAME_SEQUENCE)
            {
                //Clear display quickly instead of "fill_display" approach
                for (uint8_t i = 0; i < MINIMAX_BOARD_ROWS; i++)
                       {
						   // draw a black O and X on each square to clear board
                           for (uint8_t k = 0; k < MINIMAX_BOARD_COLUMNS; k++)
                           {
                               ticTacToeDisplay_drawO(i,k, true); // true to erase character
                               ticTacToeDisplay_drawX(i, k, true); // true to erase character
                           }
                       }
                // reset variables:

                //reset game board
                  minimax_initBoard(&game_board);

                //reset total moves drawn on board
                move_count =0;

                 //reset Counters
                 autoplay_counter=0;
                 adcCounter =0;

                  //reset game board score
                  game_score = MINIMAX_NOT_ENDGAME;
				  
				  // return to first state of game
                  currentState = never_touched_st;
            }

			// remain in state
            else
                currentState = game_over_st;
            break;

        default:
            printf("ticTacToeControl_tick state update: hit default\n\r");
            break;
        }



        // Perform state action next.
            switch (currentState) {
            case init_st:
				// update welcome screen counter
                welcomescreen_counter++; 
                break;
            case never_touched_st:
                //increment autoplay_counter
                autoplay_counter++;
                break;
            case waiting_for_move_st:
                //do nothing, just wait
                break;
            case adc_Counter_running_st:
                //increment adc_Counter
                adcCounter++;
                break;
            case waiting_release_st:
                // do nothing, just wait
                break;
            case computer_move_st:
            {
				//Temporary variables to store computer's selected move
                uint8_t row;
                uint8_t column;
                // make decision using minimax:

                minimax_computeNextMove(&game_board,!player_is_X, &row, &column);

                //update board with move:
                player_is_X ? game_board.squares[row][column] = MINIMAX_O_SQUARE : game_board.squares[row][column] = MINIMAX_X_SQUARE;
                // update display with move
                player_is_X ? ticTacToeDisplay_drawO(row, column, false) : ticTacToeDisplay_drawX(row,column, false); // false means do not erase.

                // store current game score 
                game_score = minimax_computeBoardScore(&game_board, !player_is_X);
                //increase move counter
                move_count++;
            }
            break;
            case game_over_st:
                // do nothing, just wait
                break;
            default:
                printf("ticTacToeControlControl_tick state action: hit default\n\r");
                break;
            }


}
