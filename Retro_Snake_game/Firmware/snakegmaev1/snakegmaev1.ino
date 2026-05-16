// ================= LIBRARIES =================
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <EEPROM.h>

// ================= DISPLAY =================
Adafruit_PCD8544 display = Adafruit_PCD8544(11, 10, 9, 8, 7);
// ================= CONSTANTS =================
#define SCREEN_W 84
#define SCREEN_H 48
#define CELL 4

// Play area (matches drawRect(0,8,84,40))
#define PLAY_X_MIN 0
#define PLAY_X_MAX SCREEN_W  // 84
#define PLAY_Y_MIN 8
#define PLAY_Y_MAX (PLAY_Y_MIN + 40)  // 48

// Grid dimensions for food/snake alignment
#define GRID_W (PLAY_X_MAX / CELL)                 // 21 columns
#define GRID_H ((PLAY_Y_MAX - PLAY_Y_MIN) / CELL)  // 10 rows

// ================= HIGH SCORE =================
#define EEPROM_ADDR 0       // EEPROM address to store high score (0-1023)
int highScore = 0;          // Current session high score
bool newHighScore = false;  // Flag to trigger save + visual feedback

// ================= BUTTONS =================
#define BTN_UP 2
#define BTN_DOWN 3
#define BTN_RIGHT 4
#define BTN_LEFT 5
#define BUZZER 6

// ================= STATES =================
enum State { MENU,
             GAME,
             HELP,
             DIFFICULTY };
State state = MENU;

// ================= MENU =================
int menuIndex = 0;
bool soundOn = true;
int difficulty = 1;  // 0=Easy, 1=Medium, 2=Hard, 3=Extreme

// ================= SNAKE =================
#define MAX_SNAKE 45
struct Point {
  int x;
  int y;
};

Point snake[MAX_SNAKE];
int length;
int dx, dy;
Point food;

unsigned long lastMove = 0;
int speedDelay = 120;

// ================= FUNCTION FORWARD DECLARATIONS =================
void spawnFood();
void updateDifficulty();
void resetGame();
void playTone(int freq, int dur);
void handleMenuInput();
void drawMenu();
void drawHelp();
void drawDifficulty();
void updateSnake();
bool checkCollision();
void checkFood();
void handleGameInput();
void drawGame();
void gameLoop();
void startupAnimation();


// ================= SOUND =================
void playTone(int freq, int dur) {
  if (soundOn) {
    tone(BUZZER, freq, dur);
  }
}

// ================= DIFFICULTY SPEED =================
// ✅ FIXED: Proper function syntax with braces containing the logic
void updateDifficulty() {
  if (difficulty == 0) speedDelay = 200;       // Easy
  else if (difficulty == 1) speedDelay = 120;  // Medium
  else if (difficulty == 2) speedDelay = 70;   // Hard
  else if (difficulty == 3) speedDelay = 40;   // Extreme ✅ Fixed: = instead of -
}

// ❌ Current: Food can spawn at x=80, y=44 (edge of play area)
// When snake eats it at edge, next move might immediately collide

// ✅ FIXED: Keep food 1 cell away from borders for safer gameplay
void spawnFood() {
  bool validPosition;
  do {
    validPosition = true;

    // ✅ Generate food with 1-cell margin from borders
    // Play area: x[0→83], y[8→47], CELL=4
    // Safe range: x[4→76], y[12→44]
    food.x = 4 + random(0, 19) * CELL;  // 4,8,12...76 (19 positions)
    food.y = 12 + random(0, 9) * CELL;  // 12,16...44 (9 positions)

    // Bounds safety check
    if (food.x < 4 || food.x > 76 || food.y < 12 || food.y > 44) {
      validPosition = false;
      continue;
    }

    // Check snake collision
    for (int i = 0; i < length; i++) {
      if (food.x == snake[i].x && food.y == snake[i].y) {
        validPosition = false;
        break;
      }
    }
  } while (!validPosition);
}
// ================= INITIALIZE GAME =================
void resetGame() {
  length = 3;
  snake[0] = { 40, 24 };
  snake[1] = { 36, 24 };
  snake[2] = { 32, 24 };
  dx = 4;  // move right
  dy = 0;
  updateDifficulty();
  spawnFood();  // ✅ Now compiler knows this function exists
}

// ================= READ BUTTONS =================
void handleMenuInput() {
  // ✅ Simple debouncing with millis() instead of blocking delay()
  static unsigned long lastPress = 0;
  const int DEBOUNCE = 200;

  if (millis() - lastPress < DEBOUNCE) return;

  if (digitalRead(BTN_DOWN) == LOW) {
    menuIndex = (menuIndex + 1) % 4;
    lastPress = millis();
  }
  if (digitalRead(BTN_UP) == LOW) {
    menuIndex = (menuIndex - 1 + 4) % 4;
    lastPress = millis();
  }
  if (digitalRead(BTN_RIGHT) == LOW) {
    if (menuIndex == 0) {
      resetGame();
      state = GAME;
    } else if (menuIndex == 1) {
      soundOn = !soundOn;
      playTone(soundOn ? 800 : 400, 30);  // Feedback tone
    } else if (menuIndex == 2) {
      state = HELP;
    } else if (menuIndex == 3) {
      state = DIFFICULTY;
    }
    lastPress = millis();
  }
}

// ================= DRAW MENU =================
void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);

  // ✅ Title bar (y=0 to y=9)
  display.setCursor(10, 2);
  display.println("RETRO SNAKE");
  display.drawRect(0, 0, 83, 10, BLACK);  // Width 83, height 10

  // ✅ Menu items with consistent 7px vertical spacing
  // Available space: y=11 to y=47 (37 pixels) for 5 items = ~7px each

  // Item 0: START (y=11)
  display.setCursor(0, 11);
  display.println(menuIndex == 0 ? "> START" : "  START");

  // Item 1: SOUND (y=18)
  display.setCursor(0, 19);
  display.print(menuIndex == 1 ? "> SOUND: " : "  SOUND: ");
  display.println(soundOn ? "ON" : "OFF");

  // Item 2: HELP (y=25)
  display.setCursor(0, 27);
  display.println(menuIndex == 2 ? "> HELP" : "  HELP");

  // Item 3: DIFFICULTY (y=32)
  display.setCursor(0, 35);
  display.println(menuIndex == 3 ? "> DIFFICULTY" : "  DIFFICULTY");

  display.display();
}

// ================= HELP SCREEN =================
void drawHelp() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("HELP");
  display.setCursor(0, 10);
  display.println("ARROWS = MOVE");
  display.setCursor(0, 20);
  display.println("LEFT = BACK");
  display.setCursor(0, 30);
  display.println("RIGHT = SELECT");
  display.setCursor(0, 40);
  display.println("UP = BACK");
  display.display();

  if (digitalRead(BTN_UP) == LOW) {
    state = MENU;
    delay(200);
  }
}

// ================= DIFFICULTY SCREEN =================
void drawDifficulty() {
  display.clearDisplay();
  display.setTextSize(1);

  // Header
  display.setCursor(0, 0);
  display.println("DIFFICULTY");
  display.drawLine(0, 9, 84, 9, BLACK);  // Separator

  // ✅ FIXED: Show all 4 options with visual selector (">")
  display.setCursor(22, 18);
  if (difficulty == 0) display.println("EASY");
  else if (difficulty == 1) display.println("MEDIUM");
  else if (difficulty == 2) display.println("HARD");
  else display.println("EXTREME");

  display.setCursor(0, 40);
  display.println("UP=BACK");
  display.drawLine(0, 34, 84, 34, BLACK);
  display.display();

  // ✅ FIXED: Proper 4-level cycling logic (0↔1↔2↔3)
  // Simple debouncing using millis() - non-blocking
  static unsigned long lastPress = 0;
  const int DEBOUNCE = 200;

  if (millis() - lastPress < DEBOUNCE) return;

  // ← LEFT: Cycle backward: 0→3→2→1→0
  if (digitalRead(BTN_LEFT) == LOW) {
    difficulty = (difficulty == 0) ? 3 : difficulty - 1;
    updateDifficulty();  // ✅ Apply new speed immediately
    playTone(600, 30);   // ✅ Feedback tone
    lastPress = millis();
  }

  // → RIGHT: Cycle forward: 0→1→2→3→0
  if (digitalRead(BTN_RIGHT) == LOW) {
    difficulty = (difficulty == 3) ? 0 : difficulty + 1;
    updateDifficulty();  // ✅ Apply new speed immediately
    playTone(800, 30);   // ✅ Feedback tone
    lastPress = millis();
  }

  // ↑ UP: Return to menu
  if (digitalRead(BTN_UP) == LOW) {
    playTone(1000, 50);  // ✅ Confirmation tone
    state = MENU;
    lastPress = millis();
  }
}

// ================= MOVE SNAKE =================
void updateSnake() {
  for (int i = length - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }
  snake[0].x += dx;
  snake[0].y += dy;
}

// ================= CHECK COLLISIONS =================
bool checkCollision() {
  // ✅ BORDER WALL collision (snake hits play area boundary)
  // Snake dies if head touches or crosses any border line
  if (snake[0].x < PLAY_X_MIN || snake[0].x >= PLAY_X_MAX || snake[0].y < PLAY_Y_MIN || snake[0].y >= PLAY_Y_MAX) {
    return true;  // ← Game over!
  }

  // ✅ SELF collision (snake hits its own body)
  for (int i = 1; i < length; i++) {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
      return true;
    }
  }

  return false;
}

// ================= CHECK FOOD =================
void checkFood() {
  if (snake[0].x == food.x && snake[0].y == food.y) {
    if (length < MAX_SNAKE) length++;
    spawnFood();
    playTone(1000, 50);
  }
}

// ================= HANDLE GAME INPUT =================
void handleGameInput() {
  // Prevent 180° turns
  if (digitalRead(BTN_UP) == LOW && dy == 0) {
    dx = 0;
    dy = -CELL;
  }
  if (digitalRead(BTN_DOWN) == LOW && dy == 0) {
    dx = 0;
    dy = CELL;
  }
  if (digitalRead(BTN_LEFT) == LOW && dx == 0) {
    dx = -CELL;
    dy = 0;
  }
  if (digitalRead(BTN_RIGHT) == LOW && dx == 0) {
    dx = CELL;
    dy = 0;
  }
}

// ================= DRAW GAME =================
void drawGame() {
  display.clearDisplay();

  // ✅ STEP 1: Draw PLAY AREA BORDER (visible rectangle)
  display.drawRect(PLAY_X_MIN, PLAY_Y_MIN, 84, 40, BLACK);
  display.drawLine(0, 0, 84, 0, BLACK);
  display.drawLine(0, 0, 0, 8, BLACK);
  // ✅ STEP 2: Draw Food (inside play area only)
  display.fillRect(food.x, food.y, CELL, CELL, BLACK);

  // ✅ STEP 3: Draw Snake (inside play area only)
  for (int i = 0; i < length; i++) {
    if (i == 0) {
      // 🐍 Head: filled with eyes
      display.fillRect(snake[i].x, snake[i].y, CELL, CELL, BLACK);
      display.drawPixel(snake[i].x + 1, snake[i].y + 1, WHITE);
      display.drawPixel(snake[i].x + 2, snake[i].y + 1, WHITE);
    } else {
      // ⬜ Body: outline blocks
      display.drawRect(snake[i].x, snake[i].y, CELL, CELL, BLACK);
    }
  }
  display.setTextSize(1);
  // ✅ STEP 4: Draw SCORE OUTSIDE border (y=0 to y=7)
  display.setCursor(2, 1);  // Top-left, above play area
  display.print("S:");
  display.print(length - 3);
  display.print("|H:");
  display.print(highScore);
  // ✅ Visual indicator when approaching high score
  int currentScore = length - 3;

  // Difficulty icon (single char)
  display.setCursor(58, 1);
  char diffIcon = difficulty == 0 ? 'E' : difficulty == 1 ? 'M'
                                        : difficulty == 2 ? 'H'
                                                          : 'X';
  display.print(diffIcon);

  display.display();
}

// ================= GAME LOOP =================
void gameLoop(){
  handleGameInput();

  if(millis() - lastMove > speedDelay){
    lastMove = millis();
    updateSnake();

    // ✅ Game over check
    if(checkCollision()){
      playTone(150, 100);
      delay(80);
      playTone(100, 200);
      
      int currentScore = length - 3;
      if(currentScore > highScore){
        highScore = currentScore;
        EEPROM.write(EEPROM_ADDR, highScore);
        newHighScore = true;
      }
      
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(8, 10);
      display.println("GAME OVER");
      display.setCursor(15, 25);
      display.print("Score: ");
      display.println(currentScore);
      
      display.setCursor(5, 35);
      if(newHighScore){
        display.println("NEW HIGH!");
        playTone(1500, 200);
        newHighScore = false;
      } else {
        display.println("Press UP");
      }
      display.display();
      
      while(digitalRead(BTN_UP) != LOW){
        if(digitalRead(BTN_LEFT) == LOW) break;
      }
      delay(200);
      
      display.clearDisplay();
      display.display();
      
      state = MENU;
      return;
    } // ← Closes if(checkCollision())

    checkFood();
    drawGame();
  } // ← Closes if(millis()...)
} // ← ✅ CRITICAL: Closes void gameLoop()

  // ================= STARTUP ANIMATION =================
  void startupAnimation() {
    int snakeX[10], snakeY[10], sLen = 3;
    snakeX[0] = 20;
    snakeY[0] = 28;
    snakeX[1] = 16;
    snakeY[1] = 28;
    snakeX[2] = 12;
    snakeY[2] = 28;

    String source = "TELUGU MAD THINKER";
    int posX[] = { 60, 60, 10, 10 };
    int posY[] = { 34, 10, 10, 34 };
    int totalTargets = 6;

    for (int round = 0; round < totalTargets; round++) {
      char letter;
      do {
        letter = source[random(0, source.length())];
      } while (letter == ' ');

      int fx = posX[round % 4];
      int fy = posY[round % 4];

      while (abs(snakeX[0] - fx) > 1 || abs(snakeY[0] - fy) > 1) {
        display.clearDisplay();
        display.drawRect(0, 0, 84, 48, BLACK);

        // Draw letter with background clear
        display.fillRect(fx, fy, 6, 8, WHITE);
        display.setCursor(fx, fy);
        display.print(letter);

        // Move snake body
        for (int i = sLen - 1; i > 0; i--) {
          snakeX[i] = snakeX[i - 1];
          snakeY[i] = snakeY[i - 1];
        }
        // Move head toward target
        if (snakeX[0] < fx) snakeX[0] += 2;
        else if (snakeX[0] > fx) snakeX[0] -= 2;
        if (snakeY[0] < fy) snakeY[0] += 2;
        else if (snakeY[0] > fy) snakeY[0] -= 2;

        // Draw snake
        display.fillRect(snakeX[0], snakeY[0], 4, 4, BLACK);
        display.drawPixel(snakeX[0] + 1, snakeY[0] + 1, WHITE);
        display.drawPixel(snakeX[0] + 2, snakeY[0] + 1, WHITE);
        for (int i = 1; i < sLen - 1; i++) {
          display.drawRect(snakeX[i], snakeY[i], 4, 4, BLACK);
        }
        if (sLen > 1) {
          display.fillRect(snakeX[sLen - 1] + 1, snakeY[sLen - 1] + 1, 2, 2, BLACK);
        }
        display.display();
        delay(20);
      }
      tone(BUZZER, 1000, 40);
      if (sLen < 7) sLen++;
    }

    // Final branding text
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(24, 16);
    display.println("TELUGU");
    display.setCursor(8, 26);
    display.println("MAD THINKER");
    display.display();
    delay(2000);
    display.clearDisplay();
    display.display();
  }

  // ================= SETUP =================
  void setup() {
    display.begin();
    display.setContrast(57);

    // Validate: If EEPROM is uninitialized (255) or corrupted, reset to 0
    if (highScore == 255 || highScore > 200) {  // 200 = reasonable max for this game
      highScore = 0;
      EEPROM.write(EEPROM_ADDR, 0);
    }

    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BUZZER, OUTPUT);

    randomSeed(analogRead(0));
    startupAnimation();
  }

  // ================= LOOP =================
  void loop() {
    switch (state) {
      case MENU:
        drawMenu();
        handleMenuInput();
        break;
      case GAME:
        gameLoop();
        break;
      case DIFFICULTY:
        drawDifficulty();
        break;
      case HELP:
        drawHelp();
        break;
    }
  }