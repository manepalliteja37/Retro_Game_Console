#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Nokia 5110 pins: CLK, DIN, D/C, CE, RST
Adafruit_PCD8544 display = Adafruit_PCD8544(11, 10, 9, 8, 7);

const int SCREEN_W = 84;
const int SCREEN_H = 48;
const int CELL = 4;
const int GRID_W = 21;
const int GRID_H = 12;

struct Point {
  int x;
  int y;
};

Point snake[18];
int snakeLen = 6;

int dx = 4;
int dy = 0;

// 6 clean target positions inside border
int targetX[6] = {72, 72, 20, 20, 56, 36};
int targetY[6] = {40, 12, 12, 32, 24, 40};

// choose 6 letters from "Telugu Mad Thinker"
char eatLetters[7] = {'T', 'E', 'L', 'U', 'G', 'U', 'M'};

void setup() {
  display.begin();
  display.setContrast(57);
  display.clearDisplay();
  introBoot();
  startupAnimation();
  revealDecor();
  revealRetroSnake();

}

void loop() {
}

// ================= DRAW BORDER =================
void drawBorder() {
  display.drawRect(0, 0, 84, 48, BLACK);
}

// loading animation
void introBoot() {


  // Frame 1 with progress bar
  display.clearDisplay();
  drawBorder();
  display.setCursor(16, 8);
  display.println("LOADING");
  display.drawRect(10, 28, 64, 8, BLACK);
  for (int i = 0; i < 5; i++) {
    display.fillRect(12, 30, i * 10, 4, BLACK);
    display.display();
    delay(120);
  }
  delay(300);

  // Frame 3 with snake sprite look
  display.clearDisplay();
  drawBorder();
  display.setCursor(12, 20);
  display.setTextSize(2);
  display.println("READY?");
  display.display();
  delay(900);
}

//frame 3 decor
void revealDecor() {
  // reveal animation
  display.clearDisplay();
  drawBorder();
  for (int x = 0; x < SCREEN_W; x += 6) {
    display.drawLine(x, 0, x, SCREEN_H - 1, BLACK);
    display.display();
    delay(35);
  }

  delay(200);
}
// reveal retro snake

void revealRetroSnake() {
  display.clearDisplay();
  drawBorder();
  // Decorative snake stripe
  for (int i = 8; i < 76; i += 8) {
    display.fillRect(i, 6, 4, 3, BLACK);
  }

  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(28, 14);
  display.println("RETRO");

  display.setTextSize(2);
  display.setCursor(12, 26);
  display.println("SNAKE");

  display.display();
}


// ================= DRAW CELL =================
void drawCell(int gx, int gy, bool color = BLACK) {
  display.fillRect(gx * CELL, gy * CELL, CELL, CELL, color ? BLACK : WHITE);
}

// ================= DRAW SNAKE =================
void drawSnakeRetro() {
  // body
  for (int i = 1; i < snakeLen - 1; i++) {
    display.drawRect(snake[i].x, snake[i].y, 4, 4, BLACK);
  }

  // head
  int hx = snake[0].x;
  int hy = snake[0].y;
  display.fillRect(hx, hy, 4, 4, BLACK);

  if (dx == 4) {
    display.drawPixel(hx + 4, hy + 1, BLACK);
    display.drawPixel(hx + 4, hy + 2, BLACK);
    display.drawPixel(hx + 2, hy + 1, WHITE);
  }
  else if (dx == -4) {
    display.drawPixel(hx - 1, hy + 1, BLACK);
    display.drawPixel(hx - 1, hy + 2, BLACK);
    display.drawPixel(hx + 1, hy + 1, WHITE);
  }
  else if (dy == -4) {
    display.drawPixel(hx + 1, hy - 1, BLACK);
    display.drawPixel(hx + 2, hy - 1, BLACK);
    display.drawPixel(hx + 1, hy + 1, WHITE);
  }
  else if (dy == 4) {
    display.drawPixel(hx + 1, hy + 4, BLACK);
    display.drawPixel(hx + 2, hy + 4, BLACK);
    display.drawPixel(hx + 1, hy + 2, WHITE);
  }

  // tail
  int t = snakeLen - 1;
  int tx = snake[t].x;
  int ty = snake[t].y;

  int px = snake[t - 1].x;
  int py = snake[t - 1].y;

  if (px > tx) {
    display.fillRect(tx + 1, ty + 1, 3, 2, BLACK);
    display.drawPixel(tx, ty + 2, BLACK);
  }
  else if (px < tx) {
    display.fillRect(tx, ty + 1, 3, 2, BLACK);
    display.drawPixel(tx + 3, ty + 2, BLACK);
  }
  else if (py > ty) {
    display.fillRect(tx + 1, ty + 1, 2, 3, BLACK);
    display.drawPixel(tx + 2, ty, BLACK);
  }
  else if (py < ty) {
    display.fillRect(tx + 1, ty, 2, 3, BLACK);
    display.drawPixel(tx + 2, ty + 3, BLACK);
  }
}

// ================= MOVE TOWARD TARGET =================
void moveSnakeToward(int fx, int fy) {
  for (int i = snakeLen - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }

  if (snake[0].x < fx) {
    snake[0].x += 4;
    dx = 4; dy = 0;
  }
  else if (snake[0].x > fx) {
    snake[0].x -= 4;
    dx = -4; dy = 0;
  }
  else if (snake[0].y < fy) {
    snake[0].y += 4;
    dx = 0; dy = 4;
  }
  else if (snake[0].y > fy) {
    snake[0].y -= 4;
    dx = 0; dy = -4;
  }
}

// ================= STARTUP ANIMATION =================
void startupAnimation() {
  // snake initial position
  snakeLen = 4;
  snake[0] = {8, 24};
  snake[1] = {4, 24};
  snake[2] = {0, 24};
  snake[3] = {-4, 24};

  String eaten = "";

  for (int round = 0; round < 6; round++) {
    int fx = targetX[round];
    int fy = targetY[round];
    char currentLetter = eatLetters[round];

    while (snake[0].x != fx || snake[0].y != fy) {
      display.clearDisplay();
      drawBorder();

      // top info row
      display.drawLine(0, 9, 83, 9, BLACK);
      display.setTextSize(1);
      display.setCursor(2, 1);
      display.print(eaten);

      // draw target letter instead of food block
      display.setTextSize(1);
      display.setTextColor(BLACK, WHITE);
      display.setCursor(fx - 1, fy - 2);
      display.print(currentLetter);

      moveSnakeToward(fx, fy);
      drawSnakeRetro();

      display.display();
      delay(120);
    }

    // eat effect
    for (int k = 0; k < 2; k++) {
      display.clearDisplay();
      drawBorder();
      display.drawLine(0, 9, 83, 9, BLACK);
      display.setTextSize(1);
      display.setCursor(2, 1);
      display.print(eaten);
      drawSnakeRetro();
      display.display();
      delay(60);

      display.clearDisplay();
      drawBorder();
      display.drawLine(0, 9, 83, 9, BLACK);
      display.setTextSize(1);
      display.setCursor(2, 1);
      display.print(eaten);
      display.setTextSize(1);
      display.setTextColor(BLACK, WHITE);
      display.setCursor(fx - 1, fy - 2);
      display.print(currentLetter);
      drawSnakeRetro();
      display.display();
      delay(60);
    }

    eaten += currentLetter;

    if (snakeLen < 17) {
      snake[snakeLen] = snake[snakeLen - 1];
      snakeLen++;
    }
  }

  // reveal text
  display.clearDisplay();
  drawBorder();
  display.setTextSize(1);
  display.setCursor(14, 10);
  display.print("TELUGU MAD");

  display.setCursor(20, 24);
  display.print("THINKER");
  display.display();
  delay(2000);

  display.clearDisplay();
  display.display();
}