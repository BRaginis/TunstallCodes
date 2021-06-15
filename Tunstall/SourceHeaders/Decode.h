#ifndef DECODE_H_INCLUDED
#define DECODE_H_INCLUDED

void readCompressedFileHeader
(
UINT& codeWordLength,
UDICT_UINT_CHBUFF& dictionary,
std::ifstream& readStream,
UINT& filesize
)
{
    if(!readStream.is_open())
    {
        return;
    }

    // file header consists of:
    // the length of the codeword
    readStream >> codeWordLength;

    // dictionary Size
    UINT dictSize = 0;

    readStream >> dictSize;

    UINT maxWordLength = 0;
    // the length of the longest word in the dictionary
    readStream >> maxWordLength;

    // filesize in bytes
    readStream >> filesize;

    CharBuffer byteBuffer;

    // the dictionary entries
    for(UINT idx = 0; idx < dictSize; ++idx)
    {
        // code word
        UINT codeWord = 0;

        readStream >> std::skipws >> codeWord;

        // original word length
        UINT originalWordLength = 0;

        readStream >> std::skipws >> originalWordLength;

        // original word
        // skip space before
        UCHAR readByte = 0;
        readStream >> std::noskipws >> readByte;
        for (UINT charIdx = 0; charIdx < originalWordLength; ++charIdx)
        {
            readStream >> std::noskipws >> readByte;
            byteBuffer.addByte(readByte);
        }

        // write entry to the dictionary
        dictionary.emplace(codeWord, byteBuffer);

        //clear the codeword
        memset (byteBuffer.buffer, 0, maxWordLength);
        byteBuffer.buffSize = 0;

    }
}

void codeWordsToTextWords
(
std::ifstream& readStream,
std::ofstream& writeStream,
UDICT_UINT_CHBUFF codeDictionary,
const UINT codeWordLength,
UINT filesize
)
{
    // read from the stream and form a sequence of bits from the read bytes
    // when the sequence is of size *codeWordLength*, write the corresponding dictionary entry to the file
    UCHAR lastRead = 0;
    UINT codeBitsFilled = 0;
    UINT tempStorage = 0;
    UCHAR * byteSequence = (UCHAR *) calloc(codeWordLength + 1, sizeof(UCHAR));

    // temp UINT for writing
    UINT codeToWrite = 0;
    UINT charLength = sizeof(UCHAR) * BYTE_SIZE;
    UINT bytesRead = 0;

    // read one byte for the extra space at the end
    readStream>> std::noskipws >>lastRead;

    UINT codesRead = 0;
    UINT bytesWritten = 0;
    while(readStream>> std::noskipws >>lastRead)
    {
        bytesRead++;
        // we start with all of the byte bits unused
        UINT unusedBitCount = charLength;
        while (unusedBitCount > 0)
        {
            // how many bits do we need for the codeword?
            UINT bitsNeeded = codeWordLength - codeBitsFilled;

            // we will be writing the smaller of these two
            UINT bitToWriteCount = min(bitsNeeded, unusedBitCount);

            // we need *bitsToWrite* bits from the byte:

            // 1. assign all the bits to temp storage
            tempStorage = lastRead;

            // 2. leave only bitToWriteCount most significant bits (if the byte has less unused bits than the code needs, they are placed on the left)
            tempStorage >>= (charLength - bitToWriteCount);

            // 3. place the needed bits in appropriate position
            tempStorage <<= (codeWordLength - codeBitsFilled - bitToWriteCount);

            // update the code
            codeToWrite |= tempStorage;

            // update unusedBitCount and codeBitsFilled
            unusedBitCount -= bitToWriteCount;
            codeBitsFilled += bitToWriteCount;

            // only leave the unused bits in the readByte
            lastRead <<= bitToWriteCount;

            if (codeBitsFilled == codeWordLength)
            {
                codesRead++;
                UDICT_UINT_CHBUFF::iterator found = codeDictionary.find(codeToWrite);
                if (found == codeDictionary.end())
                {
                    std::cout<<"ERROR: code word not found in the dictionary\n\n";
                    free(byteSequence);
                    return;
                }

                // we have a codeword, write the corresponding dictionary entry and refresh codeBitsFilled
                // we only write upto filesize bytes because if there might be a "tail"
                for (UINT idx = 0; idx < found->second.buffSize && filesize > bytesWritten; ++idx)
                {
                    writeStream << std::noskipws <<found->second.buffer[idx];
                    bytesWritten++;
                }
                codeBitsFilled = 0;
                codeToWrite = 0;
            }

        }
    }
    free(byteSequence);
}

bool decodeFile
(
const char * filename
)
{
    UDICT_UINT_CHBUFF readDictionary;
    // open the specified file
    std::ifstream readStream;
    readStream.open(filename, std::ifstream::binary);
    if (!readStream.is_open())
    {
        std::cout<<"Nepavyko atidaryti failo dekodavimui\n\n";
        return false;
    }
    readStream.seekg(0, std::ios_base::seekdir::_S_beg);

    // read the header
    UINT readCodeWordLength = 0;
    UINT fileSize = 0;
    readCompressedFileHeader (readCodeWordLength, readDictionary, readStream, fileSize);


    // create a new file
    char * dotIdx = strchr(filename, '.');

    std::string decodedFileName = std::string(filename, dotIdx - filename);
    decodedFileName += "_Decoded";
    std::ofstream writeStream;
    writeStream.open(decodedFileName, std::ofstream::binary);
    if(!writeStream.is_open())
    {
        readStream.close();
        std::cout<<"Nepavyko atidaryti failo irasymui\n\n";
        return false;
    }

    writeStream.seekp (0, std::ios_base::seekdir::_S_beg);

    codeWordsToTextWords (readStream, writeStream, readDictionary, readCodeWordLength, fileSize);

    readStream.close();
    writeStream.close();

    return true;
}


#endif // DECODE_H_INCLUDED
