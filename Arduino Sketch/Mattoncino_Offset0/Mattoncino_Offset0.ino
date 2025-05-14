// MATTONCINO V1.0, by Davide Gatti www.gattidavide.com - Survival hacking
// 
// Un piccolo gioco stile breakout, realizzato con l'uso di chatgpt
// Ho solo applicato alcune piccole modifiche, ma il generale è il 
// risultato di innumerevoli iterazioni con chatgpt
// nel video spiego un po' tutto il processo di realizzazione
// 
// Il gioco è molto semplice e utilizza un micro ESP32 con un display
// Oled miniaturizzato di 0.44 pollici.
//
//
// A small breakout-style game, realised with the use of chatgpt
// I have only applied a few minor modifications, but the general 
// result of countless iterations with chatgpt
// In the video I explain a bit about the whole process of realisation
// 
// The game is very simple and uses a micro ESP32 with a display
// 0.44 inch miniaturised Oled display.


#include <Wire.h>
#include <U8g2lib.h>

// Definizione dei pin e delle costanti / Definition of pins and constants
#define SOUND_PIN 4 // Pin per il suono / Pin for sound
#define BUTTON_LEFT 2 // Pulsante sinistro / Left button
#define BUTTON_RIGHT 3 // Pulsante destro / Right button
#define OFFSET_X 0 // Offset X per il disegno / X offset for drawing
#define OFFSET_Y 0 // Offset Y per il disegno / Y offset for drawing
#define BALL_SIZE 3 // Dimensione della palla / Ball size
#define BRICK_WIDTH 6 // Larghezza dei mattoncini / Brick width
#define BRICK_HEIGHT 3 // Altezza dei mattoncini / Brick height
#define NUM_BRICKS_X 10 // Numero di mattoncini in orizzontale / Number of bricks horizontally
#define NUM_BRICKS_Y 4 // Numero di mattoncini in verticale / Number of bricks vertically
#define SCREEN_WIDTH 72 // Larghezza dello schermo / Screen width
#define SCREEN_HEIGHT 40 // Altezza dello schermo / Screen height
#define PADDLE_WIDTH_INITIAL 15 // Larghezza iniziale della paletta / Initial paddle width
#define PADDLE_Y (SCREEN_HEIGHT - 4) // Posizione Y della paletta / Y position of the paddle
#define LIVES_COUNT 5 // Numero di vite iniziali / Initial number of lives


// Inizializzazione del display / Display initialization
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, U8X8_PIN_NONE, 6, 5);

// Variabili di stato del gioco / Game state variables
int paddleX = (SCREEN_WIDTH - PADDLE_WIDTH_INITIAL) / 2; // Posizione iniziale della paletta / Initial paddle position
int paddleWidth = PADDLE_WIDTH_INITIAL; // Larghezza attuale della paletta / Current paddle width
int ballX, ballY; // Posizione della palla / Ball position
int ballSpeedX = 1; // Velocità della palla sull'asse X / Ball speed on X-axis
int ballSpeedY = -1; // Velocità della palla sull'asse Y / Ball speed on Y-axis
int score = 0; // Punteggio / Score
int paddleHalf = 0; // Stato della paletta dimezzata / Paddle half state
int lives = LIVES_COUNT; // Vite rimanenti / Remaining lives
bool gameStarted = false; // Stato del gioco avviato / Game started state
bool waitingForRelease = true; // Attesa del rilascio dei pulsanti / Waiting for button release
bool firstStart = true; // Prima esecuzione del gioco / First game execution
bool paddleShrunk = false; // Stato della paletta rimpicciolita / Paddle shrunk state
int bricks[NUM_BRICKS_X][NUM_BRICKS_Y]; // Matrice dei mattoncini / Brick matrix



void setup() { // Impostazione iniziale del gioco / Initial game setup
    Serial.begin(115200); // Avvia la comunicazione seriale / Start serial communication
    u8g2.begin(); // Inizializza il display / Initialize the display
    u8g2.setContrast(255); // Imposta il contrasto massimo / Set maximum contrast
    u8g2.setBusClock(400000); // Imposta la velocità dell'I2C / Set I2C speed
    pinMode(SOUND_PIN, OUTPUT); // Imposta il pin del suono come output / Set sound pin as output
    pinMode(BUTTON_LEFT, INPUT_PULLUP); // Imposta il pulsante sinistro con pull-up interno / Set left button with internal pull-up
    pinMode(BUTTON_RIGHT, INPUT_PULLUP); // Imposta il pulsante destro con pull-up interno / Set right button with internal pull-up
    resetGame(); // Resetta il gioco / Reset the game
    if (firstStart) {
        showStartScreen(); // Mostra la schermata iniziale / Show start screen
        firstStart = false;
    }
}


void showStartScreen() { // Mostra la schermata iniziale / Show the start screen
    u8g2.clearBuffer(); // Pulisce il buffer / Clear the buffer
    u8g2.drawFrame(30, 13, 60, 15); // Disegna il riquadro del titolo / Draw title box
    u8g2.setFont(u8g2_font_boutique_bitmap_7x7_t_all); // Imposta il font / Set font
    u8g2.setCursor(OFFSET_X + 10, OFFSET_Y+10);
    u8g2.print("MATTONCINO"); // Stampa il nome del gioco / Print game name
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(OFFSET_X + 26, OFFSET_Y + 25);
    u8g2.print("by");
    u8g2.setCursor(OFFSET_X + 0, OFFSET_Y + 35);
    u8g2.print("Davide Gatti"); // Stampa l'autore / Print author
    u8g2.sendBuffer(); // Invia il buffer al display / Send buffer to display
    delay(3000); // Pausa di 3 secondi / 3-second delay
}


void playSound(int frequency, int duration) {  // Riproduce un suono con la frequenza e durata specificata / Plays a sound with the specified frequency and duration
    tone(SOUND_PIN, frequency, duration);  // Usa la funzione tone per produrre il suono / Uses the tone function to produce sound
}

void playVictoryTune() {  // Suonata di vittoria / Victory tune
    // Riproduce una sequenza di suoni per celebrare la vittoria / Plays a sequence of sounds to celebrate victory
    playSound(1200, 200);  // Riproduce il primo suono della vittoria / Plays the first victory sound
    delay(200);  // Pausa di 200ms / 200ms delay
    playSound(1400, 200);  // Suono successivo / Next sound
    delay(200);  // Pausa di 200ms / 200ms delay
    playSound(1600, 200);  // Suono successivo / Next sound
    delay(200);  // Pausa di 200ms / 200ms delay
    playSound(1800, 400);  // Suono finale della vittoria / Final victory sound
    delay(200);  // Pausa di 200ms / 200ms delay
}

void playGameOverTune() {  // Suonata di game over / Game over tune
    // Riproduce una sequenza di suoni per il game over / Plays a sequence of sounds for the game over
    playSound(500, 300);  // Primo suono / First sound
    delay(300);  // Pausa di 300ms / 300ms delay
    playSound(400, 300);  // Suono successivo / Next sound
    delay(300);  // Pausa di 300ms / 300ms delay
    playSound(300, 300);  // Suono successivo / Next sound
    delay(300);  // Pausa di 300ms / 300ms delay
    playSound(100, 600);  // Suono finale / Final sound
    delay(300);  // Pausa di 300ms / 300ms delay
}

void drawPaddle() {  // Disegna la paletta sullo schermo / Draws the paddle on the screen
    // Disegna la paletta nella posizione corrente / Draws the paddle at its current position
    u8g2.drawBox(paddleX + OFFSET_X, PADDLE_Y + OFFSET_Y, paddleWidth, 2);  // Disegna un rettangolo per la paletta / Draw a rectangle for the paddle
}

void drawBall() {  // Disegna la palla sullo schermo / Draws the ball on the screen
    // Disegna la palla nella sua posizione corrente / Draws the ball at its current position
    u8g2.drawBox(ballX + OFFSET_X, ballY + OFFSET_Y, BALL_SIZE, BALL_SIZE);  // Disegna un quadrato per la palla / Draw a square for the ball
}

void drawBricks() {  // Disegna i mattoncini sullo schermo / Draws the bricks on the screen
    // Disegna ogni mattoncino se è ancora presente / Draws each brick if it's still present
    for (int i = 0; i < NUM_BRICKS_X; i++) {  // Ciclo attraverso tutti i mattoncini orizzontali / Loop through all horizontal bricks
        for (int j = 0; j < NUM_BRICKS_Y; j++) {  // Ciclo attraverso tutti i mattoncini verticali / Loop through all vertical bricks
            if (bricks[i][j] == 1) {  // Se il mattoncino è presente / If the brick is present
                u8g2.drawBox(i * (BRICK_WIDTH + 1) + OFFSET_X, j * (BRICK_HEIGHT + 2) + OFFSET_Y, BRICK_WIDTH, BRICK_HEIGHT);  // Disegna il mattoncino / Draw the brick
            }
        }
    }
}

void resetBall() {  // Resetta la palla alla posizione iniziale / Resets the ball to its initial position
    // Posiziona la palla al centro della paletta / Positions the ball at the center of the paddle
    ballX = paddleX + paddleWidth / 2 - BALL_SIZE / 2;  // Posiziona la palla al centro della paletta / Position the ball at the center of the paddle
    ballY = PADDLE_Y - BALL_SIZE - 1;  // Posiziona la palla sopra la paletta / Position the ball just above the paddle
    waitingForRelease = true;  // Imposta l'attesa del rilascio del pulsante / Set the waiting for button release
}

void resetBricks() {  // Resetta i mattoncini alla loro posizione iniziale / Resets the bricks to their initial position
    // Ripristina ogni mattoncino / Resets each brick
    for (int i = 0; i < NUM_BRICKS_X; i++) {  // Ciclo attraverso tutti i mattoncini orizzontali / Loop through all horizontal bricks
        for (int j = 0; j < NUM_BRICKS_Y; j++) {  // Ciclo attraverso tutti i mattoncini verticali / Loop through all vertical bricks
            bricks[i][j] = 1;  // Ripristina ogni mattoncino a 1 (presente) / Resets each brick to 1 (present)
        }
    }
    resetPaddle();  // Resetta la paletta / Resets the paddle
}

void resetPaddle() {  // Resetta la paletta alla sua larghezza iniziale / Resets the paddle to its initial width
    // Ripristina la larghezza della paletta e le velocità della palla / Restores paddle width and ball speeds
    paddleWidth = PADDLE_WIDTH_INITIAL;  // Ripristina la larghezza della paletta / Restores paddle width
    ballSpeedX = 1;  // Imposta la velocità della palla sull'asse X / Sets the ball speed on the X-axis
    ballSpeedY = -1;  // Imposta la velocità della palla sull'asse Y / Sets the ball speed on the Y-axis
    paddleHalf = 0;  // Resetta lo stato della paletta dimezzata / Resets the paddle half state
}

void resetGame() {  // Resetta il gioco / Resets the game
    // Ripristina tutti i parametri di gioco / Resets all game parameters
    score = 0;  // Imposta il punteggio a 0 / Sets the score to 0
    lives = LIVES_COUNT;  // Imposta le vite al numero iniziale / Sets lives to the initial count
    paddleShrunk = false;  // Ripristina lo stato della paletta rimpicciolita / Resets the paddle shrunk state
    resetPaddle();  // Resetta la paletta / Resets the paddle
    resetBricks();  // Resetta i mattoncini / Resets the bricks
    resetBall();  // Resetta la palla / Resets the ball
}

void showLives() {  // Mostra le vite e il punteggio sul display / Displays the lives and score on the screen
    // Mostra il numero di vite e punteggio attuale / Displays the current lives and score
    u8g2.clearBuffer();  // Pulisce il buffer / Clears the buffer
    u8g2.setCursor(OFFSET_X + 10, OFFSET_Y + 30);  // Imposta la posizione del cursore / Sets the cursor position
    u8g2.print("Lives: ");  // Stampa "Vite:" / Prints "Lives:"
    u8g2.print(lives);  // Stampa il numero di vite rimanenti / Prints the number of remaining lives
    u8g2.setCursor(OFFSET_X + 10, OFFSET_Y + 10);  // Imposta un'altra posizione del cursore / Sets another cursor position
    u8g2.print("Score: ");  // Stampa "Punteggio:" / Prints "Score:"
    u8g2.print(score);  // Stampa il punteggio attuale / Prints the current score
    u8g2.sendBuffer();  // Invia il buffer al display / Sends the buffer to the display
}

void gameOverScreen() {  // Mostra la schermata di game over / Displays the game over screen
    // Gestisce la schermata finale di game over / Manages the final game over screen
    playGameOverTune();  // Riproduce la suonata di game over / Plays the game over tune
    u8g2.clearBuffer();  // Pulisce il buffer / Clears the buffer
    u8g2.setCursor(OFFSET_X + 5, OFFSET_Y + 30);  // Imposta la posizione del cursore / Sets the cursor position
    u8g2.print("Game Over!");  // Stampa "Game Over!" / Prints "Game Over!"
    u8g2.setCursor(OFFSET_X + 10, OFFSET_Y + 10);  // Imposta un'altra posizione del cursore / Sets another cursor position
    u8g2.print("Score: ");  // Stampa "Punteggio:" / Prints "Score:"
    u8g2.print(score);  // Stampa il punteggio finale / Prints the final score
    u8g2.sendBuffer();  // Invia il buffer al display / Sends the buffer to the display
    delay(2000);  // Pausa di 2 secondi / 2-second delay
    resetGame();  // Resetta il gioco / Resets the game
}

void handleBrickCollisions() {  // Gestisce le collisioni con i mattoncini / Handles brick collisions
    // Verifica se la palla colpisce un mattoncino e aggiorna il punteggio / Checks if the ball hits a brick and updates the score
    bool bricksRemaining = false;  // Variabile per controllare se rimangono mattoncini / Variable to check if there are remaining bricks
    for (int i = 0; i < NUM_BRICKS_X; i++) {  // Ciclo attraverso tutti i mattoncini orizzontali / Loop through all horizontal bricks
        for (int j = 0; j < NUM_BRICKS_Y; j++) {  // Ciclo attraverso tutti i mattoncini verticali / Loop through all vertical bricks
            if (bricks[i][j] == 1) {  // Se il mattoncino è presente / If the brick is present
                bricksRemaining = true;  // Imposta che ci sono mattoncini rimanenti / Sets that there are remaining bricks
                if (ballX + BALL_SIZE > i * (BRICK_WIDTH + 1) && ballX < i * (BRICK_WIDTH + 1) + BRICK_WIDTH &&  // Verifica la collisione con il mattoncino / Checks for collision with the brick
                    ballY + BALL_SIZE > j * (BRICK_HEIGHT + 2) && ballY < j * (BRICK_HEIGHT + 2) + BRICK_HEIGHT) {
                    ballSpeedY = -ballSpeedY;  // Rimbalza la palla / Bounces the ball
                    bricks[i][j] = 0;  // Rimuove il mattoncino / Removes the brick
                    score += 10;  // Aumenta il punteggio / Increases the score
                    playSound(1000, 100);  // Suono della collisione / Collision sound
                }
            }
        }
    }
    if (!bricksRemaining) {  // Se non ci sono più mattoncini / If no bricks remain
        playVictoryTune();  // Riproduce la suonata di vittoria / Plays the victory tune
        resetBricks();  // Resetta i mattoncini / Resets the bricks
        resetBall();  // Resetta la palla / Resets the ball
    }
}

void loop() {  // Funzione principale del gioco / Main game loop
    // Gestisce la logica principale del gioco: input, movimento della palla, collisioni / Handles the main game logic: input, ball movement, collisions
    u8g2.clearBuffer();  // Pulisce il buffer / Clears the buffer
    drawPaddle();  // Disegna la paletta / Draws the paddle
    drawBall();  // Disegna la palla / Draws the ball
    drawBricks();  // Disegna i mattoncini / Draws the bricks

    if (digitalRead(BUTTON_LEFT) == LOW && paddleX > 0) paddleX -= 2;  // Se il pulsante sinistro è premuto e la paletta non è al bordo sinistro / If the left button is pressed and the paddle is not at the left edge
    if (digitalRead(BUTTON_RIGHT) == LOW && paddleX + paddleWidth < SCREEN_WIDTH) paddleX += 2;  // Se il pulsante destro è premuto e la paletta non è al bordo destro / If the right button is pressed and the paddle is not at the right edge

    if (waitingForRelease) {  // Se stiamo aspettando il rilascio del pulsante / If we are waiting for the button release
        if (digitalRead(BUTTON_LEFT) == LOW || digitalRead(BUTTON_RIGHT) == LOW) {  // Se uno dei pulsanti è premuto / If one of the buttons is pressed
            delay(200);  // Pausa di 200ms / 200ms delay
            waitingForRelease = false;  // Imposta la fine dell'attesa / Ends the waiting state
        }
    } else {  // Se non stiamo aspettando il rilascio / If we are not waiting for the release
        ballX += ballSpeedX;  // Aggiorna la posizione della palla sull'asse X / Updates the ball position on the X-axis
        ballY += ballSpeedY;  // Aggiorna la posizione della palla sull'asse Y / Updates the ball position on the Y-axis

        if (ballX <= 0 || ballX + BALL_SIZE >= SCREEN_WIDTH) {  // Se la palla ha raggiunto il bordo orizzontale / If the ball reaches the horizontal border
            ballSpeedX = -ballSpeedX;  // Inverte la velocità della palla / Reverses the ball speed on the X-axis
            playSound(2000, 100);  // Suono del rimbalzo orizzontale / Horizontal bounce sound
        }
        if (ballY <= 0) {  // Se la palla ha raggiunto il bordo superiore / If the ball reaches the top border
            ballSpeedY = -ballSpeedY;  // Inverte la velocità della palla / Reverses the ball speed on the Y-axis
            playSound(2000, 100);  // Suono del rimbalzo verticale / Vertical bounce sound
            if (paddleHalf == 0) {  // Se la paletta non è ancora dimezzata / If the paddle is not yet halved
                paddleWidth = paddleWidth / 2;  // Dimezza la paletta / Halves the paddle width
                paddleHalf = 1;  // Imposta la paletta come dimezzata / Sets the paddle as halved
                ballSpeedX = 1.3;  // Aumenta la velocità della palla sull'asse X / Increases the ball speed on the X-axis
                ballSpeedY = -1.3;  // Aumenta la velocità della palla sull'asse Y / Increases the ball speed on the Y-axis
            }
        }

        if (ballY + BALL_SIZE >= PADDLE_Y && ballX + BALL_SIZE >= paddleX && ballX <= paddleX + paddleWidth) {  // Se la palla colpisce la paletta / If the ball hits the paddle
            ballSpeedY = -ballSpeedY;  // Inverte la velocità della palla / Reverses the ball speed on the Y-axis
            playSound(500, 100);  // Suono della collisione con la paletta / Paddle collision sound
        }

        handleBrickCollisions();  // Gestisce le collisioni con i mattoncini / Handles the brick collisions

        if (ballY >= SCREEN_HEIGHT) {  // Se la palla è uscita dallo schermo / If the ball goes out of the screen
            lives--;  // Decrementa il numero di vite / Decreases the number of lives
            if (lives > 0) {  // Se ci sono ancora vite rimaste / If there are still lives remaining
                showLives();  // Mostra le vite / Displays the lives
                resetBall();  // Resetta la palla / Resets the ball
                playSound(400, 800);  // Suono di perdita della palla / Ball loss sound
                delay(1200);  // Pausa di 1.2 secondi / 1.2-second delay
            } else {  // Se non ci sono più vite / If there are no more lives
                gameOverScreen();  // Mostra la schermata di game over / Displays the game over screen
            }
        }
    }

    u8g2.sendBuffer();  // Invia il buffer al display / Sends the buffer to the display
    delay(20);  // Pausa di 20ms / 20ms delay
}