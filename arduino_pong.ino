/*
  Sayan Electronics Pong Game 
  Connections:
  :- OLED VCC → 5V
  :- OLED GND → GND
  :- OLED SDA → A4 (Nano)
  :- OLED SCL → A5 (Nano)
*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display dimensions and reset pin
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Input pins for the buttons and buzzer
#define P1_UP 6
#define P1_DOWN 7
#define P2_UP 5
#define P2_DOWN 4
#define BUZZER_PIN 12

// Game settings (editable)
int paddleHeight = 16; // Height of the paddles
int paddleWidth = 3;   // Width of the paddles
int paddleSpeed = 2;   // Speed at which paddles move
int ballSize = 3;      // Size of the ball
float ballSpeedX = 2;  // Horizontal speed of the ball
float ballSpeedY = 1.5; // Vertical speed of the ball
int cpuDifficulty = 1; // 1 = hardest, higher is easier (CPU reacts slower)

// Game state enum to represent different modes
enum Mode { MENU, PVP, PVC, HARDCORE };
Mode gameMode = MENU; // Default game mode is MENU

  // Paddle and ball positions
int p1Y = SCREEN_HEIGHT / 2 - paddleHeight / 2;
int p2Y = SCREEN_HEIGHT / 2 - paddleHeight / 2;
float ballX = SCREEN_WIDTH / 2;
float ballY = SCREEN_HEIGHT / 2;
float vx = ballSpeedX;
float vy = ballSpeedY;

void setup() {
  // Set pin modes for buttons and buzzer
  pinMode(P1_UP, INPUT_PULLUP);
  pinMode(P1_DOWN, INPUT_PULLUP);
  pinMode(P2_UP, INPUT_PULLUP);
  pinMode(P2_DOWN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
}

void loop() {
  // Check if game is in the menu or in play mode
  if (gameMode == MENU) {
    showMenu(); // Show the game mode menu
  } else {
    playGame(); // Start the game
  }
}

// Function to display the game mode menu
void showMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 5);
  display.println("Select Game Mode:");
  display.setCursor(0, 20);
  display.println("1v1: P1 UP + P2 UP");
  display.setCursor(0, 30);
  display.println("CPU: P1 UP + P2 DOWN");
  display.setCursor(0, 40);
  display.println("HARDCORE: P1 DN+P2 DN");
  display.display();

  // Switch game modes based on button presses
  if (digitalRead(P1_UP) == LOW && digitalRead(P2_UP) == LOW) {
    delay(300); // Debounce delay
    gameMode = PVP; // Player vs Player mode
  } else if (digitalRead(P1_UP) == LOW && digitalRead(P2_DOWN) == LOW) {
    delay(300); // Debounce delay
    gameMode = PVC; // Player vs CPU mode
  } else if (digitalRead(P1_DOWN) == LOW && digitalRead(P2_DOWN) == LOW) {
    delay(300); // Debounce delay
    ballSpeedX = 3.5; // Increase ball speed in Hardcore mode
    ballSpeedY = 2.5;
    gameMode = HARDCORE; // Hardcore mode
  }
}

// Function to play the game
void playGame() {
  // Player 1 Paddle movement
  if (digitalRead(P1_UP) == LOW && p1Y > 0) p1Y -= paddleSpeed;
  if (digitalRead(P1_DOWN) == LOW && p1Y + paddleHeight < SCREEN_HEIGHT) p1Y += paddleSpeed;

  // Player 2 (CPU or Player 2) Paddle movement
  if (gameMode == PVP) {
    if (digitalRead(P2_UP) == LOW && p2Y > 0) p2Y -= paddleSpeed;
    if (digitalRead(P2_DOWN) == LOW && p2Y + paddleHeight < SCREEN_HEIGHT) p2Y += paddleSpeed;
  } else if (gameMode == PVC || gameMode == HARDCORE) {
    moveCPU(); // Move the CPU paddle
  }

  // Ball movement
  ballX += vx;
  ballY += vy;

  // Ball bounce off top and bottom walls
  if (ballY <= 0 || ballY + ballSize >= SCREEN_HEIGHT) vy *= -1;

  // Ball bounce off paddles
  if (ballX <= paddleWidth && ballY + ballSize >= p1Y && ballY <= p1Y + paddleHeight) {
    vx *= -1; // Reverse ball direction
    ballX = paddleWidth + 1; // Avoid ball getting stuck in the paddle
    playPaddleHitTone(); // Play sound on paddle hit
  }
  if (ballX + ballSize >= SCREEN_WIDTH - paddleWidth && ballY + ballSize >= p2Y && ballY <= p2Y + paddleHeight) {
    vx *= -1; // Reverse ball direction
    ballX = SCREEN_WIDTH - paddleWidth - ballSize - 1;
    playPaddleHitTone(); // Play sound on paddle hit
  }

  // Reset game if ball goes out of bounds
  if (ballX < 0 || ballX > SCREEN_WIDTH) {
    gameOver(); // Show game over screen
    return;
  }

  // Draw the game (paddles, ball)
  display.clearDisplay();
  display.fillRect(0, p1Y, paddleWidth, paddleHeight, SSD1306_WHITE); // Player 1 paddle
  display.fillRect(SCREEN_WIDTH - paddleWidth, p2Y, paddleWidth, paddleHeight, SSD1306_WHITE); // Player 2 paddle
  display.fillRect((int)ballX, (int)ballY, ballSize, ballSize, SSD1306_WHITE); // Ball
  display.display(); // Update the display

  delay(10); // Small delay for smoother gameplay
}

// Function to reset the game
void resetGame() {
  p1Y = SCREEN_HEIGHT / 2 - paddleHeight / 2; // Reset Player 1 paddle position
  p2Y = SCREEN_HEIGHT / 2 - paddleHeight / 2; // Reset Player 2 paddle position
  ballX = SCREEN_WIDTH / 2; // Reset ball position
  ballY = SCREEN_HEIGHT / 2;
  vx = (gameMode == HARDCORE) ? 3.5 : ballSpeedX; // Increase ball speed in Hardcore mode
  vy = (gameMode == HARDCORE) ? 2.5 : ballSpeedY;
  delay(1000); // Wait before starting a new game
}


// Function to control CPU paddle movement
void moveCPU() {
  if (ballY < p2Y + paddleHeight / 2 && p2Y > 0) {
    p2Y--; // Move CPU paddle up
  } else if (ballY > p2Y + paddleHeight / 2 && p2Y + paddleHeight < SCREEN_HEIGHT) {
    p2Y++; // Move CPU paddle down
  }

  // Hardcore mode: CPU moves faster
  if (gameMode == HARDCORE) {
    if (random(2) == 0) {
      p2Y += (ballY > p2Y + paddleHeight / 2) ? 1 : -1; // Move CPU paddle more aggressively
    }
  }
}

void gameOver() {
  display.clearDisplay();
  display.setTextSize(2);  // Larger text size
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Game Over");  // Display "Game Over"
  display.setTextSize(1);
  display.setCursor(27, 40);
  display.println("Press P1 UP");
  display.setCursor(33, 50);
  display.println("to restart");
  display.display();  // Update the display

  //play game over tone
  tone(BUZZER_PIN, 600, 300);  // Medium pitch
  delay(350);
  tone(BUZZER_PIN, 400, 300);  // Lower pitch
  delay(350);
  tone(BUZZER_PIN, 200, 500);  // Very low pitch for ending
  delay(550);
  noTone(BUZZER_PIN); 

  while (digitalRead(P1_UP) != LOW) {
  }
  
  delay(300);  
  gameMode = MENU; 
  resetGame(); 
}

void playPaddleHitTone() {
  tone(BUZZER_PIN, 1000, 80);   // High pitch tone
  delay(100);
  tone(BUZZER_PIN, 1200, 80);   // Slightly higher pitch tone
  delay(100);
  noTone(BUZZER_PIN);           // Stop the tone
}


