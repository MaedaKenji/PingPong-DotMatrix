#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <chrono>
#include <thread>
#include <iostream>

// Define the hardware type and connections
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4 // Number of 8x8 modules in the chain

// KIRI
#define POT_PLAYER1 13 // Pin analog untuk potensiometer pemain 1
#define POT_PLAYER2 12 // Pin analog untuk potensiometer pemain 2

// KANAN
#define DATA_PIN 15 // DIN pin
#define CS_PIN 2    // CS pin
#define CLK_PIN 4   // CLK pin
const int BUTTON1 = 16;
const int BUTTON2 = 14;

// #define VCC 17
// #define GND 16
// int vcc = 5;
// int gnd = 18;

// Create the matrix object
MD_Parola matrix_parola = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
MD_MAX72XX matrix = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

int playerX = 0 + 7;   // X-coordinate of the player (row)
int player2Y = 31 - 7; // X-coordinate of Player 2
int playerY = 0 + 7;   // X-coordinate of the player (row)
int player2X = 31 - 7; // X-coordinate of Player 2

int player1Position = 0; // Posisi pemain berdasarkan map analog
int player2Position = 0; // Posisi pemain berdasarkan map analog
int rectrow[] = {0, 1, 2};
int playerSize = sizeof(rectrow) / sizeof(rectrow[0]); // sizeof(rectrow) / sizeof(rectrow[0]);
int maxPlayerPosition = 8;

int linep1 = 6;
int linep2 = 25;
int linerow[8] = {0, 1, 2, 3, 4, 5, 6, 7};
int score1 = 0;
int score2 = 0;
unsigned long previousBallTime = 0;   // Waktu terakhir bola diperbarui
unsigned long previousPlayerTime = 0; // Waktu terakhir pemain diperbarui
unsigned long previousMatrixUpdate = 0;
unsigned long matrixRefreshInterval = 20; // Misalnya 5 ms

unsigned long button1PressTime = 0;        // Waktu saat BUTTON1 ditekan
unsigned long button2PressTime = 0;        // Waktu saat BUTTON2 ditekan
bool button1Pressed = false;               // Status BUTTON1 ditekan
bool button2Pressed = false;               // Status BUTTON2 ditekan
const unsigned long smashThreshold = 2000; // 2 detik
unsigned long currentTime;
unsigned long previousAnimateTime = 0;
int skipSpeed = 20;

bool isWaitingForServe = true;
bool animate = 1;
bool isGame = 0;
bool isOpening = 1;
bool displayOnce = 0;
bool skipNameDisplay = false; // Flag untuk melewati tampilan nama
int servingPlayer = 1;        // Pemain yang akan melakukan serve (1 untuk Player 1, 2 untuk Player 2)

// Variabel untuk bola
int ballX = 0; // Posisi awal X
int ballY = 0; // Posisi awal Y
int dx = 1;    // Arah horizontal (1: ke kanan, -1: ke kiri)
int dy = 1;    // Arah vertikal (1: ke bawah, -1: ke atas)

int normalBallSpeed = 100;          // Kecepatan normal bola dalam ms
int smashSubSpeed = 10;             // Kecepatan cepat bola dalam ms
int maximumBallSpeed = 10;          // Kecepatan maximum bola dalam ms
int ballInterval = normalBallSpeed; // Mengatur kecepatan bola
int playerInterval = 100;           // Interval pembaruan pemain (ms)
bool ballSpeedBoosted = false;      // Status apakah kecepatan bola meningkat
int buttonPressed[2] = {};

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


void drawPlayer(MD_MAX72XX *matrix, int playerPosition, int playerX, int playerSize)
{
  // Pastikan posisi pemain tidak melewati batas kiri (0)
  int start = max(0, playerPosition);

  // Pastikan pemain tidak melewati batas kanan (7)
  int end = min(7, playerPosition + playerSize - 1);

  for (int i = start; i <= end; i++)
  {
    matrix->setPoint(i, playerX, true);
  }
}

void drawPlayer1(MD_MAX72XX *matrix, int playerPosition, int playerX, int playerSize)
{
  for (int i = 0; i < playerSize; i++)
  {
    if (playerPosition + i < 8)
    {
      matrix->setPoint(rectrow[i] + playerPosition, playerX, true);
    }
  }
}

// Reset posisi bola setelah gol
void resetBall()
{
  ballX = (minCol + maxCol) / 2;     // Posisikan bola di tengah kolom
  ballY = maxRows / 2;               // Posisikan bola di tengah baris
  dx = (random(0, 2) == 0) ? 1 : -1; // Random arah horizontal
  dy = (random(0, 2) == 0) ? 1 : -1; // Random arah vertikal
}

void resetBallWithServe(int player)
{
  isWaitingForServe = true;
  servingPlayer = player;

  // Tentukan posisi awal bola berdasarkan pemain yang serve
  if (player == 1)
  {
    ballX = playerX + 1;
    ballY = player1Position + (playerSize / 2) - ((playerSize % 2 == 0) ? 1 : 0);
  }
  else
  {
    ballX = player2Y - 1;
    ballY = player2Position + (playerSize / 2) - ((playerSize % 2 == 0) ? 1 : 0);
  }

  // Bola diam hingga tombol ditekan
  dx = 0;
  dy = 0;

  // Perbarui tampilan bola di matrix
  MD_MAX72XX *matrix = matrix_parola.getGraphicObject();
  matrix->setPoint(ballY, ballX, true);
  matrix->update();
}

void resetBallWithServebagus(int servingPlayer, int playerPosition, int playerSize, int buttonPin)
{
  // Hitung posisi tengah pemain
  int playerCenter = playerPosition + (playerSize / 2);

  // Jika ukuran pemain ganjil, bola langsung berada di tengah
  if (playerSize % 2 != 0)
  {
    ballY = playerCenter;
  }
  else
  {
    // Jika ukuran pemain genap, pilih posisi di tengah (pilih salah satu sisi tengah)
    ballY = playerCenter - 1; // Pilih sisi yang lebih rendah
  }

  // Posisikan bola di kolom pemain yang servis
  ballX = (servingPlayer == 1) ? playerX : player2Y - 1;

  MD_MAX72XX *matrix = matrix_parola.getGraphicObject();
  matrix->setPoint(ballY, ballX, true);
  matrix->update();

  // // Tunggu hingga tombol ditekan untuk memulai permainan
  // while (digitalRead(buttonPin) != LOW)
  // {
  //   // matrix_parola.print("Tunggu buttin");
  // }

  // Setelah tombol ditekan, bola dilontarkan
  dx = (servingPlayer == 1) ? 1 : -1; // Tentukan arah horizontal
  // dy = (random(0, 2) == 0) ? 1 : -1;  // Random arah vertikal
  dy = 1;
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
  }
}

void displayTensDigitRight(MD_MAX72XX *matrix, int digit)
{
  if (digit < 0 || digit > 9)
  {
    Serial.println("Invalid difit in tens digit right");
    return; // Invalid digit
  }

  for (int segment = 0; segment < 15; segment++)
  {
    int row = tensDigitMap[digit][segment][0];
    int col = tensDigitMap[digit][segment][1];

    if (row == -1 && col == -1)
      break;

    matrix->setPoint(7 - row, 31 - col, true);
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
  {
    Serial.println("Invalid score in displaying right score");
    return; // Invalid score
  }

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

bool checkCollision(int playerPosition, int playerYYY)
{
  // Cek apakah bola berada di kolom pemain
  if (ballY == playerYYY)
  {
    // Cek apakah bola berada dalam rentang baris pemain
    for (int i = 0; i < playerSize; i++)
    {
      if (ballX == rectrow[i] + playerPosition)
      {
        return true; // Terdeteksi pantulan
      }
    }
  }
  return false; // Tidak ada pantulan
}

bool handleSmash(int playerPosition, int buttonPin, int &buttonPressed, int playerYYYY)
{
  // Serial.println("buttonPressed: " + String(buttonPressed) + ", DigitalRead: " + String(digitalRead(buttonPin)));
  // Serial.println("Smash attempt detected.");
  // Serial.println("ballY: " + String(ballY) + ", playerYYYY: " + String(playerYYYY));
  // Serial.println("playerPosition: " + String(playerPosition) + ", buttonPin: " + String(buttonPin));

  // Cek apakah tombol ditekan
  if (digitalRead(buttonPin) == LOW && buttonPressed == 0)
  {

    // Serial.println("buttonPressed: " + String(buttonPressed) + ", DigitalRead: " + String(digitalRead(buttonPin)));

    // Cek apakah bola berada dalam toleransi kolom pemain ±2
    if (abs(ballY - playerYYYY) <= 2)
    {
      // Cek apakah bola berada dalam baris pemain
      for (int i = 0; i < playerSize; i++)
      {
        int playerRow = rectrow[i] + playerPosition;
        // Serial.println("playerRow: " + String(playerRow) + ", ballX: " + String(ballX));
        if (ballX == playerRow)
        {
          ballInterval -= smashSubSpeed; // Smash berhasil, percepat bola
          // Serial.println("Smash success! Ball interval updated: " + String(ballInterval));
          buttonPressed = 1; // Tandai tombol sudah ditekan
          return true;       // Smash terdeteksi
        }
      }
    }
  }

  // Reset tombol ketika dilepas
  if (digitalRead(buttonPin) == HIGH && buttonPressed == 1)
  {
    buttonPressed = 0; // Reset button state
  }

  return false; // Smash tidak terjadi
}

void waitingForServe()
{
  if (isWaitingForServe)
  {
    if (currentTime - previousMatrixUpdate >= matrixRefreshInterval)
    {
      previousMatrixUpdate = currentTime;
      MD_MAX72XX *matrix = matrix_parola.getGraphicObject();
      matrix->clear();
      // Pemain dapat menggerakkan paddle selama fase serve
      player1Position = map(analogRead(POT_PLAYER1), 0, 4095, maxPlayerPosition - playerSize, 0);
      player2Position = map(analogRead(POT_PLAYER2), 0, 4095, maxPlayerPosition - playerSize, 0);

      // Update posisi bola agar selalu di tengah paddle
      if (servingPlayer == 1)
      {
        ballX = player1Position + (playerSize / 2) - ((playerSize % 2 == 0) ? 1 : 0);
        ballY = 8;
      }
      else
      {
        ballX = player2Position + (playerSize / 2) - ((playerSize % 2 == 0) ? 1 : 0);
        ballY = 31 - 8;
      }

      // Tunggu tombol ditekan
      if ((servingPlayer == 1 && digitalRead(BUTTON1) == LOW) ||
          (servingPlayer == 2 && digitalRead(BUTTON2) == LOW))
      {
        isWaitingForServe = false;
        dx = 0;
        dy = (servingPlayer == 1) ? 1 : -1;
        matrix->clear();
        return;
      }

      // Tampilkan bola dan paddle di matrix
      displayLeftScore(matrix, scoreLeft);
      displayRightScore(matrix, scoreRight);

      drawPlayer(matrix, player1Position, playerX, playerSize);
      drawPlayer(matrix, player2Position, player2Y, playerSize);
      matrix->setPoint(ballX, ballY, true);
      matrix->update();
      return;
    }
  }
}

void updateBall()
{
  // Serial.println("Current Time: " + String(currentTime));
  // Serial.println("Previous Ball Time: " + String(previousBallTime));
  // Serial.println("Current time - previous ball time: " + String(currentTime - previousBallTime));
  // Serial.println("Ball Interval: " + String(ballInterval));
  if (currentTime - previousBallTime >= ballInterval)
  {
    previousBallTime = currentTime;
    // Update posisi bola
    // Serial.println("Dx: " + String(dx) + ", Dy: " + String(dy));
    ballX += dx; // Atas atau bawah
    ballY += dy; // Maju kiri atau maju kanan
    // Serial.println("ballX: " + String(ballX));
    // Serial.println("ballY: " + String(ballY));

    // Deteksi pantulan pada tepi
    if (ballX <= 0 || ballX >= maxRows - 1)
    {
      Serial.println("Terdeteksi pantulan pada tepi");
      dx = -dx;
    }

    // Deteksi pantulan pemain 1
    if (checkCollision(player1Position, playerY))
    {
      // Perubahan dy berdasarkan posisi relatif bola terhadap paddle
      int relativePosition = ballX - player1Position;
      if (relativePosition < (playerSize / 2))
      {
        dx = -1;
      }
      else if (relativePosition > (playerSize / 2))
      {
        dx = 1;
      }
      dy = -dy;
    }

    // Deteksi pantulan pemain 2
    if (checkCollision(player2Position, player2Y))
    {
      int relativePosition = ballX - player2Position;
      if (relativePosition < (playerSize / 2))
      {
        dx = -1;
      }
      else if (relativePosition > (playerSize / 2))
      {
        dx = 1;
      }
      dy = -dy;
    }

    if (handleSmash(player1Position, BUTTON1, buttonPressed[0], playerY)) // Logika smash untuk pemain 1
    {
      Serial.println("Button pressed: " + String(buttonPressed[0]));
      Serial.println("Smash Player 1!");
    }

    if (handleSmash(player2Position, BUTTON2, buttonPressed[1], player2Y)) // Logika smash untuk pemain 2
    {
      Serial.println("Button pressed: " + String(buttonPressed[1]));
      Serial.println("Smash Player 2!");
    }

    // Deteksi gol
    if (ballY < minCol)
    {
      Serial.println("Gol kiri!");
      scoreRight++;
      resetBallWithServe(2); // Pemain 2 serve
      ballInterval = normalBallSpeed;
    }
    else if (ballY > maxCol)
    {
      Serial.println("Gol kanan!");
      scoreLeft++;
      resetBallWithServe(1); // Pemain 1 serve
      ballInterval = normalBallSpeed;
    }
  }
}

void updatePlayerPosition()
{
  // Serial.println("Current Time: " + String(currentTime));
  // Serial.println("Previous Player Time: " + String(previousPlayerTime));
  // Serial.println("Current time - previous player time: " + String(currentTime - previousPlayerTime));
  // Serial.println("Player Interval: " + String(playerInterval));

  // **Pembaharuan posisi pemain**
  if (currentTime - previousPlayerTime >= playerInterval)
  {
    previousPlayerTime = currentTime;

    // Update posisi pemain
    player1Position = map(analogRead(POT_PLAYER1), 0, 4095, maxPlayerPosition - playerSize, 0);
    player2Position = map(analogRead(POT_PLAYER2), 0, 4095, maxPlayerPosition - playerSize, 0);
    // Serial.println("player1Position: " + String(player1Position));
    // Serial.println("player2Position: " + String(player2Position));
  }
}

void updateMatrix()
{
  // // **Pembaharuan tampilan matrix**
  MD_MAX72XX *matrix = matrix_parola.getGraphicObject();

  // Serial.println("Current Time: " + String(currentTime));
  // Serial.println("Previous Matrix Update: " + String(previousMatrixUpdate));
  // Serial.println("Current time - previous matrix update: " + String(currentTime - previousMatrixUpdate));
  // Serial.println("Matrix Refresh Interval: " + String(matrixRefreshInterval));

  if (currentTime - previousMatrixUpdate >= matrixRefreshInterval)
  {
    previousMatrixUpdate = currentTime;

    matrix->clear();

    drawPlayer(matrix, player1Position, playerX, playerSize);
    drawPlayer(matrix, player2Position, player2Y, playerSize);

    // Tampilkan score
    displayLeftScore(matrix, scoreLeft);
    displayRightScore(matrix, scoreRight);

    // Tampilkan bola
    matrix->setPoint(ballX, ballY, true);

    // Perbarui tampilan LED
    matrix->update();
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(POT_PLAYER1, INPUT);
  pinMode(POT_PLAYER2, INPUT);
  pinMode(BUTTON1, INPUT_PULLUP); // Konfigurasi button dengan pull-up internal
  pinMode(BUTTON2, INPUT_PULLUP); // Konfigurasi button dengan pull-up internal

  // Inisialisasi random seed berdasarkan waktu
  randomSeed(analogRead(0));

  // Inisialisasi game
  // startGameWithRandomServe();

  // testDots();
  matrix_parola.begin(); // Initialize the parola
  matrix_parola.setIntensity(10);

  matrix_parola.displayText("5024221021 - Agus Fuad Mudhofar", PA_CENTER, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  MD_MAX72XX *matrix = matrix_parola.getGraphicObject(); // Get the graphics object
  matrix->clear();
}

void loop()
{
  // Serial.println("Animate: " + String(animate));
  // Serial.println("Skip Name Display: " + String(skipNameDisplay));
  currentTime = millis();
  if (!isGame)
  {
    if (isOpening)
    {
      if ((digitalRead(BUTTON1) == LOW || digitalRead(BUTTON2) == LOW) && !skipNameDisplay)
      {
        skipNameDisplay = true;       // Tandai untuk melewati animasi nama
        animate = false;              // Hentikan animasi berjalan
        matrix_parola.displayClear(); // Bersihkan layar

        matrix_parola.displayText("Skipping Display!", PA_CENTER, skipSpeed, skipSpeed, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        animate = true; // Aktifkan kembali animasi untuk teks baru
      }

      if (animate) // Selama animasi masih berjalan
      {
        if (matrix_parola.displayAnimate()) // Jika animasi selesai
        {
          animate = false; // Matikan animasi
          if (!skipNameDisplay)
          {
            matrix_parola.displayReset(); // Reset jika tidak dilewati
          }
          isGame = true;
          isOpening = false;
        }
      }
    }
    else {
      if (animate) {
        if (matrix_parola.displayAnimate()) {
          animate = false;
          isGame = false;
          isOpening = false;
        }
      }
    }
  }

  else
  {
    if (isWaitingForServe)
    {
      // Serial.println("Waiting for serve");
      waitingForServe();
      return;
    }
    
    updateBall();
    updatePlayerPosition();
    updateMatrix();

    // // **Reset Condition**
    if (ballInterval <= maximumBallSpeed) // Kecepatan bola maximum
      ballInterval = maximumBallSpeed;
    if (scoreLeft >= 30  || (scoreLeft >= 21 && scoreLeft - scoreRight >= 2))
    { 
      isGame = false;
      isOpening = false;
      matrix_parola.displayReset();
      matrix_parola.displayText("Player 1 Win", PA_CENTER, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    }
    else if (scoreRight >= 30 || (scoreRight >= 21 && scoreRight - scoreLeft >= 2))
    {
      isGame = false;
      isOpening = false;
      matrix_parola.displayReset();
      matrix_parola.displayText("Player 2 Win", PA_CENTER, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    }
  }
}
