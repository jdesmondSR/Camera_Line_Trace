#include "RobotSetup.hpp"
using namespace srbots;

OLEDDisplay oled(port0);
ColorDetector color(port0);


void setup() {
  RobotSetup();
  color.init();
  oled.init();

  oled.clear();
  oled.drawtext(0, "press 20");
  oled.updateDisplay();
  pinMode(20, INPUT);
  while(digitalRead(20)) delay(20);

}

int white = 1200, black = 450;
int threshold = (white + black) / 2;
void loop() {
  if(color.colorReady()){
    uint16_t r, g, b, c;
    color.getColors(r, g, b, c);
    oled.clear();
    oled.drawtext(0, (uint32_t)c);
    oled.updateDisplay();

    if(c < threshold){
      rightmotor.speed(50);
      leftmotor.speed(-10);
    }
    else {
      rightmotor.speed(-10);
      leftmotor.speed(50);
    }
  }

}
