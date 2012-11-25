#ifndef BYTEBUFFER_H_
#define BYTEBUFFER_H_

#include <stdint.h>

typedef struct 
{
    char* _buffer;
    uint32_t _size;
    uint32_t _readPos;
    /* uint32_t _writePos; */
} ByteBuffer;

void ByteBuffer_ReadSkip(ByteBuffer* bb, uint32_t size);
uint64_t ByteBuffer_ReadUInt64(ByteBuffer* bb);
uint32_t ByteBuffer_ReadUInt32(ByteBuffer* bb);
uint32_t ByteBuffer_ReadVLFUInt32(ByteBuffer* bb); /* Read7BitEncodedInt */
uint16_t ByteBuffer_ReadUInt16(ByteBuffer* bb);
uint8_t  ByteBuffer_ReadUInt8(ByteBuffer* bb);

int64_t ByteBuffer_ReadInt64(ByteBuffer* bb);
int32_t ByteBuffer_ReadInt32(ByteBuffer* bb);
int16_t ByteBuffer_ReadInt16(ByteBuffer* bb);
int8_t  ByteBuffer_ReadInt8(ByteBuffer* bb);

char* ByteBuffer_ReadCString(ByteBuffer* bb);
char* ByteBuffer_ReadString(ByteBuffer* bb, uint32_t size);

uint8_t ByteBuffer_CanRead(ByteBuffer* bb);


#endif // BYTEBUFFER_H_
