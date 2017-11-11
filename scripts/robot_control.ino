#include "Servo.h"
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
#include <avr/power.h>
#endif
//pin that the neopixel data line is connected to
#define PIN 6
//line declares the neopixel object, first number is the number of pixels (12 for jewel)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);
Servo Head;
Servo Base;
//variable for value of the tilt sensor
int tiltreading;
//also for the tilt sensor
int previous = LOW;
// the last time the output pin was toggled
int tiltstate;
//time since last event
long time = 0;
//amount of milliseconds used for debouncing the tilt sensor
long debounce = 100;
//final tilt reading

// Flag for beep
int beepFlag = 0;
int melody[][8] = {
  {262, 196, 196, 220, 196, 0, 247, 262},
  {659, 659, 659, 523, 659, 196, 0, 0}
};
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

// Defining Pins
int event = 0;
int buzzer = 13;
int tiltSensor = 5;

//accelerometer
const int xInput = A3;
const int yInput = A4;
//const int zInput = A1;


//Raw Ranges: X: 492-531, Y: 492-513, Z: 495-534
//512, 513, 534 :: 0.03G, 1.00G, 1.00G




int xRawMin = 492;
int xRawMax = 590;
 
int yRawMin = 492;
int yRawMax = 567;
 
int zRawMin = 495;
int zRawMax = 534;

// Take multiple samples to reduce noise in acceleration
const int sampleSize = 10;

// acc = 1 for slow, 2 for fast, 0 for still
int acc = 0;
int countFast = 0;
int countSlow = 0;

void setup() {
  Serial.begin(9600);
  //Tilt Related//
  //input for tilt sensor
  pinMode(tiltSensor, INPUT_PULLUP);
  //attach interrupt for tilt
  attachInterrupt(digitalPinToInterrupt(tiltSensor), tilt, CHANGE);

  //get initial tilt state
  // Vertical = 0
  // Horizontal = 1
  tiltstate = digitalRead(tiltSensor);

  //Output for piezo buzzer
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);

  //begin neopixel library
  strip.begin();
  //turns all pixels off
  strip.show();

  // attaches the servos to pin 9/10
  Head.attach(9);
  Base.attach(10);
  //initialize both servos to middle
  Head.write(90);
  Base.write(90);

  // Set the modes of each pin for the capacitance meter,
  // the output pins are plugged into pins A0,A1,A2 so they are set to input w/ a pullup resistor
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(A1), right, FALLING);
  attachInterrupt(digitalPinToInterrupt(A0), center, FALLING);
  attachInterrupt(digitalPinToInterrupt(A2), left, FALLING);

  // Initially in breathing state
  breathe();

}

void loop() {
  tiltstate = digitalRead(tiltSensor);
  // Title state low imples verticle: show
  if (tiltstate == LOW) {
    Head.write(180);
    event = millis();
  }
  // Verticke position, thus hide
  if (tiltstate == HIGH) {
    Head.write(0);
    event = millis();
  }
  if (beepFlag > 0) {
    beep(beepFlag);
    beepFlag = 0;
  }

  // If nothing has happened for 1 minute , beep.
  if ((millis() - event) > 60000) {
    beep(1);
    event = millis();
  }

  float xRaw = ReadAxis(xInput);
  float yRaw = ReadAxis(yInput);
//  float x = map(xRaw, xRawMin, xRawMax, -16.0, 16.0);
//  float y = map(yRaw, yRawMin, yRawMax, -16.0, 16.0);
  float x = min(abs(xRaw-xRawMin), abs(xRaw-xRawMax));
  float y = min(abs(yRaw-yRawMin), abs(yRaw-yRawMax));
  Serial.println(x);
  Serial.println(y);
  Serial.println("Raw:");
  Serial.println(xRaw);
  Serial.println(yRaw);
  // if value changes by more than 5 units, its a fast motion
  if((abs(x)>6) or (abs(y)>6)){
    Serial.println("fast");
    acc = 2;
    speedAction(acc);
  }
  else{
    // Considering 2 units for noise, if value changes by 2 units, it's a slow motion.
    if((abs(x) > 2) or (abs(y) > 2)){
      Serial.println("slow");
      acc = 1;
      speedAction(acc);
    }
  }

  breathe();

}

void tilt() {
  tiltreading = digitalRead(tiltSensor);
  time = millis();
}

//Function to be executed for centre/random action
void randomAction() {
  // defining 9 random behaviours
  strip.setBrightness(255);
  int opt = random(9);
  byte color = random(255);
  switch (opt) {
    case 0:
      colorWipe(strip.Color(255, 255, 255), 50);
      beepFlag = 1;
      break;
    case 1:
      colorWipe(strip.Color(255, 0, 0), 50);
      break;
    case 2:
      colorWipe(strip.Color(255, 0, 0), 50);
      beepFlag = 2;
      break;
    case 3:
      colorWipe(strip.Color(255, 255, 255), 50);
      beepFlag = 2;
      break;
    case 4:
      colorWipe(strip.Color(255, 255, 0), 50);
      break;
    case 5:
      colorWipe(strip.Color(255, 255, 0), 50);
      beepFlag = 2;
      break;
    case 6:
      colorWipe(strip.Color(255, 255, 255), 50);
      beepFlag = 1;
      break;
    case 7:
      colorWipe(strip.Color(0, 0, 255), 50);
      break;
    case 8:
      colorWipe(strip.Color(0, 0, 255), 50);
      beepFlag = 1;
      break;
    default:
      break;
  }
}

//Function to be executed with speed change
void speedAction(int acc) {
  event = millis();
  // defining 9 random behaviours
  strip.setBrightness(255);
  int opt = random(3);
  // Slow speed o/p
  if (acc==1 and countSlow>5){
    countSlow = 0;
    acc = 2;
  }
  else{
    if (acc==2 and countFast>5){
      countFast = 0;
      acc=1;
    }
  }
  if (acc==1){
    countSlow = countSlow + 1;
    switch (opt) {
      case 0:
        buzz(1);
        break;
      case 2:
        center();
        break;
      case 3:
      {
        int head_reading = Head.read();
        if (head_reading == 0 or head_reading == 90){
          Head.write(180);
        }
        else{
          Head.write(0);
        }
        break;
      }
      default:
        Head.write(180);
        delay(500);
        Head.write(0);
        delay(500);
        Head.write(180);
        delay(500);
        Head.write(0);
        delay(500);
        Head.write(180);
        delay(500);
        Head.write(0);
        break;
    }
  }
  // Fast speed o/p
  if (acc==2){
    countFast = countFast + 1;
    int opt = random(4);

    switch (opt) {
      case 0:
        {
        for(int i=0; i<10; i++){
          byte color = random(255);
          colorWipe(Wheel(color), 50);
          delay(50);
        }
        break;
      }
      case 1:
        buzz(3);
        break;
      case 2:
        blink();
        buzz(3);
        break;
      case 3:
        left();
        delay(500);
        right();
        delay(500);
        left();
        delay(50);
        right();
        delay(500);
        left();
        delay(50);
        right();
        break;
      case 4:
        Head.write(180);
        delay(500);
        Head.write(0);
        delay(500);
        Head.write(180);
        delay(500);
        Head.write(0);
        delay(500);
        Head.write(180);
        delay(500);
        Head.write(0);
        break;
      default:
        break;
    }  
  }
}

// Base trun right
void right() {
  Serial.print("right");
  Base.write(0);
  event = millis();
}

// Base turn centre
void center() {
  Serial.print("centre");
  Serial.print(Base.read());
  Base.write(90);
  randomAction();
  event = millis();
}

// Base turn left
void left() {
  Serial.print("left");
  Base.write(180);
  event = millis();
}

//Helper functions to set color for neopixle

// This function turns all the pixels one color.
// For info on the formatting see the neopixel documentation on adafruit
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    //delay(50);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Beep Function
void beep(int flag) {
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(buzzer, melody[flag - 1][thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(buzzer);
  }
}

void buzz(int n){
  for(int i=0; i<n; i++){  //i<3 for 3 beeps
    time=millis();//reset time
    while((millis()-time)<200){//this ensures that each beep is only 200ms
     digitalWrite(buzzer,HIGH);//these lines switch the value of the pin forming a ssquare wave of freq 5kHz
     delayMicroseconds(100);//if the delay is 100 microseconds each, change the delay time to have different sounds   
     digitalWrite(buzzer,LOW);
     delayMicroseconds(100);  
    }
    delay(50);
  //time between beeps
  }
  digitalWrite(buzzer,LOW);//turn the pin off
}

void breathe() {
  colorWipe(strip.Color(255, 255, 255), 50);
  for (int i = 10; i < 255; i++) {
    delay(5);
    strip.setBrightness(i);
    strip.show();
    delay(5);
  }
  for (int i = 255; i > 10; i--) {
    delay(5);
    strip.setBrightness(i);
    strip.show();
    delay(5);
  }
}

void blink(){
  uint16_t i, j; 
  j = random(256);
  for(i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
  }
  strip.show();
  delay(50);
}


// Read "sampleSize" samples and report the average
int ReadAxis(int axisPin)
{
  long reading = 0;
  analogRead(axisPin);
  delay(1);
  for (int i = 0; i < sampleSize; i++)
  {
    reading += analogRead(axisPin);
  }
  return reading/sampleSize;
}
