# 플로피 디스크에서 OS이미지를 로딩하자

## BIOS 서비스와 소프트웨어 인터럽트
BIOS는 키보드/마우스에서 디스크나 프린터까지 거의 모든 PC 주변기기를 제어하는 기능을 제공한다.
16비트 OS를 개발한다고하면 BIOS의 기능만 활용해도 OS를 개발할 수 있다.

BIOS는 우리가 일반적으로 많이 쓰는 라이브러리(Library) 파일과 달리 자신의 기능을 특별한 방법으로 외부에 제공한다.
함수의 어드레스를 인터럽트 벡터 테이블(Interrupt Vector Table)에 넣어두고, 소프트웨어 인터럽트(SWI, Software Interrupt)를 호출하는 방법을 사용한다.
인터럽트 벡터 테이블은 메모리 어드레스 0에 있는 테이블로 특정 번호의 인터럽트가 발생했을때 인터럽트를 처리하는 함수(인터럽트 핸들러, Interrupt Handler) 검색에 사용한다.
테이블의 각 항목은 인덱스에 해당하는 인터럽트가 발생했을 때 처리하는 함수 어드레스가 저장되어있으며, 각 항목은 크기가 4바이트이다.
또한 인터럽트는 최대 256개 까지 설정할 수 잇으므로 리얼 모드의 인터럽트 벡터 크기는 최대 256 * 4 = 1024바이트가 된다.
아래 표는 리얼모드에서 사용하는 인터럽트 벡터 테이블의 내용이다.
항목이 많아서 중요한 부분만 발췌했다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch5_img1.jpg)

BIOS가 제공하는 디스크 서비스를 사용하려면 위에 나와있듯이 0x13 인터럽트를 발생시켜야한다.
이는 우리가 임의로 인터럽트를 발생시킬 수 잇어야 한다는것을 의미하며, 이때 소프트웨어 인터럽트 명령을 사용한다.
SWI는 CPU에 가상으로 특정 인터럽트가 발생했다고 알리는 명령어로 int 0x13 형태로 사용한다.
만든 함수의 어드레스를 인터럽트 벡터 테이블에 넣어뒀다면 int 명령으로 언제든지 해당 함수로 이동할 수 있다.

BIOS 서비스는 SWI를 통해 호출할 수 있지만 BIOS도 만능은 아니어서 작업에 관련된 파라미터를 넘겨줘야한다.
파라미터는 레지스터를 이용해서 넘겨준다.
BIOS의 기능을 사용할 때는 AX, BX, CX, DX 레지스터와 ES 세그먼트 레지스터를 사용해서 파라미터를 넘겨주며 결과값도 레지스터를 통해 넘겨받는다.
물론 BIOS  서비스 마다 요구하는 파라미터의 수가 다르므로 서비스를 호출할 때 파라미터로 정의된 레지스터를 꼭 확인해야한다.
디스크 관련 서비스 주엥 리셋과 섹터 읽기 기능을 예로 들면 아래 표와 같다.
보면 AX레지스터는 기능 선택과 처리한 결과값을 받을 때 공통으로 사용되는 것을 알 수 잇따.
이는 BIOS의 디스크관련 서비스 뿐만 아니라 다른 서비스에도 마찬가지로 적용되는 규칙이다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch5_img2.jpg)


## OS 이미지 로딩 기능 구현
### 디스크 읽기 기능 구현
MINT64 OS의 이미지는 크게 부트로더, 보호 모드 커널, IA-32e 모드 커널로 구성되며, 각 부분은 섹터 단위로 정렬해서 하나의 부팅 이미지 파일로 합친다.
따라서 디스크의 두번째 섹터부터 읽어서 특정 메모리 어드레스에 순서대로 복사하면 이미지 로딩은 끝이다.
MINT64 OS는 OS 이미지 를 0x10000(64Kbyte)에 로딩해서 실행한다.
하지만, OS 이미지를 반드시 0x10000 위치에 로딩해야 실행되는것은 아니다.
부트로더 이후(0x07C00)에 연속해서 복사해도 OS실행에 문제는 없다.
MINT 64 OS는 0x10000하위 영역을 다른 용도로 사용하기에 남겨둔 것이다.

플로피 디스크의 첫 번째 섹터는 부트로더로 BIOS가 메모리에 로딩한다.
따라서 플로피 디스크의 두 번째 섹터부터 OS 이미지 크기만큼 읽어서 메모리에 복사하면 된다.
플로피 디스크의 섹터는 섹터 -> 헤드 -> 트랙의 순서로 배열되어 잇으므로 이 순서만 지킨다면 큰 문제 없이 로딩할 수 잇다.
섹터 배열 순서를 고려하여 작성한 C코드는 아래와 같다.
섹터 번호를 순서대로 증가시키며 읽다가 마지막 섹터에서 헤드와 트랙 번호를 증가시키는 것이 핵심 포인트이다.

``` C
int main(int argc, char *argv[]) {
    int iTotalSectorCount = 1024;
    int iSectorNumber = 2;
    int iHeadNumber = 0;
    int iTrackNumber = 0;
    // 실제 이미지를 복사할 어드레스(물리 주소)
    char *pcTargetAddress = (char *) 0x10000;

    while (1) {
        // 전체 섹터 수를 하나씩 감소시키면서 0이 될 때까지 섹터를 복사
        if (iTotalSectorCount == 0)
            break;
        iTotalSectorCount--;

        // 1 섹터를 읽어들여서 메모리 어드레스에 복사
        // BIOSReadOneSector: BIOS의 세거 읽기 기능을 호출하는 임의의 함수
        if (BIOSReadOneSector(iSectorNumber, iHeadNumber, iTrackNumber, pcTargetAddress) == ERROR)
            HandleDiskError();

        // 1 섹터는 512(0x200) 바이트이므로 복사한 수만큼 어드레스 증가 
        pcTargetAddress = pcTargetAddress + 0x200;

        // 섹터 -> 헤드 -> 트랙 순으로 번호 증가
        iSectorNumber++;
        if (iSectorNumber < 19)
            continue;

        // 헤드의 번호는 0과 1이 반복되므로 이를 편리하게 처리하기위해 XOR 연산을 사용
        // iHeadNumber = (iHeadNumber == 0x00) ? 0x00 : 0x01; 과 같은 의미
        iHeadNumber = iHeadNumber ^ 0x01;
        iSectorNumber = 1;

        if (iHeadNumber != 0)
            continue;

        iTrackNumber++;
    }
    return 0;
}

int HandleDiskError() {
    printf("DISK Error!!");
    while (1);
}
```

위의 C언어 코드를 참고하여 어셈블리어 소스 코드를 작성하면 아래와 같다.

``` asm
TOTALSECTORCOUNT:   dw  1024    ; 부트로더를 제외한 MINT64 OS이미지의 크기
                                ; 최대 1152 섹터(0x90000byte)까지 가능
SECTORNUMBER:       db  0x02    ; OS 이미지가 시작하는 섹터 번호를 저장하는 영역
HEADNUMBER:         db  0x00    ; OS 이미지가 시작하는 헤드 번호를 저장하는 영역
TRACKNUMBER:        db  0x00    ; OS 이미지가 시작하는 트랙 번호를 저장하는 영역

    ;디스크의 내용을 메모리로 복사할 어드레스(ES:BX)를 0x1000으로 설정
    mov si, 0x1000      ; OS 이미지를 복사할 어드레스(0x10000)를 세그먼트 레지스터 값으로 변환
    mov es, si          ; ES 세그먼트에 값 설정
    mov bx, 0x0000      ; BX 레지스터에 0x0000을 설정하여 복사할 어드레스를 0x1000:0000(0x10000)으로 최종 설정

    mov di, word [ TOTALSECTORCOUNT ]   ; 복사할 OS 이미지의 섹터 수를 DI 레지스터에 설정

READDATA:               ; 디스크를 읽는 코드의 시작
    ; 모든 섹터를 다 읽었는지 확인
    cmp di, 0           ; 복사할 OS의 이미지의 섹터 수를 0과 비교
    je READEND          ; 복사할 섹터 수가 0이라면 다 복사했으므로 READEND로 이동
    sub di, 0x01        ; 복사할 섹터 수를 1 감소

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; BIOS Read Function 호출
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah, 0x02                    ; BIOS 서비스 번호 2(Read Sector)
    mov al, 0x1                     ; 읽을 섹터 수는 1
    mov ch, byte [ TRACKNUMBER ]    ; 읽을 트랙 번호 설정
    mov cl, byte [ SECTORNUMBER ]   ; 읽을 섹터 번호 설정
    mov dh, byte [ HEADNUMBER ]     ; 읽을 헤드 번호 설정
    mov dl, 0x00                    ; 읽을 드라이브 번호(0=Floppy) 설정
    int 0x13                        ; 인터럽트 서비스 수행
    jc HANDLEDISKERROR              ; 에러가 발생했다면 HANDLEDISKERROR로 이동

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 복사할 어드레스와 트랙, 헤드 섹터 어드레스 계산
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    add si, 0x0020      ; 512(0x200)바이트 만큼 읽었으므로 이를 세그먼트 레지스터 값으로 변환
    mov es, si          ; ES 세그먼트 레지스터에 더해서 어드레스를 한 섹터만큼 증가

    ; 한 섹터를 읽었으므로 섹터 번호를 증가시키고 마지막 섹터(18)까지 읽었는지 판단
    ; 마지막 섹터가 아니면 섹터 읽기로 이동해서 다시 섹터 읽기 수행
    mov al, byte [ SECTORNUMBER ]   ; 섹터 번호를 AL레지스터에 설정
    add al, 0x01                    ; 섹터 번호를 1 증가
    mov byte [ SECTORNUMBER ], al   ; 증가시킨 섹터 번호를 SECTORNUMBER에 다시 설정
    cmp al, 19                      ; 증가시킨 섹터 번호를 19와 비교
    jl READDATA                     ; 섹터 번호가 19미만이라면 READDATA로 이동
    
    ; 마지막 섹터까지 읽었으면(섹터 번호가 19이면) 헤드를 토글(0->1, 1->0)하고, 섹터 번호를 1로 설정
    xor byte [ HEADNUMBER ], 0x01   ; 헤드 번호를 0x01과 XOR하여 토글(0->1, 1->0)
    mov byte [ SECTORNUMBER ], 0x01 ; 섹터 번호를 다시 1 로 설정
    
    ; 만약 헤드가 1->0으로 바뀌었으면 양쪽 헤드를 모두 읽은것이므로 아래로 이동하여 트랙 번호를 1 증가
    cmp byte [ HEADNUMBER ], 0x00   ; 헤드 번호를 0x00과 비교
    jne READDATA                    ; 헤드 번호가 0이 아니면 READDATA로 이동
    
    ; 트랙을 1 증가시킨 후, 다시 섹터 읽기로 이동
    add byte [ TRACKNUMBER ] , 0x01 ; 트랙 번호를 1 증가
    jmp READDATA                    ; READDATA로 이동
READEND:

HANDLEDISKERROR:    ; 에러를 처리하는 코드
    ; 생략
```

위의 어셈블리어 소스 코드와 디스크 리셋 기능만 부트로더에 추가하면 로딩할 준비가 끝난다.
그런데 기능은 구현했지만 화면에 출력하는 코드가 없어서 진행 상황이나 완료 유무를 확인하기 어렵다.
이번에는 화면에 진행 상태를 출력하도록 코드를 추가해보겠다

앞에서 환영 메시지를 출력하는 코드를 구현했다.
하지만 함수 형태로 구현하지 않아서 원하는 곳에서 호출할 수 없다.
무네즌 코드 구조뿐만 아니라 함수 호출에 필요한 핵심 자료구조또한 빠져있다는것이다.
이를 보완하여 함수 호출이 가능한 구조로 만들어 보겠다.

### 스택 초기화와 함수구현
