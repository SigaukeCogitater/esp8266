#include "EmonLib.h"
// Include Emon Library
EnergyMonitor emon1;
// Force EmonLib to use 10bit ADC resolution
#define ADC_BITS    10
//#define ADC_COUNTS  (1<<ADC_BITS)
#define ADC_INPUT A0
#define HOME_VOLTAGE 220.0
// Create an instance
void setup()
{
  Serial.begin(115200);
  emon1.current(ADC_INPUT, 30);
//  emon1.current(1, 111.1);             // Current: input pin, calibration.
}

void loop()
{
double amps = emon1.calcIrms(1480);  // Calculate Irms only

double watt = amps * HOME_VOLTAGE;

Serial.print("Current: ");         // Apparent power
  Serial.print(" ");
  Serial.println(amps);          // Irms

Serial.print("Power: ");         // Apparent power
  Serial.print(" ");
  Serial.println(watt);
}
