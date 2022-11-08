#include <SPI.h>
#include <Servo.h>
#include <EEPROM.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
#include <LiquidCrystal_I2C.h>

#include "DHT.h"

#define DHTPIN 12
#define DHTTYPE DHT11

Servo myservo;
MFRC522 mfrc522(53, 48); // MFRC522 mfrc522(SS_PIN, RST_PIN)
DHT dht(DHTPIN, DHTTYPE); 
SoftwareSerial mySerial(3, 2);
LiquidCrystal_I2C lcd(0x27, 16, 2);


const int ledm = 22;
const int ledk = 23;
const int ledh = 24;
const int button = 25;
const int relay1 = 26;
const int relay2 = 27;

int pos = 90;
int flag = 0;
int tampilan = 0;

/*volatile boolean sw1 = false;
uint8_t sw1ButtonState = 0;
uint8_t lastsw1ButtonState = 0;*/

int ButtonState;
int count = 0;


String tagUID = "93 E1 9E 18"; 

char password[4];
char pass[4],pass1[4];
int i=0;
char key_pressed = 0; 

const byte rows = 4;
const byte columns = 4;

char hexaKeys[rows][columns] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'*','0','#','D'},
  {'7','8','9','C'}
};

byte row_pins[rows] = {11, 10, 8, 9};
byte column_pins[columns] = {7, 6, 5, 4};

Keypad keypad_key = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);

int Bot_mtbs = 1000;
long Bot_lasttime;

boolean RFIDMode = true; 
boolean PassMode = false;
boolean SuhuMode = false; 

void setup() {
  pinMode(ledm, OUTPUT);
  
  digitalWrite(ledm, 1);
  pinMode(ledk, OUTPUT);
  pinMode(ledh, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(button, INPUT_PULLUP);

  digitalWrite(ledk, 0);
  digitalWrite(ledm, 0);
  digitalWrite(relay1, 1);
  digitalWrite(relay2, 1);  
  myservo.attach(13); 
  myservo.write(pos);

  mySerial.begin (9600);
 
  mp3_set_serial (mySerial);
  delay(10);
 
  mp3_reset();  
  delay(10);  
 
  mp3_set_volume (30); 
  delay(1000);

  lcd.init();
  lcd.backlight();

  SPI.begin();      
  mfrc522.PCD_Init();  

  lcd.clear(); 

  lcd.setCursor(0,0);
           //0123456789012345
  lcd.print("  LEMARI  ALAT  ");
  lcd.setCursor(0,1);
           //0123456789012345
  lcd.print("PENGERING JAGUNG");
  delay(5000);
  
  dht.begin();
}

void loop () {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (SuhuMode == true) {
    if (millis() > Bot_lasttime + Bot_mtbs){
      lcd.clear();
      lcd.setCursor(0,0);
               //0123456789012345
      lcd.print("Humd : ");
      lcd.print(h);
      lcd.print(" %");
      lcd.setCursor(0,1);
              //0123456789012345
      lcd.print("Temp : ");
      lcd.print(t);
      lcd.print(" *C");
      delay(500);

      Bot_lasttime = millis();
    }
    
    if (t<=34) {
      //heater hidup
      digitalWrite(relay1, LOW);
      //fan mati
      digitalWrite(relay2, HIGH);  
    }
    else if (t>38) {
      //heater mati
      digitalWrite(relay1, HIGH);
      //fan hidup 
      digitalWrite(relay2, LOW);
    }
  }
  else if (SuhuMode == false) {
      digitalWrite(relay1, HIGH);
      digitalWrite(relay2, HIGH);
  }
  
  if (RFIDMode == true) {
    digitalWrite(ledm, 1);
    digitalWrite(ledk, 0);
    digitalWrite(ledh, 0);
    digitalWrite(relay1, 1);
    digitalWrite(relay2, 1);
    
    lcd.setCursor(0,0);
             //0123456789012345
    lcd.print(" Tempelkan  Tag ");
    lcd.setCursor(0,1);
             //0123456789012345
    lcd.print("   RFID Anda!   ");

    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }

    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    String tag = "";
    for (byte j = 0; j < mfrc522.uid.size; j++) {
      tag.concat(String(mfrc522.uid.uidByte[j] < 0x10 ? " 0" : " "));
      tag.concat(String(mfrc522.uid.uidByte[j], HEX));
    }
    tag.toUpperCase();

    if (tag.substring(1) == tagUID) {
      mp3_play(1);
      lcd.clear();
      lcd.setCursor(0,0);
               //0123456789012345
      lcd.print("Akses di Terima ");
      delay(3000);
      mp3_play(3);
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Masukan Password");
      lcd.setCursor(0,1);
      lcd.print("Pass: ");
      RFIDMode = false;
      PassMode = true; 
    }
    else {
      lcd.clear();
      lcd.setCursor(0,0);
               //0123456789012345
      lcd.print(" Akses di Tolak ");
      mp3_play(2);
      delay(3000);
      lcd.clear(); 
    }
  }

  if (PassMode == true) {
    key_pressed = keypad_key.getKey(); 
    if(key_pressed=='#')
      change();
    if (key_pressed) {
      password[i++] = key_pressed;
      lcd.print("*");
    }
    if (i == 4) {
      delay(200);
      for(int h=0;h<4;h++)
      pass[h]=EEPROM.read(h);
      if (!(strncmp(password, pass, 4))) {
        lcd.clear();
        digitalWrite(ledm, 1);
        digitalWrite(ledk, 0);
        digitalWrite(ledh, 1);
        digitalWrite(relay1, 1);
        digitalWrite(relay2, 1);
                 //0123456789012345
        lcd.print("     Success    ");
        myservo.write(0);
        lcd.clear();
        i = 0;
        //RFIDMode = true;
        PassMode = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tekan Button");
        lcd.setCursor(0,1);
        lcd.print("Untuk Memulai");
        tampilan=1; 
      }
      else {
        
        lcd.clear();
             //0123456789012345
        lcd.print("      Wrong     ");
        digitalWrite(ledm, 1);
        digitalWrite(ledk, 1);
        digitalWrite(ledh, 0);
        digitalWrite(relay1, 1);
        digitalWrite(relay2, 1);
        salah();
        delay(5000);
        lcd.clear();
        i = 0;
        RFIDMode = false;
        PassMode = true; 
        flag=0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Masukan Password");
        lcd.setCursor(0,1);
        lcd.print("Pass: ");
      }
    }
  }

  if (tampilan == 1) {
     ButtonState   = digitalRead(button);
     
      if(ButtonState == 0) {
        count++;
        delay(500);
        if (count == 1) {
          myservo.write(90);
          digitalWrite(ledm, 1);
          digitalWrite(ledk, 0);
          digitalWrite(ledh, 0);
          digitalWrite(relay1,0);
          SuhuMode = true;
        }
        if (count == 2) {
          SuhuMode = false;
          myservo.write(0);
          digitalWrite(relay1, 1);
          mp3_play(5);
          lcd.setCursor(0,0);
                   //0123456789012345
          lcd.print("  Pengeringan   ");
          lcd.setCursor(0,1);
                   //0123456789012345
          lcd.print(" Telah  ,Selesai ");
        }
        if (count == 3) {
          myservo.write(90);
          count=0;
          RFIDMode = true;
          SuhuMode = false;
        }
      }
  }
}
///////////////////////////////////////////////////////////////////////////////////
void mServo() {
  myservo.write(90);
  delay(5000);
  myservo.write(0);
}

void salah() {
  if (flag==0) {
    mp3_play(4);
    flag=1;
  }
  else {
    flag=0;
  }
}
///////////////////////////////////////////////////////////////////////////////////
void change()
{
  int h=0;
  lcd.clear();
  lcd.print("Password Lama :");
  lcd.setCursor(0,1);
  while(h<4)
  {
    char key=keypad_key.getKey();
    if(key)
    {
      pass1[h++]=key;
      lcd.print("*");

    }
    key=0;
  }
  delay(500);
  
  if((strncmp(pass1, pass, 4)))
  {
    lcd.clear();
    lcd.print("Password Salah");
    lcd.setCursor(0,1);
    lcd.print("Coba Lagi");
    delay(1000);
  }
  else
  {
    h=0;
    
  lcd.clear();
  lcd.print("Password Baru :");
  lcd.setCursor(0,1);
  while(h<4)
  {
    char key=keypad_key.getKey();
    if(key)
    {
      pass[h]=key;
      lcd.print("*");
      EEPROM.write(h,key);
      h++;
    }
  }
  lcd.print("  Done......");
  delay(1000);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Masukan Password");
  lcd.setCursor(0,1);
  lcd.print("Pass: ");
  key_pressed=0;
}
