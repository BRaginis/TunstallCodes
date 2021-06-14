#ifndef TUNSTALLSTD_H_INCLUDED
#define TUNSTALLSTD_H_INCLUDED

#include "Common.h"

/**---------------------------------------------------------------------------------
    Gets frequencies of bytes in a file
    by iterating over the file and incrementing the count of current byte.

    PARAMETERS:

    frequencies is assumed to be an array of size 256 for every possible byte value.

    readerStream should be opened.

    RETURNS file size in bytes
------------------------------------------------------------------------------------*/
UINT getByteFrequencies(float* frequencies, std::ifstream& readStream)
{
    if (frequencies == nullptr || !readStream.is_open())
    {
        return 0;
    }

    // set stream position to beginning
    readStream.seekg(0, std::ios_base::seekdir::_S_beg);
    // iterate through file and count byte occurrences
    UCHAR tempChar = 0;
    float textLength = 0;
    while(readStream >> std::noskipws >> tempChar)
    {
        frequencies[tempChar]++;
        textLength++;
    }


    for (UINT idx = 0; idx < BYTE_COUNT; ++ idx)
    {
        frequencies[idx] /= textLength;
    }

    return (UINT) textLength;
}

float findLargestFrequency(UDICT_FLOAT_CHBUFF& dictionary)
{
    float maxFreq = 0;
    for(auto entry:dictionary)
    {
        if(entry.first > maxFreq)
        {
            maxFreq = entry.first;
        }
    }
    return maxFreq;
}


/**---------------------------------------------------------------------------------
CREATE DEFINITION, THIS SEEMS OK

    PARAMETERS:


------------------------------------------------------------------------------------*/
UDICT_FLOAT_CHBUFF getMostProbableByteSequences
(
float * frequencies,
UINT& codeWordLength,
UINT& maxWordLength
)
{
    // define dictionary as an unordered_multimap because multiple entries with the same key are likely
    UDICT_FLOAT_CHBUFF dict;


    // the dictionary is the set of possible bytes
    // so initialize the dictionary to initially be that
    // omit the bytes that were not encountered
    // and save the actual byteCount
    UINT actualByteCount = 0;
    for (UINT idx = 0; idx < BYTE_COUNT; ++idx)
    {
        if (frequencies[idx] == 0)
        {
            continue;
        }
        actualByteCount++;

        dict.emplace(frequencies[idx], (UCHAR)idx);
    }

    maxWordLength = 1;

    std::cout<<"Skirtingu baitu skaicius faile: "<<actualByteCount<<"\n\n";

    UINT sizeLimit = pow(2, codeWordLength);

    while (sizeLimit < actualByteCount)
    {
        codeWordLength++;
        sizeLimit *= 2;
    }

    std::cout<<"Realus kodo ilgis bitais :"<<codeWordLength<<"\n\n";

    sizeLimit -= (actualByteCount - 1);

    while(dict.size() <= sizeLimit)
    {
        // replace with findLargestElement
        //auto mostFrequentEntry = dictionary.find(findLargestFrequency(dictionary));
        auto mostFrequentEntry = dict.find(findLargestFrequency(dict));
        CharBuffer mostFrequentWord = mostFrequentEntry->second;

        // add 1 to string's length to easily modify the symbols which will be added
        mostFrequentWord.addByte('0');

        float probability = mostFrequentEntry->first;
        for (UINT idx = 0; idx < BYTE_COUNT; ++idx)
        {
            // omit non - existing bytes
            if (frequencies[idx] == 0)
            {
                continue;
            }
            float newProbability = probability * frequencies[idx];
            mostFrequentWord.buffer[mostFrequentWord.buffSize - 1] = (UCHAR)idx;

            dict.emplace(newProbability, mostFrequentWord);
        }
        if(mostFrequentWord.buffSize > maxWordLength)
        {
            maxWordLength = mostFrequentWord.buffSize;
        }
        // remove the old most popular entry
        dict.erase(mostFrequentEntry);
    }

    return dict;
}

UDICT_CHBUFF_UINT createCodeDictionary(UDICT_FLOAT_CHBUFF codeWords, const UINT codeWordSize)
{
    UDICT_CHBUFF_UINT codeDict;

    UINT codeWordCount = 0;
    // assign the code words a number from 0 to dictionary size
    for (auto entry:codeWords)
    {
        codeDict.emplace(entry.second, codeWordCount);

        codeWordCount++;
    }

    return codeDict;
}

/**---------------------------------------------------------------------------------
    Forms dictionary for a given file.

    First gets the occurrence count of each byte.

    Then uses the byte occurrence array to choose the most probable byte sequences.

    PARAMETERS:

    codeWordSize is the size of code word in bits.

    filename is the name of the file to be encoded.

    RETURNS:

    maxWordLength on success.

    0 on failure.

------------------------------------------------------------------------------------*/
UINT formDictionary
(
UINT& codeWordLength,
const char * filename,
UDICT_CHBUFF_UINT& codeDictionary,
UINT& fileSize
)
{
    const UCHAR maxValue = (UCHAR) -1;
    float byteFrequencies[maxValue] = {0};
    std::ifstream readerStream;
    readerStream.open(filename, std::ifstream::binary);
    if (!readerStream.is_open())
    {
        std::cout<<"Nepavyko atidaryti failo, programa baigia darba\n\n";
        return 0;
    }

    // get byte frequencies
    fileSize = getByteFrequencies (byteFrequencies, readerStream);
    readerStream.close();

    UINT maxWordLength = 0;
    // form the set of most probable byte sequences
    UDICT_FLOAT_CHBUFF mostProbableWords = getMostProbableByteSequences(byteFrequencies, codeWordLength, maxWordLength);

    // assign these sequences code values from 0 to the the number of dictionary entries
    codeDictionary = createCodeDictionary(mostProbableWords, codeWordLength);

    return maxWordLength;

}

#endif // TUNSTALLSTD_H_INCLUDED
