/*
   --------------------------------------------------------------------------------------------------------------------
   Example sketch/program showing how to read data from a PICC to serial.
   --------------------------------------------------------------------------------------------------------------------
   This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid

   Example sketch/program showing how to read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
   Reader on the Arduino SPI interface.

   When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
   then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
   you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
   will show the ID/UID, type and any data blocks it can read. Note: you may see "Timeout in communication" messages
   when removing the PICC from reading distance too early.

   If your reader supports it, this sketch/program will read all the PICCs presented (that is: multiple tag reading).
   So if you stack two or more PICCs on top of each other and present them to the reader, it will first output all
   details of the first and then the next PICC. Note that this may take some time as all data blocks are dumped, so
   keep the PICCs at reading distance until complete.

   @license Released into the public domain.

   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN          22         // Configurable, see typical pin layout above
#define RST_PIN         23          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance


class CryptoHalter {
  private:
    MFRC522 &mfrc522;
  public:
    CryptoHalter(MFRC522 &mfrc522)
      : mfrc522(mfrc522) {
    }
    ~CryptoHalter() {
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
};


void setup() {
  Serial.begin(57600);		// Initialize serial communications with the PC
  while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();			// Init SPI bus
  mfrc522.PCD_Init();		// Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, type, and data blocks..."));
}

void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  //
  uint32_t raw_id = 0;
  //  if (mfrc522.uid.size != sizeof(raw_id)) {
  //    Serial.println("ERROR: Card has wrong UID size");
  //  } else {
  //    memcpy(&raw_id, mfrc522.uid.uidByte, sizeof(raw_id));
  //    // raw_id_to_card_id(raw_id);
  //    Serial.println(raw_id);
  //  }
  MFRC522::MIFARE_Key key;
  memset(&key, 0xFF, sizeof(key));

  byte buffer[18];
  byte size = sizeof(buffer);
  byte trailerBlock   = 7;
  byte blockAddr      = 4;

  CryptoHalter halter(mfrc522);

  MFRC522::StatusCode status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Failed to authenticate with card");
    return false;
  }
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Failed to read sector from card");
    return false;
  }

  memcpy(&raw_id, buffer, sizeof(raw_id));
  Serial.println(raw_id);
}




boolean read_one_card() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  uint32_t raw_id = 0;
  if (mfrc522.uid.size != sizeof(raw_id)) {
    Serial.print("ERROR: Card has wrong UID size: ");
    Serial.println(mfrc522.uid.size);
    return false;
  }
  memcpy(&raw_id, mfrc522.uid.uidByte, sizeof(raw_id));
  //boolean ret = raw_id_to_card_id(raw_id);
  boolean ret = raw_id;
  // Try again with sector read
  if (ret == false) {
    MFRC522::MIFARE_Key key;
    memset(&key, 0xFF, sizeof(key));

    byte buffer[18];
    byte size = sizeof(buffer);
    byte trailerBlock   = 7;
    byte blockAddr      = 4;

    CryptoHalter halter(mfrc522);

    MFRC522::StatusCode status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.println("Failed to authenticate with card");
      return false;
    }
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
      Serial.println("Failed to read sector from card");
      return false;
    }

    memcpy(&raw_id, buffer, sizeof(raw_id));

    ret = raw_id;
  }

  if (ret == false) {
    Serial.print("Unknown card ID received: ");
    Serial.println(raw_id);
    return false;
  }

  // One card at a time
  flush_cards();

  return true;
}


void flush_cards() {
  while (mfrc522.PICC_IsNewCardPresent()) {
    mfrc522.PICC_ReadCardSerial();
  }
}

