#ifndef ENCODE_H_INCLUDED
#define ENCODE_H_INCLUDED


void writeCompressedFileHeader
(
UINT codeWordLength,
UDICT_CHBUFF_UINT dictionary,
UINT maxWordLength,
std::ofstream& writeStream,
UINT filesize
)
{
    std::cout<<"Zodyno dydis: "<<dictionary.size()<<"\n\nMaksimalus zodzio ilgis: "<<maxWordLength<<"\n\n";
    if(!writeStream.is_open())
    {
        return;
    }

    // file header consists of:
    // the length of the codeword
    writeStream << codeWordLength;
    writeStream << " ";

    // dictionary Size
    writeStream << dictionary.size();
    writeStream << " ";

    // the length of the longest word in the dictionary
    writeStream << maxWordLength;
    writeStream << " ";

    // file size in bytes (in case there's a tail which can not be matched with the dictionary)
    writeStream << filesize;
    writeStream << " ";

    // the dictionary entries
    for(auto entry:dictionary)
    {
        // code word
        writeStream << entry.second;
        writeStream << " ";

        // original word's length
        writeStream << entry.first.buffSize;
        writeStream << " ";

        // original word
        for(UINT idx = 0; idx < entry.first.buffSize; ++idx)
        {
            writeStream << std::noskipws <<entry.first.buffer[idx];
        }
        writeStream << " ";
    }

}

void shiftAndWriteTail(CharBuffer buff, UINT emptyBitCount, std::ofstream& writeStream)
{
    for (UINT idx = 0; idx < buff.buffSize - 1; ++idx)
    {
        UCHAR currentByte = buff.buffer[idx];
        UCHAR nextByte = buff.buffer[idx + 1];
        if(idx > 0)
        {
            currentByte <<= emptyBitCount;
        }

        nextByte >>= (BYTE_SIZE - emptyBitCount);
        currentByte |= nextByte;

        writeStream << currentByte;
    }
    UCHAR lastByte = buff.buffer[buff.buffSize - 1];
    if (buff.buffSize > 1)
    {
        lastByte <<= emptyBitCount;
    }

    writeStream << lastByte;
}

void textWordsToCodeWords
(
std::ifstream& readStream,
std::ofstream& writeStream,
UDICT_CHBUFF_UINT codeDictionary,
const UINT codeWordLength
)
{
    // read from the stream and form a string from the read bytes
    // whenever the string matches a dictionary entry, write the entry into the encoded file
    char lastRead = 0;
    CharBuffer byteSequence; //bit sequence

    UDICT_CHBUFF_UINT::iterator found;

    // temp byte for writing
    char byteToWrite = 0;
    UINT totalBitCount = sizeof(char) * BYTE_SIZE;
    UINT emptyBitCount = totalBitCount;


    UINT CodesWritten = 0;
    UINT bytesWritten = 0;

    while(readStream>> std::noskipws >>lastRead)
    {
        byteSequence.addByte(lastRead);

        // if the current string matches an entry, encode it: consume all of the code bits
        if(( found = codeDictionary.find(byteSequence)) !=  codeDictionary.end())
        {
            CodesWritten++;
            UINT code = found->second;

            // the amount of bits that we still need to write for this code word
            UINT unusedBits = codeWordLength;

            // temp code holder for modification
            UINT temp = code;
            // by how many bits was the code shifted in previous iterations?
            UINT prevShift = 0;
            while (unusedBits > 0)
            {
                // set bits: the minimum between the amount of unused bits for this code and the amount of empty bits in byteToWrite
                UINT bitToWriteCount = min(emptyBitCount, unusedBits);

                // write *bitToWriteCount* of the most significant bits of the current code word

                // this leaves *bitToWriteCount* most significant bits pushed to the right: b1b2b3b4b5b6b7b8 ->0000b1b2b3b4
                temp >>= (codeWordLength - bitToWriteCount);

                temp <<= (emptyBitCount - bitToWriteCount);
                // assign these bit values to byteToWrite (we know that at least this amount of leftmost bits is unused)
                byteToWrite |= temp;

                // reduce the amount of unused bits and emptyBitCount
                unusedBits -= bitToWriteCount;
                emptyBitCount -= bitToWriteCount;

                // if we filled the byte, write it
                if(emptyBitCount == 0)
                {
                    writeStream << byteToWrite;

                    bytesWritten++;
                    byteToWrite = 0;
                    emptyBitCount = totalBitCount;
                }
                // if the code is still not entirely used
                if(unusedBits > 0)
                {   // remove the used bits from the original number

                    code <<= (sizeof(UINT)*BYTE_SIZE - unusedBits - prevShift);
                    code >>= (sizeof(UINT)*BYTE_SIZE - codeWordLength);
                    prevShift += codeWordLength - unusedBits;
                    temp = code;
                }

            }
            // reset the string and its length
            memset(byteSequence.buffer, 0, byteSequence.buffSize);
            byteSequence.buffSize = 0;
        }

    }

    // if the last byte was not written, write it, the fact that it's padded with something does not change anything
    // it does! we need to remove the 0s
    // the byte which contains the end of the last code has [8 - emptyBitCount] significant bits and [emptyBitCount] bits that should be filled
    CharBuffer tailBuffer;
    if (emptyBitCount > 0)
    {
        tailBuffer.addByte(byteToWrite);
    }

    // if byte sequence is not empty, we have a tail which did not match any dictionary entries,
    // encode it and let the decoder count bytes
    if (byteSequence.buffSize > 0)
    {

        // iterate through dictionary until we find an entry for which our tail is a prefix
        // then save the entry's code
        UINT code = 0;
        for (auto entry: codeDictionary)
        {
            bool isPrefix = true;
            for (UINT byteIdx =0; byteIdx < byteSequence.buffSize; ++byteIdx)
            {
                if (byteSequence.buffer[byteIdx] != entry.first.buffer[byteIdx])
                {
                    isPrefix = false;
                    break;
                }
            }
            // we found it, save it and break
            if (isPrefix)
            {
                code = entry.second;
                break;
            }
        }

        UINT unusedBits = codeWordLength;
        // keep shifting by byte size while we can
        while (unusedBits >= BYTE_SIZE)
        {
            UINT tempCode = code;
            byteToWrite = 0;
            // pushes most significant unused 8 bits to positions b8....b1
            tempCode >>= (unusedBits - BYTE_SIZE);

            byteToWrite |= tempCode;
            tailBuffer.addByte(byteToWrite);

            unusedBits -= BYTE_SIZE;
        }

        // still have some bits left
        if(unusedBits > 0)
        {// place the unused bits in appropriate location
            byteToWrite = 0;
            code <<= (BYTE_SIZE - unusedBits);
            byteToWrite |= code;
            tailBuffer.addByte(byteToWrite);
        }

    }

    if(tailBuffer.buffSize > 0)
    {
        shiftAndWriteTail(tailBuffer, emptyBitCount, writeStream);
    }

}

std::string encodeFile
(
const char * filename,
UDICT_CHBUFF_UINT codeDictionary,
const UINT maxWordLength,
const UINT codeWordLength,
UINT fileSize,
bool usingSuffix = false
)
{

    // open the specified file
    std::ifstream readStream;
    readStream.open(filename, std::ifstream::binary);
    if (!readStream.is_open())
    {
        return "";
    }
    readStream.seekg(0, std::ios_base::seekdir::_S_beg);


    // create a new file
    char * dotIdx = strchr(filename, '.');

    std::string encodedFileName = std::string(filename, dotIdx - filename);
    if (usingSuffix)
    {
        encodedFileName += "_Suffix_Encoded_";
    }
    else
    {
        encodedFileName += "_Encoded_";
    }

    char conversionBuffer[10] = {0};
    std::string codeWordLengthString = std::string(itoa(codeWordLength, conversionBuffer, 10)) + ".tnst";

    encodedFileName += codeWordLengthString;

    std::ofstream writeStream;
    writeStream.open(encodedFileName, std::ofstream::binary);
    if(!writeStream.is_open())
    {
        readStream.close();
        return "";
    }
    writeStream.seekp (0, std::ios_base::seekdir::_S_beg);

    writeCompressedFileHeader (codeWordLength, codeDictionary, maxWordLength, writeStream, fileSize);

    textWordsToCodeWords (readStream, writeStream, codeDictionary, codeWordLength);

    readStream.close();
    writeStream.close();

    return encodedFileName;
}




#endif // ENCODE_H_INCLUDED
