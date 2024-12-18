#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <chrono>
#include <thread>
#include <iostream>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define POT_PLAYER1 13
#define POT_PLAYER2 12

#define DATA_PIN 15
#define CS_PIN 2
#define CLK_PIN 4
const int BUTTON1 = 16;
const int BUTTON2 = 14;

MD_Parola matrix_parola = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
MD_MAX72XX matrix = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

int playerX = 0 + 7;
int player2Y = 31 - 7;
int playerY = 0 + 7;
int player2X = 31 - 7;

int player1Position = 0;
int player2Position = 0;
int rectrow[] = {0, 1, 2};
int playerSize = sizeof(rectrow) / sizeof(rectrow[0]);
int maxPlayerPosition = 8;

int linep1 = 6;
int linep2 = 25;
int linerow[8] = {0, 1, 2, 3, 4, 5, 6, 7};
int score1 = 0;
int score2 = 0;
unsigned long previousBallTime = 0;
unsigned long previousPlayerTime = 0;
unsigned long previousMatrixUpdate = 0;
unsigned long matrixRefreshInterval = 50;

unsigned long button1PressTime = 0;
unsigned long button2PressTime = 0;
bool button1Pressed = false;
bool button2Pressed = false;
const unsigned long smashThreshold = 2000;
unsigned long currentTime;
unsigned long previousAnimateTime = 0;
int skipSpeed = 20;

bool isWaitingForServe = true;
bool animate = 1;
bool isGame = 0;
bool isOpening = 1;
bool displayOnce = 0;
bool skipNameDisplay = false;
int servingPlayer = 1;

int ballX = 0;
int ballY = 0;
int dx = 1;
int dy = 1;

int normalBallSpeed = 100;
int smashSubSpeed = 10;
int maximumBallSpeed = 10;
int ballInterval = normalBallSpeed;
int playerInterval = 100;
bool ballSpeedBoosted = false;
int buttonPressed[2] = {};

const int minCol = 6;
const int maxCol = 25;
const int maxRows = 8;

int scoreLeft = 0;
int scoreRight = 0;

const int tensDigitMap[10][15][2] = {
    {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {1, 4}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {-1, -1}},
    {{2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {-1, -1}},
    {{0, 0}, {1, 0}, {2, 0}, {2, 1}, {0, 2}, {1, 2}, {2, 2}, {0, 3}, {0, 4}, {1, 4}, {2, 4}},
    {{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {1, 2}, {0, 2}, {2, 3}, {2, 4}, {1, 4}, {0, 4}, {-1, -1}},
    {{0, 0}, {2, 0}, {0, 1}, {0, 2}, {2, 1}, {1, 2}, {2, 2}, {2, 3}, {2, 4}, {-1, -1}},
    {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 3}, {2, 4}, {1, 4}, {0, 4}, {-1, -1}},
    {{1, 0}, {0, 0}, {2, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 3}, {2, 4}, {1, 4}, {0, 4}, {0, 3}},
    {{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {-1, -1}},
    {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {0, 3}, {0, 4}, {1, 4}, {2, 4}, {2, 1}, {2, 3}},
    {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 3}, {2, 4}, {1, 4}, {0, 4}, {2, 1}}};

const int unitsDigitMap[10][15][2] = {
    {{5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {6, 4}, {7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {6, 0}, {-1, -1}},
    {{7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {-1, -1}},
    {{5, 0}, {6, 0}, {7, 0}, {7, 1}, {5, 2}, {6, 2}, {7, 2}, {5, 3}, {5, 4}, {6, 4}, {7, 4}, {-1, -1}, {-1, -1}},
    {{5, 0}, {6, 0}, {7, 0}, {7, 1}, {7, 2}, {6, 2}, {5, 2}, {7, 3}, {7, 4}, {6, 4}, {5, 4}, {-1, -1}, {-1, -1}},
    {{5, 0}, {7, 0}, {5, 1}, {5, 2}, {6, 2}, {7, 2}, {7, 3}, {7, 4}, {7, 1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
    {{5, 0}, {6, 0}, {7, 0}, {5, 1}, {5, 2}, {6, 2}, {7, 2}, {7, 3}, {7, 4}, {6, 4}, {5, 4}, {-1, -1}, {-1, -1}},
    {{6, 0}, {5, 0}, {7, 0}, {5, 1}, {5, 2}, {6, 2}, {7, 2}, {7, 3}, {7, 4}, {6, 4}, {5, 4}, {5, 3}, {-1, -1}},
    {{5, 0}, {6, 0}, {7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
    {{5, 0}, {6, 0}, {7, 0}, {5, 1}, {5, 2}, {6, 2}, {7, 2}, {5, 3}, {5, 4}, {6, 4}, {7, 4}, {7, 1}, {7, 3}, {-1, -1}},
    {{5, 0}, {6, 0}, {7, 0}, {5, 1}, {5, 2}, {6, 2}, {7, 2}, {7, 3}, {7, 4}, {6, 4}, {5, 4}, {7, 1}, {-1, -1}}};

void drawPlayer(MD_MAX72XX *matrix, int playerPosition, int playerX, int playerSize)
{
    int start = max(0, playerPosition);
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

void resetBall()
{
    ballX = (minCol + maxCol) / 2;
    ballY = maxRows / 2;
    dx = (random(0, 2) == 0) ? 1 : -1;
    dy = (random(0, 2) == 0) ? 1 : -1;
}

void resetBallWithServe(int player)
{
    isWaitingForServe = true;
    servingPlayer = player;

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

    dx = 0;
    dy = 0;

    MD_MAX72XX *matrix = matrix_parola.getGraphicObject();
    matrix->setPoint(ballY, ballX, true);
    matrix->update();
}

void resetBallWithServebagus(int servingPlayer, int playerPosition, int playerSize, int buttonPin)
{
    int playerCenter = playerPosition + (playerSize / 2);

    if (playerSize % 2 != 0)
    {
        ballY = playerCenter;
    }
    else
    {
        ballY = playerCenter - 1;
    }

    ballX = (servingPlayer == 1) ? playerX : player2Y - 1;

    MD_MAX72XX *matrix = matrix_parola.getGraphicObject();
    matrix->setPoint(ballY, ballX, true);
    matrix->update();

    dx = (servingPlayer == 1) ? 1 : -1;
    dy = 1;
}

void displayUnitsDigitLeft(MD_MAX72XX *matrix, int digit)
{
    if (digit < 0 || digit > 9)
        return;

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
        return;

    for (int segment = 0; segment < 15; segment++)
    {
        int originalRow = unitsDigitMap[digit][segment][0];
        int originalCol = unitsDigitMap[digit][segment][1];

        if (originalRow == -1 && originalCol == -1)
            break;

        int row = 7 - originalRow;
        int col = 31 - originalCol;

        if (row >= 0 && row < 8 && col >= 0 && col < 32)
        {
            matrix->setPoint(row, col, true);
        }
    }
}

void displayTensDigitLeft(MD_MAX72XX *matrix, int digit)
{
    if (digit < 0 || digit > 9)
        return;

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
        return;

    for (int segment = 0; segment < 15; segment++)
    {
        int row = tensDigitMap[digit][segment][0];
        int col = tensDigitMap[digit][segment][1];

        if (row == -1 && col == -1)
            break;

        matrix->setPoint(7 - row, 31 - col, true);
    }
}

void displayLeftScore(MD_MAX72XX *matrix, int score)
{
    if (score < 0 || score > 30)
        return;

    if (score >= 0 && score <= 9)
    {
        displayTensDigitLeft(matrix, score);
    }
    else if (score >= 10 && score <= 30)
    {
        int tens = score / 10;
        int units = score % 10;

        displayTensDigitLeft(matrix, tens);
        displayUnitsDigitLeft(matrix, units);
    }
}

void displayRightScore(MD_MAX72XX *matrix, int score)
{
    if (score < 0 || score > 30)
        return;

    if (score >= 0 && score <= 9)
    {
        displayTensDigitRight(matrix, score);
    }
    else if (score >= 10 && score <= 30)
    {
        int tens = score / 10;
        int units = score % 10;

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
                matrix.setPoint(row, col, true);
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
    if (ballY == playerYYY)
    {
        for (int i = 0; i < playerSize; i++)
        {
            if (ballX == rectrow[i] + playerPosition)
            {
                return true;
            }
        }
    }
    return false;
}

bool handleSmash(int playerPosition, int buttonPin, int &buttonPressed, int playerYYYY)
{
    if (digitalRead(buttonPin) == LOW && buttonPressed == 0)
    {
        if (abs(ballY - playerYYYY) <= 2)
        {
            for (int i = 0; i < playerSize; i++)
            {
                int playerRow = rectrow[i] + playerPosition;
                if (ballX == playerRow)
                {
                    ballInterval -= smashSubSpeed;
                    buttonPressed = 1;
                    return true;
                }
            }
        }
    }

    if (digitalRead(buttonPin) == HIGH && buttonPressed == 1)
    {
        buttonPressed = 0;
    }

    return false;
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
            player1Position = map(analogRead(POT_PLAYER1), 0, 4095, maxPlayerPosition - playerSize, 0);
            player2Position = map(analogRead(POT_PLAYER2), 0, 4095, maxPlayerPosition - playerSize, 0);

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

            if ((servingPlayer == 1 && digitalRead(BUTTON1) == LOW) ||
                (servingPlayer == 2 && digitalRead(BUTTON2) == LOW))
            {
                isWaitingForServe = false;
                dx = 0;
                dy = (servingPlayer == 1) ? 1 : -1;
                matrix->clear();
                return;
            }

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
    if (currentTime - previousBallTime >= ballInterval)
    {
        previousBallTime = currentTime;
        ballX += dx;
        ballY += dy;

        if (ballX <= 0 || ballX >= maxRows - 1)
        {
            dx = -dx;
        }

        if (checkCollision(player1Position, playerY))
        {
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

        if (handleSmash(player1Position, BUTTON1, buttonPressed[0], playerY))
        {
        }

        if (handleSmash(player2Position, BUTTON2, buttonPressed[1], player2Y))
        {
        }

        if (ballY < minCol)
        {
            scoreRight++;
            resetBallWithServe(2);
            ballInterval = normalBallSpeed;
        }
        else if (ballY > maxCol)
        {
            scoreLeft++;
            resetBallWithServe(1);
            ballInterval = normalBallSpeed;
        }
    }
}

void updatePlayerPosition()
{
    if (currentTime - previousPlayerTime >= playerInterval)
    {
        previousPlayerTime = currentTime;

        player1Position = map(analogRead(POT_PLAYER1), 0, 4095, maxPlayerPosition - playerSize, 0);
        player2Position = map(analogRead(POT_PLAYER2), 0, 4095, maxPlayerPosition - playerSize, 0);
    }
}

void updateMatrix()
{
    MD_MAX72XX *matrix = matrix_parola.getGraphicObject();

    if (currentTime - previousMatrixUpdate >= matrixRefreshInterval)
    {
        previousMatrixUpdate = currentTime;

        matrix->clear();

        drawPlayer(matrix, player1Position, playerX, playerSize);
        drawPlayer(matrix, player2Position, player2Y, playerSize);

        displayLeftScore(matrix, scoreLeft);
        displayRightScore(matrix, scoreRight);

        matrix->setPoint(ballX, ballY, true);

        matrix->update();
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(POT_PLAYER1, INPUT);
    pinMode(POT_PLAYER2, INPUT);
    pinMode(BUTTON1, INPUT_PULLUP);
    pinMode(BUTTON2, INPUT_PULLUP);

    randomSeed(analogRead(0));

    matrix_parola.begin();
    matrix_parola.setIntensity(10);

    matrix_parola.displayText("5024221021 - Agus Fuad Mudhofar", PA_CENTER, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    MD_MAX72XX *matrix = matrix_parola.getGraphicObject();
    matrix->clear();
}

void loop()
{
    currentTime = millis();
    if (!isGame)
    {
        if (isOpening)
        {
            if ((digitalRead(BUTTON1) == LOW || digitalRead(BUTTON2) == LOW) && !skipNameDisplay)
            {
                skipNameDisplay = true;
                animate = false;
                matrix_parola.displayClear();

                matrix_parola.displayText("Skipping Display!", PA_CENTER, skipSpeed, skipSpeed, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
                animate = true;
            }

            if (animate)
            {
                if (matrix_parola.displayAnimate())
                {
                    animate = false;
                    if (!skipNameDisplay)
                    {
                        matrix_parola.displayReset();
                    }
                    isGame = true;
                    isOpening = false;
                }
            }
        }
        else
        {
            if (animate)
            {
                if (matrix_parola.displayAnimate())
                {
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
            waitingForServe();
            return;
        }

        updateBall();
        updatePlayerPosition();
        updateMatrix();

        if (ballInterval <= maximumBallSpeed)
            ballInterval = maximumBallSpeed;
        if (scoreLeft >= 30 || (scoreLeft >= 21 && scoreLeft - scoreRight >= 2))
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