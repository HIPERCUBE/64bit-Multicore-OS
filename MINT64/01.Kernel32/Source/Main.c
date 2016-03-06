#include "Types.h"

void kPrintString(int iX, int iY, const char *pcString);

/**
 * Main 함수
 */
void Main() {
    kPrintString(0, 3, "C Language Kernel Started~!!");

    while (1);
}

/**
 * 문자열 출력 함수
 */
void kPrintString(int iX, int iY, const char *pcString) {
    CHARACTER *pstScreen = (CHARACTER *) 0xB8000;

    int i;
    pstScreen += (iY * 80) + iX;
    for (i = 0; pcString[i] != 0; i++) {
        pstScreen[i].bCharactor = pcString[i];
    }
}