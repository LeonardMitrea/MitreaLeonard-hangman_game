#include <stdio.h>
#include <delay.h> 
#include "LCD_I2C.h"
#include "UART.h"

#define F_CPU 20000000UL

interrupt [TIM1_COMPA] void timer1_compa_isr(void){
    LED1 = ~LED1; // invert LED    
} 

unsigned char length = 0, guessWord[100];
unsigned char usedLetters[26];
unsigned char tries_left = 9, guesses_left = 16;

void gameStatus(void){
    //clears the second row, fills with empty spaces
    twi_lcd_clear_row(1);
    
    //writes how many tries are left
    twi_lcd_msg("AI ");
    twi_lcd_letter(tries_left + '0');
    twi_lcd_msg(" INCERCARI");
}

void checkWord(unsigned char letter){
    unsigned char i;  
    bit           correctGuess = 0; 
    //checks is the letter is in the guessWord  
    delay_ms(100);
    for(i = 0; i< length; i++){
        if(guessWord[i] == letter){
            //moves the cursor to the exact position, completes the guessWord
            twi_lcd_setCursor(i,0);
            twi_lcd_letter(guessWord[i]);
            
            //decrement the guesses left
            guesses_left --;
            
            //indicates the letter is in the guessWord at least once 
            correctGuess = 1;  
        }         
    } 
    //if the letter wasn't in the guessWord, decrement tries_left
    if(correctGuess == 0)
        tries_left--;
    delay_ms(100);
}

int endGame(void){
    bit newGame = 0;
    
    //check if the player lost
    if(tries_left == 0){ 
        twi_lcd_clear();
        twi_lcd_msg("AI PIERDUT:");
        twi_lcd_setCursor(0,1);
        twi_lcd_msg(guessWord);
        newGame = 1;   
    }
    //check if player won
    if(guesses_left == 0){ 
        twi_lcd_clear();
        twi_lcd_msg("AI CASTIGAT:");
        twi_lcd_setCursor(0,1);
        twi_lcd_msg(guessWord);
        newGame = 1;  
    }   
    //if the game ended in a loss or win, start new one
    //wait for new line to start new game, doesn't matter how many or what characters are entered
    if(newGame){
        while(1){
            //keep loop alive
            wdogtrig(); 
            //wait for new character from uart and check if it is new line             
            if(rx_counter0)
                if(getchar() == 10)
                    return 1;
        }
    }
}

void inGame(void){
    unsigned char letter, i, index;
    unsigned char buffer_length = 0, buffer[100];
       
    while(1){ 
        //keep loop alive
        wdogtrig();
        //wait for new character from uart
        if(rx_counter0){  
            //get the character
            letter = getchar();
            //check if it is new line
            if(letter == 10){
                //must be exactly one character entered, else ignore whole buffer
                if(buffer_length != 1){  
                    twi_lcd_clear_row(1);
                    
                    twi_lcd_msg("O LITERA DOAR!");  
                    
                    delay_ms(2000);
                    gameStatus();
                }                   
                //check if the character is a capital letter
                else if(buffer[0] >= 'A' && buffer[0] <= 'Z'){                     
                    twi_lcd_clear_row(1);        
                    
                    twi_lcd_letter(buffer[0]);
                    twi_lcd_msg(":  VERIFIC...");
                    
                    delay_ms(1500);    
                    twi_lcd_clear_row(1);
                    
                    index = buffer[0] - 'A';
                    //if letter was used before, ignore it
                    if(usedLetters[index] == 1){
                        twi_lcd_msg("AI MAI PUS ");
                        twi_lcd_letter(buffer[0]);
                        delay_ms(2000);
                        gameStatus();
                    }    
                    else{
                        //verify the letter and mark it was used
                        usedLetters[index] = 1;
                        checkWord(buffer[0]);
                        gameStatus();
                        
                        //check for endGame conditions
                        if(endGame() == 1)
                            break;
                    }                                 
                }
                else{
                    //character wasn't capital letter, ignore it
                    twi_lcd_clear_row(1);
                    twi_lcd_msg("MAJUSCULE!");
                    delay_ms(2000);
                    gameStatus();
                } 
                //reset value
                buffer_length = 0;  
            }
            else{ 
                //character wasn't new line, add to the buffer
                buffer[buffer_length] = letter;
                buffer_length++; 
            }
                    
        }     
    }     
}

void beginGame(void){
    unsigned char letter,i;
    bit           valid = 1;    
    
    twi_lcd_clear();   
    twi_lcd_msg("INTRODU CUVANT:");
    delay_ms(100);      
    
    //reset values
    length       = 0;  
    tries_left   = 9;
    guesses_left = 16;   
    
    while(1){
        //keep loop alive
        wdogtrig(); 
        //wait for new character from uart              
        if(rx_counter0){
            //get the character  
            letter = getchar();
            //check if character is new line, ascii code 10, '\n', LF
            if(letter == 10){ 
                //guessWord must have minimum 3 charcaters
                if(length < 3){
                    length = 0; 
                    twi_lcd_clear();
                    twi_lcd_msg("MIN 3 LITERE");
                    twi_lcd_setCursor(0,1);
                    twi_lcd_msg("INTRODU CUVANT:");
                } 
                //guessWord must have maximum 15 charcaters
                else if(length > 15){
                    length = 0; 
                    twi_lcd_clear();
                    twi_lcd_msg("MAX 16 LITERE");
                    twi_lcd_setCursor(0,1);
                    twi_lcd_msg("INTRODU CUVANT:");
                }
                else{
                    //asume all the characters are valid before check 
                    valid = 1;
                    for(i = 0; i < length; i++)
                        //cracter must be capital letter, else error
                        if(guessWord[i] < 'A' || guessWord[i] > 'Z'){ 
                            valid  = 0;
                            length = 0;  
                            twi_lcd_clear();
                            twi_lcd_msg("DOAR MAJUSCULE");
                            twi_lcd_setCursor(0,1);
                            twi_lcd_msg("INTRODU CUVANT:");
                        }
                    if(valid == 1) 
                        //all characters were capital letters, break the loop  
                        break;
                }
            }
            else{ 
                //character wasn't new line, add it to the buffer
                guessWord[length] = letter;
                length++; 
            }
                    
        }     
    }
    //set the number of guesses needed to win = length of the guessWord
    guesses_left = length;
    
    //make sure a NULL charcater follows the word in the buffer
    guessWord[length] = 0;
    
    //clear the usedLetters vector, 0 - unused letter, 1 - used letter  
    for(i = 0; i < 26; i++)
        usedLetters[i] = 0;
    
    //print '_' * length times, to hide the charactetrs    
    twi_lcd_clear();
    for(i = 0; i < length; i++)
        twi_lcd_letter('_'); 
    
    //prepare game, autocomplete first and last letter
    //guessWord[0] - 'A'  => converts character in index, ex: 'A' - 'A' = 0; 'D' - 'A' = 4
    
    checkWord(guessWord[0]);
    usedLetters[guessWord[0] - 'A'] = 1;
    
    //it is possible that first and last letters are the same, check fot this case
    if(usedLetters[guessWord[length-1] - 'A'] == 0){
        checkWord(guessWord[length-1]);
        usedLetters[guessWord[length-1] - 'A'] = 1;
    }
    //used to display tries_left
    gameStatus();
    
    //move to in game actions
    inGame();   
}

void Game(void){
    while(1){ 
        wdogtrig();
        beginGame();
    }
}

void main(void){
    int i, r;
    unsigned char temp;
    
    
    #asm("sei")                 // enable interrupts
    Init_initController();      // Iitiez microcontrolerul                            
    delay_ms(100);
    LED1    = 1;
                   
    my_i2c_init();              // Initiez protocolul i2c
    twi_lcd_init();	            // Initiez lcd-ul pentru afisare 
    
    twi_lcd_msg("HELLO WORLD");
    delay_ms(2000);
    twi_lcd_clear();  
    
    twi_lcd_msg("SPANZURATOAREA");
    delay_ms(2000);
    twi_lcd_clear();
    
    Game();                      // begin the game 
}