#include "SourceHeaders\SuffixTree.h"
#include "SourceHeaders\TunstallStd.h"
#include "SourceHeaders\Encode.h"
#include "SourceHeaders\TunstallModified.h"
#include "SourceHeaders\Decode.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("pateikta per mazai argumentu\n");
        return 0;
    }

    std::string filename = argv[1];
    std::cout<<"Failo vardas: "<<filename<<"\n\n";
    UINT codeWordLength = atoi(argv[2]);
    std::cout<<"Kodo ilgis: "<<codeWordLength<<"\n\n";

    if (codeWordLength > 20)
    {
        std::cout<<"Ivestas per didelis skaicius, bus naudojamas kodo ilgis k = 20\n\n";
    }


    std::string flag = "";
    if (argc > 3)
    {
        flag = argv[3];
    }

    bool decode = false;
    if (strcmp(flag.c_str(), "-d") == 0)
    {
        std::cout<<"Uzkoduoti failai bus dekoduojami\n\n";
        decode = true;
    }


    UDICT_CHBUFF_UINT codeDictionary;
    UINT fileSize = 0;
    std::cout<<"\n\nVykdomas standartinis Tunstall algoritmas\n\n";
    UINT maxWordLength = formDictionary(codeWordLength, filename.c_str(), codeDictionary, fileSize);
    if (maxWordLength == 0)
    {
        std::cout<<"Nepavyko suformuoti zodyno...\n";
        return 0;
    }
    std::string encodedFileName = encodeFile (filename.c_str(), codeDictionary, maxWordLength, codeWordLength, fileSize);

    if (encodedFileName.size() > 1)
    {
        std::cout<<"Pateiktas failas uzkoduotas standartiniu Tunstall algoritmu\n\n";

        if (decode)
        {
            decodeFile(encodedFileName.c_str());
            std::cout<<"Uzkoduotas failas dekoduotas\n\n";
        }
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

    for (UINT idx = 0; idx < filesize; ++idx)
    {
        stree->AddSuffix(idx, filesize - idx);
    }

    std::cout<<"\n\nVykdomas priesagos medzio Tunstall algoritmas\n\n";
    UDICT_CHBUFF_UINT suffixDict = createTextDictionary(stree, codeWordLength, maxWordLength);
    encodedFileName = encodeFile(filename.c_str(), suffixDict, maxWordLength, codeWordLength, filesize, true);

    if (encodedFileName.size() > 1)
    {
        std::cout<<"Pateiktas failas uzkoduotas priesagos medzio Tunstall algoritmu\n\n";

        if (decode)
        {
            decodeFile(encodedFileName.c_str());
            std::cout<<"Uzkoduotas failas dekoduotas\n\n";
        }
    }
    std::cout<<"---------------------------\n\n";

    delete stree;
}
