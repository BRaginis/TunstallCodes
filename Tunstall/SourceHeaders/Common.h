#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <math.h>
#include <cstring>
#include <vector>

#define BYTE_SIZE 8
#define UINT unsigned int
#define UCHAR unsigned char
#define BYTE_COUNT 256

#define BUFFER_CAPACITY 40
#define MAX_BUFFER_CAPACITY 20



#define UDICT_FLOAT_CHBUFF std::unordered_multimap<float, CharBuffer>

#define UDICT_CHBUFF_UINT std::unordered_multimap<CharBuffer, UINT, Hasher, Comparator>

#define UDICT_UINT_CHBUFF std::unordered_multimap<UINT, CharBuffer>

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

// struct for storing bytes as chars, used instead of string because 0 means end of string and that poses problems
typedef struct CharBuffer
{
    UINT buffSize = 0;

    UCHAR buffer[BUFFER_CAPACITY] = {0};

    CharBuffer(UCHAR initByte)
    {
        addByte(initByte);
    }

    CharBuffer()
    {
    }

    void addByte(UCHAR byteToAdd)
    {
        if (buffSize >= BUFFER_CAPACITY)
        {
            return;
        }
        buffer[buffSize] = byteToAdd;
        buffSize++;
    }

    bool operator!= (const CharBuffer b2) const
    {
        if(this->buffSize != b2.buffSize)
        {
            return true;
        }

        for (UINT idx = 0; idx < b2.buffSize; ++idx)
        {
            if (this->buffer[idx] != b2.buffer[idx])
            {
                return true;
            }
        }

        return false;
    }

} CharBuffer;


class Comparator
{
public:

    bool operator()( const CharBuffer& buff1, const CharBuffer& buff2 ) const
    {
        if(buff1.buffSize != buff2.buffSize)
        {
            return false;
        }

        for (UINT idx = 0; idx < buff1.buffSize; ++idx)
        {
            if (buff1.buffer[idx] != buff2.buffer[idx])
            {
                return false;
            }
        }

        return true;
    }

};


class Hasher {
public:
    size_t operator() (CharBuffer key) const {
        size_t hash = 0;
        for(size_t idx = 0; idx < key.buffSize; idx++) {
            hash += key.buffer[idx] % 8521;
        }
        return hash;
    }
};




#endif // COMMON_H_INCLUDED
