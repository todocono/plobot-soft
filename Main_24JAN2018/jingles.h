
#ifndef JINGLES_H
#define JINGLES_H

#include "cards.h"

void error_jingle();
void card_scan_jingle(CardId card);
void play_note(CardId action_id);
void pause_jingle(boolean pausing);
void subroutine_jingle(boolean at_end);

#endif//JINGLES_H

