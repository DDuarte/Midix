#include "bytebuffer.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void ByteBuffer_ReadSkip(ByteBuffer* bb, uint32_t size)
{
    assert(bb->_readPos + size <= bb->_size);
    bb->_readPos += size;
}

#define Read(type) type val; assert(bb->_readPos + sizeof(type) <= bb->_size); val = *((type*)&bb->_buffer[bb->_readPos]); bb->_readPos += sizeof(type); return val;

uint64_t ByteBuffer_ReadUInt64(ByteBuffer* bb) { Read(uint64_t); }
uint32_t ByteBuffer_ReadUInt32(ByteBuffer* bb) { Read(uint32_t); }
uint16_t ByteBuffer_ReadUInt16(ByteBuffer* bb) { Read(uint16_t); }
uint8_t ByteBuffer_ReadUInt8(ByteBuffer* bb) { Read(uint8_t); }

int64_t ByteBuffer_ReadInt64(ByteBuffer* bb) { Read(int64_t); }
int32_t ByteBuffer_ReadInt32(ByteBuffer* bb) { Read(int32_t); }
int16_t ByteBuffer_ReadInt16(ByteBuffer* bb) { Read(int16_t); }
int8_t ByteBuffer_ReadInt8(ByteBuffer* bb) { Read(int8_t); }


uint32_t ByteBuffer_ReadVLFUInt32( ByteBuffer* bb )
{
    uint32_t value;
    uint8_t c;

    if ((value = ByteBuffer_ReadUInt8(bb)) & 0x80)
    {
        value &= 0x7f;
        do
        {
            value = (value <<7) + ((c = ByteBuffer_ReadUInt8(bb)) & 0x7f);
        } while (c & 0x80);
    }
    return (value);

    /*
    uint32_t result, i, shift;
    uint8_t val, realVal;

    result = 0;

    for (i = 0, shift = 0; i < 4; ++i, shift += 7) /* read max 4 bytes (max value 0xFFFFFFF) *
    {
        val = ByteBuffer_ReadUInt8(bb);
        realVal = val & 0x7F;
        result |= realVal << shift;
        if (!(val & 0x80)) /* if highbit is 1, continue reading otherwise stop*
            break;
    }

    return result;
    */
}

char* ByteBuffer_ReadCString(ByteBuffer* bb)
{
    uint32_t oldReadPos;
    uint32_t size;
    char* str;

    oldReadPos = bb->_readPos;
    size = 0;

    while (ByteBuffer_ReadUInt8(bb) != 0) { size++; } /* increases _readPos */

    str = (char*)malloc(size + 1);

    memcpy(str, &bb->_buffer[oldReadPos], size);

    str[size] = 0; /* null terminator */

    return str;
}

char* ByteBuffer_ReadString(ByteBuffer* bb, uint32_t size)
{
    char* str;

    assert(bb->_readPos + size <= bb->_size);

    str = (char*)malloc(size + 1);

    memcpy(str, &bb->_buffer[bb->_readPos], size);

    str[size] = 0; /* null terminator */

    bb->_readPos += size;

    return str;
}

uint8_t ByteBuffer_CanRead(ByteBuffer* bb)
{
    return bb->_readPos >= bb->_size - 1;
}
