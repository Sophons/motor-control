/**
 * File: Driver.cpp
 */
#include <signal.h>
#include <rc/time.h>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <curses.h>

#include "Driver.hpp"
#include "Encoder.hpp"
#include "PID.hpp"
#include "library/gpio.h"
#include "library/pwm.h"
#include "library/util.h"
using namespace std;

#define STEP 1

static int running = 0;

/**
 * Handle Ctrl-C interrupt
 */
static void __signal_handler(__attribute__ ((unused)) int dummy) {
  running = 0;
  return;
}

/**
 * 
 */
void setup(void) {
  // Curses initialization
  initscr(); // start curses
  cbreak();  // react to keys instantly w/o Enter
  noecho();  // don't echo input to screen
  keypad(stdscr, TRUE);  // register arrow keys
  nodelay(stdscr, TRUE); // don't wait for keyboard input 
}

/**
 * Main driver of the motor control for the snake.
 */
int main() {
  double value = 0.0;   // angle we want motor to spin to
  char buf[20];    // buffer to print angle and position vals

  // Register keyboard interrupt
  signal(SIGINT, __signal_handler);
  running = 1;

  // Set up environment
  setup();

  // Encoder setup
  //shared_ptr<Encoder> enc1(new Encoder(2, 0x40));
  shared_ptr<Encoder> enc2(new Encoder(2, enc_addr[0]));
  //shared_ptr<Encoder> enc3(new Encoder(3, 0x41));

  // PID setup
  shared_ptr<PID> pid1(new PID(0.08, 0.0, 0.0, enc2));
  
  mvaddstr(0, 1, "Angle: 0");
  mvaddstr(1, 1, "Position: 0");

  // Main control loop
  while (running) {
    // Keyboard input
    int ch = getch();
    if (ch == KEY_LEFT) {
      value -= STEP;
    }
    else if (ch == KEY_RIGHT) {
      value += STEP;
    }
    else if (ch == 'q') {
      running = 0;
    }

    clear(); // refresh the terminal window

    // Exit the control loop if no longer reading encoder.
    if (enc2->calcRotation() == -1) {
      running = 0;
      continue;
    }

    double angle = enc2->getAngle();
    pid1.updatePWM(&pwm3, &dir3, value, true);

    sprintf(buf, "Angle: %d", value);
    mvaddstr(0, 1, buf);
    
    memset(buf, '\0', sizeof(char));
    sprintf(buf, "Position: %f", angle);
    mvaddstr(1, 1, buf);

    sleep(DT);
  }

  endwin();  // restore terminal from curses

  return 0;
}
