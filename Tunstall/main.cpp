#include "SourceHeaders\SuffixTree.h"
#include "SourceHeaders\TunstallStd.h"
#include "SourceHeaders\Encode.h"
#include "SourceHeaders\TunstallModified.h"
#include "SourceHeaders\Decode.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Pateikta per mazai argumentu\n\n");
        return 0;
    }

    std::string filename = argv[1];

    bool decode = false;
    UINT codeWordLength = 0;

    // maybe the 2nd argument is the flag '-d'?
    if (strcmp(argv[2], "-d") == 0)
    {
        // check if the provided file has the extension ".tnst"
        std::string delimiter = ".";
        size_t pos = filename.find_last_of(delimiter);

        std::string extension = "ext";

        if(pos != std::string::npos)
        {
            extension = filename.substr(pos + 1);
        }

        if (strcmp(extension.c_str(), "tnst") != 0)
        {
            std::cout<<"Pateiktas failas nera uzkoduotas sios programos...\n\n";
            return 0;
        }

        std::cout<<"Pasirinktas dekodavimo rezimas\n\n";
        decode = true;
    }
    else
    {
        codeWordLength = strtol(argv[2], nullptr, 10);
        if (codeWordLength > 20 || codeWordLength < 1)
        {
        std::cout<<"Ivestas netinkamas kodo zodzio ilgis bitais, bus naudojamas kodo zodzio ilgis k = 8\n\n";
        codeWordLength = 8;
        }
        std::cout<<"Kodo zodzio ilgis: "<<codeWordLength<<"\n\n";
    }

    if (decode)
    {
        if (decodeFile(filename.c_str()))
        {
            std::cout<<"Pateiktas failas dekoduotas\n\n";
        }
        return 0;
    }

    UDICT_CHBUFF_UINT codeDictionary;
    UINT fileSize = 0;
    std::cout<<"Vykdomas standartinis Tunstall algoritmas\n\n";
    UINT maxWordLength = formDictionary(codeWordLength, filename.c_str(), codeDictionary, fileSize);
    if (maxWordLength == 0)
    {
        std::cout<<"Nepavyko suformuoti zodyno\n\n";
        return 0;
    }
    std::string encodedFileName = encodeFile (filename.c_str(), codeDictionary, maxWordLength, codeWordLength, fileSize);

    if (encodedFileName.size() > 1)
    {
        std::cout<<"Pateiktas failas uzkoduotas standartiniu Tunstall algoritmu\n\n";
    }

    std::cout<<"---------------------------\n\n";

    UINT filesize = 0;
    UCHAR * text = readFileIntoBuffer(filename.c_str(), filesize);

    if (text == nullptr)
    {
        std::cout<<"Nepavyko alokuoti atminties failui\n\n";
        return 0;
    }

    SuffixTree * stree = new SuffixTree (text);

    std::cout<<"Formuojamas priesagu medis\n\n";
    for (UINT idx = 0; idx < filesize; ++idx)
    {
        stree->AddSuffix(idx, filesize - idx);
    }

    std::cout<<"Vykdomas priesagos medzio Tunstall algoritmas\n\n";
    UDICT_CHBUFF_UINT suffixDict = createTextDictionary(stree, codeWordLength, maxWordLength);
    encodedFileName = encodeFile(filename.c_str(), suffixDict, maxWordLength, codeWordLength, filesize, true);

    if (encodedFileName.size() > 1)
    {
        std::cout<<"Pateiktas failas uzkoduotas priesagos medzio Tunstall algoritmu\n\n";
    }
    std::cout<<"---------------------------\n\n";

    delete stree;
}
