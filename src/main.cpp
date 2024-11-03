#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Define the hardware type and connections
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4 // Number of 8x8 modules in the chain

#define DATA_PIN 4 // DIN pin
#define CS_PIN 2   // CS pin
#define CLK_PIN 15  // CLK pin
#define VCC 17
#define GND 16

// Create the matrix object
MD_Parola matrix_parola = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
MD_MAX72XX matrix = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void testDots()
{
  for (uint8_t device = 0; device < MAX_DEVICES; device++)
  {
    for (uint8_t row = 0; row < 8*4; row++)
    {
      for (uint8_t col = 0; col < 8*4; col++)
      {
        matrix.setPoint(row, col, true); // Turn on each dot for each device
      }
    }
  }
  delay(2000);
  matrix.clear();
}

void setup()
{
  Serial.begin(9600);
  pinMode(VCC, OUTPUT);
  digitalWrite(VCC, HIGH);
  pinMode(GND, OUTPUT);
  digitalWrite(GND, LOW);
  matrix.begin();         // Initialize the matrix
  matrix_parola.begin(); // Initialize the parola
  testDots();
  matrix_parola.setIntensity(5);
  matrix_parola.displayText("5024221021 - Agus Fuad Mudhofar", PA_CENTER, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

void loop()
{
  if (matrix_parola.displayAnimate())
  {                        // Update the animation
    matrix_parola.displayReset(); // Reset to loop the animation
  }
}

