hear-now
========

hear&amp;now is a lightweight interactive audio mixer library for cool dudes

caveat emptor
=============

hear&amp;now is in early development, and should not be considered thread-safe, work-safe, or dishwasher-safe.  it is being developed by a couple of dudes who haven't really developed a ton of audio software before.  the terms of the license for hear&amp;now explicitly preclude you from using it to throw a rave at a nuclear power plant or hospital.

architecture
============

disclaimers
===========
we've only tested the following set of native audio formats:

    44100Hz / 8-bit / mono

it's a pretty small set.  where you read "native X", read "native X in 
principle, but really it has to be one of those formats I saw earlier."

Audio
=====

the Audio object is an minimal PCM audio device.  it plays audio at the 
sample rate and bit depth you request when opening it, and will not hold 
your hand by resampling or mixing down multiple channels to one.

the programming model for Audio is simple: you pass audio in on your own
thread when you deem it necessary; Audio will give you a callback whenever a
buffer is finished so you can free it or keep track of how much audio Audio
still has ready or whatever your consumer needs to do.

Mixer
=====

the Mixer object is a simple audio mixer.  its input is an arbitrary number of
floating-point (0-1.0) audio streams with the native sample rate and number of 
channels.  it converts these audio streams to the native bit depth and gives
the mixed stream to Audio.

the programming model for Mixer is based around callbacks: audio generators
register a generator callback with Mixer, which Mixer then calls whenever it 
feels that it needs to go to the audio store to get some more audio.

automation, Cmds, and CmdQueues
===============================

audio generators (or, really, pretty much anything) can be controlled
automatically by Cmd objects.  they create CmdQueues and expose them to their
controllers.  the owner of the CmdQueue dequeues Cmds and handles them in a
generator-specific fashion.

Sequencer
=========

the Sequencer object is a minimal audio sequencer.  it lies outside of the
Audio/Mixer/generator stack.  the Sequencer deals with musical time -- that
is, time as governed by the tempo of a performance, rather than the audio
device's sampling rate -- and provides you the ability to schedule 
automation commands based on musical time measurements.

the atomic time unit of the Sequencer is the "jiffy", and the mapping from
samples onto jiffies is governed by the Sequencer's "jiffy tempo".  (it's
intended that a jiffy be equal to the length of a 256th note; under this
assumption, jiffy tempo is approximately equal to real tempo in 4/4 time.)

the primary metaphor of the Sequencer is the "trigger", a Cmd saved up to send
to a CmdQueue at a given future time.  just before the appropriate sample is
rendered, the Sequencer populates fields in the Cmd telling the generator the
exact sample to begin executing the Cmd at.