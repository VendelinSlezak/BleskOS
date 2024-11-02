//BleskOS PONG game

#include "bleskos_system_calls.h"

//definition of all used methods
void frame_of_game(unsigned int (*system_call)(unsigned int call, unsigned int args[]));
void draw_player(unsigned int (*system_call)(unsigned int call, unsigned int args[]), int column, int line);

//definition of used global variables
int ball_column, ball_line, ball_x_movement, ball_y_movement, player_1_line, player_2_line, player_1_score, player_2_score;

//from here starts execution of program
void main(unsigned int (*system_call)(unsigned int call, unsigned int args[])) {
 CALL_SET_PROGRAM_MODE(PROGRAM_TEXT_MODE); //program is working in text mode

 //we do not want text cursor to be on screen
 CALL_HIDE_CURSOR();

 //set initial variables
 reset_game:
 player_1_score = 0;
 player_2_score = 0;
 ball_column = 40;
 ball_line = CALL_GET_RANDOM_NUMBER(1, 50, 1);
 ball_x_movement = CALL_GET_RANDOM_NUMBER(-1, 1, 2);
 ball_y_movement = CALL_GET_RANDOM_NUMBER(-1, 1, 2);
 player_1_line = 25;
 player_2_line = 25;

 //draw game screen
 CALL_DRAW_EMPTY_SQUARE(0, 0, 82, 52, 0x0000FF); //game will be happenning only in this square 80x50
 CALL_DRAW_FULL_SQUARE(1, 1, 80, 50, 0x000000); //game will be happenning only in this square 80x50

 CALL_SET_CH_COLOR_ON_BLOCK(0, 0, 82, 1, 0xFFFFFF); //we want to write white text to first line
 CALL_PRINT_TO_BLOCK(0, 0, 82, PARAMETER_ALIGN_CENTER, "You can quit game by ESC");
 CALL_PRINT_NUMBER(1, 0, player_1_score);
 CALL_PRINT_NUMBER(80, 0, player_2_score);
 
 CALL_DRAW_FULL_SQUARE(ball_column, ball_line, 1, 1, 0xFFFFFF); //draw ball
 draw_player(system_call, 1, player_1_line); //draw first player
 draw_player(system_call, 80, player_2_line); //draw second player

 //start game with 20 FPS
 CALL_MOVE_EXECUTION_TO_METHOD(frame_of_game, 20);

 //return from game, this can mean three situations
 if(player_1_score==5) { //first player won
  CALL_SET_COLORS_ON_BLOCK(1, 1, 80, 50, 0x000000, 0x00FF00);
  CALL_PRINT_TO_BLOCK(0, 25, 82, PARAMETER_ALIGN_CENTER, "Green player won!");
 }
 else if(player_2_score==5) { //second player won
  CALL_SET_COLORS_ON_BLOCK(1, 1, 80, 50, 0x000000, 0xFF0000);
  CALL_PRINT_TO_BLOCK(0, 25, 82, PARAMETER_ALIGN_CENTER, "Red player won!");
 }
 else { //ESC was pressed, exit program
  return;
 }

 //win screen
 CALL_PRINT_TO_BLOCK(0, 27, 82, PARAMETER_ALIGN_CENTER, "Press Enter for new game");
 while(1) {
  CALL_WAIT_FOR_KEYBOARD();

  //start game again
  if(CALL_GET_KEY_VALUE()==KEY_ENTER) {
   goto reset_game;
  }

  //exit program
  if(CALL_GET_KEY_VALUE()==KEY_ESC) {
   break;
  }
 }
}

void frame_of_game(unsigned int (*system_call)(unsigned int call, unsigned int args[])) {
 //exit game
 if(CALL_GET_ACTUAL_KEY_VALUE()==KEY_ESC) {
  CALL_EXIT_EXECUTION_OF_METHOD();
  return;
 }

 //move player 1
 if(CALL_IS_THIS_KEY_PRESSED(KEY_UP)==STATUS_TRUE && player_1_line>1) {
  player_1_line--;
  draw_player(system_call, 1, player_1_line);
 }
 if(CALL_IS_THIS_KEY_PRESSED(KEY_DOWN)==STATUS_TRUE && player_1_line<46) {
  player_1_line++;
  draw_player(system_call, 1, player_1_line);
 }

 //move player 2
 if(CALL_IS_THIS_KEY_PRESSED(KEY_Q)==STATUS_TRUE && player_2_line>1) {
  player_2_line--;
  draw_player(system_call, 80, player_2_line);
 }
 if(CALL_IS_THIS_KEY_PRESSED(KEY_A)==STATUS_TRUE && player_2_line<46) {
  player_2_line++;
  draw_player(system_call, 80, player_2_line);
 }

 //test if some player scored point
 if(ball_column==1 && ball_x_movement==-1) {
  //increase score
  player_2_score++;

  //update score on screen
  CALL_PRINT_NUMBER(80, 0, player_2_score);

  //second player win
  if(player_2_score==5) {
   CALL_EXIT_EXECUTION_OF_METHOD();
   return;
  }

  //delete ball from screen
  CALL_DRAW_FULL_SQUARE(ball_column, ball_line, 1, 1, 0x000000);

  //reset ball variables
  ball_column = 40;
  ball_line = CALL_GET_RANDOM_NUMBER(1, 50, 1);
  ball_x_movement = CALL_GET_RANDOM_NUMBER(-1, 1, 2);
  ball_y_movement = CALL_GET_RANDOM_NUMBER(-1, 1, 2);

  //continue game
  return;
 }
 if(ball_column==80 && ball_x_movement==1) {
  //increase score
  player_1_score++;

  //updare score on screen
  CALL_PRINT_NUMBER(1, 0, player_1_score);

  //first player win
  if(player_1_score==5) {
   CALL_EXIT_EXECUTION_OF_METHOD();
   return;
  }

  //delete ball from screen
  CALL_DRAW_FULL_SQUARE(ball_column, ball_line, 1, 1, 0x000000);

  //reset ball variables
  ball_column = 40;
  ball_line = CALL_GET_RANDOM_NUMBER(1, 50, 1);
  ball_x_movement = CALL_GET_RANDOM_NUMBER(-1, 1, 2);
  ball_y_movement = CALL_GET_RANDOM_NUMBER(-1, 1, 2);

  //continue game
  return;
 }

 //hide ball
 CALL_DRAW_FULL_SQUARE(ball_column, ball_line, 1, 1, 0x000000);

 //change ball direction
 if((ball_column==2 && ball_x_movement==-1 && ball_line>=player_1_line && ball_line<(player_1_line+5)) || (ball_column==79 && ball_x_movement==1 && ball_line>=player_2_line && ball_line<(player_2_line+5))) {
  ball_x_movement *= -1;
 }
 if((ball_line==1 && ball_y_movement==-1) || (ball_line==50 && ball_y_movement==1)) {
  ball_y_movement *= -1;
 }

 //move ball
 ball_column += ball_x_movement;
 ball_line += ball_y_movement;

 //draw ball on new location
 CALL_DRAW_FULL_SQUARE(ball_column, ball_line, 1, 1, 0xFFFFFF);
}

void draw_player(unsigned int (*system_call)(unsigned int call, unsigned int args[]), int column, int line) {
 CALL_DRAW_FULL_SQUARE(column, 1, 1, line, 0x000000);

 dword_t color = 0x00FF00; //first player is green
 if(column==80) { //second player is red
  color = 0xFF0000;
 }
 CALL_DRAW_FULL_SQUARE(column, line, 1, 5, color);

 CALL_DRAW_FULL_SQUARE(column, line+5, 1, 51-line-5, 0x000000);
}