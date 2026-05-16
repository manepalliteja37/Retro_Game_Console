// ================= LIBRARIES =================
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Nokia pins (SCLK, DIN, DC, CS, RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(11, 10, 9, 8, 7);

// ================= BUTTONS =================
#define BTN_LEFT 5
#define BTN_RIGHT 4
#define BTN_UP 2
#define BTN_DOWN 3

#define BUZZER 6

// ================= STATES =================
enum State {MENU, GAME, PAUSE, GAMEOVER, SOUND_MENU, HELP, DIFFICULTY, SLEEP};
State state = MENU;

// ================= MENU =================
int menuIndex = 0;
const char* menuItems[] = {"Start", "Difficulty", "Sound", "Help", "Exit"};

bool soundOn = true;
int difficulty = 1;

// ================= SNAKE =================
#define MAX_SNAKE 50
int snakeX[MAX_SNAKE];
int snakeY[MAX_SNAKE];
int snakeLength;

int foodX, foodY;
int dirX, dirY;

// ================= SCORE =================
int score = 0;
int lastScore = 0;

// ================= SPEED =================
int speedDelay = 100;

// ================= SLEEP =================
unsigned long lastActivityTime = 0;
const unsigned long sleepTimeout = 20000;
// ================= BUTTON DEBOUNCE =================
unsigned long lastBtnTime = 0;
const unsigned long btnInterval = 120;  // ms between accepted presses
// Add after btnInterval declaration (~line 57)
unsigned long btnPressStart = 0;
const unsigned long longPressThreshold = 500; // ms for pause
bool upBtnHeld = false;
// ================= SOUND =================
void playTone(int f,int d){
  if(soundOn){
    noTone(BUZZER);
    tone(BUZZER,f,d);
  }
}
void soundMove(){ playTone(700,20); }
void soundSelect(){ playTone(1000,40); }
void soundBack(){ playTone(500,30); }
void soundEat(){ playTone(1200,40); }
void soundDead(){ playTone(300,200); }

// ================= BUTTON HELPER =================
bool btnPressed(int pin){
  if(digitalRead(pin) == LOW){
    if(millis() - lastBtnTime > btnInterval){
      lastBtnTime = millis();
      return true;
    }
  }
  return false;
}
// ================= SPEED =================
void updateSpeed(){
  int base;
  if(difficulty==0) base=200;
  else if(difficulty==1) base=120;
  else if(difficulty==2) base=70;
  else base=40;

  int level = score/5;
  speedDelay = base - (level*5);

  if(speedDelay < 20) speedDelay = 20;
}

// ================= RESET =================
void resetGame(){
  snakeLength = 5;

  for(int i=0;i<snakeLength;i++){
    snakeX[i] = 40 - i*2;
    snakeY[i] = 24;
  }

  dirX = 1;
  dirY = 0;

  spawnFood();  // ← New function (defined below)

  score = 0;
}

// ================= MENU =================
void drawMenu(){
  display.clearDisplay();
  display.setTextSize(1);

  // Title (centered)
  display.setCursor(24, 0);
  display.print("SNAKE");

  // Menu items: 5 rows, 7px spacing, starts at Y=9
  // Y positions: 9, 16, 23, 30, 37 → all fit safely in 48px height
  const int startY = 9;
  const int spacing = 7;

  for(int i=0; i<5; i++){
    int yPos = startY + (i * spacing);

    // Draw selector ">"
    display.setCursor(0, yPos);
    if(i == menuIndex) display.print(">");
    else display.print(" ");

    // Draw menu text aligned to X=10
    display.setCursor(10, yPos);
    display.print(menuItems[i]);
  }

  display.display();
}
// ================= SOUND MENU =================
void drawSoundMenu(){
  display.clearDisplay();

  display.setCursor(0,0);
  display.println("SOUND");

  display.setCursor(0,20);
  display.print("Status: ");
  display.println(soundOn ? "ON" : "OFF");

  display.setCursor(0,40);
  display.println("UP: Back");

  display.display();

  if(digitalRead(BTN_LEFT)==LOW || digitalRead(BTN_RIGHT)==LOW){
    soundOn = !soundOn;
    soundSelect();
    delay(200);
  }

  if(digitalRead(BTN_UP)==LOW){
    soundBack();
    state = MENU;
    delay(200);
  }
}
// ================= SPAWN FOOD =================
void spawnFood(){
  bool valid;
  do{
    valid = true;
    // Align to 2px grid, strictly inside border (X:2-82, Y:10-46)
    foodX = random(0, 41) * 2 + 2;        // 2, 4, 6... 82
    foodY = random(0, 19) * 2 + 10;       // 10, 12, 14... 46
    
    // Check collision with snake body
    for(int i=0; i<snakeLength; i++){
      if(snakeX[i] == foodX && snakeY[i] == foodY){
        valid = false;
        break;
      }
    }
  } while(!valid);
}
// ================= DIFFICULTY =================
void drawDifficulty(){
  display.clearDisplay();

  display.setCursor(0,0);
  display.println("DIFFICULTY");

  display.setCursor(0,20);

  if(difficulty==0) display.println("Easy");
  else if(difficulty==1) display.println("Medium");
  else if(difficulty==2) display.println("Hard");
  else display.println("Extreme");

  display.setCursor(0,40);
  display.println("UP: Back");

  display.display();

  if(digitalRead(BTN_LEFT)==LOW){
    difficulty--;
    if(difficulty<0) difficulty=3;
    soundMove();
    delay(200);
  }

  if(digitalRead(BTN_RIGHT)==LOW){
    difficulty++;
    if(difficulty>3) difficulty=0;
    soundMove();
    delay(200);
  }

  if(digitalRead(BTN_UP)==LOW){
    soundBack();
    state=MENU;
    delay(200);
  }
}

// ================= HELP =================
void drawHelp(){
  display.clearDisplay();

  display.setCursor(0,0);
  display.println("HELP");

  display.setCursor(0,15);
  display.println("L/R: Move");

  display.setCursor(0,25);
  display.println("UP: Pause");

  display.setCursor(0,35);
  display.println("DOWN: Nav");

  display.display();

  if(digitalRead(BTN_UP)==LOW){
    state=MENU;
    delay(200);
  }
}

// ================= PAUSE =================
void drawPause(){
  display.clearDisplay();
  display.setCursor(20,20);
  display.println("PAUSE");

  display.display();

  if(digitalRead(BTN_UP)==LOW){
    state=GAME;
    delay(300);
  }
}

// ================= SLEEP =================
void drawSleep(){
  display.clearDisplay();
  display.setCursor(20,20);
  display.println("Zzz...");
  display.display();

  if(digitalRead(BTN_LEFT)==LOW ||
     digitalRead(BTN_RIGHT)==LOW ||
     digitalRead(BTN_UP)==LOW ||
     digitalRead(BTN_DOWN)==LOW){
     lastActivityTime = millis(); 
    state = MENU;

  }
}

// ================= GAME =================
void gameLoop(){

// === Handle UP button: Short press = Move Up, Long press = Pause ===
if(digitalRead(BTN_UP) == LOW){
    if(btnPressStart == 0){
        btnPressStart = millis(); // Start timing
    }
    else if(millis() - btnPressStart >= longPressThreshold && !upBtnHeld){
        // Long press detected → PAUSE
        upBtnHeld = true;
        soundSelect();
        state = PAUSE;
        return;
    }
} else {
    // Button released
    if(btnPressStart != 0 && millis() - btnPressStart < longPressThreshold){
        // Short press → Move Up (if not reversing)
        if(dirY != 1){ 
            dirX = 0; 
            dirY = -1; 
            soundMove(); 
        }
    }
    btnPressStart = 0;
    upBtnHeld = false;
}

  display.clearDisplay();
// Draw border (leaves top 2 rows free for score, 2px padding on edges)
display.drawRect(0, 8, 84, 40, BLACK);
// INPUT - with debouncing + all 4 directions
if(btnPressed(BTN_LEFT) && dirX != 1){ dirX = -1; dirY = 0; soundMove(); }
if(btnPressed(BTN_RIGHT) && dirX != -1){ dirX = 1; dirY = 0; soundMove(); }
if(btnPressed(BTN_DOWN) && dirY != -1){ dirX = 0; dirY = 1; soundMove(); }
//if(btnPressed(BTN_UP) && dirY != 1){ dirX = 0; dirY = -1; soundMove(); }  // ← ADDED

  // MOVE
  for(int i=snakeLength-1;i>0;i--){
    snakeX[i]=snakeX[i-1];
    snakeY[i]=snakeY[i-1];
  }

  snakeX[0]+=dirX*2;
  snakeY[0]+=dirY*2;

  // WALL
  // WALL (matches inner edge of the border)
// WALL (triggers when snake touches the border line)
if(snakeX[0] < 2 || snakeX[0] > 81 || snakeY[0] < 10 || snakeY[0] > 45){
    soundDead();
    lastScore=score;
    state=GAMEOVER;
    return;
  }

  // SELF
  for(int i=1;i<snakeLength;i++){
    if(snakeX[0]==snakeX[i] && snakeY[0]==snakeY[i]){
      soundDead();
      lastScore=score;
      state=GAMEOVER;
      return;
    }
  }

  // FOOD
  // Precise 2x2 pixel collision check
if(snakeX[0] <= foodX+1 && snakeX[0]+1 >= foodX && 
   snakeY[0] <= foodY+1 && snakeY[0]+1 >= foodY){
    if(snakeLength<MAX_SNAKE) snakeLength++;
    score++;
    soundEat();
    spawnFood();
  }

  // DRAW
  display.setCursor(0,0);
  display.print(score);

  display.fillRect(foodX,foodY,2,2,BLACK);

  for(int i=0;i<snakeLength;i++){
    display.fillRect(snakeX[i],snakeY[i],2,2,BLACK);
  }

  updateSpeed();

  display.display();
  delay(speedDelay);
}

// ================= GAME OVER =================
void drawGameOver(){
  display.clearDisplay();

  display.setCursor(10,10);
  display.println("GAME OVER");

  display.setCursor(10,25);
  display.print("Score:");
  display.println(lastScore);

  display.display();

  if(digitalRead(BTN_UP)==LOW){
    state=MENU;
    delay(300);
  }
}

// ================= SETUP =================
void setup(){

  display.begin();
  display.setContrast(57);

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);

  randomSeed(analogRead(0));

  lastActivityTime = millis();
}

// ================= LOOP =================
void loop(){

  if(digitalRead(BTN_LEFT)==LOW ||
     digitalRead(BTN_RIGHT)==LOW ||
     digitalRead(BTN_UP)==LOW ||
     digitalRead(BTN_DOWN)==LOW){
    lastActivityTime = millis();
  }

  if(millis()-lastActivityTime > sleepTimeout){
    state = SLEEP;
  }

  if(state==MENU){
    drawMenu();

    // DOWN button moves cursor DOWN (index increases)
    if(btnPressed(BTN_DOWN)){
      menuIndex = (menuIndex + 1) % 5;
      soundMove();
    }
    // UP button moves cursor UP (index decreases)
    if(btnPressed(BTN_UP)){
      menuIndex = (menuIndex - 1 + 5) % 5;
      soundMove();
    }

    // RIGHT button selects
    if(btnPressed(BTN_RIGHT)){
      soundSelect();
      if(menuIndex==0){ resetGame(); state=GAME; }
      else if(menuIndex==1){ state=DIFFICULTY; }
      else if(menuIndex==2){ state=SOUND_MENU; }
      else if(menuIndex==3){ state=HELP; }
      else if(menuIndex==4){ state=SLEEP; }
    }
  }

  else if(state==GAME) gameLoop();
  else if(state==PAUSE) drawPause();
  else if(state==GAMEOVER) drawGameOver();
  else if(state==SOUND_MENU) drawSoundMenu();
  else if(state==HELP) drawHelp();
  else if(state==DIFFICULTY) drawDifficulty();
  else if(state==SLEEP) drawSleep();
}