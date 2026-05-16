// ================= LIBRARIES =================
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <EEPROM.h>

// ================= DISPLAY =================
Adafruit_PCD8544 display = Adafruit_PCD8544(11, 10, 9, 8, 7);
const int SCREEN_W = 84;
const int SCREEN_H = 48;
const int CELL = 4;

// ================= BUTTONS & HARDWARE =================
#define BTN_UP 2
#define BTN_DOWN 3
#define BTN_RIGHT 4
#define BTN_LEFT 5
#define BUZZER 6

// ================= STATES =================
enum State {MENU, GAME, HELP, DIFFICULTY, GAME_OVER}; // ✅ PAUSE removed
State state = MENU;

// ================= TIMING & DEBOUNCE =================
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_MS = 180;
unsigned long lastActivity = 0;
unsigned long lastMove = 0;

// ================= SCORE & EEPROM =================
int score = 0;
int highScore = 0;
const int EEPROM_HIGH_SCORE_ADDR = 0;

// ================= MENU & SETTINGS =================
int menuIndex = 0;
bool soundOn = true;
int difficulty = 1; // 0=Easy, 1=Medium, 2=Hard

// ================= SNAKE =================
#define MAX_SNAKE 30
struct Point { int x, y; };
Point snake[MAX_SNAKE];
int length;
int dx, dy;
Point food;
int speedDelay = 120;

// ================= FUNCTION PROTOTYPES =================
void drawSnake(bool retro = true);
void handleMenuInput();
void handleGameInput();
void resetGame();
void updateDifficulty();
void spawnFood();
void drawBorder();
bool isButtonPressed(int pin);
bool isAnyButtonPressed();

// ================= 8-BIT SFX PACK =================
void sfxBootStart()  
  { if(soundOn)
    { tone(BUZZER,440,80); 
     delay(40); 
     tone(BUZZER,880,100); 
    }
  }
void sfxLoadTick()   
  { if(soundOn)
    { tone(BUZZER, 600 + (random(3)*150), 25); }
  }
void sfxReady()      
  { if(soundOn)
    { tone(BUZZER,1200,50); 
      delay(30); 
      tone(BUZZER,1600,80); 
    } 
  }
void sfxBrand()      
  { if(soundOn)
    { tone(BUZZER,523,90); 
      delay(60); 
      tone(BUZZER,659,90); 
      delay(60); 
      tone(BUZZER,784,120); 
    } 
  }
void sfxTitle()      
  { if(soundOn)
    { tone(BUZZER,440,70); 
      delay(40); 
      tone(BUZZER,554,70); 
      delay(40); 
      tone(BUZZER,659,70); 
      delay(40); 
      tone(BUZZER,880,150); 
    } 
  }
void sfxMenuMove()   
  { if(soundOn)
    { tone(BUZZER,800,25); } 
  }
void sfxSelect()     
  { if(soundOn)
    { tone(BUZZER,600,40); }
  }
void sfxEat()        
  { if(soundOn)
    { tone(BUZZER,1000,40); 
      delay(20); 
      tone(BUZZER,1500,40);
    } 
  }
void sfxWall()       
  { if(soundOn)
    { tone(BUZZER,150,100); } 
  }
void sfxGameOver()   
  { if(soundOn)
    { for(int f=600; f>200; f-=50)
      { tone(BUZZER,f,40); 
        delay(20); 
      } 
      noTone(BUZZER); 
    } 
  }
void sfxHighScore()  
  { if(soundOn)
    { tone(BUZZER,1500,100); 
      delay(50); 
      tone(BUZZER,2000,150); 
    } 
  }
void sfxBack(){
  // 🔊 Descending "go back" blip (distinct from menu move/select)
  if(soundOn){
    tone(BUZZER, 700, 30);   // Start mid-tone
    delay(30);               // Brief pause for note separation
    tone(BUZZER, 400, 40);   // Drop to lower tone = "back" feel
  }
}

// ================= SETUP =================
void setup() {
  display.begin();
  display.setContrast(58);
  EEPROM.get(EEPROM_HIGH_SCORE_ADDR, highScore);
   // Validate: must be 0-999 (reasonable game range)
  if(highScore < 0 || highScore > 999) {
    highScore = 0; // Reset garbage to 0
   // Optional: Pre-write 0 to EEPROM to prevent future garbage reads
    EEPROM.put(EEPROM_HIGH_SCORE_ADDR, highScore);
  }

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  introBoot();
  revealDecor();
  revealRetroSnake();
}

// ================= MAIN LOOP =================
void loop() {

  // 🎮 STATE HANDLER (Clean & Non-Blocking)
  switch(state) {
    case MENU:        drawMenu(); handleMenuInput(); break;
    case GAME:        gameLoop(); break;
    case HELP:        drawHelp(); break;
    case DIFFICULTY:  drawDifficulty(); break;
    case GAME_OVER:   drawGameOver(); break;
  }
}

// ================= NON-BLOCKING INPUT =================
bool isButtonPressed(int pin) {
  if (digitalRead(pin) == LOW) {
    unsigned long now = millis();
    if (now - lastButtonPress >= DEBOUNCE_MS) {
      lastButtonPress = now;
      return true;
    }
  }
  return false;
}

bool isAnyButtonPressed() {
  if (digitalRead(BTN_UP)==LOW || digitalRead(BTN_DOWN)==LOW ||
      digitalRead(BTN_LEFT)==LOW || digitalRead(BTN_RIGHT)==LOW) {
    unsigned long now = millis();
    if (now - lastButtonPress >= DEBOUNCE_MS) {
      lastButtonPress = now;
      return true;
    }
  }
  return false;
}

// ================= MENU HANDLER =================
void handleMenuInput() {
  if(isButtonPressed(BTN_DOWN)) 
   { menuIndex = (menuIndex + 1) % 4; 
     sfxMenuMove(); 
   }
  if(isButtonPressed(BTN_UP))   
   { menuIndex = (menuIndex - 1 + 4) % 4; 
     sfxMenuMove();
   }
  if(isButtonPressed(BTN_RIGHT)) {
    sfxSelect();
    if(menuIndex == 0) 
     { resetGame(); state = GAME; }
    else if(menuIndex == 1) 
     { soundOn = !soundOn; }
    else if(menuIndex == 2) 
     { state = HELP; }
    else if(menuIndex == 3) 
     { state = DIFFICULTY; }
  }
}

// ================= GAME INPUT (SIMPLIFIED - NO PAUSE) =================
void handleGameInput() {
  unsigned long now = millis();
  
  // 🎮 Directional controls (debounced)
  if(isButtonPressed(BTN_UP) && dy == 0) {
    dx = 0; dy = -4; 
    lastActivity = now;
  }
  if(isButtonPressed(BTN_DOWN) && dy == 0) {
    dx = 0; dy = 4; 
    lastActivity = now;
  }
  if(isButtonPressed(BTN_RIGHT) && dx == 0) {
    dx = 4; dy = 0; 
    lastActivity = now;
  }
  if(isButtonPressed(BTN_LEFT) && dx == 0) {
    dx = -4; dy = 0;
    lastActivity = now;
  }
}

// ================= GAME LOOP =================
void gameLoop() {
  handleGameInput();
  if(millis() - lastMove > speedDelay) {
    lastMove = millis();
    updateSnake();
    if(checkCollision()) {
      sfxWall();
      sfxGameOver(); // 🔊 Game over melody
      if(score > highScore) {
        highScore = score;
        EEPROM.put(EEPROM_HIGH_SCORE_ADDR, highScore);
      }
      state = GAME_OVER; return;
    }
    checkFood();
    drawGame();
  }
}

// ================= HELPERS =================
void updateDifficulty() {
  if(difficulty == 0) speedDelay = 180;
  else if(difficulty == 1) speedDelay = 120;
  else if(difficulty == 2) speedDelay = 70;
  else speedDelay = 40;
}

void spawnFood() {
  food.x = (random(SCREEN_W/4)) * 4;
  food.y = (random(SCREEN_H/4)) * 4;
}

bool checkCollision() {
  if(snake[0].x < 0 || snake[0].x >= SCREEN_W || snake[0].y < 0 || snake[0].y >= SCREEN_H) return true;
  for(int i = 1; i < length; i++) if(snake[0].x == snake[i].x && snake[0].y == snake[i].y) return true;
  return false;
}

// ================= CHECK FOOD =================
void checkFood(){
  if(snake[0].x == food.x && snake[0].y == food.y){
    if(length < MAX_SNAKE) length++;
    score++; 
    
    // 👇 VISUAL FLASH at eaten food position (BEFORE spawning new food)
    display.fillRect(food.x-1, food.y-1, CELL+2, CELL+2, WHITE);
    display.display();
    delay(30); // Brief blocking pause - acceptable for retro "pop" feel
    display.fillRect(food.x-1, food.y-1, CELL+2, CELL+2, BLACK); // Clear flash
    
    // 👇 Spawn NEW food AFTER flash
    spawnFood();
    
    // 👇 Keep existing sound (already respects soundOn)
    sfxEat();
  }
}

void updateSnake() {
  for(int i = length-1; i > 0; i--) snake[i] = snake[i-1];
  snake[0].x += dx; snake[0].y += dy;
}

void resetGame() {
  score = 0; length = 3;
  snake[0] = {40, 24}; 
  snake[1] = {36, 24}; 
  snake[2] = {32, 24};
  dx = 4; dy = 0;
  updateDifficulty(); 
  spawnFood();
  lastMove = millis(); 
  lastActivity = millis();
}

// ================= DRAWING =================
void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10,2); 
  display.drawRect(0,0,84,11,BLACK);
  display.println(F("RETRO SNAKE"));
  display.setCursor(0,14); 
  display.println(menuIndex==0 ? F("> START") : F("  START"));
  display.setCursor(0,22); 
  display.print(menuIndex==1 ? F("> SOUND: ") : F("  SOUND: "));
  display.println(soundOn ? F("ON") : F("OFF"));
  display.setCursor(0,30); 
  display.println(menuIndex==2 ? F("> HELP") : F("  HELP"));
  display.setCursor(0,38); 
  display.println(menuIndex==3 ? F("> DIFFICULTY") : F("  DIFFICULTY"));
  display.display();
}

void drawHelp() {
  display.clearDisplay();
  display.println(F("HELP"));
  display.setCursor(0,10); 
  display.println(F("UP/DOWN = MENU"));
  display.setCursor(0,20); 
  display.println(F("UP = BACK"));
  display.setCursor(0,30); 
  display.println(F("RIGHT = SELECT"));
  display.setCursor(0,40); 
  display.println(F("LEFT = MOVE")); // ✅ Updated: no pause mention
  display.display();
  if(isButtonPressed(BTN_UP)) {
   sfxBack();  // 🔊 Play back sound
   state = MENU;
  }
}

void drawDifficulty() {
  display.clearDisplay();
  display.setCursor(0,2);
  display.println(F("DIFFICULTY"));
  display.drawLine(0,12,84,12,BLACK);
  display.setCursor(10,17);
  if(difficulty==0) 
   display.println(F("EASY"));
  else if(difficulty==1) 
   display.println(F("MEDIUM"));
  else if(difficulty==2) 
   display.println(F("HARD"));
  else display.println(F("EXTREME"));
  display.drawLine(0,29,84,29,BLACK);
  display.setCursor(0,32); 
  display.println(F("UP=BACK"));
  display.display();

  if(isButtonPressed(BTN_LEFT))  difficulty = (difficulty==0)?3:difficulty-1;
  if(isButtonPressed(BTN_RIGHT)) difficulty = (difficulty==3)?0:difficulty+1;
    // 👇 Back navigation with SFX
  if(isButtonPressed(BTN_UP)){
    sfxBack();  // 🔊 Play back sound
    state = MENU;
  }
}

void drawGameOver() {
  display.clearDisplay(); 
  drawBorder();
  display.setTextSize(1); 
  display.setCursor(14, 3); 
  display.println(F("GAME OVER"));
  display.drawLine(0, 11, 83, 11, BLACK);
  display.setCursor(10, 14); 
  display.print(F("SCORE: ")); 
  display.println(score);
  display.setCursor(10, 24); 
  display.print(F("HIGH:  ")); 
  display.println(highScore);
  display.drawLine(0, 36, 83, 36, BLACK);
  display.setCursor(14, 39); 
  display.println(F("PRESS UP"));
  display.display();
  // 👇 Return to menu with SFX
  if(isButtonPressed(BTN_UP)){
    sfxBack();  // 🔊 Play back sound (returning to menu)
    state = MENU;
  }
}

void drawBorder() 
 { display.drawRect(0, 0, SCREEN_W, SCREEN_H, BLACK); }

void drawGame() {
  display.clearDisplay(); 
  drawBorder();
  display.fillRect(food.x, food.y, CELL, CELL, BLACK);
  drawSnake(true);
  display.display();
}

void drawSnake(bool retro) {
  if (length < 1) return;
  for (int i = 1; i < length - 1; i++) {
    if (retro) {
      display.drawRect(snake[i].x, snake[i].y, CELL, CELL, BLACK);
     }
    else {
      display.fillRect(snake[i].x, snake[i].y, CELL, CELL, BLACK);
     }
  }
  int hx = snake[0].x, hy = snake[0].y;
  if (retro) {
    display.fillRect(hx, hy, CELL, CELL, BLACK);
    if (dx == 4) 
     { display.drawPixel(hx+4, hy+1, BLACK); 
       display.drawPixel(hx+4, hy+2, BLACK); 
       display.drawPixel(hx+2, hy+1, WHITE); }
    else if (dx == -4) 
     { display.drawPixel(hx-1, hy+1, BLACK); 
       display.drawPixel(hx-1, hy+2, BLACK); 
       display.drawPixel(hx+1, hy+1, WHITE); }
    else if (dy == -4) 
     { display.drawPixel(hx+1, hy-1, BLACK); 
       display.drawPixel(hx+2, hy-1, BLACK); 
       display.drawPixel(hx+1, hy+1, WHITE); }
    else if (dy == 4) 
     { display.drawPixel(hx+1, hy+4, BLACK); 
       display.drawPixel(hx+2, hy+4, BLACK); 
       display.drawPixel(hx+1, hy+2, WHITE); }
  }
  if (length >= 2 && retro) {
    int t = length - 1, tx = snake[t].x, ty = snake[t].y, px = snake[t-1].x, py = snake[t-1].y;
    if (px > tx) 
     { display.fillRect(tx+1, ty+1, 3, 2, BLACK); 
       display.drawPixel(tx, ty+2, BLACK); }
    else if (px < tx) 
     { display.fillRect(tx, ty+1, 3, 2, BLACK);
       display.drawPixel(tx+3, ty+2, BLACK); }
    else if (py > ty) 
     { display.fillRect(tx+1, ty+1, 2, 3, BLACK); 
       display.drawPixel(tx+2, ty, BLACK); }
    else if (py < ty) 
     { display.fillRect(tx+1, ty, 2, 3, BLACK); 
       display.drawPixel(tx+2, ty+3, BLACK); }
  } else if (length >= 2) {
    display.fillRect(snake[length-1].x, snake[length-1].y, CELL, CELL, BLACK);
  }
}


// ================= OPTIMIZED STARTUP ANIMATION =================
void introBoot() {
    // Frame 1: Loading + Progress Bar
    sfxBootStart();
    display.clearDisplay();
    drawBorder();
    display.setCursor(16, 8);
    display.println(F("LOADING"));       // F() saves ~7 bytes RAM
    display.drawRect(10, 28, 64, 8, BLACK);
    display.display();

    for (int i = 1; i <= 6; i++) {
        display.fillRect(12, 30, 56, 4, WHITE);          // Clear bar area
        display.fillRect(12, 30, i * 9, 4, BLACK);       // Draw progress
        display.display();
        sfxLoadTick(); // 🔊 Loading blip per segment
        delay(70); // Snappier than 120ms
    }
    delay(150);

    // Frame 2: READY?
    display.clearDisplay();
    drawBorder();
    display.setCursor(12, 20);
    display.setTextSize(2);
    display.println(F("READY?"));
    display.display();
    sfxReady();
    delay(600); // Reduced from 1000ms

    // Frame 3: TELUGU MAD THINKER
    display.clearDisplay();
    drawBorder();
    display.setTextSize(1);
    display.setCursor(14, 10);
    display.print(F("TELUGU MAD"));
    display.setCursor(20, 24);
    display.print(F("THINKER"));
    display.display();
    sfxBrand();  // 🔊 Cultural pride chime
    delay(600);
}

void revealDecor() {
    display.clearDisplay();
    drawBorder();
    for (int x = 0; x < SCREEN_W; x += 6) {
        display.drawLine(x, 0, x, SCREEN_H - 1, BLACK);
        display.display();
        delay(18); // Faster vertical reveal
    }
    delay(80);
}

void revealRetroSnake() {
    display.clearDisplay();
    drawBorder();
    for (int i = 8; i < 76; i += 8) {
        display.fillRect(i, 6, 4, 3, BLACK);
    }
    display.setTextColor(BLACK);
    display.setTextSize(1);
    display.setCursor(28, 14);
    display.println(F("RETRO"));
    display.setTextSize(2);
    display.setCursor(12, 26);
    display.println(F("SNAKE"));
    display.display();
    sfxTitle(); // 🔊 Arcade fanfare finale
    delay(400);
}