#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
// define HARDWARE_TYPE matrix_parola
#define MAX_DEVICES 4
#define MATRIX_WIDTH (8 * MAX_DEVICES) // Total lebar matriks dalam piksel

#define DATA_PIN 4
#define CS_PIN 2
#define CLK_PIN 15
#define VCC 17
#define GND 16
#define POT_PIN 13 // Pin potensiometer
#define POT_PIN2 26

MD_Parola matrix_parola = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
MD_MAX72XX matrix = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

int playerY = 0;                   // Y-coordinate of the player (row)
int player2Y = 0;                  // Y-coordinate of Player 2
const int playerHeight = 4;        // Height of the player (4 dots)
const int playerWidth = 1;         // Width of the player (4 dots)
const int edgePadding = 6;         // Padding (pixels) from each edge
unsigned long previousMillis = 0;  // Previous time
const unsigned long interval = 10; // Update interval in ms (100 Hz)

// Scores
int scorePlayer1 = 0;
int scorePlayer2 = 0;

// Font data for digits (6x8 pixels, flipped horizontally)
const uint8_t digits[10][8] = {
    {0x1F, 0x11, 0x11, 0x11, 0x1F, 0x00, 0x00, 0x00}, // 0
    {0x00, 0x00, 0x01, 0x3F, 0x00, 0x00, 0x00, 0x00}, // 1
    {0x11, 0x19, 0x15, 0x13, 0x11, 0x00, 0x00, 0x00}, // 2
    {0x11, 0x11, 0x15, 0x15, 0x0E, 0x00, 0x00, 0x00}, // 3
    {0x03, 0x05, 0x09, 0x3F, 0x01, 0x00, 0x00, 0x00}, // 4
    {0x1E, 0x15, 0x15, 0x15, 0x12, 0x00, 0x00, 0x00}, // 5
    {0x1F, 0x15, 0x15, 0x15, 0x10, 0x00, 0x00, 0x00}, // 6
    {0x10, 0x10, 0x1F, 0x11, 0x12, 0x00, 0x00, 0x00}, // 7
    {0x1F, 0x15, 0x15, 0x15, 0x1F, 0x00, 0x00, 0x00}, // 8
    {0x1E, 0x15, 0x15, 0x15, 0x1F, 0x00, 0x00, 0x00}  // 9
};

void rotateDigit90(const uint8_t original[5], uint8_t rotated[8])
{
  for (int row = 0; row < 8; row++)
  {
    rotated[row] = 0; // Reset row
    for (int col = 0; col < 5; col++)
    {
      if (original[col] & (1 << row))
      {                                   // Cek bit di kolom asli
        rotated[row] |= (1 << (4 - col)); // Geser ke posisi baru
      }
    }
  }
}

// Contoh untuk memutar semua angka
void rotateAllDigits(const uint8_t digits[10][5], uint8_t digits_rotated[10][8])
{
  for (int i = 0; i < 10; i++)
  {
    rotateDigit90(digits[i], digits_rotated[i]);
  }
}

void drawDigitVertical(int digit, int x, int y)
{
  if (digit < 0 || digit > 9)
    return; // Ensure valid digit
  for (int col = 0; col < 5; col++)
  {
    for (int row = 0; row < 8; row++)
    {
      bool pixel = digits[digit][col] & (1 << row);
      matrix.setPoint(y + row, x + col, pixel);
    }
  }
}

// Draw a digit rotated 90 degrees
void drawDigit(int digit, int x, int y)
{
  if (digit < 0 || digit > 9)
    return; // Ensure valid digit

  uint8_t rotated[8];                    // Store rotated digit
  rotateDigit90(digits[digit], rotated); // Rotate the digit

  for (int row = 0; row < 8; row++)
  {
    for (int col = 0; col < 5; col++)
    {
      if (rotated[row] & (1 << col))
      {                                          // Check if the bit is set
        matrix.setPoint(y + row, x + col, true); // Draw the pixel
      }
    }
  }
}
// Draw scores on the matrix
// Draw scores on the matrix
void drawScores()
{
  // Clear only the score area (optional)
  matrix.clear();

  // Player 1's score on the left
  drawDigit(scorePlayer1 / 10, edgePadding, 0);     // Tens place (top)
  drawDigit(scorePlayer1 % 10, edgePadding + 6, 0); // Ones place (below tens)

  // Player 2's score on the right
  drawDigit(scorePlayer2 / 10, MATRIX_WIDTH - edgePadding - 5, 0);  // Tens place (top)
  drawDigit(scorePlayer2 % 10, MATRIX_WIDTH - edgePadding - 11, 0); // Ones place (below tens)
}

void drawPlayer(int yPosition, int xOffset)
{
  for (int row = 0; row < playerHeight; row++)
  {
    if (yPosition + row < 8)
    { // Ensure within bounds of a single matrix
      matrix.setPoint(yPosition + row, xOffset, true);
    }
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
        matrix.setPoint(row, col, true);
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

  matrix.begin();
  matrix.control(MD_MAX72XX::INTENSITY, 5); // Set brightness
  matrix.clear();                           // Clear the matrix

  // matrix_parola.begin();
  // testDots();
  // matrix_parola.setIntensity(5);
  // matrix_parola.displayText("5024221021 - Agus Fuad Mudhofar", PA_CENTER, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

void loop()
{
  unsigned long currentMillis = millis();

  // Perbarui layar hanya jika interval waktu tercapai
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    int potValue = analogRead(POT_PIN);                    // Read pot value
    int potValue2 = analogRead(POT_PIN2);                  // Read pot for Player 2
    playerY = map(potValue, 0, 4095, 0, 8 - playerHeight); // Map pot value to display rows
    player2Y = map(potValue2, 0, 4095, 0, 8 - playerHeight);

    drawPlayer(playerY, edgePadding);                               // Draw player at the new position
    drawPlayer(player2Y, MATRIX_WIDTH - edgePadding - playerWidth); // Player 2 on the right (xOffset = rightmost matrix)

    matrix.clear();
    drawScores();                                         // Draw the scores
    drawPlayer(playerY, edgePadding);                     // Draw Player 1
    drawPlayer(player2Y, MATRIX_WIDTH - edgePadding - 1); // Draw Player 2

    // if (matrix_parola.displayAnimate())
    // {
    //   matrix_parola.displayReset();
    // }
  }
}
