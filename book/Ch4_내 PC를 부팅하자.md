# 내 PC를 부팅하자

## 부팅과 부트로더
모든 OS들은 512바이트 크기의 작은 코드에서 시작한다.
512바이트의 작은 코드는 **부트로더(Boot loader)라고 불리며, OS의 나머지 코드를 메모리에 복사해 실행한다.

### 부팅과 BIOS
**부팅(Booting)**은 PC가 켜진 후에 OS가 실행되기 전까지 수행되는 일련의 작업과정을 의미한다.
부팅 과정에서 수행하는 작업에는 프로세서 초기화(멀티코어 관련 처리 포함), 메모리와 외부 디바이스 검사 및 초기화, 부트로더를 메모리에 복사하고 OS를 시작하는 과정 등이 포함된다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch4_img1.jpg)

MINT64 OS의 부팅과정이다.
PC환경에서는 부팅 과정 중 하드웨어와 관련된 작업을 BIOS(Basic Input/Output System)가 담당하며, BIOS에서 수행하는 각종 테스트나 초기화를 POST(Power On Self Test)라고 부른다.

**BIOS**는 메인보드에 포함된 펨웨어(Firmware)의 일종으로, 이름 그대로 입출력을 담당하는 작은 프로그램이다.
보통 PC 메인보드에 롬(ROM)이나 플래시 메모리로 존재하며, 전원이 켜짐과 동시에 프로세서가 가장 먼저 실행하는 코드이다.
BIOS는 부팅 옵션 설정이나 시스템 전반적인 설정 값(Configuration)을 관리하는 역할도 겸하며, 설정값으로 시스템을 초기화하여 OS를 실행할 수 있는 환경을 만든다.
BIOS에서 제공하는 기능은 인터럽트를 통해 사용할 수 없으며, MS-DOS 같은 과거의 16비트 OS는 BIOS의 기능에 많이 의존했다.
MINT64 OS도 OS 이미지를 메모리에 복사하고 GUI 모드로 변환할 때 BIOS의 기능을 사용한다.

BIOS는 부팅 과정에서 시스템 초기화 외에 수많은 작업을 하지만, 그중에서 우리에게 가장 중요한 것은 부트로더 이미지를 메모리로 복사하는 단계이다.
부트로더는 **부트스트랩(Bootstrap) 코드**라고도 불리며 우리가 BIOS에서 처음으로 제어를 넘겨받는 부분이다.
부트로더는 플로피 디스크나 하드 디스크 등 저장 매체의 가장 앞부분에 존재한다.
PC는 디스크나 플래시 메모리 등 다양한 장치로 부팅할 수 있으므로 BIOS는 POST가 완료된 후 여러 장치를 검사하여 앞부분에 부트로더가 있는지 확인한다.
부트로더가 존재한다면 코드를 0x7C00 어드레스에 복사한 후 프로세서가 0x7C00 어드레스부터 코드를 수행하도록 한다.
부팅가능한 모든 장치를 검사했는데도 부트로더를 찾을 수 없다면 BIOS 'Operating System Not Found'와 같은 메시지를 출력하고 작업을 중단한다.

부트로더가 디스크에서 메모리로 복사되어 실행되었다는것은 BIOS에 의해 PC가 정상적으로 구동되었다는 것을 의미한다.
다시 말하면 우리가 만든 OS를 메모리에 올려서 실행할 준비가 된것이다.

### 부트 로더의 역할과 구성
부트로더는 플로피 디스크나 하드 디스크 같은 외부 저장 매체에 있으며, 저장 매체에서 가장 첫번째 섹터 MBR(Master Boot Record)에 있는 작은 프로그램이다.
섹터(Sector)는 디스크를 구성하는 데이터의 단위로, 섹터 하나는 512바이트로 구성된다.
부트로더의 가장 큰 역할은 OS 실행에 필요한 환경을 설정하고, OS 이미지를 메모리에 복사하는 일이다.
부트로더는 BIOS가 가장 먼저 실행하는 중요한 프로그램이므로 기능이 다양하다고 생각할지도 모른다.
하지만, 부트로더는 크기가 512바이트로 정해져 있다.
즉 공간 제약이 있어서 처리할 수 있는 기능이 한정된다.
이렇게 작은 공간에 다양한 기능을 우겨 넣는 일은 무리이므로 대부분 부트로더는 OS 이미지를 메모리로 복사하고 제어를 넘겨주는 정형화된 작업을 수행한다.
4장과 5장에서 만들 부트로더 역시 OS 이미지를 디스크에서 메모리로 복사하는 역할만 수행한다.

부트 로더의 크기 문제는 부트로더의 기능을 최소화해서 해결했다.
그렇다면 BIOS에 첫번째 섹터가 부트로더란 것을 어떻게 알려줄까? 그리고 BIOS는 디스크에서 읽은 첫 번째 섹터가 정상적인 부트로더인지 어떻게 판단할까?

디스크를 부팅할 용도로 사용하지 않는다면, 첫번째 섹터는 부트로더가 아닌 일반 데이터가 저장된다.
만약 BIOS가 실수로 데이터를 메모리에 올려 실행한다면 모니터에 번쩍하는 섬광과 함께 PC가 리부팅된다.
이러한 사태를 방지하려면 BIOS는 첫번째 섹터에 있는 데이터가 부트로더인지 확인해야한다.

이를 위해 BIOS는 읽어들인 512바이트 중에 가장 마지막 2바이트의 값이 0x55, 0xAA인지 검사해서 부트로더인지 확인해야한다.
읽은 데이터가 0x55, 0xAA로 끝나지 않는다면 데이터로 인식하고 부팅 과정을 더 진행하지 않는다.
첫번째 섹터에 부트로더가 아닌 데이터를 저장할 생각이라면 마지막 2바이트는 0x55, 0xAA가 아닌 다른 값으로 써야한다.

## 부트로더 제작을 위한 준비
이제부터 만들 OS의 이름을 **MINT64 OS**라고 지칭하겠다.
본격적인 프로그래밍에 앞서, MINT64 OS 프로젝트를 생성한다.
책에서는 이클립스로 진행하지만, 편의를 위해 Jetbrains의 Clion으로 진행하겠다.

### 프로젝트 생성
Clion에서 MINT64라는 프로젝트를 생성한다.
특별히 설명할 내용은 없으므로 건너 뛰도록 하겠다.
[MINT64](https://github.com/HIPERCUBE/64bit-Multicore-OS/tree/master/MINT64)보고 따라오면 된다.

### MINT64 OS의 디렉터리 구조 생성
MINT64 OS는 리얼모드, 보호모드, IA-32e모드용 코드를 나눠서 관리한다.
이러한 파일을 디렉터리 하나로 관리하는 것은 매우 비효율적이므로 디렉터리 구조를 다래 그림과 같이 항목별로 나누었다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch4_img2.jpg)

부트로더 디렉터리와 대부분 유틸리티 디렉터리는 다른 디렉터리와 달리 소스파일 디렉터리와 임시 파일 디렉터리를 구분하지 않는다.
부트로더는 어셈블리어 파일 하나로 이루어진 작은 프로그램이므로 굳이 디렉터리를 구분하지 않아도 관리가 가능하다.
하지만, 보호모드 커널, IA-32e 커널, 각 응용프로그램은 여러 파일로 복잡하게 구성되므로 혼잡함을 줄이려고 임시 파일 디렉터리를 별로도 생성했다.

아래 이미지는 Clion project explorer 화면이다.
이렇게 디렉터리를 설정해주면 된다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch4_img3.png)

이제 빌드할때 필요한 make 파일을 작성해보도록 하겠다.

### makefile 작성
make문법의 기본 형식은 다음과 같이 Target, Dependency, Command 세 부분으로 구성되어 있다.

``` make
Target: Dependency ...
  Command
  Command
  ...
```

Target은 일반적으로 생성할 파일을 나타내며, 특정 라벨(Label)을 지정하여 해당 라벨과 관련된 부분만 빌드하는것도 가능하다.

Dependency는 Target 생성에 필요한 소스파일이나 오브젝트파일등을 나타낸다.

Command는 Dependency에 관련된 파일이 수정되면 실행할 명령을 의미한다.
Command에는 명령창이나 terminal에서 실행할 명령 또는 프로그램을 기술한다.
Command앞에 빈 공간은 TAB이다. 반드시 TAB으로 작성해야한다. 그렇지 않으면 make가 정상적으로 실행되지 않을수 있다.

예제를 통해 살펴보도록하자.
아래 예제는 a.c와 b.c 파일을 gcc로 빌드하여 output를 생성하는 makefile 예제이다.
다음과 같이 makefile을 생성한 뒤에 명령창이나 터미널에서 make를 입력하면 간단히 빌드할 수 있다.

``` make
# a.c, b.c를 통해서 output 파일을 생성하는 예제

all: output

a.o: a.c
    gcc -c a.c

b.o: b.c
    gcc -c b.c

output: a.o, b.o
    gcc -o output a.o b.o
```

가장 윗부분에 all이라고 표기한 특별한 Target이 있다.
all은 make를 실행하면서 옵션으로 Target을 직접 명시하지 않았을때 기본적으로 사용하는 Target이다.
여러 Target을 빌드할때 all Target의 오른쪽에 순서대로 나열하면 한번에 처리할 수 있다.

make는 빌드를 수행하는 도중에 다른 make를 실행할 수 있다.
이는 빌드 단계를 세부적으로 나누고, 계층적으로 수행할 수 있음을 의미한다.
최상위 디렉터리의 하위에 Library 디렉터리가 있고, 빌드 과정에서 Library 디렉터리를 빌드해야한다면 -C 옵션을 사용해서 다음과 같이 간단히 처리할 수 있다.

``` make
# output을 빌드한다
all: output

# Library 디렉터리로 이동한 후 make 수행
libtest.a:
    make -C Library

output.o: output.c
    gcc -c output.c

output: libtest.a output.o
    gcc -o output output.c -ltest -L./
```

`-ltest -L./`라는 부분이 있다.
`-l`옵션 다음에 오는건 `libtest.a` 라이브러리 파일이다.
앞에 `lib`는 빼고 온다. 예를들어 `libm.a`를 링크하고 싶을때는 `-lm`으로 작성한다.
맨뒤에 `-L./`는 라이브러리 파일의 경로를 나타낸다. 여기서는 현재 디렉토리를 가리킨다.

### MINT64용 makefile 생성
최상단 디렉토리에 makefile을 만들고 아래처럼 작성한다.

``` make
all: BootLoader Disk.img

BootLoader:
    @echo
    @echo =============== Build Boot Loader ===============
    @echo

    make -C 00.BootLoader

    @echo
    @echo =============== Build Complete ===============
    @echo

Disk.img: 00.BootLoader/BootLoader.bin
    @echo
    @echo =============== Disk Image Build Start ===============
    @echo

    cp 00.BootLoader/BootLoader.bin Disk.img

    @echo
    @echo =============== All Build Complete ===============
    @echo

clean:
    make -C 00.BootLoader clean
    rm -f Disk.img
```

최상위 makefile의 목적은 OS 이미지 생성을 위해 각 하위 디렉터리의 makefile을 실행하는 것이다.
지금은 부트 로더만 있으므로 해당 디렉터리로 이동해서 빌드하고, 빌드한 결과물을 복사하여 OS이미지를 생서하는것이 전부이다.

최상위 디렉터리에 makefile을 생성했으니 다음은 부트로더 디렉터리에 makefile을 생성할 차례이다.
MINT64 OS의 최사우이 디렉터리에 makefile을 생성하는 것과 같은 방법으로 00.BootLoader디렉터리에 makefile을 생성한다.

``` make
all: BootLoader.bin

BootLoader.bin: BootLoader.asm
    nasm -o BootLoader.bin BootLoader.asm
    
clean:
    rm -f BootLoader.bin 
```

부트로더 makefile의 목적은 BootLoader.asm 파일을 nasm 어셈블리어 컴파일러로 빌드하여 BootLoader.bin 파일을 생성하는 것이다.
clean Target이 정의되어 잇으며 자신의 디렉터리에 있는 BootLoader.bin 파일을 삭제한다.

최상위 디렉터리의 makefile로 빌드하면 BootLoader.asm파일이 없으므로 아래와 같은 에러가 발생한다.

```

=============== Build Boot Loader ===============

make -C 00.BootLoader
make[1]: *** No rule to make target `BootLoader.asm', needed by `BootLoader.bin'.  Stop.
make: *** [BootLoader] Error 2
```

참고로 이런 에러가 발생할 수 있다.
```
makefile:4: *** missing separator.  Stop.
```
Command 앞에 tab이 오지 않아서 그런것이다.
Clion에서 tab으로 입력해도 스페이스 여러개로 입력된것처럼 입력되기도 한다.
vim같은걸로 수정하는걸 추천한다.
