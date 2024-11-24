#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Define the hardware type and connections
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4 // Number of 8x8 modules in the chain

#define DATA_PIN 4 // DIN pin
#define CS_PIN 2   // CS pin
#define CLK_PIN 15 // CLK pin
#define VCC 17
#define GND 16

// Create the matrix object
MD_Parola matrix_parola = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
MD_MAX72XX matrix = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

int playerY = 0 + 7;   // Y-coordinate of the player (row)
int player2Y = 31 - 7; // Y-coordinate of Player 2
int rectrow[4] = {2, 3, 4, 5};
int linep1 = 6;
int linep2 = 25;
int linerow[8] = {0, 1, 2, 3, 4, 5, 6, 7};
int score1 = 0;
int score2 = 0;

// Segment map for digits in the "tens" position (rows 0-2)
const int tensDigitMap[10][15][2] = {
    // Kolom, Baris
    // Digit 0
    {
        {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {1, 4}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {-1, -1}},

    // Digit 1
    {

        {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {-1, -1}},

    // Digit 2
    {
        {0, 0}, {1, 0}, {2, 0}, {2, 1}, {0, 2}, {1, 2}, {2, 2}, {0, 3}, {0, 4}, {1, 4}, {2, 4}},

    // Digit 3
    {
        {0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {1, 2}, {0, 2}, {2, 3}, {2, 4}, {1, 4}, {0, 4}, {-1, -1}},

    // Digit 4
    {
        {0, 0}, {2, 0}, // Vertical line on the left
        {0, 1},
        {0, 2}, // Middle connection
        {2, 1},
        {1, 2},
        {2, 2}, // Horizontal line in the middle
        {2, 3},
        {2, 4},  // Vertical line on the right
        {-1, -1} // End of valid segments
    },

    // Digit 5
    {
        {0, 0}, {1, 0}, {2, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 3}, {2, 4}, {1, 4}, {0, 4}, {-1, -1}},

    // Digit 6
    {
        {1, 0}, {0, 0}, {2, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 3}, {2, 4}, {1, 4}, {0, 4}, {0, 3}},

    // Digit 7
    {
        {0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {-1, -1}},

    // Digit 8
    {
        {0, 0}, {1, 0}, {2, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {0, 3}, {0, 4}, {1, 4}, {2, 4}, {2, 1}, {2, 3}},

    // Digit 9
    {
        {0, 0}, {1, 0}, {2, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 3}, {2, 4}, {1, 4}, {0, 4}, {2, 1}}

};

const int unitsDigitMap[10][15][2] = {
    // Kolom, Baris
    // Digit 0
    {
        {5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {6, 4}, {7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {6, 0}, {-1, -1}},

    // Digit 1
    {
        {7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},

    // Digit 2
    {
        {5, 0}, {6, 0}, {7, 0}, {7, 1}, {5, 2}, {6, 2}, {7, 2}, {5, 3}, {5, 4}, {6, 4}, {7, 4}, {-1, -1}, {-1, -1}},

    // Digit 3
    {
        {5, 0}, {6, 0}, {7, 0}, {7, 1}, {7, 2}, {6, 2}, {5, 2}, {7, 3}, {7, 4}, {6, 4}, {5, 4}, {-1, -1}, {-1, -1}},

    // Digit 4
    {
        {5, 0}, {7, 0}, {5, 1}, {5, 2}, {6, 2}, {7, 2}, {7, 3}, {7, 4}, {7, 1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},

    // Digit 5
    {
        {5, 0}, {6, 0}, {7, 0}, {5, 1}, {5, 2}, {6, 2}, {7, 2}, {7, 3}, {7, 4}, {6, 4}, {5, 4}, {-1, -1}, {-1, -1}},

    // Digit 6
    {
        {6, 0}, {5, 0}, {7, 0}, {5, 1}, {5, 2}, {6, 2}, {7, 2}, {7, 3}, {7, 4}, {6, 4}, {5, 4}, {5, 3}, {-1, -1}},

    // Digit 7
    {
        {5, 0}, {6, 0}, {7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},

    // Digit 8
    {
        {5, 0}, {6, 0}, {7, 0}, {5, 1}, {5, 2}, {6, 2}, {7, 2}, {5, 3}, {5, 4}, {6, 4}, {7, 4}, {7, 1}, {7, 3}, {-1, -1}},

    // Digit 9
    {
        {5, 0}, {6, 0}, {7, 0}, {5, 1}, {5, 2}, {6, 2}, {7, 2}, {7, 3}, {7, 4}, {6, 4}, {5, 4}, {7, 1}, {-1, -1}}};

void displayUnitsDigitLeft(MD_MAX72XX *matrix, int digit)
{
  if (digit < 0 || digit > 9)
    return; // Invalid digit

  for (int segment = 0; segment < 15; segment++)
  {
    int row = unitsDigitMap[digit][segment][0];
    int col = unitsDigitMap[digit][segment][1];

    if (row == -1 && col == -1)
      break;

    matrix->setPoint(row, col, true);
  }
}

void displayUnitsDigitRight(MD_MAX72XX *matrix, int digit)
{
  if (digit < 0 || digit > 9)
    return; // Invalid digit
    

  for (int segment = 0; segment < 15; segment++)
  {

    int originalRow = unitsDigitMap[digit][segment][0];
    int originalCol = unitsDigitMap[digit][segment][1];

    // Jika elemen asli adalah {-1, -1}, maka hentikan iterasi
    if (originalRow == -1 && originalCol == -1)
      break;

    // Transformasi flip
    int row = 7 - originalRow;
    int col = 31 - originalCol;

    // Pastikan row dan col valid sebelum menyalakan titik
    if (row >= 0 && row < 8 && col >= 0 && col < 32)
    {
      matrix->setPoint(row, col, true);
    }

    // Serial.print("Lighting up row: ");
    // Serial.print(row);
    // Serial.print(", col: ");
    // Serial.println(col);
    // delay(1000);
  }
}

void displayTensDigitLeft(MD_MAX72XX *matrix, int digit)
{
  if (digit < 0 || digit > 9)
    return; // Invalid digit

  for (int segment = 0; segment < 15; segment++)
  {
    int row = tensDigitMap[digit][segment][0];
    int col = tensDigitMap[digit][segment][1];

    if (row == -1 && col == -1)
      break;

    matrix->setPoint(row, col, true);
    // Serial.print("Lighting up row: ");
    // Serial.print(row);
    // Serial.print(", col: ");
    // Serial.println(col);
    // delay(1000);

    // if (row != -1 && col != -1)
    // {
    //   matrix->clear();
    //   Serial.print("Lighting up row: ");
    //   Serial.print(row);
    //   Serial.print(", col: ");
    //   Serial.println(col);

    //   matrix->setPoint(row, col, true);
    //   delay(1000);
    // }
  }
}

void displayTensDigitRight(MD_MAX72XX *matrix, int digit)
{
  if (digit < 0 || digit > 9)
    return; // Invalid digit

  for (int segment = 0; segment < 15; segment++)
  {
    int row = tensDigitMap[digit][segment][0];
    int col = tensDigitMap[digit][segment][1];

    if (row == -1 && col == -1)
      break;
    
    // matrix->clear();
    matrix->setPoint(7-row, 31 - col, true);
    // Serial.print("Lighting up row: ");
    // Serial.print(7-row);
    // Serial.print(", col: ");
    // Serial.println(31 - col);
    // delay(1000);

  }
}

// Score
void displayLeftScore(MD_MAX72XX *matrix, int score)
{
  if (score < 0 || score > 30)
    return; // Invalid score

  if (score >= 0 && score <= 9)
  {
    // Display score as a single-digit (use the tens position)
    displayTensDigitLeft(matrix, score);
  }
  else if (score >= 10 && score <= 30)
  {
    // Split the score into tens and units
    int tens = score / 10;
    int units = score % 10;

    // Display tens and units in their respective positions
    displayTensDigitLeft(matrix, tens);
    displayUnitsDigitLeft(matrix, units);
  }
}

void displayRightScore(MD_MAX72XX *matrix, int score)
{
  if (score < 0 || score > 30)
    return; // Invalid score

  if (score >= 0 && score <= 9)
  {
    // Display score as a single-digit (use the tens position)
    displayTensDigitRight(matrix, score);
  }
  else if (score >= 10 && score <= 30)
  {
    // Split the score into tens and units
    int tens = score / 10;
    int units = score % 10;

    // Display tens and units in their respective positions
    displayTensDigitRight(matrix, tens);
    displayUnitsDigitRight(matrix, units);
  }
}
void testDots()
{
  for (uint8_t device = 0; device < MAX_DEVICES; device++)
  {
    for (uint8_t row = 0; row < 8 * 4; row++)
    {
      for (uint8_t col = 0; col < 8 * 4; col++)
      {
        matrix.setPoint(row, col, true); // Turn on each dot for each device
      }
    }
  }
  delay(2000);
  matrix.clear();
}

void drawLine(MD_MAX72XX *matrix, int linerow[], int linep1, int linep2)
{
  for (int i = 0; i < 8; i++)
  {
    matrix->setPoint(linerow[i], linep1, true);
    matrix->setPoint(linerow[i], linep2, true);
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(VCC, OUTPUT);
  digitalWrite(VCC, HIGH);
  pinMode(GND, OUTPUT);
  digitalWrite(GND, LOW);
  matrix_parola.begin(); // Initialize the parola
  // testDots();
  matrix_parola.setIntensity(1);
  // matrix_parola.displayText("5024221021 - Agus Fuad Mudhofar", PA_CENTER, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

  MD_MAX72XX *matrix = matrix_parola.getGraphicObject(); // Get the graphics object
  // setPoint(row, col, true?); column 0 - 31, row 0 - 7
  // Draw rectangle 1 (column 0+7)

  // displayTensDigitLeft(matrix, 2);  // Tens digit (rows 0-2)
  // displayUnitsDigitLeft(matrix, 0); // Units digit (rows 5-7)
}

void loop()
{
  MD_MAX72XX *matrix = matrix_parola.getGraphicObject();
  matrix->clear();
  for (int i = 0; i < 4; i++)
  {
    matrix->setPoint(rectrow[i], playerY, true); // Turn on each dot
  }

  // Draw rectangle 2 (column 31-7)
  for (int i = 0; i < 4; i++)
  {
    matrix->setPoint(rectrow[i], player2Y, true);
  }

  drawLine(matrix, linerow, linep1, linep2);
  

  for (int i = 0; i < 10; i++)
  {
    displayTensDigitLeft(matrix, i);
    Serial.println("Display Units Digit Left : " + String(i));
    delay(1000);
    matrix->clear();
    displayUnitsDigitLeft(matrix, i); // Units digit (rows 5-7)
    Serial.println("Display Units Digit Left : " + String(i));
    delay(1000);
    matrix->clear();
    displayTensDigitRight(matrix, i); // Tens digit (rows 0-2)
    Serial.println("Display Tens Digit Right : " + String(i));
    delay(1000);
    matrix->clear();
    displayUnitsDigitRight(matrix, i); // Units digit (rows 5-7)
    Serial.println("Display Units Digit Right : " + String(i));
    delay(1000);
    matrix->clear();
  }
  matrix->clear();
  delay(5);
}
