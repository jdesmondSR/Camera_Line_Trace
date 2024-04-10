

// map function that constrains bad input
// value, origional range, modified range
int cmap(int val, int olow, int ohigh, int mlow, int mhigh) {
  return constrain(map(val, olow, ohigh, mlow, mhigh), mlow, mhigh);
}

// struct to make motor control easier
struct Motor {
  uint8_t fpin;                             //fwd pin
  uint8_t rpin;                             //reverse pin
  void speed(int val);                      //function setting speed
} leftmotor{ 10, 11 }, rightmotor{ 8, 9 };  // init right away

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


void setup() {
  // Serial1 is UART0:
  Serial1.setRX(17);         // UART0 RX GP1
  Serial1.setTX(16);         // UART0 TX GP1
  Serial1.setFIFOSize(128);  // optional... for larger messages
  Serial1.begin(9600);

  for (int i = 8; i <= 11; i++)
    pinMode(i, OUTPUT);

  //Serial.begin(9600);  // This is the Serial USB (send to serial monitor)
  delay(2000);
  //while(!Serial);                      // wait until Serial Monitor connects

  //Serial.println("Enter some text:");  // Prompt user for some text
  //while (!Serial.available()){}        // wait for it

  //while (Serial.available()){          // Get the text byte by byte and send to Raspberry PI
  //  Serial1.print((char) Serial.read());
  //}
  Serial1.print('s');
}

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

    int values[2];
    for(int i = 0; i < 2; i++){
      values[i] = Serial1.parseInt();
    }
    //Serial.printf("Left: %3d\tRight: %3d\n", values[0], values[1]);


    leftmotor.speed(values[0]);
    rightmotor.speed(values[1]);

    
    //Serial.println("\n");
  }
}