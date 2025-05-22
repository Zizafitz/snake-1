#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <chrono>
#include <thread>

/*
 * Simple Snake Game for the Terminal
 * ----------------------------------
 * This program implements a very basic snake game that runs in a text
 * terminal. It demonstrates how you can create a simple game loop in C++
 * and react to user input without relying on complex external libraries.
 * The code is heavily commented so that beginners can follow along.
 */

//---------------------------------------------------------------------------
// Helper functions for raw keyboard input
//---------------------------------------------------------------------------

// When the terminal is in "raw" mode we can read key presses without
// waiting for the user to hit Enter. This is necessary for real-time games.
void setRaw(bool enable) {
    static struct termios oldt;   // stores the original terminal settings
    struct termios newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);   // save current terminal settings
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // disable canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt); // apply new settings
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restore saved settings
    }
}

// Returns true if a key has been pressed (non-blocking)
bool kbhit() {
    int bytesWaiting;
    ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
    return bytesWaiting > 0;
}

//---------------------------------------------------------------------------
// Main game
//---------------------------------------------------------------------------

int main() {
    // Dimensions of the playing field
    const int width = 20;
    const int height = 20;

    // The snake is represented as a list of coordinates.
    // The first element is always the current head position.
    std::vector<std::pair<int,int>> snake;
    snake.push_back({width/2, height/2}); // start roughly in the middle

    // Direction: 1=left, 2=right, 3=up, 4=down
    int dir = 2; // start moving right

    // Random number setup for the fruit position
    std::srand(std::time(nullptr));
    std::pair<int,int> fruit = {std::rand()%width, std::rand()%height};

    bool gameOver = false;

    // Put the terminal into raw mode so we can capture key presses
    setRaw(true);

    while(!gameOver) {
        //-------------------------------------------------------------------
        // Draw the board
        //-------------------------------------------------------------------
        std::cout << "\033[H\033[2J"; // Clear screen using escape codes
        for(int y=0; y<height; ++y) {
            for(int x=0; x<width; ++x) {
                if(x==0 || x==width-1 || y==0 || y==height-1) {
                    std::cout << '#'; // draw border
                } else if(x==fruit.first && y==fruit.second) {
                    std::cout << 'F'; // draw fruit
                } else {
                    bool printed = false;
                    for(auto &s : snake) {
                        if(s.first == x && s.second == y) {
                            std::cout << 'O'; // draw snake segment
                            printed = true;
                            break;
                        }
                    }
                    if(!printed) std::cout << ' ';
                }
            }
            std::cout << '\n';
        }
        std::cout << "Use arrow keys to move. Ctrl+C to quit.\n";

        //-------------------------------------------------------------------
        // Handle user input
        //-------------------------------------------------------------------
        if(kbhit()) {
            char c;
            if(read(STDIN_FILENO, &c, 1) > 0) {
                if(c == '\033') {
                    char seq[2];
                    if(read(STDIN_FILENO, seq, 2) == 2 && seq[0]=='[') {
                        switch(seq[1]) {
                            case 'A': dir = 3; break; // up
                            case 'B': dir = 4; break; // down
                            case 'C': dir = 2; break; // right
                            case 'D': dir = 1; break; // left
                        }
                    }
                }
            }
        }

        //-------------------------------------------------------------------
        // Update the snake position
        //-------------------------------------------------------------------
        std::pair<int,int> head = snake.front();
        switch(dir) {
            case 1: head.first--;  break; // left
            case 2: head.first++;  break; // right
            case 3: head.second--; break; // up
            case 4: head.second++; break; // down
        }
        // Insert new head at the beginning of the vector
        snake.insert(snake.begin(), head);

        //-------------------------------------------------------------------
        // Collision detection
        //-------------------------------------------------------------------
        // Check if the snake hit the wall
        if(head.first <= 0 || head.first >= width-1 ||
           head.second <= 0 || head.second >= height-1)
            gameOver = true;

        // Check if the snake hit itself
        for(size_t i = 1; i < snake.size(); ++i)
            if(snake[i] == head) gameOver = true;

        // Check if fruit is eaten
        if(head == fruit) {
            fruit = {std::rand()%width, std::rand()%height};
        } else {
            // remove the tail if no fruit eaten
            snake.pop_back();
        }

        //-------------------------------------------------------------------
        // Delay to control game speed
        //-------------------------------------------------------------------
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Restore the terminal before exiting
    setRaw(false);
    std::cout << "Game Over!\n";
    return 0;
}

