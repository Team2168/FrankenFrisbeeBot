#include <Servo.h>

//#define SERIAL_DEBUG

typedef struct {
  int input_pin;         //The input pin from the RX
  int output_pin;        //The pin wired to the output device (motor controller/solenoid)
  unsigned long pwm_val; //Value in uS read from the input pin, to be written to the output pin
  Servo servo;           //The servo object associated with this signal
} sig_type;

sig_type signals[] = {                 //ENABLE SIGNAL MUST BE LAST!
  {.input_pin = 7,  .output_pin =  5}, //RX Ch1 - Lift Speed
  {.input_pin = 8,  .output_pin =  2}, //RX Ch2 - Right DT Motor Speed
  {.input_pin = 9,  .output_pin =  3}, //RX Ch3 - Left DT Motor Speed
  {.input_pin = 10, .output_pin =  4}, //RX Ch4 - Intake Speed (output 5)
  {.input_pin = 11, .output_pin =  6}, //RX Ch5 - Intake Solenoid (output 6)
  {.input_pin = 12, .output_pin = -1}  //RX Ch6 - Robot Enable/Disable (no output pin)
};
const int num_signals = sizeof(signals) / sizeof(sig_type);
const int enable_signal = num_signals - 1;

const int ENABLE_uS   = 1900; //PWM on time above which robot outputs will be enabled.
const int SAFE_uS     = 1490; //When timeout from RX, or robot not enabled, output this value.
const int DISABLE_uS  = 1100; //PWM on time below which robot output is "DISABLED"

const int timeout  = 8000; //Time in uS to wait for state change on a PWM input pin.
                           //Note, for this timeout period to work, the servo frequency setting
                           //  on the transmitter (FS-I6) should be set accordingly (e.g. 200+ Hz)

const int DISTANCE_SENSOR_PIN = A0;  //IR Distance sensor pin.
double avg_distance = 0.0;
int switch_input = 0;
int last_switch_input = 0;

const int CUBE_ALL_IN = 450;
const int CUBE_SEEN = 170;


//Runs once on startup
void setup() {
  //Configure input pins and output servo channels
  for (int i = 0; i < num_signals; i++) {
    pinMode(signals[i].input_pin, INPUT);
    
    if(signals[i].output_pin >= 0) { //Don't create a servo output for the enable signal
      //Init the servo position to our calibrated safe value (zero)
      //  before attaching to avoid unwanted movement during init.
      signals[i].servo.writeMicroseconds(SAFE_uS);
      signals[i].servo.attach(signals[i].output_pin);
    }
  }

  #ifdef SERIAL_DEBUG
    Serial.begin(115200);
  #endif
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
    //Read all remaining pwm signals (not enable pin or the intake motor/solenoid)
    for(int i = 0; i < num_signals - 3; i++) {
      signals[i].pwm_val = pulseIn(signals[i].input_pin, HIGH, timeout);
      #ifdef SERIAL_DEBUG
        Serial.print(i+1);
        Serial.print(":");
        Serial.print(signals[i].pwm_val);
        Serial.print("  ");
      #endif
      signals[i].servo.writeMicroseconds(signals[i].pwm_val);
    }

      //Command specific to the intake. 
      //Three position switch - Up, outtake w/ armsclosed
      //                      - Middle, stop intaking, leave the arms alone
      //                      - Down, intake w/ arms open
      avg_distance = getDistance(); // Update sensor distance
      switch_input = pulseIn(signals[4].input_pin, HIGH, timeout);

      //ATTEMPT INTAKE - Switch is down all the way
      if(switch_input >= ENABLE_uS) {
        if(last_switch_input < ENABLE_uS) {
          //FIRST TIME COMMANDING INTAKE - open arms no matter what
          //  sensor sees the arms and doesn't start intaking
          signals[4].servo.writeMicroseconds(DISABLE_uS);  //Open arms
        }
        //INTAKE - Switch is down all the way
        if(avg_distance >= 450) { //CUBE_SEEN
          //We see a cube, close the arms
          signals[4].servo.writeMicroseconds(ENABLE_uS);   //Close arms
        
        } else if(avg_distance >= 170) { //CUBE ALL IN = 450, Seen = 170 **CUBE_ALL_IN
          //keep* INTAKING
          signals[3].servo.writeMicroseconds(DISABLE_uS);  //Intake **changed safeus to disableus**
        } else {
          //No cube yet, run intake with arms open
          signals[3].servo.writeMicroseconds(DISABLE_uS);  //Intake
          signals[4].servo.writeMicroseconds(DISABLE_uS);  //Open arms
        }
      } else if(switch_input <= DISABLE_uS) {
        //OUTTAKE - Switch is up all the way
        signals[3].servo.writeMicroseconds(ENABLE_uS);   //Outtake
        signals[4].servo.writeMicroseconds(ENABLE_uS);   //Close arms
      } else {
        //STOP - Switch is in the middle
        signals[3].servo.writeMicroseconds(SAFE_uS);
        //signals[4].servo.writeMicroseconds(); //Leave the arms alone
      }
      
      last_switch_input = switch_input;
  }
  
  #ifdef SERIAL_DEBUG
    Serial.println("");
  #endif
}

int getDistance() {
  avg_distance = (0.75 * avg_distance) + (0.25 * analogRead(DISTANCE_SENSOR_PIN));

#ifdef SERIAL_DEBUG
  Serial.print("d:");
  Serial.print(analogRead(DISTANCE_SENSOR_PIN));
  Serial.print("  a:");
  Serial.println(avg_distance);
#endif
  return avg_distance;
}
