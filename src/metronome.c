/*
 * hear&now - a simple interactive audio mixer for cool kids
 * copyright (c) 2012 Colin Bayer & Rob Hanlon
 */

#include "hn.h"
#include "sequencer.h"

/* 
 * the "metronome" is the part of the sequencer where musical time lives.
 * the sequencer proper only recognizes samples and jiffies; the metronome
 * turns jiffies into beats and bars.
 */

