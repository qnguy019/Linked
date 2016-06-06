#Linked
name1 = Quynh Nguyen
ucrnetid = qnguy019
Google Drive Folder: http://bit.ly/1U7XFjF
##Introduction
Linked is a two-player cooperative game that involves solving mini games to keep your partner alive. The objective of the game is to survive as long as possible.

##How To
There will be five mini games that each player will play (4 individual and 1 cooperative). Players will continuously rotate through the four individual mini games. After every minute, the players will be interrupted and play the cooperative minigame. Then the cycle continues, with the difficulty of the mini games increasing..
As time progresses, the player’s life will decrease. The only way to increase your life is for your partner to win their minigame. A player fails the minigame either by putting in the incorrect answer or the minigame time duration running out. If that is the case, then their partner will not receive any life. 
The mini games become harder over time by either decreasing the amount of time to input an answer or increasing a sequence so it will be harder to memorize. The player’s life will also decrease faster as time passes. 
For joystick input, you have to move in the direction that you want to go and press A to confirm your choice. The other instructions for each mini game are given during the game. 
Assignment 1: Single commands, Multi commands, Exit command, Comments

##File Information
#####src
Contains: 5110.c, 5110.h, timeout.h, GameHandler.h, HealthHandler.h, LCD_Pictures.h, MiniGameHandler.h, TimerHandler.h, io.c, io.h, main.c, miniGames.h, timeout.h, usart.h

main.c: runs the main game. cycles through the task scheduler.

5110.c, 5110.h, timerout.h: files needed to output text onto the LCD screen. Not my original code. (C)2013 Radu Motisan, www.pocketmagic.net, Based on a work by Tony Myatt - 2007

GameHandler.h: controls the main flow of the game, including the menu, game start, and end game stats.

TimerHandler.h: keeps track of how much time has passed and increases the difficulty of the game depending on how much time has passed.

HealthHandler.h: controls the logic behind the players' health. Deals with decreasing it over time and increasing health when the other player wins their minigame. 

MiniGameHandler.h: keeps track of all the mini games and when to play them.

miniGames.h: contains all the state machines of each mini game.

LCD_Pictures.h: contains all the bitmap arrays of the pictures I created. I made the bitmaps in Photoshop and converted it to an array using the LCD Assistant program 

usart.h: contains the code needed for USART communication between the two microcontrollers. Not my original code. Given to me by UCR CS120B instructors.

io.c, io.h: mainly used this file for the delay_ms(int miliSec) function. Not my original code. Give to me by UCR CS120B instructors.

##Bugs

- In the first page of the Instructions, pressing B to go back to the previous page will show nothing.
- The program will occasionally skip the instruction part of the mini game and go straight to the answer state
- The program will occasionally show that the player has failed a mini game before they even started it
- Sometimes the timer will be off between both players and one player will start the maze a second after the other player.

If you have any questions about my project, you can email me at qnguy019@ucr.edu.


