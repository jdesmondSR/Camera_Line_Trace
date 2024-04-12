// for the oled
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// for color sensor
#include "Adafruit_APDS9960.h"

// color sensor
Adafruit_APDS9960 apds;

// oled object and print function
Adafruit_SSD1306 display(128, 64, &Wire, -1);
void print(int line_number, char s[]) {
  display.setCursor(0, line_number * 8);
  display.print(s);
}


// map function that constrains bad input
// value, origional range, modified range
int cmap(int val, int olow, int ohigh, int mlow, int mhigh) {
  return constrain(map(val, olow, ohigh, mlow, mhigh), mlow, mhigh);
}

// struct to make motor control easier
struct Motor {
  uint8_t fpin;         //fwd pin
  uint8_t rpin;         //reverse pin
  void speed(int val);  //function setting speed
} leftmotor{ 8, 9 }, rightmotor{ 10, 11 };

//} leftmotor{ 10, 11 }, rightmotor{ 8, 9 };  // init right away

// speed function for setting motor speed
void Motor::speed(int val) {
  int map_speed = cmap(abs(val), 0, 100, 0, 255);
  // check if its forward or reverse
  if (val > 0) {
    analogWrite(fpin, map_speed);
    analogWrite(rpin, 0);
  } else {
    analogWrite(fpin, 0);
    analogWrite(rpin, map_speed);
  }
}


void serial_flush() {
  while (Serial1.available() > 0) {
    Serial1.read();
  }
}





void setup() {
  Serial.begin(9600);
  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();
  //delay(2000);

  // Serial1 is UART0:
  Serial1.setRX(17);         // UART0 RX GP1
  Serial1.setTX(16);         // UART0 TX GP1
  Serial1.setFIFOSize(128);  // optional... for larger messages
  Serial1.begin(115200);

  for (int i = 8; i <= 11; i++)
    pinMode(i, OUTPUT);

  //Serial.begin(9600);  // This is the Serial USB (send to serial monitor)
  delay(2000);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ;  // Don't proceed, loop forever
  }
  Serial.println("OLED fine");
  // start by clearing the buffer
  display.clearDisplay();
  display.setTextSize(1);  // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);

  // color sensor setup
  if (apds.begin(30, APDS9960_AGAIN_4X) == 0) {
    Serial.println("APDS allocation failed");
    while (1) delay(10);
  }
  // enable sensor
  apds.enableColor(true);


  char buf[40];
  sprintf(buf, "press gp20 to begin");
  display.clearDisplay();
  print(0, buf);
  display.display();

  pinMode(20, INPUT);
  pinMode(21, INPUT);
  while (digitalRead(20)) delay(20);

  // leftmotor.speed(10);
  // rightmotor.speed(0);
  // delay(2000);
  // leftmotor.speed(0);
  // rightmotor.speed(10);
  // delay(2000);
  // leftmotor.speed(-10);
  // rightmotor.speed(0);
  // delay(2000);
  // leftmotor.speed(0);
  // rightmotor.speed(-10);
  // delay(2000);


  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("STARTING");
  display.display();


  //while(!Serial);                      // wait until Serial Monitor connects

  //Serial.println("Enter some text:");  // Prompt user for some text
  //while (!Serial.available()){}        // wait for it

  //while (Serial.available()){          // Get the text byte by byte and send to Raspberry PI
  //  Serial1.print((char) Serial.read());
  //}

  Serial1.print('s');
}

int motor_status = 1;
int silver_detect = 0;

void loop() {
  /*
  while(Serial1.available()){              // if PI  responded
    char received_val = Serial1.read(); // get byte  
    Serial.print(received_val);         // print out byte in Serial Monitor
    Serial1.print(received_val);        // send byte back to PI
  }
  delay(10);                           // let's slow things down so we humans can read it 
  */

  // read in the motor speed commands
  char buf[100];
  if (Serial1.available()) {

    /*
    delay(10);
    Serial.print(".available -> ");
    Serial.println(Serial1.available());
    int i = 0;
    */

    // READING IT THE OLD FASHIONED WAY
    /*
    while (Serial1.available()) {
      Serial.print("inside loop .available -> ");
      Serial.println(Serial1.available());
      buf[i] = Serial1.read();
      i++;
    }
    buf[i] = '\0';

    Serial.print("Size of buf: ");
    Serial.println(strlen(buf));
    Serial.print("Received String: ");
    Serial.println(buf);

    */

    // READ STRING UNTIL METHOD
    //String s = Serial.readStringUntil('x');


    // Parse int method
    /*
    int values[4];
    for(int i = 0; i < 4; i++){
      values[i] = Serial1.parseInt();
    }

    Serial.print("Parsed integers: ");
    for(int i = 0; i < 4; i++){
      Serial.print(values[i]);
      Serial.print(" ");
    }
    */



    // PARSE UNTIL REACH '['
    //char ch;
    //while((ch = Serial1.read()) != '[') delay(2);
    Serial.print("Serial available: ");
    Serial.println(Serial1.available());
    int values[2];
    for (int i = 0; i < 2; i++) {
      values[i] = Serial1.parseInt();
    }
    Serial.printf("Left: %3d\tRight: %3d\n", values[0], values[1]);

    // LAZY METHOD
    while (Serial1.available()) {
      //Serial.println("DATA STILL AVAILABLE AFTER THE PARSE INTS...");
      Serial1.read();
    }

    if (silver_detect == 0) {
      display.clearDisplay();
      sprintf(buf, "L: %4d", values[0]);
      print(0, buf);
      sprintf(buf, "R: %4d", values[1]);
      print(2, buf);
      display.display();
    }

    if (digitalRead(21) == 0) {
      // button pressed
      motor_status = !motor_status;
      while (digitalRead(21) == 0) delay(20);  // delay while pressed down;
    }

    if(digitalRead(20) == 0 && silver_detect == 1){
      silver_detect = 0;
    }


    uint16_t r, g, b, c;
    apds.getColorData(&r, &g, &b, &c);
    if (c > 4000) {
      // silver detected
      silver_detect = 1;
      display.clearDisplay();
      sprintf(buf, "DONE SILVER");
      print(2, buf);
      display.display();
    }

    if (motor_status == 1 && silver_detect == 0) {
      leftmotor.speed(values[0]);
      rightmotor.speed(values[1]);
    } else {
      leftmotor.speed(0);
      rightmotor.speed(0);
    }


    //Serial.println("\n");
  }
}