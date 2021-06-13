#ifndef SUFFIXTREE_H_INCLUDED
#define SUFFIXTREE_H_INCLUDED

#include "Common.h"

class Node
{

public:
    Node
    (
     Node* prnt = nullptr,
     UCHAR *startp = nullptr,
     UCHAR * endp = nullptr,
     UINT lCount = 0,
     UCHAR * suffStart = 0
     )
    {
        parent = prnt;
        startPtr = startp;
        endPtr = endp;
        leafCount = lCount;
        suffixStart = suffStart;
    }

    void AddChild(Node* node)
    {
        children.push_back(node);
    }

    void increaseLeafCount()
    {
        leafCount++;
    }

    Node * getParent()
    {
        return parent;
    }

    void clearChildren()
    {
        children.clear();
    }

    std::vector<Node*> getChildren()
    {
        return children;
    }

    void setChildren(const std::vector<Node*>& vec)
    {
        children = vec;
    }

    UINT getLeafCount()
    {
        return leafCount;
    }

    UCHAR * getStart()
    {
        return startPtr;
    }

    UCHAR * getEnd()
    {
        return endPtr;
    }

    UINT getLength()
    {
        if (startPtr == nullptr)
        {
            return 0;
        }
        return endPtr - startPtr + 1;
    }

    void setEnd(UCHAR * newEnd)
    {
        endPtr = newEnd;
    }

    UCHAR* getSuffixStart()
    {
        return suffixStart;
    }

private:

    Node* parent;
    std::vector<Node*> children;
    // pointers to places in original text
    UCHAR * startPtr;
    UCHAR * endPtr;
    UINT leafCount;
    UCHAR* suffixStart;

};


class SuffixTree
{
    public:

    SuffixTree (UCHAR * text)
    {
        textBuffer = text;
        root = new Node();
    }

    ~SuffixTree()
    {
        for (Node* node: nodes)
        {
            delete node;
        }
        delete root;
        free(textBuffer);
    }

    Node* getRoot()
    {
        return root;
    }

    void AddSuffix(UINT offset, UINT length)
    {
        if (textBuffer == nullptr || length == 0)
        {
            std::cout<<"early return\n\n";
            return;
        }

        AddSuffix (&textBuffer[offset], length, root, 0);
    }

    // gets the word associated with the path from root to endNode, but no longer than maxWordLength
    CharBuffer getPathWord
    (
     Node * endNode,
     UINT& wordLength,
     UINT maxWordLength = MAX_BUFFER_CAPACITY
    )
    {
        CharBuffer buffer;
        UCHAR * startPtr = endNode->getSuffixStart();
        if (startPtr == nullptr)
        {
            std::cout<<"Start is null\n\n";
            return buffer;
        }
        UCHAR * endPtr = endNode->getEnd();
        // if it is an empty leaf, use parent's end and start
        if (endPtr == nullptr)
        {
            startPtr = endNode->getParent()->getSuffixStart();
            endPtr = endNode->getParent()->getEnd();
        }
        wordLength = min (endPtr - startPtr + 1, maxWordLength);

        for (UINT idx = 0; idx < wordLength; ++idx)
        {
            buffer.addByte(startPtr[idx]);
        }

        return buffer;
    }

    private:

    void AddSuffix
    (
     UCHAR * suffixStart,
     UINT length,
     Node* currentNode,
     UINT suffixIdx
    )
    {
        // if we are here, it means that at least the 1st symbols match or we are at root
        UINT currentNodeIdx = 0;
        UCHAR * nodeStart = currentNode->getStart();

        UINT currentNodeTextLength = currentNode->getLength();

        // compare until we reach the end of current node text, the suffix end or find a mismatch
        while ((currentNodeIdx < currentNodeTextLength) &&
               (suffixIdx < length) &&
               (nodeStart[currentNodeIdx] == suffixStart[suffixIdx]))
               {
                   currentNodeIdx++;
                   suffixIdx++;
               }

        auto childVector = currentNode->getChildren();
        Node * currentParent = currentNode->getParent();

        // case 1: total match but suffix is not consumed:
        if (currentParent == nullptr || (currentNodeIdx == currentNodeTextLength && suffixIdx < length))
        {
            Node * match = nullptr;
            // see if there is a matching child
            for (auto child:childVector)
            {
                if (child->getStart() != nullptr && child->getStart()[0] == suffixStart[suffixIdx])
                {
                    match = child;
                    break;
                }
            }

            // if so, traverse into it
            if (match != nullptr)
            {
                AddSuffix(suffixStart, length, match, suffixIdx);
                currentNode->increaseLeafCount();
            }
            else
            { // else, add a new leaf
                Node * newNode = new Node(currentNode, suffixStart + suffixIdx, suffixStart + length - 1, 0, suffixStart);
                nodes.push_back(newNode);
                currentNode->increaseLeafCount();
                currentNode->AddChild(newNode);

                // if we are adding a leaf to a node which was previously a leaf but is not the root, we need to add another leaf to it, marking the previously added suffix end
                if (currentNode -> getLeafCount() == 1 && currentNode->getParent() != nullptr)
                {
                    Node * additionalLeaf = new Node(currentNode, nullptr, nullptr, 0, suffixStart);
                    nodes.push_back(additionalLeaf);
                    currentNode->increaseLeafCount();
                    currentNode->AddChild(additionalLeaf);
                }

            }

        } // case 2: the suffix is totally matched and consumed, add an empty leafnode to mark the end off a suffix
        else if (currentNodeIdx == currentNodeTextLength && suffixIdx == length)
        {
            Node * newNode = new Node(currentNode, nullptr, nullptr, 0, suffixStart);
            nodes.push_back(newNode);
            currentNode->increaseLeafCount();
            currentNode->AddChild(newNode);

        } // case 3: the suffix is matched but is shorter or the suffix differs at some point
        else if (currentNodeIdx < currentNodeTextLength)
        {
            // we need to branch this edge by updating the current node and adding 2 children
            // one of which will hold all of the current node's children
            UCHAR * newEnd = currentNode->getStart() + currentNodeIdx - 1;
            UCHAR * newNodeStart = currentNode->getStart() + currentNodeIdx;
            UCHAR * newNodeEnd = currentNode->getEnd();
            currentNode->setEnd(newEnd);

            // create a new node which will have all of the current node's children and will be one of the 2 children of the current node
            Node* newNode = new Node (currentNode, newNodeStart, newNodeEnd, currentNode->getLeafCount(), currentNode->getSuffixStart());
            newNode->setChildren(currentNode->getChildren());

            UCHAR * newLeafNodeStart = nullptr;
            UCHAR * newLeafNodeEnd = nullptr;

            // if there are still suffix text left, create a new node with the text
            if (suffixIdx < length)
            {
                newLeafNodeStart = suffixStart + suffixIdx;
                newLeafNodeEnd = suffixStart + length - 1;
            }

            Node *newLeafNode = new Node (currentNode, newLeafNodeStart, newLeafNodeEnd, 0, suffixStart);

            currentNode->clearChildren();
            currentNode->AddChild(newNode);
            currentNode->AddChild(newLeafNode);
            nodes.push_back(newNode);
            nodes.push_back(newLeafNode);
            // we added a leaf
            currentNode->increaseLeafCount();

            // this means that until now this node had no children, so we have to add 2 to its leafcount instead of 1
            if (newNode->getLeafCount() == 0)
            {
                currentNode->increaseLeafCount();
            }

        }

    }

    Node * root;
    std::vector<Node*> nodes;
    UCHAR * textBuffer;

};


#endif // SUFFIXTREE_H_INCLUDED
