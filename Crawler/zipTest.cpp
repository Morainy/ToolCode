#include <cstring>

#include <cstdlib>

#include <iostream>

#include "zlib.h"

using namespace std;

 

int main()

{

    int err;

    Byte compr[200], uncompr[200];    // big enough

    uLong comprLen, uncomprLen;

    const char* hello = "12345678901234567890123456789012345678901234567890";

 

    uLong len = strlen(hello) + 1;

    comprLen  = sizeof(compr) / sizeof(compr[0]);

 

    err = compress(compr, &comprLen, (const Bytef*)hello, len);

 

    if (err != Z_OK) {

        cerr << "compess error: " << err << '\n';

        exit(1);

    }

    cout << "orignal size: " << len

         << " , compressed size : " << comprLen << '\n';

 

    strcpy((char*)uncompr, "garbage");

 

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);

 

    if (err != Z_OK) {

        cerr << "uncompess error: " << err << '\n';

        exit(1);

    }

    cout << "orignal size: " << len

         << " , uncompressed size : " << uncomprLen << '\n';

 

    if (strcmp((char*)uncompr, hello)) {

        cerr << "BAD uncompress!!!\n";

        exit(1);

    } else {

        cout << "uncompress() succeed: \n" << (char *)uncompr;

    }

}