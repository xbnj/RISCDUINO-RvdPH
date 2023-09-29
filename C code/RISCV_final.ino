///////////////////////////////////////////////////////////////////////////////////////
////                                                                               ////
////                SMART AND AUTOMATED FISH AQUARIUM SYSTEM                       ////
////                 members:- Alwin Dsouza,Prabal Raj                             ////
////                                                                               ////
////                                                                               ////
///////////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------------------------------
                 
#include <OneWire.h>                              //Library for Temperature sensor 
#include <LiquidCrystal_I2C.h>                    //Library for I2C interfaced LCD display
#include <Keypad.h>                               //Library for keypad
#include <Servo.h>                                //Library for servo motor

//------------------------------------------------------------------------------------------------------

Servo myservo;// object creation of servo motor for the automatic fish food feeder

//------------------------------------------------------------------------------------------------------

const int ledPin12 = 12;//GREEN LED 
const int ledPin13 = 13;//RED ALERT

//------------------------------------------------------------------------------------------------------
//Sensor pin configurations

OneWire oneWire(10);                              // pin declaration for temperature sensor
int turbidity_sensor = A0;                        // pin declaration for turbidity sensor

//------------------------------------------------------------------------------------------------------

//We are using LCD DISPLAY with an I2C interface 
LiquidCrystal_I2C lcd(0x27, 16, 2);              //  LCD I2C address and size

//We are then using 3x4 KEYPAD to take input from user as per the requirements
const byte ROWS = 4;                            // 4 rows on the keypad
const byte COLS = 3;                            // 3 columns on the keypad
char keys[ROWS][COLS] = 
{
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};                  // Row pins of the keypad
byte colPins[COLS] = {2, 3, 4};                     // Column pins of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ); // mapping alloted pins to keypad 
//----------------------------------------------------------------------------------------------------------

int numberOfFishes = 0;                           //initializing variable for storing number Of Fishes
int numberOfFeedings = 0;                         //initializing variable for storing number Of Feedings
int feedingIntervalHours = 0;                     //initializing variable for storing fwwding interval (in hours)

//-----------------------------------------------------------------------------------------------------------

/*function for operating the servo motor which is used for the automatic fish  feedingthe feeding of the fish
 in based on fixed intervals which is taken as input from user based on the input i.e. number of hours, the 
 servo motor will move. hence moving the feeder and eventually dispensing the food into the aquarium.*/

void operateServoWithDelay(long delayTime) 
{
  int pos = 0;     // Initialize the servo position
  
  while (true) {
    // Move the feeder to the maximum position (180 degrees)
    for (pos = 0; pos <= 180; pos += 1) 
    {
      myservo.write(pos);
      delay(15); 
    }

    // Move the feeder back to the minimum position (0 degrees)
    for (pos = 180; pos >= 0; pos -= 1) 
    {
      myservo.write(pos);
      delay(15);
    }
    
    // Wait for the specified delay time
    delay(delayTime);
  }
}

//---------------------------------------------------------------------------------------------

/*The below function is used to sense temperature inside the aquarium and convert it to degree 
celsius. this sensor is useful in monitoring the suitable conditions inside the water
the sensor we are using is immersible in water which is named as ds18b20 */

float readTemperature(OneWire &ds18b20) 
{
  byte data[9];
  ds18b20.reset();
  ds18b20.write(0xCC);  
  ds18b20.write(0x44);  
  delay(750);                     // Wait for conversion (750ms for 12-bit resolution)

  ds18b20.reset();
  ds18b20.write(0xCC);  
  ds18b20.write(0xBE);  

  for (int i = 0; i < 9; i++) 
  {
    data[i] = ds18b20.read();
  }

  // Calculating temperature
  int16_t rawTemperature = ((int16_t)data[1] << 8) | data[0];
  float temperatureC = (float)rawTemperature / 16.0;

  return temperatureC;
}

//-----------------------------------------------------------------------------------------------

 /*Function for sensing turbidity inside aquarium.As we know cleaning of aquarium is one the most
  important task and if not done on proper time can cause harm to health and can also be fatal for 
  fishes.Now there are several parameters which helps in determining the state of water as good or 
  bad.  One such is called TURBIDITY.Here we are  using turbidity sensor to check whether water 
  inside   aquarium is ok or not.*/

int readTurbidity() 
{
  int turbidityValue = analogRead(turbidity_sensor);
  return map(turbidityValue, 0, 640, 100, 0);            /* mapping values from a level of 0 to 100
                                                            so that conditions can be checked in a
                                                            much more easier way */
                                                      
}
//-----------------------------------------------------------------------------------------------

/*Now to make an alert system we have used 2 LED's and a buzzer. they come into action when certain
conditions are not up to the mark or say it is not favourable for the aquarium. For increase in 
turbidity there is an alarm and cleaning warning through RED LED and simultaneously a BUZZER
connected to same pin*/

void led_alert(int turbidityValue) 
{
  if (turbidityValue < 15) 
  {
    
    digitalWrite(12, HIGH);  // Turn on LED on pin 12(GREEN LED)
    digitalWrite(13, LOW);  // Turn off LED on pin 13(RED LED)
  } else if (turbidityValue > 15) 
  {
    digitalWrite(12, LOW);  // Turn off GREEN LED

    // turn on red led with blinking alert ,buzzer will be associated with the same pin
    digitalWrite(13, HIGH);
    delay(2000);
    digitalWrite(13, LOW);
    delay(2000);
  
  } 
}

//-----------------------------------------------------------------------------------------------------

/*Fucntion of keypad modulefor taking input from user.As the system requirement may vary, To make it 
adjustable we have to take input of certain set of values from the user and our system will function based
 on that. For example:- Taking input of the interval in hours in which we need to feed the fish which will 
 automatically help to feed them precisely without any human intervention and delay in feeding.
 Even if the owner is away from home they can rely on mechanism */
int getInputFromKeypad() 
{
  String input = "";
  char key;
  while (true)
   {
    key = keypad.getKey();
    if (key) 
    {
      if (key == '#') 
      {
        // User pressed '#' to confirm input
        int value = input.toInt();
        Serial.println("Entered: " + input);
        return value;
      } 
      else 
      {
        // Append the key to the input string
        input += key;
        Serial.print(key);
        lcd.print(key);
      }
    }
  }
}

//-----------------------------------------------------------------------------------------

void setup() 
{
  myservo.attach(11);
  lcd.init();
  lcd.backlight();
  lcd.print("WELCOME");
  delay(2000);
  lcd.clear();
  lcd.print("Enter the details");


  pinMode(ledPin12, OUTPUT);                                                // GREEN LED 
  pinMode(ledPin13, OUTPUT);                                                // RED LED & BUZZER
}
//------------------------------------------------------------------------------------------

void loop() 
{
   long delayTime =  feedingIntervalHours* 3600000L;/* Since we Know delay in this IDE is in 
   so we convert entered input in integer value to hourse manually by using this conversion.*/

   int turbidity = readTurbidity();                                          //initializing turbidity sensor
   float temperature = readTemperature(oneWire);                             //initializing temperature sensor
   
  if (numberOfFishes == 0) 
  { lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("No. of Fish:            ");                                  
    lcd.setCursor(6, 1);
    lcd.print("Enter:");
    numberOfFishes = getInputFromKeypad();                                   //using UDF defined earlier,taking input of no. of fishes.
  } 
  else if (numberOfFeedings == 0)  
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Feedings:        ");
    lcd.setCursor(9, 1);
    lcd.print("Enter:");
    numberOfFeedings = getInputFromKeypad();                                 //using UDF defined earlier,taking input of no. of times user want to feed fishes.
  }
   else if (feedingIntervalHours == 0) 
   {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Interval (hours):");
    lcd.setCursor(16, 1);
    lcd.print("Enter:");
    feedingIntervalHours = getInputFromKeypad();                             //using UDF defined earlier,taking input of interval in hours for feeding.
   }
  
  lcd.clear();
  
  // Displaying temperature and turbidity on the LCD

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");                                                          
  lcd.print(temperature);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Turbidity: ");
  lcd.print(turbidity);

  led_alert(turbidity);                                        //calling function for monitoring conditions
  operateServoWithDelay( delayTime);                           //Activating automatic food feeder accordingly

}

