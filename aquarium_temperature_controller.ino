#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// PINS declaration
#define HEATER_PIN 7
#define FAN_PIN 8
#define THERMOMETER_PIN 5

// Configurable constants
#define NUMBER_OF_THERMOMETER_READS 100
#define TEMPERATURE_THRESHOLD 26.5

// Creating the required structures for the libs
// PS: You can use I2CScanner in order to find the correct value for the first
// parameter of the lcd structure.
LiquidCrystal_I2C lcd(0x27,16,2);
OneWire oneWire(THERMOMETER_PIN);
DallasTemperature sensors(&oneWire);


// Simple swap function, used in bubble sort
void swap(float *xp, float *yp)  
{  
    float temp = *xp;  
    *xp = *yp;  
    *yp = temp;  
}  
  
// A function to implement bubble sort  
void bubbleSort(float arr[], int n)  
{  
    int i, j;  
    for (i = 0; i < n-1; i++)      
      
    // Last i elements are already in place  
    for (j = 0; j < n-i-1; j++)  
        if (arr[j] > arr[j+1])  
            swap(&arr[j], &arr[j+1]);  
}  

void setup()
{
  // Calling the required initialization methods of the libs
  sensors.begin();
  lcd.init();

  // Turning the lcd backlight ON
  lcd.setBacklight(HIGH);

  // Configuring the PINS
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  // Starting the serial module
  Serial.begin(9600);

  // This delay is needed because of the relay module, if we start too
  // fast it can be a problem in the event of electrical malfunction and the
  // board getting constantly restarted.
  delay(1000);
}
 
void loop()
{
  bool fan = true;
  bool heater = false;

  // We'll build an array with N samples of temperature readings from our thermometer
  // this is needed because the temperature variation in this sensor is huge. The number
  // of samples will be determined by the NUMBER_OF_THERMOMETER_READS constant.
  
  float temperature_readings[NUMBER_OF_THERMOMETER_READS];

  Serial.print("Will start to read ");
  Serial.print(NUMBER_OF_THERMOMETER_READS);
  Serial.println(" temperature samples");
  
  for (int i = 0; i < NUMBER_OF_THERMOMETER_READS; i++) {
    // The DallasTemperature library works by calling this method to populate the temperature
    // and then calling the conversion method to get the temperature in the desired scale.
    sensors.requestTemperatures(); 
    temperature_readings[i] = sensors.getTempCByIndex(0);
  }

  // Bellow we'll calculate the median of the array of temp reads
  // the code could be a lot simpler if we know the NUMBER_OF_THERMOMETER_READS
  // beforehand but I'd like to make it flexible to changing the NUMBER_OF_THERMOMETER_READS
  // only to have more precise readings.

  Serial.println("Will start to sort the array of temperature samples");

  // First, we need to sort the array
  bubbleSort(temperature_readings, NUMBER_OF_THERMOMETER_READS);

  float temperature;

  Serial.println("Will start to calculate the median");

  // The median changes if the NUMBER_OF_THERMOMETER_READS is an even number
  if (NUMBER_OF_THERMOMETER_READS % 2 == 0) {
    // the upper half is the division result itself because the array start from 0
    int upper_half = NUMBER_OF_THERMOMETER_READS/2;
    // then we just need to subtract 1 from the upper half to get the bottom half
    int bottom_half = upper_half-1;
    
    // finally calculate the median
    temperature = ((temperature_readings[bottom_half]+temperature_readings[upper_half])/2);
    
  } else {
    // If the NUMBER_OF_THERMOMETER_READS is an odd number, we just need to get the element
    // that is in the center of the sample
    int median_index = (int) (NUMBER_OF_THERMOMETER_READS /2);
    
    temperature = temperature_readings[median_index];
  }

  Serial.println(temperature);

  // Bellow we have the code that decides wether we should turn ON the
  // fans or the heater based on the median of the temperature sample and
  // the TEMPERATURE_THRESHOLD constant.
  if (temperature < TEMPERATURE_THRESHOLD) {
    fan = false;
    heater = true;
  } else if(temperature > TEMPERATURE_THRESHOLD) {
    fan = true;
    heater = false;
  } else {
    fan = false;
    heater = false;
  }


  // Go to the first line, first char
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("c");

  // Go to the next LCD line, first char
  lcd.setCursor(0,1);
  lcd.print("FAN:");
  if (fan) {
    digitalWrite(FAN_PIN, LOW);    
    lcd.print("ON");
  }else{
    digitalWrite(FAN_PIN, HIGH);
    lcd.print("OFF");
  }
  lcd.print(" HEAT:");
  if (heater) {
    digitalWrite(HEATER_PIN, LOW);
    lcd.print("ON ");
  }else{
    digitalWrite(HEATER_PIN, HIGH);
    lcd.print("OFF");  
  }

  Serial.println("Will sleep");
  // We'll delay for a longer period because of electrical safety for
  // our fans and heater connected to our relay module.
  delay(30000);
}
