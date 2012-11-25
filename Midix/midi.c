#include "midi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

int Load(const char* fileName, char** buffer, int* size);

//! Byte swap unsigned short
uint16_t swap_uint16( uint16_t val ) 
{
    return (val << 8) | (val >> 8 );
}

//! Byte swap short
int16_t swap_int16( int16_t val ) 
{
    return (val << 8) | ((val >> 8) & 0xFF);
}

//! Byte swap unsigned int
uint32_t swap_uint32( uint32_t val )
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | (val >> 16);
}

//! Byte swap int
int32_t swap_int32( int32_t val )
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | ((val >> 16) & 0xFFFF);
}

int MIDI_ReadFile(MIDI* midi, const char* fileName)
{
    char* buffer;
    int size;

    if (Load(fileName, &buffer, &size) != 0)
    {
        printf("MIDI_ReadFile: Could not load file %s\n", fileName);
        return 1;
    }

    midi->bb._readPos = 0;
    midi->bb._buffer = buffer;
    midi->bb._size = size;

    return 0;
}

int MIDI_ReadHeaderChunk(MIDI* midi)
{
    char* chunkType;
    uint32_t length;
    uint16_t fileType;
    uint16_t trackCount;
    uint16_t division;

    chunkType = ByteBuffer_ReadString(&midi->bb, 4);

    if (strcmp(chunkType, "MThd") != 0)
    {
        printf("MIDI_ReadHeaderChunk: This is not a header chunk, chunk type is %s\n", chunkType);
        free(chunkType);
        return 1;
    }

    free(chunkType);

    length = swap_uint32(ByteBuffer_ReadUInt32(&midi->bb));

    if (length != 6)
    {
        printf("MIDI_ReadHeaderChunk: This is not a standard header chunk, length was %u instead of 6\n", length);
        return 1;
    }

    fileType = swap_int16(ByteBuffer_ReadUInt16(&midi->bb));

    if (fileType != MIDI_FILE_TYPE_SINGLE_TRACK && fileType != MIDI_FILE_TYPE_MULTI_TRACK_SYNCH && fileType != MIDI_FILE_TYPE_MULTI_TRACK_ASYNCH)
    {
        printf("MIDI_ReadHeaderChunk: This is not a standard header chunk, file type was %u instead of [0,2]\n", fileType);
        return 1;
    }

    midi->FileType = fileType;

    trackCount = swap_int16(ByteBuffer_ReadUInt16(&midi->bb));

    if (trackCount < 0 || trackCount > 0xFFFF)
    {
        printf("MIDI_ReadHeaderChunk: This is not a standard header chunk, number of tracks was %u instead of [0,0xFFFF]\n", trackCount);
        return 1;
    }

    midi->TrackCount = trackCount;

    division = swap_int16(ByteBuffer_ReadUInt16(&midi->bb));

    if (division < 0 || division > 0xFFFF)
    {
        printf("MIDI_ReadHeaderChunk: This is not a standard header chunk, number of tracks was %u instead of [0,0xFFFF]\n", division);
        return 1;
    }

    midi->Division = division;

    return 0;
}

int MIDI_ReadTrackChunk(MIDI* midi, int id)
{
    char* chunkType;
    uint32_t length, length2, pos, delta;
    uint8_t value, oldValue, leftNybble, rightNybble, eventType, data1, data2;
    MIDIEvent event;

    chunkType = ByteBuffer_ReadString(&midi->bb, 4);

    if (strcmp(chunkType, "MTrk") != 0)
    {
        printf("MIDI_ReadTrackChunk (%i): This is not a track chunk, chunk type is %s\n", id, chunkType);
        free(chunkType);
        return 1;
    }

    free(chunkType);

    length = swap_uint32(ByteBuffer_ReadUInt32(&midi->bb));

    pos = midi->bb._readPos;

    while (midi->bb._readPos - pos < length)
    {
        delta = ByteBuffer_ReadVLFUInt32(&midi->bb);

        value = ByteBuffer_ReadUInt8(&midi->bb);

        leftNybble  = value >> 4;
        rightNybble = value & 0xF;

        if (!(leftNybble & 0x8)) /* running status (rs) on */
        {
            midi->bb._readPos -= sizeof(uint8_t); /* roll back 1 byte */

            value = oldValue;
            leftNybble = value >> 4;
            rightNybble = value & 0xF;

            goto eventLabel; /* rs is only enabled for midi events */
        }

        if (value == 0xFF) /* meta-event */
        {
            eventType = ByteBuffer_ReadUInt8(&midi->bb);
            length2 = ByteBuffer_ReadVLFUInt32(&midi->bb);

            ByteBuffer_ReadSkip(&midi->bb, length2);

            /*
            str = ByteBuffer_ReadString(&midi->bb, length2);
            printf("meta event: %u, str: %s\n", eventType, str);

            free(str);
            */
        }
        else if (value == 0xF0 || value == 0xF7) /* sysex event */
        {
            length2 = ByteBuffer_ReadVLFUInt32(&midi->bb);

            ByteBuffer_ReadSkip(&midi->bb, length2);

            /*
            str = ByteBuffer_ReadString(&midi->bb, length2);
            printf("sysex event: %u, str: %s\n", value, str);

            free(str);
            */
        }
        else /* event */
        {
eventLabel:
            if (leftNybble != 0xC && leftNybble != 0xD)
            {
                data1 = ByteBuffer_ReadUInt8(&midi->bb);
                data2 = ByteBuffer_ReadUInt8(&midi->bb);
            }
            else
            {
                data1 = ByteBuffer_ReadUInt8(&midi->bb);
                data2 = 0;
            }

            event.Delta = delta;
            event.Command = leftNybble;
            event.Channel = rightNybble;
            event.Generic.Data1 = data1;
            event.Generic.Data2 = data2;

            MIDI_AddEvent(midi, &event, id);
        }

        oldValue = value;
    }

    return 0;
}

int Load(const char* fileName, char** buffer, int* size)
{
    FILE* file;
    size_t result;

    if (!fileName)
        return 1;

    file = fopen(fileName, "rb");
    if (!file)
        return 1;

    // Get file size
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    rewind(file);

    if (!*size)
        return 1;

    *buffer = (char*)malloc(*size * sizeof(char));
    if (!*buffer)
        return 1;

    result = fread(*buffer, sizeof(char), *size, file);
    if (result != *size)
        return 1;

    fclose(file);
    return 0;
}

static double midi[128];

void MIDI_NoteToFrequencyInit()
{
    int i;
    double a;

    a = 440.0;
    for (i = 0; i < 127; ++i)
        midi[i] = (a / 32.0) * pow(2.0, (i - 9.0) / 12.0);
}

double MIDI_NoteToFrequency(int note)
{
    return midi[note];
}

void MIDI_AddEvent(MIDI* midi, MIDIEvent* event, int id)
{
    if (midi->_numElements[id] == midi->_numAllocated[id])
    {
        if (midi->_numAllocated[id] == 0)
            midi->_numAllocated[id] = 10;
        else
            midi->_numAllocated[id] *= 2;

        midi->Events[id] = (MIDIEvent*)realloc(midi->Events[id], (midi->_numAllocated[id] * sizeof(MIDIEvent)));
    }

    midi->Events[id][midi->_numElements[id]] = *event;
    midi->_numElements[id]++;
}
