#include <Servo.h>

#define SERIAL_DEBUG

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
  {.input_pin = 10, .output_pin =  4}, //RX Ch4 - Intake Speed
  {.input_pin = 11, .output_pin =  6}, //RX Ch5 - Intake Solenoid
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

const int CUBE_ALL_IN = 600;
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

  getDistance(); // Update sensor distance
  
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
      //Three position switch - Up, outtake w/ arms closed
      //                      - Middle, stop intaking, Close the arms
      //                      - Down, intake w/ arms open
      
      switch_input = pulseIn(signals[4].input_pin, HIGH, timeout);
      
      if(switch_input >= ENABLE_uS) {
        if(last_switch_input < ENABLE_uS) {
          //Reset average distance to 0.0 so that we force intake to run for a bit on switch toggle
          avg_distance = 0.0; 
          //FIRST TIME COMMANDING INTAKE - open arms no matter what
          //  sensor sees the arms and doesn't start intaking
          signals[4].servo.writeMicroseconds(DISABLE_uS);  //Open arms
        }
        //INTAKE - Switch is down all the way
        if(avg_distance >= CUBE_ALL_IN) {
          //STOP INTAKING
          signals[3].servo.writeMicroseconds(SAFE_uS);     //Stop intakeing
          signals[4].servo.writeMicroseconds(DISABLE_uS);  //Open arms
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
        signals[4].servo.writeMicroseconds(ENABLE_uS);   //Close arms
      }
      
      last_switch_input = switch_input;
  }
  
  #ifdef SERIAL_DEBUG
    Serial.println("");
  #endif
}

int getDistance() {
  avg_distance = (0.8 * avg_distance) + (0.2 * analogRead(DISTANCE_SENSOR_PIN));

#ifdef SERIAL_DEBUG
  Serial.print("d:");
  Serial.print(analogRead(DISTANCE_SENSOR_PIN));
  Serial.print("  a:");
  Serial.println(avg_distance);
#endif
  return avg_distance;
}
