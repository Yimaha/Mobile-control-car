/* 
 * TEJ3M1 Final Project - Arduino Car
 * Student Names: Henry Xue, Justin Cai
 * Teacher Name: Mr. Benum
 * 
 * Description:
 * This program is for the Arduino car which has the ability
 * to move forward, turn and stop automatically when there
 * are obstacles nearby. It can also light up automatically
 * in the dark, but you can disable this function so that the
 * lights would always be turned on. It is controlled remotely
 * by using an Android phone via a Bluetooth module. The car
 * also has a recorder function to record a series of actions
 * and replay them.
 * 
 * Due Date: Jan. 23, 2017
 */

// Variables declaration and initialization

// Pin related variables
int pinMotorLeft = 5;
int pinMotorRight = 6;
int pinMotorBoth1 = 10;  // To activate both motors simultaneously, 
int pinMotorBoth2 = 9;  // both pin 10 and 9 need to be activated
int pinLED1 = 12;
int pinLED2 = 13;
int pinLDR = 1;
int pinIR = 3;

// LED related variables
int flagLightsAlwaysOn = 0; 
int valueLDR = 0; 
int BRIGHT_DARK_BOUNDARY = 1010;

// Motion related variables
int flagForwardEnabled = 1;
int CAR_SPEED = 255;

// IR sensor related variables
int IRStatus = 1;
int prevIRStatus = 1;

// Command related variables
int rcmd[5];
int cmd;

// Recorder related variables
int flagRecording = 0;
struct recordNodes
{
  int cmd;
  unsigned long mstime;
};
recordNodes record[50];
int index = 0;

void setup()
{
  // Setting up pin modes
  pinMode(pinMotorLeft, OUTPUT);
  pinMode(pinMotorRight, OUTPUT);
  pinMode(pinLDR, INPUT);
  pinMode(pinLED1, OUTPUT);
  pinMode(pinLED2, OUTPUT);
  pinMode(pinIR, INPUT);
  
  // Begin Bluetooth communication 
  Serial.begin(9600);
  Serial.flush();
}

void loop()
{  
  // Section for receiving commands from the Android phone and handling them
  if (Serial.available())
  {
    // In the actual testing of the Bluetooth module, results showed that
    // the command received via Bluetooth could be different from what
    // was originally sent. In order to minimize this error, we send each
    // command for five times and pick up the ones that could be divided
    // by ten. That also explains why all the commands are multiples of ten.
    for (int i = 0; i < 5; i++)
    {
      rcmd[i] = Serial.read(); // "rcmd" stands for raw commands
    }
    for (int i = 0; i < 5; i++)
    {
      if (rcmd[i] % 10 == 0)
      {
        cmd = rcmd[i];
        break;
      }
    }
    
    // The following code will be executed when the recorder is active.
    if (flagRecording == 1)
    {
      record[index].cmd = cmd;
      record[index].mstime = millis();
      index++;
    }
    
    // Pass the command to the command handler
    functionalCommandHandler(cmd);
    recorderCommandHandler(cmd);
    
    Serial.flush();
  }

  // Section for reading the light sensor status and controlling the LEDs
  if (flagLightsAlwaysOn == 0)
  {
    valueLDR = analogRead(pinLDR);
    if (valueLDR > BRIGHT_DARK_BOUNDARY) // If it is dark
    {
      turnOnLED();
    }
    else if (valueLDR <= BRIGHT_DARK_BOUNDARY) // If it is bright
    {
      turnOffLED();
    }
  }

  // Section for interacting with the infrared sensor
  // Previous IR status will be recorded so that the code inside the if
  // statement will be executed only when the new received IR status is
  // different from the previous one.
  IRStatus = digitalRead(pinIR);
  if (IRStatus == 0 && prevIRStatus == 1)
  {
    Serial.print('O');  // "O" stands for obstacles
    stopCar();
    prevIRStatus = 0; 
    flagForwardEnabled = 0;
  }
  else if (IRStatus == 1 && prevIRStatus == 0)
  {
    Serial.print('S');  // "S" stands for safe
    prevIRStatus = 1;
    flagForwardEnabled = 1;
  }

  delay(80);
}

void functionalCommandHandler(int cmd)
{
  // All the commands are coded as multiples of ten
  switch(cmd)
  {
    case 10:
      if (flagForwardEnabled == 1)  // When no obstables are detected nearby
      {
        moveForward();
      }
      break;
    case 20:
      turnLeft();
      break;
    case 30:
      turnRight();
      break;
    case 40:
      stopCar();
      break;
    case 50:
      flagLightsAlwaysOn = 1;
      turnOnLED();
      break;
    case 60:
      flagLightsAlwaysOn = 0;
      break;
  }
}

void recorderCommandHandler(int cmd)
{
  // All the commands are coded as multiples of ten
  switch(cmd)
  {
    case 70:
      // Start recording
      flagRecording = 1;
      index = 0;
      for (int i = 0; i < 50; i++)  // Clear the previous record
      {
        record[i].cmd = 0;
        record[i].mstime = 0;
      }
      record[0].mstime = millis();  // Record the time when the recorder starts
      index++; // Move to the next record node
      break;
    case 80:
      // End recording
      flagRecording = 0;
      break;
    case 90:
      // Play the record
      for (int i = 0; i < index - 1; i++)
      {
        // Execute the recorded command
        functionalCommandHandler(record[i].cmd);

        // Delay for a period of time until the next recorded command is executed
        delay(record[i + 1].mstime - record[i].mstime);
      }
      break;
  }
}

void moveForward()
{
  // The -5 deviation is used to make sure the car goes straightforward
  analogWrite(pinMotorBoth1, CAR_SPEED - 5);
  analogWrite(pinMotorBoth2, CAR_SPEED);
}

void turnLeft()
{
  analogWrite(pinMotorLeft, 0);
  analogWrite(pinMotorRight, CAR_SPEED);
}

void turnRight()
{
  analogWrite(pinMotorRight, 0);
  analogWrite(pinMotorLeft, CAR_SPEED);
}

void stopCar()
{
  analogWrite(pinMotorLeft, 0);
  analogWrite(pinMotorRight, 0);
  analogWrite(pinMotorBoth2, 0);
  analogWrite(pinMotorBoth1, 0);
}

void turnOnLED()
{
  digitalWrite(pinLED1, HIGH);
  digitalWrite(pinLED2, HIGH);
}

void turnOffLED()
{
  digitalWrite(pinLED1, LOW);
  digitalWrite(pinLED2, LOW);
}
