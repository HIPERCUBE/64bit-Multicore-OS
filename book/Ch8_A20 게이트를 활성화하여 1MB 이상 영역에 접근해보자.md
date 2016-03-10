# A20 게이트를 활성화하여 1MB 이상 영역에 접근해보자

## IA-32e 모드 커널과 메모리 맵
이번장에서 진행할 내용
 - **IA-32e** 모드 커널을 실행하기 위한 준비 작업으로 PC에 설치된 메모리가 64MB 이상인지 검사
 - **IA-32e** 모드 커널이 위치할 영역을 모두 0으로 초기화
 - 부팅과정을 완료하고 나서 1MB 이상의 메모리에 정상적으로 접근되는지 확인

1MB 이상의 메모리에 접근해야 하는 이유
> 부트로더에 의해 커널 이미지가 메모리에 로딩되는 어드레스는 **0x10000**이다.
> 만약 1MB 이하의 어드레스중에서 비디오 메모리가 위치하는 **0xA0000** 이하를 커널 공간으로 사용한다고 가정하면, 보호 모드 커널과 IA-32e 모드 커널의 최대 크기는 (0xA0000 - 0x10000)이 되어 576KB 정도가 된다.
> 커널 이미지외에도 초기화되지 않은 영역(.bss 섹션)의 공간도 필요하다.
> 추후에 멀티태스킹과 파일 시스템과 같은 기능이 추가되어 커널이 커진다면 576KB로는 부족하다.

MINT64 OS는 이러한 문제를 해결하기 위해 커널 이미지를 모두 0x10000 어드레스에 복사하되, 덩치가 큰 **IA-32e** 모드 커널은 2MB의 어드레스로 복사하여 2MB ~ 6MB의 영역을 별도로 할당했다.
따라서 IA-32e 모드의 커널영역은 모드 섹션을 포함하여 총 **4MB**의 크기가 된다.
아래 그림은 MINT 64 OS의 메모리 맵이다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch8_img1.jpg)

왜 IA-32e 모드 커널이 위치할 영역을 0으로 초기화 할까?
커널 이미지로 덮어써버리기때문에 초기화할 필요가 없지 않을까라고 생각했다.
책에서는 이렇게 말한다.
> 미리 초기화 하는 이유는 IA-32e 커널 이미지가 초기화되지 않은 영역을 포함하고 있지 않기 때문이다.
> 비록 커널은 초기화되지 않은 영역을 사용하지만, 커널 이미지에는 초기화 되지 않은 영역이 제외되어 있다.
> 따라서 커널 이미지를 옮길 때도 이 영역은 해당되지 않으며, 이미지를 옮길 영역을 미리 0으로 초기화 하지 않는다면 어떤 임의의 값이 들어 있을 것이다.
> 이런 상태에서 IA-32e 모드 커널이 실행되면 0으로 참조되어야 할 변수들이 0이 아닌 값으로 설정되어, 루프를 빠져 나오지 못한다든지 잘못된 조건문이 실행된다든지 하는 문제가 발생할 가능성이 있다.
> 0으로 초기화 하는 이유는 미연을 방지하기 위해서이다.

## IA-32e 모드 커널을 위한 메모리 초기화
### 메모리 초기화 기능 추가
위에서 말한 1MB부터 6MB 영역까지를 모두 0으로 초기화 하는 기능은 C 코드로 구현할 것이다.
소스코드는 앞에서 만든 [01.Kernel32/Source/Main.c]()에 추가할 것이다.

기존 소스 코드
``` C
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
```

변경된 소스 

``` C
#include "Types.h"

void kPrintString(int iX, int iY, const char *pcString);

BOOL kInitializeKernel64Area();

/**
 * Main 함수
 */
void Main() {
    DWORD i;

    kPrintString(0, 3, "C Language Kernel Started~!!");

    // IA-32e 모드의 커널 영역을 초기화
    kInitializeKernel64Area();
    kPrintString(0, 4, "IA-32e Kernel Area Initialization Complete");

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
```

### 빌드와 실행
앞에서 makefile을 작성했으므로 추가로 작성할 필요가 없다.
make하고 QEMU를 돌려보면 아래 스크린샷처럼 성공할수도 있지만 실패할 수도 있다.
책에서는 QEMU에서는 정상적으로 작동하지만 실제 PC에서는 제대로 작동하지 않는다고 한다.

이 문제의 원인은 PC가 하위 기종에 대한 호환성을 유지하기 위해 어드레스 라인을 비활성화했기 때문이다.
어드레스 라인에 대한 내용은 뒤에서 설명할 것이다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch8_img2.png)

## 1MB 어드레스와 A20 게이트
### A20 게이트의 의미와 용도
초창기 **XT PC**는 최대 1MB까지 어드레스에 접근할 수 있었다.
하지만 리얼 모드에서는 세그먼트와 오프셋으로 1MB가 넘는 **0x10FFEF**까지 접근할 수 있다.
하드웨어의 한계로 1MB가 넘는 어드레스로 접근하는 경우 **0x10FFEF**로 인식되었다.

이후에 16MB 어드레스 까지 접근가능한 **AT PC**가 생겼고, XT PC의 특수한 어드레스 계산법(1MB 이상의 어드레스를 1MB 이하의 어드레스에 매핑)으로인해 기존 **XT PC**용 프로그램을 실행하는데 문제가 생겼다.
이러한 호환성 문제를 해결하기 위해 도입된것이 **A20 게이트**이다.

A20은 **A**ddress의 20번째 비트를 뜻하며, 20번째 비트를 활성화하거나 비활성화하여 **XT PC**의 어드레스 계산 방식과 호환성을 유지시킨다.
A20 게이트가 비활성화되면 어드레스 라인 20번째(1MB의 위치)가 항상 0으로 고정되므로 선형 주소가 0x10FFEF가 되더라도 0xFFEF로 처리할 수 있다.

**AT PC**

## A20 게이트 적용과 메모리 크기 검사
