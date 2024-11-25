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
#define POT_PLAYER1 A0 // Pin analog untuk potensiometer pemain 1
#define POT_PLAYER2 A10 // Pin analog untuk potensiometer pemain 2

// Create the matrix object
MD_Parola matrix_parola = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
MD_MAX72XX matrix = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

int playerY = 0 + 7;   // Y-coordinate of the player (row)
int player2Y = 31 - 7; // Y-coordinate of Player 2
int player1Position = 0; // Posisi awal pemain 1
int player2Position = 0; // Posisi awal pemain 2
int rectrow[4] = {2, 3, 4, 5};
int linep1 = 6;
int linep2 = 25;
int linerow[8] = {0, 1, 2, 3, 4, 5, 6, 7};
int score1 = 0;
int score2 = 0;

// Variabel untuk bola
int ballX = 0; // Posisi awal X
int ballY = 0; // Posisi awal Y
int dx = 1;    // Arah horizontal (1: ke kanan, -1: ke kiri)
int dy = 1;    // Arah vertikal (1: ke bawah, -1: ke atas)

// Batas area
const int minCol = 6;  // Kolom minimal (7)
const int maxCol = 25; // Kolom maksimal (26)
const int maxRows = 8; // Baris maksimal (8 baris)

// Variabel untuk skor
int scoreLeft = 0;  // Skor pemain kiri
int scoreRight = 0; // Skor pemain kanan

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

    // Digit\ 5
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

void resetBall();
void drawLine(MD_MAX72XX *matrix, int *linerow, int linep1, int linep2);


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
  // player1Position = map(analogRead(POT_PLAYER1), 0, 4095, 0, 4); // Rentang 0-3
  // player2Position = map(analogRead(POT_PLAYER2), 0, 4095, 0, 4); // Rentang 0-3

  // for (int i = 0; i < 4; i++)
  // {
  //   matrix->setPoint(rectrow[i], playerY, true); // Turn on each dot
  // }

  // // Draw rectangle 2 (column 31-7)
  // for (int i = 0; i < 4; i++)
  // {
  //   matrix->setPoint(rectrow[i], player2Y, true);
  // }

  // drawLine(matrix, linerow, linep1, linep2);

  // Update posisi bola
  ballX += dx;
  ballY += dy;

  
  // Deteksi pantulan pada tepi atas dan bawah
  if (ballY <= 0 || ballY >= maxRows - 1)
    dy = -dy;

  // Deteksi pantulan pada pemain 1
  if (ballX == playerY && ballY >= rectrow[0] && ballY <= rectrow[3])
  {
    dx = -dx; // Balik arah horizontal
    Serial.println("Pantul dari pemain 1!");
  }

  // Deteksi pantulan pada pemain 2
  if (ballX == player2Y && ballY >= rectrow[0] && ballY <= rectrow[3])
  {
    dx = -dx; // Balik arah horizontal
    Serial.println("Pantul dari pemain 2!");
  }

  // Deteksi gol
  if (ballX < minCol)
  {
    // Gol ke gawang kiri
    scoreRight++;
    resetBall();
    Serial.println("Gol ke gawang kiri! Skor kanan: " + String(scoreRight));
    Serial.println("Row: " + String(ballY) + ", Col: " + String(ballX));
  }
  else if (ballX > maxCol)
  {
    // Gol ke gawang kanan
    scoreLeft++;
    resetBall();
    Serial.println("Gol ke gawang kanan! Skor kiri: " + String(scoreLeft));
    Serial.println("Row: " + String(ballY) + ", Col: " + String(ballX));
  }

  // Tampilkan bola baru
  matrix->setPoint(ballY, ballX, true);

  // // Gambar pemain 1 berdasarkan posisi
  for (int i = 0; i < 4; i++)
  {
    if (player1Position + i < 8)
    { // Pastikan tidak keluar dari layar
      matrix->setPoint(rectrow[i] + player1Position, playerY, true);
    }
  }

  // Gambar pemain 2 berdasarkan posisi
  for (int i = 0; i < 4; i++)
  {
    if (player2Position + i < 8)
    { // Pastikan tidak keluar dari layar
      matrix->setPoint(rectrow[i] + player2Position, player2Y, true);
    }
  }

  // for (int i = 0; i < 10; i++)
  // {
  //   displayTensDigitLeft(matrix, i);
  //   Serial.println("Display Units Digit Left : " + String(i));
  //   delay(1000);
  //   matrix->clear();
  //   displayUnitsDigitLeft(matrix, i); // Units digit (rows 5-7)
  //   Serial.println("Display Units Digit Left : " + String(i));
  //   delay(1000);
  //   matrix->clear();
  //   displayTensDigitRight(matrix, i); // Tens digit (rows 0-2)
  //   Serial.println("Display Tens Digit Right : " + String(i));
  //   delay(1000);
  //   matrix->clear();
  //   displayUnitsDigitRight(matrix, i); // Units digit (rows 5-7)
  //   Serial.println("Display Units Digit Right : " + String(i));
  //   delay(1000);
  //   matrix->clear();
  // }
  delay(200);
}

// Reset posisi bola setelah gol
void resetBall()
{
  ballX = (minCol + maxCol) / 2;     // Posisikan bola di tengah kolom
  ballY = maxRows / 2;               // Posisikan bola di tengah baris
  dx = (random(0, 2) == 0) ? 1 : -1; // Random arah horizontal
  dy = (random(0, 2) == 0) ? 1 : -1; // Random arah vertikal
}

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
    matrix->setPoint(7 - row, 31 - col, true);
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
