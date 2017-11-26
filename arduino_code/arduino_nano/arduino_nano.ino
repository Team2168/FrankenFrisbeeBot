#include <Servo.h>

//#define SERIAL_DEBUG

typedef struct {
  int input_pin;         //The input pin from the RX
  int output_pin;        //The pin wired to the output device (motor controller/solenoid)
  unsigned long pwm_val; //Value in uS read from the input pin, to be written to the output pin
  Servo servo;           //The servo object associated with this signal
} sig_type;

sig_type signals[] = {                 //ENABLE SIGNAL MUST BE LAST!
  {.input_pin = 7,  .output_pin =  4}, //RX Ch1 - Shooter Speed
  {.input_pin = 8,  .output_pin =  2}, //RX Ch2 - Right DT Motor Speed
  {.input_pin = 9,  .output_pin =  3}, //RX Ch3 - Left DT Motor Speed
  {.input_pin = 10, .output_pin =  5}, //RX Ch4 - Raise Solenoid
  {.input_pin = 11, .output_pin =  6}, //RX Ch5 - Fire Solenoid
  {.input_pin = 12, .output_pin = -1}  //RX Ch6 - Robot Enable/Disable (no output pin)
};
const int num_signals = sizeof(signals) / sizeof(sig_type);
const int enable_signal = num_signals - 1;

const int ENABLE_uS = 1900; //PWM on time above which robot outputs will be enabled.
const int SAFE_uS   = 1490; //When timeout from RX, or robot not enabled, output this value.

const int timeout = 8000; //Time in uS to wait for state change on a PWM input pin.
                          //Note, for this timeout period to work, the servo frequency setting
                          //  on the transmitter (FS-I6) should be set accordingly (e.g. 200+ Hz)


//Runs once on startup
void setup() {
  //Configure input pins and output servo channels
  for (int i = 0; i < num_signals; i++) {
    pinMode(signals[i].input_pin, INPUT);
    
    if(signals[i].output_pin >= 0) {
      //Don't create a PWM output channel for the enable/disable signal
      signals[i].servo.attach(signals[i].output_pin);
    }
  }

  Serial.begin(115200);
}

//Runs repeatedly during execution
void loop() {
  signals[enable_signal].pwm_val = pulseIn(signals[enable_signal].input_pin, HIGH, timeout);
  
  //If robot isn't enabled, safe all outputs
  if(signals[enable_signal].pwm_val < ENABLE_uS) {
    for(int i = 0; i < num_signals - 1; i++) {
      signals[i].servo.writeMicroseconds(SAFE_uS);
    }
    #ifdef SERIAL_DEBUG
      Serial.print("Robot disabled. ");
      Serial.print(signals[enable_signal].pwm_val);
    #endif
  } else {  //else enable outputs
    //Read all remaining pwm signals (not enable pin)
    for(int i = 0; i < num_signals - 1; i++) {
      signals[i].pwm_val = pulseIn(signals[i].input_pin, HIGH, timeout);
      #ifdef SERIAL_DEBUG
        Serial.print(i+1);
        Serial.print(":");
        Serial.print(signals[i].pwm_val);
        Serial.print("  ");
      #endif
      signals[i].servo.writeMicroseconds(signals[i].pwm_val);
    }
  }
    #ifdef SERIAL_DEBUG
      Serial.println("");
    #endif
}

