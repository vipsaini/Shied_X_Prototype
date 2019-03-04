#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <Entropy.h>
#include <ssss.h>
#include <wordlist.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);




#define OLED_DC    30   //DC  used
#define OLED_RESET 28   //RES used
#define OLED_MOSI  26   //D1  used
#define OLED_CLK   24   //D0  used
#define OLED_CS    32   //not used







Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);















char buffer[12][12];

void pin_setup(void);
void display_words(void);

void setup() 
{     
      pinMode(22, OUTPUT);
      digitalWrite(22, HIGH);  
      pinMode(23, OUTPUT);
      digitalWrite(23, HIGH);

      Entropy.initialize();      
      Serial.begin(9600);
      display.begin(SSD1306_SWITCHCAPVCC);
      nfc.begin();
      
      display.clearDisplay();
      display.setRotation(2);
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(10,10);
      display.print("CYPHEROCK");
      display.display();
      delay(1000);

      start:
      display.clearDisplay();
      display.setRotation(2);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("Press left to recoverseed");
      display.println("Press right to shard a seed");
      display.display();

      uint8_t choice = 0;
      while(choice == 0)
      {
        int sensorValue1 = analogRead(A0);
        int sensorValue2 = analogRead(A2);
        delay(100);
        int sensorValue3 = analogRead(A0);
        int sensorValue4 = analogRead(A2); 
        if(sensorValue1 - sensorValue3 > 500)
        {
          choice = 1;
        }
        if(sensorValue2 - sensorValue4 > 500)
        {
          choice = 2;
        }    
      }
      
     if(choice == 1)
     { 
      display.clearDisplay();
      display.setRotation(2);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("Press left/right to  shard old/new seed");
      display.display();

     
      while(1)
      {
        int sensorValue1 = analogRead(A0);
        int sensorValue2 = analogRead(A2);
        delay(100);
        int sensorValue3 = analogRead(A0);
        int sensorValue4 = analogRead(A2); 
        if(sensorValue1 - sensorValue3 > 500)
        {
          goto old_seed;
        }
        if(sensorValue2 - sensorValue4 > 500)
        {
          goto new_seed;
        }    
      }
          
          old_seed:
          new_seed:
      
          enter_pin:

          pin_setup();

          //display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print("Press left/right to  re-enter/use the PIN");
          display.display();
         
          while(1)
          {
            int sensorValue1 = analogRead(A0);
            int sensorValue2 = analogRead(A2);
            delay(100);
            int sensorValue3 = analogRead(A0);
            int sensorValue4 = analogRead(A2); 
            if(sensorValue1 - sensorValue3 > 500)
            {
              break;

            }
        
            if(sensorValue2 - sensorValue4 > 500)
            {
              goto enter_pin;
            }
            
          }

                 
          int index[12];
          for(int i = 0; i < 12; i++) 
          {
              index[i] = Entropy.random(2047);
              strcpy_P(buffer[i], (char*)pgm_read_word(&(string_table[index[i]])));
          }
          
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(2);
          display.setTextColor(WHITE);
          display.setCursor(10,0);
          display.println("  SEED");
          display.display();
          display.print("GENERATED!!!!");
          display.display();
          delay(2000);

          display_words();
         
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(2);
          display.setCursor(0,0);
          display.println("GENERATING");
          display.println("SHARES...");
          display.display();
          delay(1000);

          String conc_string = "";
          for(int i = 0; i < 12; i++) 
          {
             conc_string += (String)buffer[i];
        
             if(i == 11) 
             {
                continue;
             }
             conc_string += ",";
              
          }
                   
           uint8_t payload = conc_string.length();
           uint8_t data[payload];
           for(int i = 0; i < payload; i++) 
           {
               data[i] = conc_string[i];
           }
           
           uint8_t n_shares = 5;
           uint8_t threshold = 2; 
           uint8_t **shares = new uint8_t *[n_shares];        
           for(int i = 0; i < n_shares; i++) 
           {
              shares[i] = new uint8_t[payload + 2];
           }
          create_shares(data, n_shares, threshold, shares, payload);
          
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(2);
          display.setCursor(0,0);
          display.println("   SHARES");
          display.println("GENERATED.!!!");
          display.display();   
          delay(4000); 
        
          String UID1;
          String UID2;
          String UID3;
          String UID4;
          String UID5;
                   
          Serial.println("Tap 1st card to read secret 1");
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print("Tap 1st card to read secret 1");
          display.display();
        
         
          while(nfc.tagPresent() == 0){}//wait for nfc card
         
          if (nfc.tagPresent()) 
            {
        
                bool success = nfc.erase();
                if (success) {
                    Serial.println("\nTag Erased");        
                } else {
                    Serial.println("\nUnable to erase tag.");
                }
        
            }
        
          if (nfc.tagPresent()) {
                NfcTag tag = nfc.read();
                UID1 = tag.getUidString();
                NdefMessage message = NdefMessage();
                message.addMimeMediaRecord("secret1", shares[0], payload + 2);
                bool success = nfc.write(message);
                if (success) 
                {
                    Serial.println("Success. Tag written");
                } else 
                {
                    Serial.println("Write failed");
                }
               
            }
        
            if (nfc.tagPresent())
            {
              NfcTag tag = nfc.read();
              if (tag.hasNdefMessage()) // every tag won't have a message
                {
                  NdefMessage message = tag.getNdefMessage();
                  NdefRecord record = message.getRecord(0);
                  int payloadLength = record.getPayloadLength();
                  uint8_t payload1[payloadLength];
                  record.getPayload(payload1);
                  for(int i = 0; i < payloadLength; i++)
                  {
                    Serial.print(payload1[i]);
                    Serial.print(",");
                  }
                  Serial.println();
                }
            }
        
          Serial.println("Tap 2nd card to read secret 2");
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print("Tap 2nd card to read secret 2");
          display.display();
        
          check_UID1:
          
          while(nfc.tagPresent())//wait for previous tag to lift
          {
              NfcTag tag = nfc.read();
              UID2 = tag.getUidString();
              if(UID2 == UID1);
              delay(1000);
          }
          
          while(nfc.tagPresent() == 0){}//wait till new tag appears
          
          if (nfc.tagPresent()) //check UID of new tag if present
            {
                NfcTag tag = nfc.read();
                UID2 = tag.getUidString();
                if(UID2 == UID1)
                {
                  goto check_UID1;
                }
            }
          
        
          if (nfc.tagPresent()) 
            {
        
                bool success = nfc.erase();
                if (success) {
                    Serial.println("\nTag erased");        
                } else {
                    Serial.println("\nUnable to erase tag.");
                }
        
            }  
        
          if (nfc.tagPresent()) 
            {
                NfcTag tag = nfc.read();
                UID2 = tag.getUidString();
                NdefMessage message = NdefMessage();
                message.addMimeMediaRecord("secret2", shares[1], payload + 2 );
                bool success = nfc.write(message);
                if (success) 
                {
                    Serial.println("Success. Tag written");
                } else 
                {
                    Serial.println("Write failed");
                }
            }
        
            if (nfc.tagPresent())
            {
              NfcTag tag = nfc.read();
              if (tag.hasNdefMessage()) // every tag won't have a message
                {
                  NdefMessage message = tag.getNdefMessage();
                  NdefRecord record = message.getRecord(0);
                  int payloadLength = record.getPayloadLength();
                  uint8_t payload1[payloadLength];
                  record.getPayload(payload1);
                  for(int i = 0; i < payloadLength; i++)
                  {
                    Serial.print(payload1[i]);
                    Serial.print(",");
                  }
                  Serial.println();
                }
            }
        
          Serial.println("Tap 3rd card to read secret 3");
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print("Tap 3rd card to read secret 3");
          display.display();
        
          check_UID2:
          
          while(nfc.tagPresent())//wait for previous tag to lift
          {
              NfcTag tag = nfc.read();
              UID3 = tag.getUidString();
              if(UID3 == UID2 || UID3 == UID1);
              delay(1000);
          }
        
          if (nfc.tagPresent()) //check UID of new tag if present
            {
                NfcTag tag = nfc.read();
                UID3 = tag.getUidString();
                if(UID3 == UID2 || UID3 == UID1)
                {
                  goto check_UID2;
                }
            }
        
          if (nfc.tagPresent()) 
            {
        
                bool success = nfc.erase();
                if (success) {
                    Serial.println("\nTag erased");        
                } else {
                    Serial.println("\nUnable to erase tag.");
                }
        
            }
        
          if (nfc.tagPresent()) 
            {
                NfcTag tag = nfc.read();
                UID3 = tag.getUidString();
                NdefMessage message = NdefMessage();
                message.addMimeMediaRecord("secret3", shares[2], payload+2 );
                bool success = nfc.write(message);
                if (success) 
                {
                    Serial.println("Success. Tag written");
                } else 
                {
                    Serial.println("Write failed");
                }
            }
        
          if (nfc.tagPresent()) {
              NfcTag tag = nfc.read();
              if (tag.hasNdefMessage()) // every tag won't have a message
                {
                  NdefMessage message = tag.getNdefMessage();
                  NdefRecord record = message.getRecord(0);
                  int payloadLength = record.getPayloadLength();
                  uint8_t payload1[payloadLength];
                  record.getPayload(payload1);
                  for(int i = 0; i < payloadLength; i++)
                  {
                    Serial.print(payload1[i]);
                    Serial.print(",");
                  }
                  Serial.println();
                }
          }
        
          Serial.println("Tap 4th card to read secret 4");
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print("Tap 4th card to read secret 4");
          display.display();
        
        
          check_UID3:
          
          while(nfc.tagPresent())//wait for previous tag to lift
          {
              NfcTag tag = nfc.read();
              UID4 = tag.getUidString();
              if(UID4 == UID3 || UID4 == UID2 || UID4 == UID1);
              delay(1000);
          }
        
          if (nfc.tagPresent()) //check UID of new tag if present
            {
                NfcTag tag = nfc.read();
                UID4 = tag.getUidString();
                if(UID4 == UID3 || UID4 == UID2 || UID4 == UID1)
                {
                  goto check_UID3;
                }
            }
        
          if (nfc.tagPresent()) 
            {
        
                bool success = nfc.erase();
                if (success) {
                    Serial.println("\nTag erased");        
                } else {
                    Serial.println("\nUnable to erase tag.");
                }
        
            }
        
          if (nfc.tagPresent()) 
            {
                NfcTag tag = nfc.read();
                UID4 = tag.getUidString();
                NdefMessage message = NdefMessage();
                message.addMimeMediaRecord("secret4", shares[3], payload+2 );
                bool success = nfc.write(message);
                if (success) 
                {
                    Serial.println("Success. Tag written");
                } else 
                {
                    Serial.println("Write failed");
                }
            }
        
          if (nfc.tagPresent()) {
              NfcTag tag = nfc.read();
              if (tag.hasNdefMessage()) // every tag won't have a message
                {
                  NdefMessage message = tag.getNdefMessage();
                  NdefRecord record = message.getRecord(0);
                  int payloadLength = record.getPayloadLength();
                  uint8_t payload1[payloadLength];
                  record.getPayload(payload1);
                  for(int i = 0; i < payloadLength; i++)
                  {
                    Serial.print(payload1[i]);
                    Serial.print(",");
                  }
                  Serial.println();
                }
          }
        
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print("Sharing done, erasing memory...");
          display.display();   
          delay(4000);
          goto start;  
     }

     else if(choice == 2) 
     {
          enter_pin2:

          pin_setup();

          //display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print("Press left/right to  re-enter/use the PIN");
          display.display();
         
          while(1)
          {
            int sensorValue1 = analogRead(A0);
            int sensorValue2 = analogRead(A2);
            delay(100);
            int sensorValue3 = analogRead(A0);
            int sensorValue4 = analogRead(A2); 
            if(sensorValue1 - sensorValue3 > 500)
            {
              break;

            }
        
            if(sensorValue2 - sensorValue4 > 500)
            {
              goto enter_pin2;
            }
            
          }

          String UID6;
          String UID7;
         
          uint8_t payload;
          uint8_t **r_shares = new uint8_t *[2];         
          Serial.println("Tap any 2 cards to regenerate recovery seed"); 
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print("Tap a card with a shared secret");
          display.display();
          
         
          while(nfc.tagPresent() == 0){}//wait for a new nfc card
        
            if (nfc.tagPresent())
            {
              NfcTag tag = nfc.read();
              UID6 = tag.getUidString();
              if (tag.hasNdefMessage()) // every tag won't have a message
                {
                  NdefMessage message = tag.getNdefMessage();
                  NdefRecord record = message.getRecord(0);
                  payload = record.getPayloadLength();
                  r_shares[0] = new uint8_t[payload];
                  record.getPayload(r_shares[0]);
                  for(int i = 0; i < payload; i++)
                  {
                    Serial.print(r_shares[0][i]);
                    Serial.print(",");
                  }
                  Serial.println();
                }
            }
        
          Serial.println("Tap another card with a shared secret");
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print("Tap another card with a shared secret");
          display.display();
        
          check_UID6:
          
          while(nfc.tagPresent())
          {
              NfcTag tag = nfc.read();
              UID7 = tag.getUidString();
              if(UID7 == UID6);   
              delay(1000);
          }
          
          while(nfc.tagPresent() == 0){}//wait for a new nfc card
          
          if (nfc.tagPresent()) 
            {
                NfcTag tag = nfc.read();
                UID7 = tag.getUidString();
                if(UID7 == UID6)
                {
                  goto check_UID6;
                }
        
            }
        
            if (nfc.tagPresent())
            {
              NfcTag tag = nfc.read();
              UID7 = tag.getUidString();
              if (tag.hasNdefMessage()) // every tag won't have a message
                {
                  NdefMessage message = tag.getNdefMessage();
                  NdefRecord record = message.getRecord(0);
                  payload = record.getPayloadLength();
                  r_shares[1] = new uint8_t[payload];
                  record.getPayload(r_shares[1]);
                  for(int i = 0; i < payload; i++)
                  {
                    Serial.print(r_shares[1][i]);
                    Serial.print(",");
                  }
                  Serial.println();
                }
            }
        
         Serial.println("remove the card now");
         display.clearDisplay();
         display.setRotation(2);
         display.setTextSize(2);
         display.setTextColor(WHITE);
         display.setCursor(0,0);
         display.println("GENERATING");
         display.println("SEED");
         display.display();
          
          while(nfc.tagPresent()){}
          
          payload -= 2;
          uint8_t secret_array[payload];
          
          extract_secret(r_shares, secret_array, 2 , payload);

          for(uint8_t i = 0; i < payload; i++) 
          {
            Serial.print(secret_array[i]);
            Serial.print(" ");
          }
          
          String final_secret[12];
          uint8_t j = 0;
          
          for(uint8_t i = 0; i < 12; i++) 
          {
            String temp = "";
            while(j < payload) 
            {
              if(secret_array[j] == 44)
              {
                final_secret[i] = temp;
                j++;
                break;
              }
        
              temp += char(secret_array[j]);
              j++;
            }
        
            if(j == payload)
            {
              final_secret[i] = temp;
            }
         }


         for(uint8_t i = 0; i < 12; i++) 
         {
            Serial.println(final_secret[i]);
         }

         display.clearDisplay();
         display.setRotation(2);
         display.setTextSize(2);
         display.setTextColor(WHITE);
         display.setCursor(0,0);
         display.println("SEED");
         display.println("GENERATED");
         display.display();
         delay(1000);
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.println("Press left/right to  see words or press   both to return. Now  press right");
          display.display();
          while(1)
          {
            int sensorValue1 = analogRead(A0);
            int sensorValue2 = analogRead(A2);
            delay(100);
            int sensorValue3 = analogRead(A0);
            int sensorValue4 = analogRead(A2); 
            if(sensorValue1 - sensorValue3 > 500)
            {
              break;
            }
        
            if(sensorValue2 - sensorValue4 > 500)
            {
              //do nothing     
            }
            
          }

          uint8_t next_word = 0;
          
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print("*********************");
          display.display();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,10);
          display.print(next_word+1);
          display.display();
          display.setRotation(2);
          display.setTextSize(2);
          display.setTextColor(WHITE);
          display.setCursor(15,10);
          display.print(final_secret[next_word]);
          display.display();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,25);
          display.print("*********************");
          display.display();

          while(1)
          {
            int sensorValue1 = analogRead(A0);
            int sensorValue2 = analogRead(A2);
            delay(100);
            int sensorValue3 = analogRead(A0);
            int sensorValue4 = analogRead(A2); 
            if(sensorValue1 - sensorValue3 > 500)
            {
                if(next_word == 11){next_word = 0;}
                else        
                  next_word = next_word + 1;
                  display.clearDisplay();
                  display.setRotation(2);
                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0,0);
                  display.print("*********************");
                  display.display();
                  display.setRotation(2);
                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0,10);
                  display.print(next_word+1);
                  display.display();
                  display.setRotation(2);
                  display.setTextSize(2);
                  display.setTextColor(WHITE);
                  display.setCursor(15,10);
                  display.print(final_secret[next_word]);
                  display.display();
                  display.setRotation(2);
                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0,25);
                  display.print("*********************");
                  display.display();                        
                 
            }
        
            if(sensorValue2 - sensorValue4 > 500)
            {
                if(next_word == 0){next_word = 11;}
                else
                  next_word = next_word - 1;
                  display.clearDisplay();
                  display.setRotation(2);
                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0,0);
                  display.print("*********************");
                  display.display();
                  display.setRotation(2);
                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0,10);
                  display.print(next_word+1);
                  display.display();
                  display.setRotation(2);
                  display.setTextSize(2);
                  display.setTextColor(WHITE);
                  display.setCursor(15,10);
                  display.print(final_secret[next_word]);
                  display.display();
                  display.setRotation(2);
                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0,25);
                  display.print("*********************");
                  display.display();                  
            }
        
            if((sensorValue2 == sensorValue4 || sensorValue1 == sensorValue3) && sensorValue1 != 0 && sensorValue2 != 0)
            {
                bool lift = 0;
                while( lift == 0)
                {
                  sensorValue1 = analogRead(A0);
                  sensorValue2 = analogRead(A2);
                  delay(200);
                  sensorValue3 = analogRead(A0);
                  sensorValue4 = analogRead(A2); 
                  if(sensorValue1 - sensorValue3 > 500 || sensorValue2 - sensorValue4 > 500)
                  {
                     lift = 1;     
                  }
                  
                }
                break;
            }     
          }
          goto start;
     }
          
}

void loop()
{}

void pin_setup(void)
{
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(2);
          display.setTextColor(WHITE);
          display.setCursor(10,0);
          display.print("ENTER PIN");
          display.display();
          delay(1000);

          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.println("Press left/right to  change or press both to select an option. Now press right.");
          display.display();
          
   
      while(1)
      {
        int sensorValue1 = analogRead(A0);
        int sensorValue2 = analogRead(A2);
        delay(100);
        int sensorValue3 = analogRead(A0);
        int sensorValue4 = analogRead(A2); 
        if(sensorValue1 - sensorValue3 > 500)
        {
          break;
        }
        if(sensorValue2 - sensorValue4 > 500)
        {
          //do nothing
        }    
      }

          
          uint8_t pin_number[10] = {0,1,2,3,4,5,6,7,8,9};
          uint8_t received_pin[6];    
          uint8_t next_number = 0;
          uint8_t next_pin_number = 0;  
          uint8_t pin_set = 0;                 
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(2);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print(pin_number[0]);
          display.display();

          while(pin_set == 0)
          {
            int sensorValue1 = analogRead(A0);
            int sensorValue2 = analogRead(A2);
            delay(100);
            int sensorValue3 = analogRead(A0);
            int sensorValue4 = analogRead(A2); 
            if(sensorValue1 - sensorValue3 > 500)
            {
                if(next_number == 9){next_number = 0;}
                else        
                  next_number = next_number + 1;
                display.setRotation(2);
                display.setTextSize(2);
                display.setTextColor(WHITE,BLACK);
                display.setCursor(0,0);
                display.print(pin_number[next_number]);//
                display.display();         
                 
            }
        
            if(sensorValue2 - sensorValue4 > 500)
            {
                if(next_number == 0){next_number = 9;}
                else
                  next_number = next_number - 1;
                display.setRotation(2);
                display.setTextSize(2);
                display.setTextColor(WHITE,BLACK);
                display.setCursor(0,0);
                display.print(pin_number[next_number]);// 
                display.display();
            }
        
            if((sensorValue2 == sensorValue4 || sensorValue1 == sensorValue3) && sensorValue1 != 0 && sensorValue2 != 0)
            {
                bool lift = 0;
                while( lift == 0)
                {
                  sensorValue1 = analogRead(A0);
                  sensorValue2 = analogRead(A2);
                  delay(200);
                  sensorValue3 = analogRead(A0);
                  sensorValue4 = analogRead(A2); 
                  if(sensorValue1 - sensorValue3 > 500 || sensorValue2 - sensorValue4 > 500)
                  {
                     lift = 1;     
                  }
                  
                }
        
                display.setRotation(2);
                display.setTextSize(2);
                display.setTextColor(WHITE,BLACK);
                display.setCursor(next_pin_number*20,15);
                display.print(pin_number[next_number]);// 
                display.display();
                received_pin[next_pin_number] = pin_number[next_number];
                next_pin_number = next_pin_number + 1;
                if(next_pin_number == 6)
                {
                  pin_set = 1;
                }
            }
            
          }
 
          display.clearDisplay();       
          for(int i = 0; i < 6; i++)
          {
                display.setRotation(2);
                display.setTextSize(1);
                display.setTextColor(WHITE,BLACK);
                display.setCursor(5+i*20,25);
                display.print(received_pin[i]);
                display.display();
          }
}
void display_words(void)
{
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.println("Press left/right to  see words or press   both to return. Now press right");
          display.display();
          while(1)
          {
            int sensorValue1 = analogRead(A0);
            int sensorValue2 = analogRead(A2);
            delay(100);
            int sensorValue3 = analogRead(A0);
            int sensorValue4 = analogRead(A2); 
            if(sensorValue1 - sensorValue3 > 500)
            {
              break;
            }
        
            if(sensorValue2 - sensorValue4 > 500)
            {
              //do nothing     
            }
            
          }

          uint8_t next_word = 0;
          
          display.clearDisplay();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print("*********************");
          display.display();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,10);
          display.print(next_word+1);
          display.display();
          display.setRotation(2);
          display.setTextSize(2);
          display.setTextColor(WHITE);
          display.setCursor(15,10);
          display.print(buffer[next_word]);
          display.display();
          display.setRotation(2);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,25);
          display.print("*********************");
          display.display();

          while(1)
          {
            int sensorValue1 = analogRead(A0);
            int sensorValue2 = analogRead(A2);
            delay(100);
            int sensorValue3 = analogRead(A0);
            int sensorValue4 = analogRead(A2); 
            if(sensorValue1 - sensorValue3 > 500)
            {
                if(next_word == 11){next_word = 0;}
                else        
                  next_word = next_word + 1;
                  display.clearDisplay();
                  display.setRotation(2);
                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0,0);
                  display.print("*********************");
                  display.display();
                  display.setRotation(2);
                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0,10);
                  display.print(next_word+1);
                  display.display();
                  display.setRotation(2);
                  display.setTextSize(2);
                  display.setTextColor(WHITE);
                  display.setCursor(15,10);
                  display.print(buffer[next_word]);
                  display.display();
                  display.setRotation(2);
                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0,25);
                  display.print("*********************");
                  display.display();                        
                 
            }
        
            if(sensorValue2 - sensorValue4 > 500)
            {
                if(next_word == 0){next_word = 11;}
                else
                  next_word = next_word - 1;
                  display.clearDisplay();
                  display.setRotation(2);
                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0,0);
                  display.print("*********************");
                  display.display();
                  display.setRotation(2);
                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0,10);
                  display.print(next_word+1);
                  display.display();
                  display.setRotation(2);
                  display.setTextSize(2);
                  display.setTextColor(WHITE);
                  display.setCursor(15,10);
                  display.print(buffer[next_word]);
                  display.display();
                  display.setRotation(2);
                  display.setTextSize(1);
                  display.setTextColor(WHITE);
                  display.setCursor(0,25);
                  display.print("*********************");
                  display.display();                  
            }
        
            if((sensorValue2 == sensorValue4 || sensorValue1 == sensorValue3) && sensorValue1 != 0 && sensorValue2 != 0)
            {
                bool lift = 0;
                while( lift == 0)
                {
                  sensorValue1 = analogRead(A0);
                  sensorValue2 = analogRead(A2);
                  delay(200);
                  sensorValue3 = analogRead(A0);
                  sensorValue4 = analogRead(A2); 
                  if(sensorValue1 - sensorValue3 > 500 || sensorValue2 - sensorValue4 > 500)
                  {
                     lift = 1;     
                  }
                  
                }
                break;
            }     
          }
}

