#include "Types.h"

/**
 * 함수 선언
 */
void kPrintString(int iX, int iY, const char *pcString);

BOOL kInitializeKernel64Area();

BOOL kIsMemoryEnough();


/**
 * Main 함수
 */
void Main() {
    DWORD i;

    kPrintString(0, 3, "C Language Kernel Started...................[Pass]");

    // 최소 메모리 크기를 만족하는지 검사
    kPrintString(0, 4, "Minimum Memory Size Check...................[    ]");
    if (kIsMemoryEnough() == FALSE) {
        kPrintString(45, 4, "Fail");
        kPrintString(0, 5, "Not Enough Memory~!! MINT64 OS Requires Over 64Mbyte Memory~!!");
        while (1);
    } else {
        kPrintString(45, 4, "Pass");
    }

    // IA-32e 모드의 커널 영역을 초기화
    kPrintString(0, 5, "IA-32e Kernel Area Initialize...............[    ]");
    if (kInitializeKernel64Area() == FALSE) {
        kPrintString(45, 5, "Fail");
        kPrintString(0, 6, "Kernel Area Initialization Fail~!!");
        while (1);
    }
    kPrintString(45, 5, "Pass");

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

/**
 * IA-32e 모드용 커널 영역을 0으로 초기화
 */
BOOL kInitializeKernel64Area() {
    DWORD *pdwCurrentAddress;

    // 초기화를 시작할 어드레스인 0x100000(1MB)을 설정
    pdwCurrentAddress = (DWORD *) 0x100000;

    // 마지막 어드레스인 0x600000(6MB)까지 루프를 돌면서 4바이트씩 0으로 채움
    while ((DWORD) pdwCurrentAddress < 0x600000) {
        *pdwCurrentAddress = 0x00;

        // 0으로 저장한 후 다시 읽었을 때 0이 나오지 않으면 해당 어드레스를
        // 사용하는데 문제가 생긴 것이므로 더이상 진행하지 않고 종료
        if (*pdwCurrentAddress != 0)
            return FALSE;

        // 다음 어드레스로 이동
        pdwCurrentAddress++;
    }

    // 작업을 맞친후 정상적으로 완료되었다고 TRUE 반환
    return TRUE;
}

/**
 * MINT64 OS를 실행하기에 충분한 메모리를 가지고 있는지 체크
 */
BOOL kIsMemoryEnough() {
    DWORD *pdwCurrentAddress;

    // 0x100000(1MB)부터 검사 시작
    pdwCurrentAddress = (DWORD *) 0x100000;

    // 0x4000000(64MB)까지 루프를 돌면서 확인
    while ((DWORD) pdwCurrentAddress < 0x4000000) {
        *pdwCurrentAddress = 0x12345678;

        // 0x12345678로 설정한 후 다시 읽었을 때 0x12345678이 나오지 않으면
        // 해당 어드레스를 사용하는데 문제가 생긴 것이므로 더이상 진행하지 않고 종료
        if (*pdwCurrentAddress != 0x12345678) {
            return FALSE;
        }

        // 1MB씩 이동하면서 확인
        pdwCurrentAddress += (0x100000 / 4);
    }
    return TRUE;
}