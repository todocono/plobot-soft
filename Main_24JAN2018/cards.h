
#ifndef CARDS_H
#define CARDS_H

#include <Arduino.h>

enum CardId {
  kCardNull = 0,
  kCardStart,
  kCardReset,
  kCardForward,
  kCardLeft,
  kCardRight,
  kCardBackward,
  kCardLift,
  kCardListen,
  kCardSee,
  kCardFeel,
  kCardRepeat,
  kCardStore,
  kCardRandom,
  kCardTeach,
  kCardBoost,
  kCardPause,
  kCardSound,
  kCardRed,
  kCardGreen,
  kCardOrange,
  kCardYellow,
  kCardPink,
  kCardPurple,
  kCardBlue,
  kCardMusicDo,
  kCardMusicRe,
  kCardMusicMi,
  kCardMusicFa,
  kCardMusicSol,
  kCardMusicLa,
  kCardMusicSi
};

void init_cards();
CardId read_one_card();
boolean card_sets_condition(CardId card);
void flush_cards();
boolean is_key_card(CardId card);
boolean is_note_card(CardId card);

const unsigned max_cards = 200;

class CardSequence {
  public:
    CardSequence();

    void clear();
    boolean queue(CardId card);
    CardId atIndex(int idx)const;
    int count()const;

    boolean empty()const;
  private:
    int n_cards_queued;
    CardId cards_queued[max_cards];
};

CardId raw_int_card_id(int raw_int);

#endif//CARDS_H
