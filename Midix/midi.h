#ifndef MIDI_H_
#define MIDI_H_

#include "bytebuffer.h"

#define MIDI_FILE_TYPE_SINGLE_TRACK       0
#define MIDI_FILE_TYPE_MULTI_TRACK_SYNCH  1
#define MIDI_FILE_TYPE_MULTI_TRACK_ASYNCH 2


typedef struct
{
    int Delta;
    int Command;
    int Channel;

    union
    {
        struct 
        {
            int NoteNumber;
            int Velocity;
        } NoteOff; /* 8 */

        struct 
        {
            int NoteNumber;
            int Velocity;
        } NoteOn; /* 9 */

        struct 
        {
            int NoteNumber;
            int Velocity;
        } KeyAfterTouch; /* 10 */

        struct 
        {
            int ControllerNumber;
            int NewValue;
        } ControlChange; /* 11 */

        struct 
        {
            int NewProgramNumber;
        } ProgramPatchChange; /* 12 */

        struct 
        {
            int ChannelNumber;
        } ChannelAfterTouch; /* 13 */

        struct 
        {
            int Bottom; /* LSB */
            int Top; /* MSB */
        } PitchWheelChange; /* 14 */

        struct 
        {
            int Data1;
            int Data2;
        } Generic;
    };

} MIDIEvent;

typedef struct 
{
    ByteBuffer bb;

    /* Header */
    int FileType;
    int TrackCount;
    int Division; /* delta-time ticks per quarter note (120)*/

    MIDIEvent** Events;
    int* _numElements;
    int* _numAllocated;

} MIDI;

int MIDI_ReadFile(MIDI* midi, const char* fileName);
int MIDI_ReadHeaderChunk(MIDI* midi);
int MIDI_ReadTrackChunk(MIDI* midi, int id);

void MIDI_AddEvent(MIDI* midi, MIDIEvent* event, int id);

void MIDI_NoteToFrequencyInit();
double MIDI_NoteToFrequency(int note);

#endif // MIDI_H_
