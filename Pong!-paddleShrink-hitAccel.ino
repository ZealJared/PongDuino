#include "U8glib.h"

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST); // initialize u8g library for our display

  long startTime = 0; // millis() must be greater than this to run program (hard pause)
  long pauseAfter = 0; // program will pause for 1000ms when millis() reaches this number
  

  float x = 128 / 2; // ball X coord (center default)
  float y = 64 / 2; // ball Y coord (center default)
  int dir[] = {-1, 1}; // array to hold ball directions
  float dx = 0; // ball horizontal direction
  float dy = 1; // ball vertical direction
  int br = 4; // ball radius
  
  float vprime = 2; // default velocity
  float v = vprime; // active velocity (initialized to default)
  float dv = 0.1; // delta velocity (rate of ball accelleration after paddle hit)
  float mv = 10; // max velocity

  float psv = 2; // paddle size variance (subtracted from paddle size on ball hit)
  int psm = 3; // paddle size minimum (paddle wont get smaller than this)
  
  float p1s = 50; // paddle 1 size (size of player 1's paddle) initialized to default
  float p1y = 15; // paddle 1 Y coord (vertical position of player 1's paddle)

  float p2s = 50; // paddle 2 size (size of player 2's paddle) initialized to default
  float p2y = 15; // paddle 2 Y coord (vertical position of player 2's paddle)

  int p1score = 0; // player 1's score
  int p2score = 0; // player 2's score

void draw(void) {
  u8g.drawHLine(0, 0, 128);
  u8g.drawHLine(0, 63, 128);
  // set score keeping font
  u8g.setFont(u8g_font_5x7);
  // move text input position
  u8g.setPrintPos(10, 9);
  // print player 1's score
  u8g.print(p1score);
  // move text input position
  u8g.setPrintPos(100, 9);
  // print player 2's score
  u8g.print(p2score);
  // draw paddle for player 1
  u8g.drawBox(0, round(p1y), 2, round(p1s));
  // draw paddle for player 2
  u8g.drawBox(128 - 2, round(p2y), 2, round(p2s));
  // draw ball
  u8g.drawDisc(round(x), round(y), br);
}

void setup(void) {
  Serial.begin(9600);
  // randomSeed is neccessary to get varying random numbers each time program starts
  int rand = analogRead(0);
  randomSeed(rand);
  // get ball horizontal direction from direction array
  dx = dir[random(2)];
  // enable pins for indicator LEDs
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
}

void loop(void) {
  // if the time has come to start
  if(millis() > startTime){
    // if players' paddles are too small, set them to the paddle size minimum
    if(p1s < psm){
      p1s = psm;
    }
    if(p2s < psm){
      p2s = psm;
    }
    //p1s =  map(analogRead(4), 0, 1023, 10, 50); // get paddle size for player 1 from pot on A4
    //p2s =  map(analogRead(5), 0, 1023, 10, 50); // get paddle size for player 2 from pot on A5
    //dv = map(analogRead(3), 0, 1023, 1, 10); // get acceleration value from pot on A3
    //dv = dv / 10; // convert acceleration to appropriate float value
    p1y = map(analogRead(2), 0, 1023, 0, 64 - round(p1s)); // get player 1 paddle vertical position from pot on A2
    p2y = map(analogRead(1), 0, 1023, 0, 64 - round(p2s)); // get player 2 paddle vertical position from pot on A1
    // u8g picture loop
    u8g.firstPage();
    do {
      draw();
    } while( u8g.nextPage() );
  
    // startTime = millis() + (1000/60); // delay next draw to reduce framerate
    x+=dx*v; // adjust ball horizontal position by direction X * velocity
    y+=dy*v; // adjust ball vertical position by direction Y * velocity
    if(x <= 2 + br){ // ball is less than (ball radius + paddle width) from left edge
      if(y <= p1y + round(p1s) + br && y >= p1y - br){ // ball is colliding with paddle
        dx = 1; // switch ball direction
        v += dv; // accelerate velocity
        p1s -= psv; // decrease paddle size
        Serial.println("Player 1 Hit!");
      } else { // ball misses paddle
        Serial.println("Player 1 Miss!");
        p2score += 1; // player 2 scores
        digitalWrite(12, HIGH); // indicate with LED's
        digitalWrite(11, LOW);
        reset(1); // reset ball position to center
      }
    }
    if(x >= (128 - br) - 2){ // if ball is closer to right edge than (paddle width + ball radius)
      if(y <= p2y + round(p2s) + br && y >= p2y - br){ // ball is colliding with paddle
        dx = -1; // switch horizontal ball direction
        v += dv; // accelerate velocity
        p2s -= psv; // shrink paddle
        Serial.println("Player 2 Hit!");
      } else { // ball misses
        Serial.println("Player 2 Miss!");
        p1score += 1; // player 1 gets a point
        digitalWrite(11, HIGH); // indicate with LED's
        digitalWrite(12, LOW);
        reset(-1); // reset ball position
      }
    }
    if(y <= br || y >= 64 - br){ // if center of ball is less than (ball radius) from top or bottom edge of screen
      dy *= -1; // switch vertical direction
    }
    if(v >= mv){ v = mv; } // if ball is going too fast, set velocity to max velocity
    if(pauseAfter > 0 && millis() > pauseAfter){ // if pauseAfter is set, and we have reached the time to pause after
      startTime = millis() + 1000; // delay program for 1000ms
      pauseAfter = 0; // reset pauseAfter
    }
  }
}

void reset(int direct){ // reset ball with direction toward point scorer
  float ytweak = random(1, 10); // randomly tweak ball vertical speed and direction to adjust ball flight angle
  dy = (ytweak / 11) * dir[random(2)];
  v = vprime; // reset ball velocity
  dx = direct; // direct ball horizontally toward point scorer
  x = 128 / 2; // center ball horizontally
  y = 64 / 2; // center ball vertically
  startTime = millis() + 500; // pause for 500ms (before ball reset is drawn on screen)
  pauseAfter = millis() + 520; // pause for 1000ms after 520ms (after ball reset is drawn on screen)
}


