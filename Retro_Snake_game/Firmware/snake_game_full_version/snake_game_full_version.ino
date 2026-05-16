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

// 1. Heart (8x8)
const unsigned char PROGMEM heart_icon[] = {
  0x00, 0x66, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x18
};
// 2. Smiley (8x8)
const unsigned char PROGMEM smiley_icon[] = {
  0x3C, 0x42, 0xA5, 0x81, 0xA5, 0x99, 0x42, 0x3C
};
// screw icon
const unsigned char screwdriver_icon[] PROGMEM = {
  0x18, 0x3C, 0x7E, 0x3C, 0x18, 0x18, 0x18, 0x18   
};

// ================= BUTTONS & HARDWARE =================
#define BTN_UP 2
#define BTN_DOWN 3
#define BTN_RIGHT 4
#define BTN_LEFT 5
#define BUZZER 6

// ================= STATES =================
enum State {MENU, GAME, HELP, DIFFICULTY, GAME_OVER, SCORES, ABOUT}; 
State state = MENU;

// ================= SCROLLING MENU =================
const char* menuItems[] = {
  "START", "SOUND", "HELP", 
  "DIFFICULTY", "SCORES", "ABOUT"
};

const int MENU_ITEMS = 6;
const int VISIBLE_ITEMS = 4; // Fits on 84x48 screen with 8px spacing

// ================= SCORE & EEPROM (TOP 4) =================
int score = 0;
int highScores[4] = {0, 0, 0, 0};  // 👈 Top 4 scores array
const int EEPROM_ADDR = 0;          // 👈 Single base address (8 bytes total)

// ================= TIMING & DEBOUNCE =================
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_MS = 180;
unsigned long lastActivity = 0;
unsigned long lastMove = 0;

// ================= MENU & SETTINGS =================
int menuIndex = 0;
bool soundOn = true;
int difficulty = 1; // 0=Easy, 1=Medium, 2=Hard 3=Extreme

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

// ================= 8-BIT SFX PACK functions =================
//boot start sound
void sfxBootStart()  
  { if(soundOn)
    { tone(BUZZER,440,80); 
     delay(40); 
     tone(BUZZER,880,100); 
    }
  }
//loading sound
void sfxLoadTick()   
  { if(soundOn)
    { tone(BUZZER, 600 + (random(3)*150), 25); }
  }
// ready sound
void sfxReady()      
  { if(soundOn)
    { tone(BUZZER,1200,50); 
      delay(30); 
      tone(BUZZER,1600,80); 
    } 
  }
// telugu mad thinker sound
void sfxBrand()      
  { if(soundOn)
    { tone(BUZZER,523,90); 
      delay(60); 
      tone(BUZZER,659,90); 
      delay(60); 
      tone(BUZZER,784,120); 
    } 
  }
// rtero snake sound
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
// button movement sound
void sfxMenuMove()   
  { if(soundOn)
    { tone(BUZZER,800,25); } 
  }
// button select sound
void sfxSelect()     
  { if(soundOn)
    { tone(BUZZER,600,40); }
  }
// snake eat sound
void sfxEat()        
  { if(soundOn)
    { tone(BUZZER,1000,40); 
      delay(20); 
      tone(BUZZER,1500,40);
    } 
  }
// snake wall hit sound
void sfxWall()       
  { if(soundOn)
    { tone(BUZZER,150,100); } 
  }
// game over screen sound
void sfxGameOver()   
  { if(soundOn)
    { for(int f=600; f>200; f-=50)
      { tone(BUZZER,f,40); 
        delay(20); 
      } 
      noTone(BUZZER); 
    } 
  }
// highscore sound
void sfxHighScore()  
  { if(soundOn)
    { tone(BUZZER,1500,100); 
      delay(50); 
      tone(BUZZER,2000,150); 
    } 
  }
// back button sound
void sfxBack(){
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
  // 👇 LOAD & VALIDATE TOP 4 SCORES
  bool corrupted = false;
  for(int i = 0; i < 4; i++) {
    EEPROM.get(EEPROM_ADDR + (i * 2), highScores[i]);
    // Validate: reasonable score range 0-9999
    if(highScores[i] < 0 || highScores[i] > 9999) {
      highScores[i] = 0;
      corrupted = true;
    }
  }
  // If any corruption found, reset all to 0
  if(corrupted) {
    for(int i = 0; i < 4; i++) {
      EEPROM.put(EEPROM_ADDR + (i * 2), 0);
    }
  }
//pullup to read button value without noise
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

  // 👇 Updated state switch
  switch(state){
    case MENU:        drawMenu(); handleMenuInput(); break;
    case GAME:        gameLoop(); break;
    case HELP:        drawHelp(); break;
    case DIFFICULTY:  drawDifficulty(); break;
    case GAME_OVER:   drawGameOver(); break;
    case SCORES:      drawHighScores(); break;  
    case ABOUT:       drawAbout(); break;        
  }
}

//debounce
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
  if(isButtonPressed(BTN_DOWN)){
    menuIndex = (menuIndex + 1) % MENU_ITEMS;
    sfxMenuMove();
  }
  if(isButtonPressed(BTN_UP)){
    menuIndex = (menuIndex - 1 + MENU_ITEMS) % MENU_ITEMS;
    sfxMenuMove();
  }
  if(isButtonPressed(BTN_RIGHT)) {
    sfxSelect();
    if(menuIndex == 0) 
     { resetGame(); state = GAME; }
    else if(menuIndex == 1) 
     { soundOn = !soundOn; 
           // Optional: Visual feedback on menu line
      display.setCursor(60, 22);  // Right side of SOUND line
      display.print(soundOn ? F("ON") : F("OFF"));
      display.display(); 
      delay(100);
     }
    else if(menuIndex == 2) 
     { state = HELP; }
    else if(menuIndex == 3) 
     { state = DIFFICULTY; }
    else if(menuIndex == 4) 
    { state = SCORES; }  // 👈 New
    else if(menuIndex == 5) 
    { state = ABOUT; } 
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
      updateHighScores(score);  // 👈 Handles sorting + EEPROM save + SFX
      state = GAME_OVER; 
      return;
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

// ================= SAVE HIGH SCORE (TOP 4) =================
void updateHighScores(int newScore){
  // Insert new score in sorted descending order
  for(int i = 0; i < 4; i++){
    if(newScore > highScores[i]){
      // Shift lower scores down to make room
      for(int j = 3; j > i; j--){
        highScores[j] = highScores[j-1];
      }
      highScores[i] = newScore;
      // Play fanfare ONLY if new #1 score
      if(i == 0 && soundOn) sfxHighScore();
      break; // Stop after inserting
    }
  }
  // Save all 4 scores to EEPROM (2 bytes each = 8 bytes total)
  for(int i = 0; i < 4; i++){
    EEPROM.put(EEPROM_ADDR + (i * 2), highScores[i]);
  }
}
//food random on game play
void spawnFood() {
  food.x = (random(SCREEN_W/4)) * 4;
  food.y = (random(SCREEN_H/4)) * 4;
}
// self collison and wall collison check
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
// after eating update snake length
void updateSnake() {
  for(int i = length-1; i > 0; i--) snake[i] = snake[i-1];
  snake[0].x += dx; snake[0].y += dy;
}
// reset game to start fresh game
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

// ================= SCROLLING MENU DRAW =================
void drawMenu(){
  display.clearDisplay();
  
  // 👇 Title bar
  display.setCursor(10, 0);
  display.setTextSize(1);
  display.println(F("RETRO SNAKE"));
  display.drawLine(0, 10, 83, 10, BLACK);
  
  // 👇 Scrolling logic: keep cursor centered when possible
  int start = menuIndex - 1;
  if(start < 0) start = 0;
  if(start > MENU_ITEMS - VISIBLE_ITEMS) start = MENU_ITEMS - VISIBLE_ITEMS;
  
  // 👇 Draw visible items (8px vertical spacing)
  for(int i = 0; i < VISIBLE_ITEMS; i++){
    int idx = start + i;
    display.setCursor(0, 14 + (i * 8));
    
    // Cursor indicator
    if(idx == menuIndex) display.print(F("> "));
    else display.print(F("  "));
    
    // Menu item text
    display.println(menuItems[idx]);
  }
  display.display();
}

// Help section page
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
// difficulty section page
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
// game over screen page
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
  display.println(highScores[0]);
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

// ================= HIGH SCORES SCREEN =================
void drawHighScores(){
  display.clearDisplay();
  display.setCursor(10, 1); 
  display.println(F("HIGH SCORES"));
  display.drawLine(0, 9, 83, 9, BLACK);
  
  for(int i = 0; i < 4; i++){
    display.setCursor(6, 11 + (i * 8));
    display.print(i + 1); 
    display.print(F(". "));
    display.println(highScores[i]);
  }
  display.setCursor(30, 40); 
  display.println(F("UP = BACK"));
  display.display();
  
  if(isButtonPressed(BTN_UP)){
    sfxBack();
    state = MENU;
  }
}

// ================= ABOUT SCREEN (3 PAGES) =================
void drawAbout(){
  static int page = 0;  // 0, 1, or 2 - persists across calls
  
  display.clearDisplay();
  display.setCursor(0, 0); 
  display.println(F("ABOUT"));
  display.drawLine(0, 8, 83, 8, BLACK);
  
  // 👇 PAGE 0: Brand Story
  if(page == 0){
    display.setCursor(0, 10); 
    display.println(F("Build with"));
    display.drawBitmap(60,10, heart_icon, 8, 8, BLACK); // ❤
    display.setCursor(0, 20); 
    display.println(F("Telugu Mad"));
    display.setCursor(0, 30); 
    display.println(F("Thinker"));
    display.drawBitmap(46,30, smiley_icon, 8, 8, BLACK); // 😊
    display.setCursor(60, 40); 
    display.println(F(">>"));
  }
  
  // 👇 PAGE 1: Childhood dream
  else if(page == 1){
    display.setCursor(0, 10); 
    display.println(F("Childhood"));
    display.setCursor(0, 20); 
    display.println(F("DREAM into"));
    display.setCursor(0, 30); 
    display.println(F("REALITY"));
    display.drawBitmap(44, 30, screwdriver_icon, 8, 8, BLACK); 
    display.setCursor(60, 40); 
    display.println(F(">>"));
  }
  
  // 👇 PAGE 2: help of ai + telugu mad thinker
  else if(page == 2){
    display.setCursor(0, 10); 
    display.println(F("with help of"));
    display.setCursor(0, 20); 
    display.println(F("ChatGPT, Qwen"));
    display.setCursor(0, 30); 
    display.println(F("& TMT"));
    display.setCursor(52, 40); 
    display.println(F("<<"));
  }
  
  display.display(); // 👈 SINGLE display() AFTER all drawing ✅
  
  // 👇 Button handling with your existing isButtonPressed() + debounce
  if(isButtonPressed(BTN_RIGHT)){
    page = (page + 1) % 3;  // Cycle: 0→1→2→0
    sfxMenuMove();  // Subtle page-turn blip
  }
    // 👇 LEFT Button: Previous Page (0 -> 2 -> 1 -> 0)
  if(isButtonPressed(BTN_LEFT)){
    page = (page + 2) % 3; // Adding (TotalPages - 1) is a clean way to go back in modulo
    sfxMenuMove();  // Slightly lower pitch for "back"
  }
  if(isButtonPressed(BTN_UP)){
    state = MENU; // Return to menu
    sfxBack(); // Back sound
  }
}
// boundary border
void drawBorder() 
 { display.drawRect(0, 0, SCREEN_W, SCREEN_H, BLACK); }

void drawGame() {
  display.clearDisplay(); 
  drawBorder();
  display.fillRect(food.x, food.y, CELL, CELL, BLACK);
  drawSnake(true);
  display.display();
}
// snake draw
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