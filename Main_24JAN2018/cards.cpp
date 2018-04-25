
#include "cards.h"
#include <Arduino.h>
#include <MFRC522.h>


#define SS_PIN          22
#define RST_PIN         23


MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void init_cards() {
  mfrc522.PCD_Init();		        // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
}



CardId raw_id_to_card_id(uint32_t raw_id) {
  CardId ret = kCardNull;

  switch (raw_id) {
    case 4022845438:
      ret = kCardStart;
      break;
    case 4023366910:
      ret = kCardReset;
      break;
    case 4022881806:
      ret = kCardForward;
      break;
    case 4023044334:
      ret = kCardLeft;
      break;
    case 4024163854:
      ret = kCardRight;
      break;
    case 4024580926:
      ret = kCardBackward;
      break;
    case 4024373918:
      ret = kCardLift;
      break;
    case 4023109790:
      ret = kCardListen;
      break;
    case 4023814350:
      ret = kCardSee;
      break;
    case 4022752894:
      ret = kCardFeel;
      break;
    case 4023838654:
      ret = kCardRepeat;
      break;
    case 4023479230:
      ret = kCardStore;
      break;
    case 4022925038:
      ret = kCardRandom;
      break;
    case 4022726558:
      ret = kCardTeach;
      break;
    case 4023381310:
      ret = kCardBoost;
      break;
    case 4023822254:
      ret = kCardPause;
      break;
    case 4022660942:
      ret = kCardSound;
      break;
    case 4024090958:
      ret = kCardRed;
      break;
    case 4022888430:
      ret = kCardGreen;
      break;
    case 4022892430:
      ret = kCardOrange;
      break;
    case 4023831934:
      ret = kCardYellow;
      break;
    /*
        case 4023282862:
          ret = kCardPink;
          break;
        case 4024213694:
          ret = kCardPurple;
          break;
    */
    case 4023282862:
      ret = kCardPurple;
      break;
    case 4024213694:
      ret = kCardPink;
      break;
    case 4024006366:
      ret = kCardBlue;
      break;
    case 4023906254:
      ret = kCardMusicDo;
      break;
    case 4024583486:
      ret = kCardMusicRe;
      break;
    case 4024035966:
      ret = kCardMusicMi;
      break;
    case 4024720814:
      ret = kCardMusicFa;
      break;
    case 4024715182:
      ret = kCardMusicSol;
      break;
    case 4023629246:
      ret = kCardMusicLa;
      break;
    case 3310867317:
      ret = kCardMusicSi;
      break;
  }

  return ret;
}

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

CardId read_one_card() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return kCardNull;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return kCardNull;
  }

  uint32_t raw_id = 0;
  if (mfrc522.uid.size != sizeof(raw_id)) {
    Serial.print("ERROR: Card has wrong UID size: ");
    Serial.println(mfrc522.uid.size);
    return kCardNull;
  }
  memcpy(&raw_id, mfrc522.uid.uidByte, sizeof(raw_id));
  CardId ret = raw_id_to_card_id(raw_id);

  // Try again with sector read
  if (ret == kCardNull) {
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
      return kCardNull;
    }
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
      Serial.println("Failed to read sector from card");
      return kCardNull;
    }

    memcpy(&raw_id, buffer, sizeof(raw_id));

    ret = raw_id_to_card_id(raw_id);
  }

  if (ret == kCardNull) {
    Serial.print("Unknown card ID received: ");
    Serial.println(raw_id);
    return kCardNull;
  }

  // One card at a time
  flush_cards();

  return ret;
}

void flush_cards() {
  while (mfrc522.PICC_IsNewCardPresent()) {
    mfrc522.PICC_ReadCardSerial();
  }
}

boolean is_key_card(CardId card) {
  return card == kCardStart ||
         card == kCardReset ||
         card == kCardRed ||
         card == kCardGreen ||
         card == kCardOrange ||
         card == kCardYellow ||
         card == kCardPink ||
         card == kCardPurple ||
         card == kCardBlue ||
         card == kCardStore;
}

boolean card_sets_condition(CardId card) {
  switch (card) {
    case kCardSee:
    case kCardListen:
    case kCardFeel:
    case kCardRandom:
    case kCardTeach:
      return true;
  }
  return false;
}

boolean is_note_card(CardId card) {
  return card >= kCardMusicDo && card <= kCardMusicSi;
}

CardSequence::CardSequence() {
  clear();
}

void CardSequence::clear() {
  n_cards_queued = 0;
  memset(&cards_queued[0], 0, sizeof(cards_queued));
}

boolean CardSequence::queue(CardId card) {
  if (n_cards_queued == max_cards)
    return false;
  cards_queued[n_cards_queued++] = card;
  return true;
}

CardId CardSequence::atIndex(int idx)const {
  if (idx < 0 || idx >= n_cards_queued)
    return kCardNull;
  return cards_queued[idx];
}

int CardSequence::count()const {
  return n_cards_queued;
}

boolean CardSequence::empty()const {
  return n_cards_queued == 0;
}

CardId raw_int_card_id(int raw_int) {
  CardId ret = kCardNull;

  switch (raw_int) {
    case 1:
      ret = kCardStart;
      break;
    case 2:
      ret = kCardReset;
      break;
    case 3:
      ret = kCardForward;
      break;
    case 4:
      ret = kCardLeft;
      break;
    case 5:
      ret = kCardRight;
      break;
    case 6:
      ret = kCardBackward;
      break;
    case 7:
      ret = kCardLift;
      break;
    case 8:
      ret = kCardListen;
      break;
    case 9:
      ret = kCardSee;
      break;
    case 10:
      ret = kCardFeel;
      break;
    case 11:
      ret = kCardRepeat;
      break;
    case 12:
      ret = kCardStore;
      break;
    case 13:
      ret = kCardRandom;
      break;
    case 14:
      ret = kCardTeach;
      break;
    case 15:
      ret = kCardBoost;
      break;
    case 16:
      ret = kCardPause;
      break;
    case 17:
      ret = kCardSound;
      break;
    case 18:
      ret = kCardRed;
      break;
    case 19:
      ret = kCardGreen;
      break;
    case 20:
      ret = kCardOrange;
      break;
    case 21:
      ret = kCardYellow;
      break;
    case 22:
      ret = kCardPurple;
      break;
    case 23:
      ret = kCardPink;
      break;
    case 24:
      ret = kCardBlue;
      break;
    case 25:
      ret = kCardMusicDo;
      break;
    case 26:
      ret = kCardMusicRe;
      break;
    case 27:
      ret = kCardMusicMi;
      break;
    case 28:
      ret = kCardMusicFa;
      break;
    case 29:
      ret = kCardMusicSol;
      break;
    case 30:
      ret = kCardMusicLa;
      break;
    case 31:
      ret = kCardMusicSi;
      break;
  }
  
  return ret;
}

