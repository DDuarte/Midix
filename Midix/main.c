#define _CRT_SECURE_NO_WARNINGS

#include "midi.h"
#include "speaker.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    MIDI midi;
    int i;
    int j;
    int k;

    midi.Events = NULL;
    midi._numAllocated = 0;
    midi._numElements = 0;

    if (MIDI_ReadFile(&midi, "sweet.mid") != 0)
    {
        printf("MIDI_ReadFile failed\n");
        return 1;
    }

    if (MIDI_ReadHeaderChunk(&midi) != 0)
    {
        printf("MIDI_ReadHeaderChunk failed\n");
        return 1;
    }

    midi.Events = (MIDIEvent**)malloc(midi.TrackCount*sizeof(MIDIEvent*));
    midi._numAllocated = (int*)malloc(midi.TrackCount*sizeof(int));
    midi._numElements = (int*)malloc(midi.TrackCount*sizeof(int));

    memset(midi._numElements, 0, midi.TrackCount*sizeof(int));
    memset(midi._numAllocated, 0, midi.TrackCount*sizeof(int));

    for (i = 0; i < midi.TrackCount; ++i)
    {
        midi.Events[i] = (MIDIEvent*)malloc(0);

        printf("**** Track %i\n", i);
        if (MIDI_ReadTrackChunk(&midi, i) != 0)
        {
            printf("MIDI_ReadTrackChunk %i failed\n", i);
            return 1;
        }
    }

    MIDI_NoteToFrequencyInit();

    for (i = 0; i < 1000; ++i)
    for (j = 0; j < midi.TrackCount; ++j)
            if (midi.Events[j][i].Command == 9 && midi.Events[j][i].NoteOn.Velocity != 0)
                speaker_test(MIDI_NoteToFrequency(midi.Events[j][i].NoteOn.NoteNumber), 300);


    return 0;
}
