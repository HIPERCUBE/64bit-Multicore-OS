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

**AT PC**는 부팅후, A20 게이트를 무조건 0으로 설정하여 **XT PC**와의 호환성을 유지했으며, **A20 게이트**를 ㅗ할성화했을때만 20번째 어드레스 비트가 정상적으로 동작하게했다.
아래 그림은 A20 게이트의 비활성화에 따른 선형 주소의 변화에 대해서 나타낸 것이다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch8_img3.jpg)

**A20 게이트**가 비활성화된 상태에서는 어드레스 라인의 20번째 비트가 항상 9으로 설정되므로 홀수 MB에는 접근할 수 없다.
최초 부팅된 이후의 상태는 A20 게이트가 비활성화된 상태이므로 1~4MB 어드레스를 초기화하면 홀수 MB 영역을 제외한 0~1MB와 2~3MB 영역을 초기화하게된다.
하지만, 0~1영역은 BIOS와 보호 모드커널 영역으로 사용하고 있으므로 현재 커널이 수행 중인 부분이 초기화하게되어 문제가 생긴다.
아래 그림은 A20 게이트 비활성화에 따라 접근 가능한 메모리 공간의 변화를 나타낸 것이다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch8_img4.jpg)

### A20 게이트 활성화 방법
A20 게이트를 활성화 하는 방법
 - 키보드 컨트롤러 Keyboard Controller로 활성화
  - AT PC 초창기 시절부터 사용되던 방법
  - 속도가 느리고 소스코드가 복잡하다
  - PS/2 방식의 키보드/마우스를 지원하는 PC라면 어디서나 사용가능
 - 시스템 컨트롤 포트 System Controll Port로 활성화
  - 키보드 컨트롤러의 대안
  - 시스템 제어에 관련된 I/O 포트를 통해 A20 게이트를 활성화하는 방법
  - 키보드 컨트롤러를 사용하는 것보다 속도가 빠르고 소스코드가 간략함
 - BIOS 서비스로 활성화
  - 486 프로세서를 지원하는 BIOS에서 처음 도입
  - BIOS 서비스 중에 시스템에 관련된 서비스를 통해 A20 게이트를 활성화하는 방법

PC 코어수가 2개 이상이거나, 64비트를 지원하는 프로세서라면 위의 3가지 방법중에서 2가지 이상을 지원할 것이다.

### 시스템 컨트롤 포트로 A20 게이트 활성화하기
**시스템 컨트롤 포트**는 I/O 포트 어드레스의 **0x92**에 위치하며, A20 게이트부터 하드 디스크 LED에 이르기까지 여러가지 시스템 옵션 담당

| 비트 | 읽기/쓰기 모드 | 설명 |
| :--: | :--: | :-- |
| 7<br/>6 | 읽기와 쓰기 | 하드디스크 LEB 제어<br/>모두 9으로 설정하면 LED가 꺼지며, 그 외의 경우는 LED가 켜짐 |
| 5<br/>4 | - | 사용 안함 |
| 3 | 읽기와 쓰기 | 부팅 패스워드 접근 제어<br/>1로 설정하면 전원을 다시 인가할때까지 CMOS 레지스터에 설정된 부팅 패스워드를 삭제하지 못하며, 0으로 설정하면 부팅 패스워드를 삭제할 수 있음 |
| 2 | 읽기 전용 | 사용 안함 |
| 1 | 읽기와 쓰기 | A20 게이트 제어<br/>1로 설정하면 A20 게이트를 활성화하며, 0으로 설정하면 비활성화함 |
| 0 | 쓰기 전용 | 빠른 시스템 리셋<br/>1로 설정하면 시스템 리셋(리얼 모드로 전환)을 수행하며, 0으로 설정하면 아무 변화 없음 |

목적은 A20 게이트를 활성화하는 것이므로 시스템 컨트롤 포트의 비트 1만 1로 설정한다.
시스템 컨트롤 포트는 I/O 포트에 있으므로 접근하려면 별도의 명령어(`in`, `out`)를 사용해야한다.
아래 코드는 in/out 명령어로 시스템 컨트롤 포트에 접근하여 A20 게이트를 활성화하는 코드이다.

``` asm
in al, 0x92     ; 시스템 컨트롤 포트(0x92)에서 1바이트를 읽어 AL 레지스터에 저장

or al, 0x02     ; 읽은 값에 A20 게이트 비트(비트 1)를 1로 설정
and al, 0xFE    ; 시스템 리셋 방지를 위해 0xFE와 AND 연산하여 비트 0을 0으로 설정

out 0x92, al    ; 시스템 컨트롤 포트(0x92)에 변경된 값을 1바이트 설정
```

### BIOS 서비스로 A20 게이트 활성화 방법
A20 게이트 관련 설정 값을 AX 레지스터에 넣고 나서 A20 게이트를 활성화하는 BIOS 시스템 서비스(인터럽트 벡터 0x15)를 호출하면 된다.
BIOS의 시스템 서비스에 A20 게이트 관련 기능이 들어있다.
인터럽트 벡터 테이블의 0x15에 위치한다.
아래 표는 BIOS 시스템 중 A20 게이트 관련 기능을 나타낸 것이다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch8_img5.jpg)

아래는 BIOS 서비스를 호출하여 A20 게이트를 활성화하는 코드이다.

``` asm
mov ax, 0x2401      ; A20 게이트 활성화 서비스 설정
int 0x15            ; BIOS 인터럽트 서비스 호출

jc .A20GATEERROR    ; A20 게이트 활성화가 성공했는지 확인
jmp .A20GATESUCCESS

.A20GATEERROR:
    ; 에러 핸들링

.A20GATESUCCESS:
    ; 성공 처리
```

## A20 게이트 적용과 메모리 크기 검사
