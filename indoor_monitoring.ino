#include <DHT.h>
#include <DHT_U.h>
#include <Nokia_LCD.h>
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;

unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 60000;  //the value is a number of milliseconds

char monthsOfTheYear[12][4] = {"IAN", "FEB", "MAR", "APR", "MAI", "IUN", "IUL", "AUG", "SEP", "OCT", "NOV", "DEC"}; 
char daysOfTheWeek[7][12] = {"Duminica", "Luni", "Marti", "Miercuri", "Joi", "Vineri", "Sambata"}; 

int gas_sensor = A0; //Sensor pin
float m = -0.318; //Slope
float b = 1.133; //Y-Intercept
float R0 = 5.5; //Sensor Resistance in fresh air from previous code

#define DHTPIN 7
#define DHTTYPE DHT22

DHT temp_hum_sensor(DHTPIN, DHTTYPE);
Nokia_LCD lcd(8 /* CLK */, 9 /* DIN */, 10 /* DC */, 11 /* CE */, 12 /* RST */);

/**
 * Or, if you would like to control the backlight on your own, init the lcd without the last argument
 * Nokia_LCD lcd(13, 12, 11, 10, 9);
*/

void setup() {
  // Initialize the screen
  lcd.begin();
  // Turn on the backlight
  lcd.setBacklight(false);
  // Set the contrast
  lcd.setContrast(45);  // Good values are usualy between 40 and 60
  lcd.clear();
  temp_hum_sensor.begin();

  startMillis = millis();  //initial start time
  while (!Serial); // for Leonardo/Micro/Zero
  Serial.begin(57600);
  if (! rtc.begin()) {
   Serial.println("Couldn't find RTC");
   while (1);
  }
  if (! rtc.isrunning()) {
   Serial.println("RTC is NOT running!");
   // following line sets the RTC to the date & time this sketch was compiled
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   // This line sets the RTC with an explicit date & time, for example to set
   // January 21, 2014 at 3am you would call:
   // rtc.adjust(DateTime(2020, 1, 1, 0, 0, 0));
  }
}

void loop() {
  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= period || currentMillis <= period)  //test whether the period has elapsed
  {
    float sensor_volt=0; //Define variable for sensor voltage
    float RS_gas=0; //Define variable for sensor resistance
    float ratio=0; //Define variable for ratio
    float sensorValue = analogRead(gas_sensor); //Read analog values of sensor
    sensor_volt = sensorValue * (5.0 / 1023.0); //Convert analog values to voltage
    RS_gas = ((5.0 * 10.0) / sensor_volt) - 10.0; //Get value of RS in a gas
    ratio = RS_gas / R0;   // Get ratio RS_gas/RS_air
  
    double ppm_log = (log10(ratio) - b) / m; //Get ppm value in linear scale according to the the ratio value
    double ppm = pow(10, ppm_log); //Convert ppm value to log scale
    double percentage = ppm / 10000; //Convert to percentage
  
    float hum = temp_hum_sensor.readHumidity();
    float temp = temp_hum_sensor.readTemperature();
  
    DateTime now = rtc.now();
  
    lcd.setCursor(0,0);
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);lcd.print(" ");lcd.print(now.hour());lcd.print(":");lcd.println(now.minute());
    lcd.print(now.day());
    lcd.print(" ");
    lcd.print(monthsOfTheYear[now.month()-1]);
    lcd.print(" ");
    lcd.println(now.year());
    lcd.print("\n");
    lcd.print("Temp:");lcd.print(temp);lcd.println(" *C");
    lcd.print("Hum:");lcd.print(hum);lcd.println(" %");
    lcd.print("CH4:");lcd.print(ppm);lcd.println(" ppm");
    startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED state.
  }

  // over 2000 is baaad
}
