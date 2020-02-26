// By Euan Traynor, 2020

//References: 
// 1) https://www.youtube.com/watch?v=drU3dQa-8CE
// 2) https://www.youtube.com/watch?v=OleCp_TAXC8
// 3) https://www.youtube.com/watch?v=FxaTDvs34mM

//Libraries imported:
// 1) https://github.com/adafruit/RTClib
// 2) Python: pyserial

//The first main part of my script imports libraries, sets up variables and selects pin no.
#include <Wire.h>
#include <RTClib.h>
#include <string.h>

//gets level percentage of bin usage at any current given time (pin setup).
int ultra_1_trigPin = 6;    // Trigger
int ultra_1_echoPin = 5;    // Echo
int ultra_2_trigPin = 8;    // Trigger
int ultra_2_echoPin = 10;    // Echo
long duration1;
int distance1;
long duration2;
int distance2;
long final_distance;


//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 10;       
 
//the time when the sensor outputs a low impulse
long unsigned int lowIn;        
 
//the amount of milliseconds the sensor has to be low
//before we assume all motion has stopped
long unsigned int pause = 200; 
 
boolean lockLow = true;
boolean takeLowTime; 

int pir = 9;
int state = LOW;
int val = 0;

long full_capacity = 30; //change to value of bin height
long width_of_bin = 25; //change to value of bin width

long percentage_red = full_capacity*0.30;    //15 units (CHANGE!!!)
long percentage_orange = full_capacity*0.60; //30 units (CHANGE!!!)
//if greater than 60%, show red LED
int item_counter;

int led_green = 13;
int led_orange = 12;
int led_red = 11;
int led_alert = 2;

String bin_is_full = "false";
String bin_is_empty = "false";
const int buzzer = 3;

const int buttonPin = 4;
int buttonState = 0; 

String current_date, current_day;
String current_time;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String final_date_and_time;

//This part of my code sets up and enables the pins for data transfer.
//It also allows for time to calibrate the pir (motion) sensor
RTC_DS1307 rtc;
void setup() {
  //Serial Port begin
  Serial.begin (9600);
  //Define inputs and outputs
  pinMode(ultra_1_trigPin, OUTPUT);
  pinMode(ultra_1_echoPin, INPUT);
  pinMode(ultra_2_trigPin, OUTPUT);
  pinMode(ultra_2_echoPin, INPUT);
  pinMode(pir, INPUT);
  pinMode(led_green, OUTPUT);
  pinMode(led_orange, OUTPUT);
  pinMode(led_red, OUTPUT);
  pinMode(led_alert, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(buttonPin, INPUT);

  if (!rtc.begin()) {
    Serial.println("RTC NOT WORKING");
    //close()
    while(true);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  //give the pir sensor some time to calibrate and warm up
  Serial.println("calibrating pir sensor (10 seconds)");
  for(int i = 0; i < calibrationTime; i++){
    delay(1000);
  }
  Serial.println("PIR SENSOR ACTIVE");
  delay(50);
}

//This is my main loop code.
void loop() {

  //This small if statement checks whether the button has been pressed. 
  //If it has, it will turn off the buzzer and LEDs (if activated) as well as shut down the program.
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    digitalWrite(led_green, LOW);
    digitalWrite(led_orange, LOW);
    digitalWrite(led_red, LOW);
    
    digitalWrite(led_alert, HIGH);
    delay(500);
    digitalWrite(led_alert, LOW);
    delay(500);
    digitalWrite(led_alert, HIGH);
    delay(500);
    digitalWrite(led_alert, LOW);
    
    Serial.print("\n -> Program Exit\n");
    delay(100);
    exit(0);
  }

  //This statement checks if the PIR sensor has detected motion.
  //This section also grabs the data (such as level of rubbish, etc.)
  if(digitalRead(pir) == HIGH){
     digitalWrite(led_alert, HIGH);
     if(lockLow){ 
         //makes sure we wait for a transition to LOW before any further output is made:
        lockLow = false;           
        Serial.println("---");
        Serial.print("motion detected at ");
        Serial.print(millis()/1000);
        Serial.println(" sec");
        //         delay(50);

        Serial.print("Item thrown in bin! :)\n");
        item_counter += 1;
        Serial.print((String)"Times bin has been used: " + item_counter + "\n");
    
        delay(700); // delay... otherwise the level ultrasonic sensor may read object as it the rubbish falls.
        
        //get first rubbish level distance
        digitalWrite(ultra_1_trigPin, LOW);
        delayMicroseconds(5);
        digitalWrite(ultra_1_trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(ultra_1_trigPin, LOW);
        duration1 = pulseIn(ultra_1_echoPin, HIGH);
        distance1 = duration1*0.034/2;
        Serial.print("Ultrasonic 1 Distance = ");
        Serial.print(distance1);
        Serial.println(" cm");

        //get second rubbish level distance
        digitalWrite(ultra_2_trigPin, LOW);
        delayMicroseconds(5);
        digitalWrite(ultra_2_trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(ultra_2_trigPin, LOW);
        duration2 = pulseIn(ultra_2_echoPin, HIGH);
        distance2 = duration2*0.034/2;
        Serial.print("Ultrasonic 2 Distance = ");
        Serial.print(distance2);
        Serial.println(" cm");

        //get avergae of distances
        final_distance = (distance2 + distance1) / 2;
        Serial.print("Final Approximate Distance = ");
        Serial.print(final_distance);
        Serial.println(" cm");

        //turn on the LEDs depending on the height of the rubbish
        if (final_distance <= percentage_red) {
          digitalWrite(led_red, HIGH);
          digitalWrite(led_green, LOW);
          digitalWrite(led_orange, LOW);
          bin_is_full = "true";
          bin_is_empty = "false";
        } else if (final_distance <= percentage_orange) {
          digitalWrite(led_orange, HIGH);
          digitalWrite(led_green, LOW);
          digitalWrite(led_red, LOW);
          bin_is_full = "false";
          bin_is_empty = "false";
        } else {
          digitalWrite(led_green, HIGH);
          digitalWrite(led_orange, LOW);
          digitalWrite(led_red, LOW);
          bin_is_full = "false";
          bin_is_empty = "true";
        }
      
        Serial.print("Bin is full: ");
        Serial.println(bin_is_full);
        Serial.print("Bin is empty: ");
        Serial.println(bin_is_empty);

        //This section will retreive the date, time and weekday.
        DateTime now = rtc.now();
        current_date = (String)now.day() + "/" + now.month() + "/" + now.year();
        current_day = (String)daysOfTheWeek[now.dayOfTheWeek()];
        current_time = (String)now.hour() + ":" + now.minute() + ":" + now.second();
        final_date_and_time = (String)current_date + " (" + current_day + ") " + current_time;
        Serial.print("Date: ");
        Serial.println(final_date_and_time);
      
        Serial.print("\n");
      }        
      takeLowTime = true;
   }

   //check if the button hasn't been pushed, and if so,
   //check whether motion from previous recording has finsished.
   if(digitalRead(pir) == LOW){      
     digitalWrite(led_alert, LOW);

     if(takeLowTime){
      lowIn = millis();
      takeLowTime = false;
      }
     if(!lockLow && millis() - lowIn > pause){ 
         //makes sure this block of code is only executed again after
         //a new motion sequence has been detected
         lockLow = true;                       
         Serial.print("motion ended at ");      //output
         Serial.print((millis() - pause)/1000);
         Serial.println(" sec");
//           delay(50);
         }
     }

  //play buzzer if bin is full.
  if (bin_is_full == "true") {
    tone(buzzer, 350);
    delay(100);
    noTone(buzzer);
    delay(10);
  }
  
  //time delay 
  delay(10);

  //The code will now repeat.
}
