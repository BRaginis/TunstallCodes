#ifndef TUNSTALLMODIFIED_H_INCLUDED
#define TUNSTALLMODIFIED_H_INCLUDED

#include "Common.h"

UCHAR * readFileIntoBuffer(const char * filename, UINT &filesize)
{
    std::ifstream readStream;
    readStream.open(filename, std::ifstream::binary);
    if (!readStream.is_open())
    {
        return nullptr;
    }

    // get length of file:
    readStream.seekg (0, readStream.end);
    int length = readStream.tellg();
    if (length < 0)
    {
        readStream.close();
        return nullptr;
    }
    filesize = length;

    UCHAR * buffer = (UCHAR *) calloc (filesize + 1, sizeof(UCHAR));
    if (buffer == nullptr)
    {
        readStream.close();
        return nullptr;
    }

    UCHAR byteRead = 0;
    UINT byteIdx = 0;
    readStream.seekg(0, std::ios_base::seekdir::_S_beg);
    while (readStream >> std::noskipws >>byteRead)
    {
        buffer[byteIdx] = byteRead;
        byteIdx++;
    }

    return buffer;

}

Node* addMostFrequentNodeChildren(std::vector<Node*>& vocabularyNodes)
{
    UINT maxFrequency = 0;
    auto mostFrequentEntry = vocabularyNodes.begin();
    auto iter = vocabularyNodes.begin();

    // find most frequent entry
    while (iter != vocabularyNodes.end())
    {
        if ((*iter)->getLeafCount() > maxFrequency)
        {
            maxFrequency = (*iter)->getLeafCount();
            mostFrequentEntry = iter;
        }
        iter++;
    }

    // this means the dictionary nodes are all leaves and we need to stop
    if (maxFrequency == 0)
    {
        return nullptr;
    }

    // remove the entry and add all of its children
    auto children = (*mostFrequentEntry)->getChildren();
    Node * lastRemoved = *mostFrequentEntry;
    vocabularyNodes.erase(mostFrequentEntry);
    for (Node* child:children)
    {
        vocabularyNodes.push_back(child);
    }

    return lastRemoved;
}

UDICT_CHBUFF_UINT createTextDictionary
(
SuffixTree* stree,
UINT& codeWordLength,
UINT& maxWordLength
)
{
    // populate set of nodes
    // initially the nodes are the 1st level children (children of root)
    std::vector<Node*> vocabularyNodes = stree->getRoot()->getChildren();
    UINT vocabularySize = vocabularyNodes.size();
    UINT sizeLimit = pow(2, codeWordLength);

    while (sizeLimit < vocabularySize)
    {
        sizeLimit *= 2;
        codeWordLength++;
    }

    Node* lastRemoved = stree->getRoot();
    while (vocabularyNodes.size() < sizeLimit && lastRemoved != nullptr)
    {
        lastRemoved = addMostFrequentNodeChildren(vocabularyNodes);
    }

    if (vocabularyNodes.size() > sizeLimit)
    { // undo last removal:

        // remove the last *child count* entries from the vocabulary
        UINT childCount = lastRemoved->getChildren().size();
        for (UINT idx = 0; idx < childCount; ++idx)
        {
            vocabularyNodes.pop_back();
        }

        // add the last removed node
        vocabularyNodes.push_back(lastRemoved);

    }

    // form textWords corresponding to these nodes and add them to dictionary
    UDICT_CHBUFF_UINT codeDictionary;
    UINT entryCount = 0;
    std::vector<CharBuffer> bufferVector;
    maxWordLength = 0;
    for(Node* node:vocabularyNodes)
    {
        UINT length = 0;
        codeDictionary.emplace(stree->getPathWord(node, length), entryCount);
        if (length > maxWordLength)
        {
            maxWordLength = length;
        }
        entryCount++;
    }

    return codeDictionary;
}

#endif // TUNSTALLMODIFIED_H_INCLUDED
