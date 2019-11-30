// do ctrl + maj + m to open the shell 
// Library for screen : https://github.com/carlosefr/pcd8544

//Port relier au sensor
int sensor = A1;

//Etat du pin à T
int on = LOW;
//Etat du pin à T - 1
int prevon = LOW;

//Counter pouvoir savoir à quel bit on est
int bitCount = 0;
//Valeur pour marquer les début d'étude d'intervalle
unsigned long startTime = 0;
//Valeur pour savoir combien de temps il y a eu entre 
unsigned long elapsedTime = 0;

//Booleen pour savoir si il a deja vu un raise up
int hasSeenFirstFallingEdge = 0;
//Booleen pour savoir si il a deja vu un raise down
int hasSeenFirstRisingEdge = 0;

#include <PCD8544.h>
// A custom glyph (a smiley)...
static const byte glyph[] = { B00010000, B00110100, B00110000, B00110100, B00010000 };


static PCD8544 lcd;

//Tableau de stockage des intervals de temps
int times[8][8] = {0};

//Temps calculé selon le protocole pour savoir combien de nanosecondes un 1 sera codé
const unsigned long maxTimeForOne = 1300;

//Différent state de la lecture du message :
// STOP : en attente du bit de STOP
// START : La start condition a été lu
// READING : Lecture des bits en cours
const int STOP = 0, START = 1, READING = 2, BEGIN = 3;

//Initialisation
int stateMachine = BEGIN;

//Compter pour savoir quel lettre on est en train de lire 
int lettercounter = 0;
//Compter pour savoir quel bit de la lettre on est en train de lire
int bitcounter = 0;

void setup() {
  Serial.begin(9600);

  lcd.begin(84, 48);
  // Add the smiley to position "0" of the ASCII table...
  lcd.createChar(0, glyph);
  
  Serial.println("--- RUNNING ---");
  delay(4000);
}

void loop() {

    switch (stateMachine) {
      case BEGIN:
        ///Ces lignes font les rafraichissement de l'état du digital read
        prevon = on;
        on = digitalRead(sensor);
        ///

        //On on passe de l etat haut à l etat bas
        if (prevon == HIGH && on == LOW) {
          //Récuperation du temps du depart
          startTime = micros();
          elapsedTime = 0;
          //On a vu un falling edge
          hasSeenFirstFallingEdge = 1;
        }
        //On prend le temps de fin pour connaitre l interval de temps
        if (hasSeenFirstFallingEdge) elapsedTime = micros() - startTime;

        //Si jamais le temps correspond au bit de stop
        if (elapsedTime > 20000) {
          hasSeenFirstFallingEdge = 0;
          hasSeenFirstRisingEdge = 0;
          stateMachine = STOP;
        }
        break;
      case STOP:
        prevon = on;
        on = digitalRead(sensor);
        if (prevon == HIGH && on == LOW) {
          times[8][8] = {0};
          lettercounter = 0;
          bitcounter = 0;
        }
        break;
        
      case READING:
        prevon = on;
        on = digitalRead(sensor);
        
        if(prevon == LOW && on == HIGH){
          startTime = micros();
        }
        if(prevon == HIGH && on == LOW){
          elapsedTime = micros() - startTime;

          times[lettercounter][bitcounter] = elapsedTime;
          
          bitcounter++;

          // Gerer la sortie de lecture
          if (bitcounter == 8 && lettercounter == 8) {
            lcd.setCursor(0, 0);
            for (int c = 0; c < 8; c++) {
              byte mask = B10000000;
              byte value = 0;
              for (int b = 0; b < 8; b++) {
                if (times[c][b] < maxTimeForOne) {
                  value = value | mask;
                }
                mask = mask >> 1;
              }
              
              lcd.print(char(value));
              Serial.write(value);
            }
            
            
            Serial.println("\nSTOP READING");
            delay(1000);
            stateMachine = BEGIN;
          }

          // Incrémenter le compteur de caractère si on dépasse le compteur de bit
          if (bitcounter >= 8) {
            lettercounter++;
            bitcounter = 0;
          }
        }
        break;           
    }
}
