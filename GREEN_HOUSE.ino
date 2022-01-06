/* This program uses the millis function which is used as a simple delay. 
 * The defect of the delay is that it pauses the program completely, 
 * so if you press a button while you are in delay the input will not be updated. 
 * WITH THE MILLIS FUNCTION IT DOESN'T HAPPEN, YOU CAN DO MORE OPERATIONS TOGETHER.
 */
 /* I use watchdog to reset everything */
#include <avr/io.h>
#include <avr/wdt.h>
#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}
/* variables for clock */
unsigned long ore = 0;
unsigned long minuti = 0;
unsigned long secondi = 0;
unsigned long orologio = 1000;
int refresh_schermo = 0;
/* variable for millis function */
unsigned long millisecondi;
/* variables for lamps */
unsigned long tempo1 = 0; /* tempo1 and orologio they have to start together otherwise the whole thing is out of sync */
int reset_auto = 0;
int volte_luce_buio = 0;
/* variabes for display 16x2 */
unsigned long tempo2 = 2000;
unsigned long tempo3 = 4000;
unsigned long tempo4 = 6000;
int lampada = LOW;
/* variables for capacitive soil moisure sensor
 * use two variables for soil umidity because the first
 * is refresh every eight second for command pump
 * the second is refresh every cycle for print the output
 * in to lcd 16x2
 */
int umid_terreno = 0;
int umid_terreno_lcd = 0;
unsigned long tempo5 = 8000;
int pompa = LOW;

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,16,2);

#include "DHT.h"
#define DHT_PIN 2
#define DHT_TIPO DHT22
DHT dht(DHT_PIN, DHT_TIPO);

void setup() {
  Serial.begin(19200);
  pinMode(13, OUTPUT);
  pinMode(3, OUTPUT);
  
  lcd.init();
  lcd.clear();
  lcd.backlight();
}


void loop() {
  
  umid_terreno_lcd = analogRead(A0);
  /* maps umid_terreno_lcd to get the value from 0 to 100 percent */
  umid_terreno_lcd = map(umid_terreno_lcd, 290, 380, 100, 0);
  /* read dht22 humidity and temperature and save in float variables */
  float umidita = dht.readHumidity();
  float temperatura = dht.readTemperature();

  /* CHECK IF DHT READINGS ARE SUCCESSFUL
   * if reading fails, try again
   */
  if (isnan(umidita) || isnan(temperatura)) {
    Serial.println("Errore nell lettura DHT!");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERRORE DHT");
    delay(1000);
    return;
  }

  millisecondi = millis();



/* command output every 12 hours = 43.200.000 milliseconds. 
 * turning it on and then off repeatedly
 */
  if (millisecondi >= tempo1) {
    tempo1 = tempo1 + 43200000;
    if (lampada == LOW) {
      digitalWrite(13,lampada = HIGH);
    }
    else {
      digitalWrite(13,lampada = LOW);
    }
  }



/* enters the if when curent is greater than equal to time2
 * in this case 2000 milliseconds. poi impostiamo subito il
 * tempo di refresh, then we immediately set the refresh time, 
 * so how long it should wait to enter the if.
 */
 /* HUMIDITY SOIL -----------------------------------------------------------*/
  if (millisecondi >= tempo2) {
    tempo2 = (tempo2 + (2000*4));

   lcd.clear();
   lcd.setCursor(0,0);
   lcd.print("umidita del");
   lcd.setCursor(0,1);
   lcd.print("terreno:");
   lcd.setCursor(8,1);
   lcd.print(umid_terreno);
   lcd.setCursor(11,1);
   lcd.print("%");
  }
 /* FINISH HUMIDITY SOIL  -------------------------------------------------------*/


 /* HUMIDITY AND TEMPERATURE AIR ----------------------------------------------------*/
  if (millisecondi >= tempo3) {
   tempo3 = (tempo3 + (2000*4));

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("umidita: ");
    lcd.print(umidita);
    lcd.setCursor(0,1);
    lcd.print("temper.: ");
    lcd.print(temperatura);
   }
   /* FINISH HUMIDITY AND TEMPERATURE AIR------------------------------------------------*/

   /* CLOCK -------------------------------------------------------------------------- */
   /* use the variable orologio for enter in the if every second.
    * print a 12-hour timer on the screen with a duration of 4seconds and a refresh of every 1 second
    * when the 12 hours are up it starts again automatically and sets reset_auto = 1.
    * the reset_auto variable is used to reset ATmega328-PU and then reset the milliseconds 
    * variable to zero before it does so automatically every 50 days or so.
    */
  if (millisecondi >= orologio) {
    secondi = secondi + 1;
    orologio = orologio + 1000;
    if (secondi > 59) {
      minuti = minuti + 1;
      secondi = 0;
      if (minuti > 59) {
        ore = ore + 1;
        minuti = 0;
        if (ore > 11) {
        /* we use this if to make the reset happen only when the 12 hours of darkness of the 45th day have passed so that we can reset the arduino.
         * If we didn't do this, the reset might happen at the time the lights WERE 12 HOURS ON. So once reset they would stay on for another 12 hours. 
         * (When the sketch start/restarts the lamps will automatically light for 12 hours).
         */
          if (ore > 11 && volte_luce_buio > 1078) {
            reset_auto = 1;
          }
          else {
            reset_auto = 0;
          }
            volte_luce_buio = volte_luce_buio + 1;
            ore = 0;
        }
      }
    }
  }


   if (millisecondi >= tempo4) {
    tempo4 = (tempo4 + (1000));
    lcd.clear();
    lcd.setCursor(5,0);
    lcd.print("timer:");
    lcd.setCursor(0,1);
    lcd.print("h:");
    lcd.setCursor(2,1);
    lcd.print(ore);
    lcd.setCursor(6,1);
    lcd.print("m:");
    lcd.setCursor(8,1);
    lcd.print(minuti);
    lcd.setCursor(12,1);
    lcd.print("s:");
    lcd.setCursor(14,1);
    lcd.print(secondi);
    
    refresh_schermo = refresh_schermo +1;
    if (refresh_schermo > 3) {
      refresh_schermo = 0;
      tempo4 = (tempo4 + (2000*2));
    }
    }

    /* FINISH CLOCK ----------------------------------------------------------------*/


/* SECURITY SET TO 45 DAYS = 3.888.000.000 MILLISECONDS -----------------------*/
 /* this if is needed because once millis returns to zero, 
  * the variable orologio will be at a very high value...
  * (FROM A QUICK CALCULATION IT SHOULD RESULT 4,320,000,000 MILLISECONDS) 
  * so as soon as milli returns to zero then I reset all arduino. 
  * BUT ONLY WHEN THE TIMER HAS FINISHED 12 HOURS.
 */

 //  if (millisecondi >= 3888000000 && reset_auto == 1) {
 if (reset_auto == 1) {
/*reset automatically all*/
  Reset_AVR();
}
/* FINISH SECURITY -----------------------------------------------------------------*/

/* OPERATE PUMP FOR WATERING ------------------------------------------------------*/
if (millisecondi >= tempo5) {
  umid_terreno = analogRead(A0);
  umid_terreno = map(umid_terreno, 290, 380, 100, 0);
  tempo5 = tempo5 + 8000;
  
  if (umid_terreno <= 50) {
    if (pompa == LOW) {
      digitalWrite(3, pompa = HIGH);
    }
  }
  if (umid_terreno > 50) {
    digitalWrite(3, pompa = LOW);
  }
}
/* FINISH PUMP ----------------------------------------------------------------------------*/
  /* print some debugging information in the serial monitor */
  Serial.print("millisecondi: ");
  Serial.print(millisecondi);
  Serial.print("   ");
  Serial.print("tempo1: ");
  Serial.print(tempo1);
  Serial.print("  ");
  Serial.print("tempo2: ");
  Serial.print(tempo2);
  Serial.print("  ");
  Serial.print("tempo3: ");
  Serial.print(tempo3);
  Serial.print("  ");
  Serial.print("tempo4: ");
  Serial.print(tempo4);
  Serial.print("  ");
  Serial.print("orologio: ");
  Serial.print(orologio);
  Serial.print("  ");
  Serial.print("ore: ");
  Serial.print(ore);
  Serial.print("  ");
  Serial.print("minuti: ");
  Serial.print(minuti);
  Serial.print("  ");
  Serial.print("secondi: ");
  Serial.print(secondi);
  Serial.print("  ");
  Serial.print("umid_te: ");
  Serial.print(umid_terreno);
  Serial.print("  ");
  Serial.print("umid_te_lcd: ");
  Serial.print(umid_terreno_lcd);
  Serial.print("  ");
  Serial.print("tempo5: ");
  Serial.print(tempo5);
  Serial.print("  ");
  Serial.print("pompa: ");
  Serial.println(pompa);
}
