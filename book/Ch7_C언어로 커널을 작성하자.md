# C언어로 커널을 작성하자

## 실행 가능한 C 코드 커널 생성 방법
지금까지 작성한 소스는 전부다 단일 파일로 구성되고, 각 파일은 NASM 컴파일러를 통해 Binary 형태로 생성되었다.
현재 보모 모드 커널 역시 엔트리 포인트 소스파일 EntryPoint.s 하나로 구성되어 있으며, 512 바이트로 정렬되어 OS 이미지 파일에 결합되는 구조를 하고 있다.

이번 장에서는 C 소스 파일을 추가하고, 이를 빌드하여 보호 모드 커널 이미지에 통합하는 것이다.
C언어로 작성한 커널을 보호 모드 엔트리 포인트의 뒷부분에 연결하고 엔트리 포인트에서는 C커널의 시작 부분으로 이동하는것이 전부이다.

C코드는 어셈블리어 코드와 달리 컴파일과 링크 과정을 거쳐서 최종 결과물이 생성된다.
컴파일 과정은 소스 파일을 중간 단계인 오브젝트 파일(Object file)로 변환하는 과정을 소스 파일을 해석하여 코드 영역과 데이터 영역으로 나누고, 이러한 메모리 영역에 대한 정보를 생성하는 단계이다.
링크 단계는 오브젝트 파일들의 정보를 취합하여 실행 파일에 통합하며, 필요한 라이브러리 등을 연결해주는 역하릉 하는 단계이다.
아래 그림은 이러한 빌드 과정을 나타낸 것이다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch7_img1.jpg)

### 빌드 조건과 제약 사항
엔트리 포인트가 C 코드를 실행하려면 적어도 아래의 세 가지 제약 조건은 만족해야한다.

**1. C 라이브러리를 사용하지 않도록 빌드해야한다.**
부팅된 후 보호 모드 커널이 실행되면 C 라이브러리가 없으므로 라이브러리에 포함된 함수를 호출할 수 없다.
만약 커널 코드에서 어플리케이션 소스처럼 `printf()` 함수를 사용한다면, C 라이브럴리를 동적링크(Dynamic Link)또는 정적 링크(Static Link)하게 된다.
하지만 커널은 자신을 실행하기 위한 최소한의 환경만 설정하므로 라이브러리의 함수를 실행할 수 없다.

**2. 0x10200 위치에서 실행하도록 빌드해야한다.**
0x10000의 위치는 6장에서 작성한 섹터크기의 보호 모드 엔트리 포인트가 있으므로, 결합된 C코드는 512바이트 이후인 0x10200 위치부터 로딩된다.
따라서 C로 작성한 커널 부분은 빌드할때 0x10200 위치에서 실행되는 것을 전제로 해야하며, 해당 위치의 코드는 C 코드 중에 가장 먼저 실행되어야 하는 함수(엔트리 포인트)가 위치해야한다.
커널이 실행되는 어드레스가 중요한 이유는 선형 주소를 참조하게 생성된 코드나 데이터 때문이다.
C언어에서 전역 변수의 어드레스나 함수의 어드레스르 참조하는 경우, 실제로 존재하는 선형 주소로 변환된다.
따라서 메모리에 로딩되는 어드레스가 변한다면, 이러한 값들 역시 변경해줘야 정상적으로 작동한다.
아래와 같은 C코드가 있을때, 0x0000에서 로딩되어 실행되는 경우와 0x10200에서 로딩되어 실행되는 경우의 어셈블리어 코드는 아래와 같다. (g_iIndex 변수는 메모리 어드레스의 가장 앞쪽에 위치한다고 가정)

```   C
/**
 * C 코드
 */
/* 생략 */

int g_iIndex = 0;

void AddIndex() {
  g_iIndex++;
}
```

``` asm
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 0x0000에 로딩된 어셈블리어 코드
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ~ 생략 ~

g_iIndex: DD  0x00000000    ; g_iIndex 변수는 로딩된 메모리 어드레스 처음에 위치한다고 가정

AddIndex:
  ; ~ 생략 ~
  
  mov eax, dword [ 0x0000 ] ; g_iIndex 변수의 어드레스에 접근하여 값을 EAX에 저장
  add eax, 1
  mov dword [ 0x0000 ], eax ; 1 증가도니 값을 g_iIndex 변수에 접근하여 다시 저장
  
  ; ~ 생략 ~
```

``` asm
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 0x10200에 로딩된 어셈블리어 코드
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ~ 생략 ~

g_iIndex: DD  0x00000000    ; g_iIndex 변수는 로딩된 메모리 어드레스 처음에 위치한다고 가정

AddIndex:
  ; ~ 생략 ~
  
  mov eax, dword [ 0x10200 ] ; g_iIndex 변수의 어드레스에 접근하여 값을 EAX에 저장
  add eax, 1
  mov dword [ 0x10200 ], eax ; 1 증가도니 값을 g_iIndex 변수에 접근하여 다시 저장
  
  ; ~ 생략 ~
```
위 코드를 보면 메모리에 로딩되는 어드레스에 따라 전역 변수의 어드레스에 접근하는 부분이 변한다는 것을 알 수 있다.
이러한 이유로 커널이 0x10200의 어드레스에서 실행되게 빌드하는 것이 필요하다.

**3. 코드나 데이터 외에 기타 정보를 포함하지 않는 순수한 바이너리 파일 형태여야한다.**
일반적으로 GCC를 통해 실행파일을 생성하면 **ELF** 파일 포맷이나 PE파일 포맷과 같이 특정 OS에서 실행할 수 있는 포맷으로 생성된다.
이러한 파일 포맷들은 실행하는데 필요한 코드와 데이터 정보 이외의 불필요한 정보를 포함하고 있다.
해당 파일 포맷을 그대로 사용하게되면 엔트리 포인트에서 파일 포맷을 해석하여 해당 정보에 따라 처리하는 기능이 포함되어야 하므로 코드가 복잡해진다.
만일 부트 로더나 보호 모드 엔트리포인트처럼 코드와 데이터만 포함된 바이너리 파일 형태를 사용한다면, 엔트리 포인트에서 해당 어드레스로 점프(jmp)하는 것만으로 C언어를 실행할 수 있다.

### 소스 파일 컴파일 - 라이브러리 사용하지 않는 오브젝트 파일 생성 방법
C코드를 컴파일하여 오브젝트 파일을 생성하는 방법은 아주 간단하다.
GCC 옵션으로 `-c`만 추가해주면 된다.

라이브러리를 사용하지 않고 홀로 동작할 수 있는 형태로 빌드해야한다.
`-ffreestanding`이라는 GCC 컴파일 옵션을 사용해야한다.

```
gcc -c -m32 -ffreestanding Main.c
```

`-m32` 옵션은 GCC가 32비트 코드 생성하도록 설정하는것이다.
일반적으로 GCC는 기본으로 64비트 코드를 생성한다.

### 오브젝트 파일 링크 - 라이브러리를 사용하지 않고 특정 어드레스에서 실행 가능한 커널 이미지 파일 생성 방법
오브젝트 파일을 링크하여 실행 파일을 만드는 방법은 소스 파일을 컴파일 하는 방법보다 까다롭다.
실행 파일을 구성하는 섹션의 배치와 로딩될 어드레스, 코드 내에서 가장 먼저 실행될 코드인 엔트리 포인트를 지정해줘야 하기 때문이다.
특히 섹션을 배치하는 작업은 오브젝트 파일이나 실행파일 구조와 관련이 있으므로 다른 작업보다 좀 더 까다로울 수 있다.
하지만, 섹션을 배치하는 방식과 크기 정렬 방식에 따라서 OS 메모리 구조와 크기가 달라지므로, 한번 아래 내용을 읽고 넘어가길을 권장한다.

섹션 배치를 다시 하는 이유는 실행 파일이 링크될때 코드나 데이터 이외의 디버깅 관련 정보와 심볼(Symbol, 함수나 변수의 이름) 정보가 포함되기 때문이다.
이러한 정보는 커널을 실행할때 불필요하므로, 최종 바이너리 파일을 생성할 때 이를 제거하려고 섹션을 재배치하는것이다.
섹션을 재배치하여 코드와 데이터를 실행 파일 앞쪽으로 이동시키면 손쉽게 나머지 부분을 제거할 수 있다.

### 섹션 배치와 링커 스크립트, 라이브러리를 사용하지 않는 링크
**섹션(Section)**은 실행 파일 또는 오브젝트 파일에 있으며 공통된 속성(코드, 데이터, 각종 심볼과 디버깅 정보 등)을 담는 영역을 뜻한다.
실행 파일이나 오브젝트 파일에는 무수히 많은 섹션이 있지만 핵심 역할을 하는 섹션은 3가지가 있다.

**1. 실행 가능한 코드가 들어있는 .text 섹션**
.text 섹션은 작성한 main()이나 함수의 실제 코드가 저장되는 영역이다.
프로그램이 실행되면 코드를 수정할 일이 거의 없으므로 읽기전용(Read-Only) 속성을 가진다.

**2. 초기화 된 데이터가 들어있는 .data 섹션**
.data 섹션은 초기화된 전역 변수(Global Variable) 혹은 0이 아닌 값으로 정적 변수(Static Variable)등을 포함하며, 데이터를 저장하는 섹션이기 때문에 일반적으로 읽기/쓰기 속성을 가진다.

**3. 세 번째 초기화되지않은 데이터가 들어 있는 .bss 섹션**
.bss 섹션에 포함되는 데이터는 .data에 포함되는 데이터와 거의 같으나 초기화가 되지 않은 변수만 포함한다는 거싱 차이점이다.
.bss 섹션은 0으로 초기화될 뿐, 실제 데이터가 없으므로 실행 파일이나 오브젝트 파일 상에는 별도의 영역을 차지하지 ㅏㄴㅎ는다.
하지만, 메모리에 로딩되었을때 코드는 해당 여역 변수들의 초깃값이 0이락도 가정한다.
따라서 정상적으로 프로그램을 실행하려면 메모리에 로딩할때, .bss 여역을 모두 0으로 초기화해야한다.

소스코드를 컴파일하여 생성한 오브젝트 파일은 각 섹션의 크기와 파일 내에 있는 오프셋 정보만 들어있다.
오브젝트 파일은 중간 단계의 생성물로, 다른 오브젝트 파일과 합쳐지기 때문이다.
합쳐지는 순서에 다라서 섹션의 어드레스는 얼마든지 변경될 수 있다.

오브젝트 파일들을 결합하여 정리하고 실제 메모리에 로딩될 위치를 결정하는 것이 바로 링커(Linker)이며, 이러한 과정을 링크(Link) 또는 링킹(Linking)이라고 부른다.
아래 그림은 이러한 링크 과정을 그림으로 나타낸것이다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch7_img2.jpg)

링커의 주된 역할은 오브젝트 파일 모아 섹션을 통합하고 그에 따라 어드레스를 조정하며, 외부 라이브러리에 있는 함수를 연결해주는 것이다.
하지만, 두드리기만 하면 금은보화가 나오는 요술 방망이가 아니므로 링커가 실행파일을 만들려면 파일 구성에 대한 정보가 필요하다.
바로 이때 사용하느것이 **링커 스크립트(Linker Script)**이다.

링커 스크립트에는 각 섹션의 배치 순서와 시작 어드레스, 섹션 크기 정렬 등의 정보를 저장해 놓은 텍스트 형태의 파일로 다음과 같이 굉장히 복잡한 내용이 담겨 있다.
아래의 링커 스크립트를 보면 세부적인 내용까지는 알 수 없지만, 낯익은 .text와 .data, .bss가 보이는 것을 봐서 각 섹션에 대한 정보를 나타낸다는 것을 알 수 있다.

``` x
/* Default linker script, for normal executables */
OUTPUT_FORMAT("elf32-i386", "elf32-i386",
	      "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
SEARCH_DIR("/opt/cross/i386-pc-linux/lib32"); SEARCH_DIR("/opt/cross/i386-pc-linux/lib");
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  PROVIDE (__executable_start = SEGMENT_START("text-segment", 0x08048000)); . = SEGMENT_START("text-segment", 0x08048000) + SIZEOF_HEADERS;
  .interp         : { *(.interp) }
  .note.gnu.build-id : { *(.note.gnu.build-id) }
  .hash           : { *(.hash) }
  .gnu.hash       : { *(.gnu.hash) }
  .dynsym         : { *(.dynsym) }
  .dynstr         : { *(.dynstr) }
  .gnu.version    : { *(.gnu.version) }
  .gnu.version_d  : { *(.gnu.version_d) }
  .gnu.version_r  : { *(.gnu.version_r) }
  .rel.init       : { *(.rel.init) }
  .rel.text       : { *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*) }
  .rel.fini       : { *(.rel.fini) }
  .rel.rodata     : { *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*) }
  .rel.data.rel.ro   : { *(.rel.data.rel.ro .rel.data.rel.ro.* .rel.gnu.linkonce.d.rel.ro.*) }
  .rel.data       : { *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*) }
  .rel.tdata	  : { *(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*) }
  .rel.tbss	  : { *(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*) }
  .rel.ctors      : { *(.rel.ctors) }
  .rel.dtors      : { *(.rel.dtors) }
  .rel.got        : { *(.rel.got) }
  .rel.bss        : { *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*) }
  .rel.ifunc      : { *(.rel.ifunc) }
  .rel.plt        :
    {
      *(.rel.plt)
      PROVIDE_HIDDEN (__rel_iplt_start = .);
      *(.rel.iplt)
      PROVIDE_HIDDEN (__rel_iplt_end = .);
    }
  .init           :
  {
    KEEP (*(SORT_NONE(.init)))
  }
  .plt            : { *(.plt) *(.iplt) }
  .text           :
  {
    *(.text.unlikely .text.*_unlikely .text.unlikely.*)
    *(.text.exit .text.exit.*)
    *(.text.startup .text.startup.*)
    *(.text.hot .text.hot.*)
    *(.text .stub .text.* .gnu.linkonce.t.*)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
  }
  .fini           :
  {
    KEEP (*(SORT_NONE(.fini)))
  }
  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);
  .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
  .rodata1        : { *(.rodata1) }
  .eh_frame_hdr : { *(.eh_frame_hdr) }
  .eh_frame       : ONLY_IF_RO { KEEP (*(.eh_frame)) }
  .gcc_except_table   : ONLY_IF_RO { *(.gcc_except_table
  .gcc_except_table.*) }
  /* These sections are generated by the Sun/Oracle C++ compiler.  */
  .exception_ranges   : ONLY_IF_RO { *(.exception_ranges
  .exception_ranges*) }
  /* Adjust the address for the data segment.  We want to adjust up to
     the same address within the page on the next page up.  */
  . = ALIGN (CONSTANT (MAXPAGESIZE)) - ((CONSTANT (MAXPAGESIZE) - .) & (CONSTANT (MAXPAGESIZE) - 1)); . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
  /* Exception handling  */
  .eh_frame       : ONLY_IF_RW { KEEP (*(.eh_frame)) }
  .gcc_except_table   : ONLY_IF_RW { *(.gcc_except_table .gcc_except_table.*) }
  .exception_ranges   : ONLY_IF_RW { *(.exception_ranges .exception_ranges*) }
  /* Thread Local Storage sections  */
  .tdata	  : { *(.tdata .tdata.* .gnu.linkonce.td.*) }
  .tbss		  : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  }
  .init_array     :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array ))
    PROVIDE_HIDDEN (__init_array_end = .);
  }
  .fini_array     :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array ))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }
  .ctors          :
  {
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  }
  .dtors          :
  {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  }
  .jcr            : { KEEP (*(.jcr)) }
  .data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro .data.rel.ro.* .gnu.linkonce.d.rel.ro.*) }
  .dynamic        : { *(.dynamic) }
  .got            : { *(.got) *(.igot) }
  . = DATA_SEGMENT_RELRO_END (SIZEOF (.got.plt) >= 12 ? 12 : 0, .);
  .got.plt        : { *(.got.plt)  *(.igot.plt) }
  .data           :
  {
    *(.data .data.* .gnu.linkonce.d.*)
    SORT(CONSTRUCTORS)
  }
  .data1          : { *(.data1) }
  _edata = .; PROVIDE (edata = .);
  . = .;
  __bss_start = .;
  .bss            :
  {
   *(.dynbss)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections.
      FIXME: Why do we need it? When there is no .bss section, we don't
      pad the .data section.  */
   . = ALIGN(. != 0 ? 32 / 8 : 1);
  }
  . = ALIGN(32 / 8);
  . = SEGMENT_START("ldata-segment", .);
  . = ALIGN(32 / 8);
  _end = .; PROVIDE (end = .);
  . = DATA_SEGMENT_END (.);
  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end ) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* DWARF 3 */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }
  /* DWARF Extension.  */
  .debug_macro    0 : { *(.debug_macro) }
  .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }
}
```
링커 스크립트의 사용법에 대해 자세히 다루기에는 내용이 너무 많다.
다행히 기존 링크 스크립트에서 섹션 크기 정렬에 대한 내용만 수정하면 된다.
따라서 섹션을 배치하고 정렬하는데 필요한 기본적인 내용만 간단히 알아보겠다.

[GCC 크로스 컴파일러](https://github.com/HIPERCUBE/64bit-Multicore-OS/tree/master/util/CrossCompiler)을 열어보면, 아래와 같은 구조가 반복되는 것을 알 수 있다.
링커 스크립터의 구조를 아래에 표시된 기본 형식에 대입해보면 SectionName과 그 내부 오브젝트 파일에서 통합할 섹션의 이름과 정렬할 기준값, 그리고 섹션의 초깃값을 쉽게 찾을 수 있다.

``` x
SECTIONS
{
  SectionName Load Address  : ; 섹션 이름과 메모리에 로드할 어드레스
  {
    *(.text)                  ; 오브젝트 파일의 섹션 중에 SectionName에 통합할 섹션 이름
    ; ~ 생략 ~
    . = ALIGN (AlignValue)    ; 현재 어드레스를 Align Value에 맞추어 이동 다음 섹션의 시작은
  } = 0x00000000              ; 섹션을 채울 기본 값
  ; ~ 생략 ~
}
```

이제 위의 내용을 이용하여 GCC를 크로스 컴파일한 후 생성된 32비트용 링커 스크립트 파일을 정리해 보겠다.
32비트용 링커 스크립트 파일은 [CrossCompiler/x86_64-pc-linux/lib/ldscripts/elf_i386.x](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/util/CrossCompiler/x86_64-pc-linux/lib/ldscripts/elf_i386.x)이다.
이 파일을 [01.Kernel32/elf_i386.x](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/MINT64/01.Kernel32/elf_i386.x)라는 이름으로 저장하여 재배치 작업을 준비한다.

섹션의 재배치는 텍스트나 데이터와 관계없는 섹션(.tdata, .tbss, .ctors, .got 등)의 기본 구조, 즉 'SectionName{...}' 부분 전체를 코드 및 데이터 섹션의 뒷부분ㅇ로 이동하거나, 코드 및 데이터에 관련된 섹션(.text, .data, .bss, .rodata)을 가장 앞으로 이동하는 것이 수월하므로 관련된 섹션을 링커 스크립트의 가장 앞쪽으로 이동하겠다.
섹션 크기 정렬 부분은 `ALIGN()` 부분의 값을 수정함으로써 변경하 ㄹ수 있다.
크기 정렬 값은 임의 값으로 설정해도 괜찮지만, 편의상 데이터 섹션의 시작을 섹터 크기(512바이트)에 맞추겠다.
이후에 커널의 공간이 부족하다면 이 값을 더 작게 줄임으로써 보호 모드 커널이 차지하는 비중을 줄일 수 있다.

아래는 섹션 배치 및 섹션 정렬이 사용된 링커 스크립트 파일의 내용이다.
직접 수정하기가 부답된다면 [01.Kernel32/elf_i386.x](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/MINT64/01.Kernel32/elf_i386.x)이 파일을 가져다 사용하면 된다.

``` x
/* Default linker script, for normal executables */
OUTPUT_FORMAT("elf32-i386", "elf32-i386",
	      "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
SEARCH_DIR("/usr/cross/x86_64-pc-linux/lib");
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  PROVIDE (__executable_start = 0x08048000); . = 0x08048000 + SIZEOF_HEADERS;
/*********************************************************************************/
/*  섹션 재배치로 인해 앞으로 이동된 부분 */
  .text 0x10200          :
  {
    *(.text .stub .text.* .gnu.linkonce.t.*)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
  } =0x90909090

  .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
  .rodata1        : { *(.rodata1) }

  /* 데이터 영역의 시작을 섹터 단위로 맞춤 */
  . = ALIGN (512);

  .data           :
  {
    *(.data .data.* .gnu.linkonce.d.*)
    SORT(CONSTRUCTORS)
  }
  .data1          : { *(.data1) }

  __bss_start = .;
  .bss            :
  {
   *(.dynbss)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections.
      FIXME: Why do we need it? When there is no .bss section, we don't
      pad the .data section.  */
   . = ALIGN(. != 0 ? 32 / 8 : 1);
  }
  . = ALIGN(32 / 8);
  . = ALIGN(32 / 8);
  _end = .; PROVIDE (end = .);
/*********************************************************************************/

  .interp         : { *(.interp) }
  .note.gnu.build-id : { *(.note.gnu.build-id) }
  .hash           : { *(.hash) }
  .gnu.hash       : { *(.gnu.hash) }
  .dynsym         : { *(.dynsym) }
  .dynstr         : { *(.dynstr) }
  .gnu.version    : { *(.gnu.version) }
  .gnu.version_d  : { *(.gnu.version_d) }
  .gnu.version_r  : { *(.gnu.version_r) }
  .rel.init       : { *(.rel.init) }
  .rela.init      : { *(.rela.init) }
  .rel.text       : { *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*) }
  .rela.text      : { *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*) }
  .rel.fini       : { *(.rel.fini) }
  .rela.fini      : { *(.rela.fini) }
  .rel.rodata     : { *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*) }
  .rela.rodata    : { *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*) }
  .rel.data.rel.ro   : { *(.rel.data.rel.ro* .rel.gnu.linkonce.d.rel.ro.*) }
  .rela.data.rel.ro   : { *(.rela.data.rel.ro* .rela.gnu.linkonce.d.rel.ro.*) }
  .rel.data       : { *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*) }
  .rela.data      : { *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*) }
  .rel.tdata	  : { *(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*) }
  .rela.tdata	  : { *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*) }
  .rel.tbss	  : { *(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*) }
  .rela.tbss	  : { *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*) }
  .rel.ctors      : { *(.rel.ctors) }
  .rela.ctors     : { *(.rela.ctors) }
  .rel.dtors      : { *(.rel.dtors) }
  .rela.dtors     : { *(.rela.dtors) }
  .rel.got        : { *(.rel.got) }
  .rela.got       : { *(.rela.got) }
  .rel.bss        : { *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*) }
  .rela.bss       : { *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*) }
  .rel.plt        : { *(.rel.plt) }
  .rela.plt       : { *(.rela.plt) }
  .init           :
  {
    KEEP (*(.init))
  } =0x90909090
  .plt            : { *(.plt) }
  .fini           :
  {
    KEEP (*(.fini))
  } =0x90909090
  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  }
  .init_array     :
  {
     PROVIDE_HIDDEN (__init_array_start = .);
     KEEP (*(SORT(.init_array.*)))
     KEEP (*(.init_array))
     PROVIDE_HIDDEN (__init_array_end = .);
  }
  .fini_array     :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }

/*********************************************************************************/
/* 섹션 재배치로 인해 이동된 부분 */
  _edata = .; PROVIDE (edata = .);

  /* Thread Local Storage sections  */
  .tdata	  : { *(.tdata .tdata.* .gnu.linkonce.td.*) }
  .tbss		  : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
/*********************************************************************************/
  .ctors          :
  {
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  }
  .dtors          :
  {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  }
  .jcr            : { KEEP (*(.jcr)) }
  .data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro* .gnu.linkonce.d.rel.ro.*) }
  .dynamic        : { *(.dynamic) }
  .got            : { *(.got) }

  .got.plt        : { *(.got.plt) }
  .eh_frame_hdr : { *(.eh_frame_hdr) }
  .eh_frame       : ONLY_IF_RO { KEEP (*(.eh_frame)) }
  /* Exception handling  */
  .gcc_except_table   : ONLY_IF_RO { *(.gcc_except_table .gcc_except_table.*) }
  .eh_frame       : ONLY_IF_RW { KEEP (*(.eh_frame)) }
  .gcc_except_table   : ONLY_IF_RW { *(.gcc_except_table .gcc_except_table.*) }

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* DWARF 3 */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }
  .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) }
}
```

이렇게 수정한 링커 스크립트를 이용해서 라이브러리를 사용하지 않고 실행 파일을 생성하는 방법은 아래와 같다.
CrossCompiler안에 있는 [CrossCompiler/bin/x86_64-pc-linux-ld](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/util/CrossCompiler/bin/x86_64-pc-linux-ld)을 사용해서 빌드한다.

```
> ./64bit-Multicore-OS/util/CrossCompiler/bin/x86_64-pc-linux-ld -melf_i386 -T elf_i386.x -nostdlib Main.o -o Main.elf
```

| 명령어 | 설명 |
| :--: | :-- |
| -melf_i386 | 기본적으로 64비트 코드를 생성하므로 32비트 실행 파일을 만들기 위해 설정한 옵션 |
| -T elf_i386.x | elf_i386.x 링커 스크립트를 이용해서 링크 수행 |
| -nostdlib | 표준 라이블러리 Standard Library 를 사용하지 않고 링크 수행 |
| -o Main.elf | 링크하여 생성할 파일 이름 |

###  로딩할 메모리 어드레스와 엔트리 포인트 지정
어셈블리어로 작성된 부트로더나 보호 모드 엔트리 포인트처럼 C 코드 역시 로딩될 메모리를 미리 예측하고 그에 맞춰 이미지를 생성하는 것이 중요하다.
만약 이미지를 로딩할 어드레스에 맞춰서 생성하지 않는다면 전역변수와 같이 선형 어드레스를 직접 참조하는 코드는 모두 잘못된 어드레스에 접근하기 때문이다.

메모리에 로딩하는 어드레스를 지정하는 링커 스크립트를 수정하는 방법과 링커(LD) 프로그램의 명령줄(Command Line) 옵션으로 지정하는 방식 2가지가 있다.
링커 스크립트를 통해 수정하려면 스크립트 파일의 '.text' 섹션을 아래와 같이 수정한다. '.text' 섹션의 어드레스를 수정하면 그 이후에 있는 '.data'와 '.bss'같은 섹션은 자동으로 '.text'가 로딩되는 어드레스 이후로 계산되므로 다른 섹션들은 수정하지 않아도 된다.
보호 모드 커널은 부트 로더에 의해 0x10000에 로디오디며, 0x10000의 어드레스에는 512 바이트 크기의 보호 모드 엔트리 포인트(EntryPoint.s) 코드가 있으니 C코드는 0x10200 어드레스 부터 시작할 것이다.
아래 그림은 보호 모드 엔트리 포인트와 C언어 커널이 결합된 이미지가 로딩된 메모리의 배치를 나타낸 것이다.

![](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/img/Ch7_img3.jpg)

링커 스크립트를 수정해서 로딩할 메모리 어드레스를 지정하려면 이렇게 수정해야한다.

``` x
.text 0x10200 :     ;.text 섹션을 0x10200에 로딩하도록 지정
{
  ; ~ 생략 ~
}
```

링커 스크립트를 사용하지 않고, 커맨트 라인 옵션으로 지정하는 방법은 아래와 같다.

```
> ./64bit-Multicore-OS/util/CrossCompiler/bin/x86_64-pc-linux-ld -Ttext 0x10200 Main.o -o Main.elf
```

엔트리 포인트 역시 링커 스크립트 또는 커맨드 라인 옵션으로 지정할 수 있다.
링크 스크립트를 통해 지정하려면 스크립트 파일의 상단에 있는 `ENTRY()` 부분을 다음과 같이 수정한다.

``` x
; ~ 생략 ~

OUTPUT_ARCH(i386)
ENTRY(Main)
SEARCH_DIR("/usr/cross/x86_64-pc-linux/lib");

; ~ 생략 ~
```

커맨드 라인 옵션으로 엔트리 포인트를 지정하는 방법은 아래와 같다.

```
> ./64bit-Multicore-OS/util/CrossCompiler/bin/x86_64-pc-linux-ld -e Main Main.o -o Main.elf
```

사실 엔트리 포인트를 링커에 지정하는 작업은 빌드의 결과물이 OS에 의해 실행 가능한 파일 포맷(리눅스의 elf 파일 포맷, 윈도우의 PE 파일 포맷 등)일때만 의미가 있다.
실행 파일을 바이너리 형태로 변환하는 MINT64 OS의 경우는 엔트리 포인트 정보가 제거되므로 엔트리 포인트는 큰 의미가 없으며, 단순히 링크 시에 발행하는 경고(Warning)를 피하려고 설정한 것이다.
하지만 앞서 설명햇듯이 0x10000 어드레스에 존재하는 보호 모드 엔트리 포인트는 0x10200 어드레스로 이동(jmp)하므로, C 코드의 엔트리 포인트를 해당 어드레스에 강제로 위치시킬 필요가 없다.

그럼 어떻게 해야 특정 함수를 가장 앞쪽에 위치시킬 수 있을까?
특정 함수를 실행 파일의 가장 앞쪽에 두려면 두가지 순서를 조작해야 한다.

**1. 오브젝트 파일 내의 함수간의 순서**
오브젝트 파일은 소스 파일로부터 생성되고, 컴파일러는 특별한 옵션이 없는 한 소스파일에 정의된 함수의 순서대로 오브젝트 파일의 내용을 생성한다.
따라서 C 소스 파일을 수정하여 엔트리 포인트 함수르 가장 상위로 옮겨줌으로써 오브젝트 파일에 포함된 함수의 순서를 변경할 수 있다.

**2. 실행 파일 내의 함수간의 순서**
컴파일러와 마찬가지로 실행 파일은 오브젝트 파일로 부터 실행되고, 링커는 특별한 옵션이 없는 한 입력으로 주어진 오브젝트 파일의 순서대로 통합하여 실행 파일을 생성한다.
따라서 엔트리 포인트가 포함된 오브젝트 파일을 가장 앞쪽으로 옮겨줌으로써 C코드의 엔트리 포인트르 0x10200에 위치시킬 수 있다.

소스 파일 내의 함수 위치와 오브제긑 파일의 순서를 변경하는 방법을 알았으니, 나머지는 실제 코드를 통해 살펴보기로 하고, 다음 섹션으로 넘어가서 실행 파일을 바이너닐 파일로 변경하는 방법에 대해 살펴보도록 하겠다.

### 실행 파일을 바이너리 파일로 변환
컴파일과 링크 과정을 거쳐 생성된 실행 파일은 코드 섹션과 데이터 섹션 이외의 정보를 포함하므로 이를 제거하여 부트로더나 보호 모드 엔트리 포인트와 같이 순수한 바이너리 파일 형태로 변환해야 한다.
따라서 실행 파일에서 불필요한 섹션을 제외하고 꼭 필요한 코드 섹션과 데이터 섹션만 추출해야 하는데, objcopy 프로그램을 사용하면 이러한 작업을 손쉽게 처리할 수 있다.
objcopy프로그램은 [64bit-Multicore-OS/util/CrossCompiler/bin/x86_64-pc-linux-objcopy](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/util/CrossCompiler/bin/x86_64-pc-linux-objcopy)에 위치한다.

objcopy는 실행 파일 또는 오브젝트 파일을 다른 포맷으로 변환하거나 특정 섹션을 추출하여 파일로 생성해주는 프로그램으로 binutils에 포함되어 있다.
objcopy는 옵션이 굉장히 많지만 섹션을 추출하여 바이너리로 바꾸는 작업만 수행하므로 -j,-S, -O 옵션에 대해서만 알아보겠다.

`-j` 옵션은 실행 파일에서 해당 섹션만 추출하는 옵션이며, `.text` 섹션만 추출하려면 `-j .text`와 같이 사용하면 된다.

`-S` 옵션은 실행 파일에서 재배치 정보와 심볼을 제거하는 옵션이며, C언어 커널은 함수 이름이나 변수 이름을 사용할 일이 없으므로 제거한다.

`-O` 옵션은 새로 생성할 파일의 포맷을 지정하는 옵션이며, 실행 파일을 바이너리 파일 포맷으로 변환하려면 `-O binary`처럼 사용한다.
다음은 Kernel32.elf파일에서 코드 섹션과 데이터에 관련된 섹션만 추출하여 바이너리 형태의 Kernel32.bin 파일을 만드는 예를 나타낸 것이다.

```
> ./64bit-Multicore-OS/util/CrossCompiler/bin/x86_64-pc-linux-objcopy -j .text -j .data -j .rodata -j .bss -S binary Kernel32.elf Kernel32.bin
```


## C 소스 파일 추가와 보호 모드 엔트리 포인트 통합

### C 소스 파일 추가
C 커널의 엔트리 포인트가 될 Main.c 소스 파일을 생성하기에 앞서, 여러 소스 파일에서 공통으로 사용할 헤더 파일부터 생서하겠다.
이 헤더 파일은 보호 모드 커널 전반에 걸쳐 사용할 것으로, 기본 데이터 타입과 자료구조를 정의하는데 사용한다.
헤더 파일 생성을 위해 01.Kernel32 하위에있는 Source 디렉터리에 [Types.h](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/MINT64/01.Kernel32/Source/Types.h)파일을 만들고 아래와 같이 입력한다.

``` C
#ifndef __TYPES_H__
#define __TYPES_H__


#define BYTE    unsigned char
#define WORD    unsigned short
#define DWORD   unsigned int
#define QWORD   unsigned long
#define BOOL    unsigned char

#define TRUE    1
#define FALSE   0
#define NULL    0


#pragma pack(push, 1)


// 비디오 모드 중 텍스트 모드 화면을 구성하는 자료구조
typedef struct kCharactorStruct {
    BYTE bCharactor;
    BYTE bAttribute;
} CHARACTER;

#pragma pack(pop)
#endif /* __TYPES_H__ */
```

**CHARACTER**타입은 텍스트 모드 화면을 구성하는 문자 하나를 나타내는 구조체로 텍스트 모드용 비디오 메모리(0xB8000)에 문자를 편하게 출력할 목적으로 추가했다.
`#pragma pack`은 구조체의 크기정렬(Size Align)에 관련된 지시어(Directive)로 구조체의 크기를 1바이트로 정렬하여 추가적인 메모리 공간을 더 할당하지 않게 한다.

공용 헤더 파일을 추가했으니, C 코드 엔트리 포인트 파일을 생성할 차례이다.
마찬기지로 01.Kernel의 Source 디렉터리에 [Main.c](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/MINT64/01.Kernel32/Source/Main.c)파일을 추가하고, 아래와 같이 입력한다.

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

    pstScreen += (iY * 80) + iX;
    for (int i = 0; pcString[i] != 0; i++) {
        pstScreen[i].bCharactor = pcString[i];
    }
}
```

`Main()` 함수는 C코드의 엔트리 포인트 함수로써 0x10200 어드레스에 위치하며, 6장에서 작성한 보호 모드 엔트리 포인트코드에서 최초로 실행되는 코드이다.
코드를 보혐 `Main()` 함수를 가장 앞쪽으로 위치시켜, 컴파일 시에 코드 섹션의 가장 앞쪽에 위치하게 한 것을 알 수 있다.
`Main()`함수의 내부는 `kPrintString()`함수를 사용해서 메시지를 표시하고 무한 루프를 수행하게 작성되었다.

### 보호 모드 엔트리 포인트 코드 수정
6장에서 작성한 보호 모드 커널의 엔트리 포인트 코드 [EntryPoint.s](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/MINT64/01.Kernel32/Source/EntryPoint.s)는 화면에 보호 모드로 전환했다는 메시지를 출력하고 나서 무한 루프를 수행하도록 작성했다.
이제는 보호 모드 엔트리 포인트 이후에 C 커널 코드가 있으므로 무한 루프를 수행하는 코드를 수정하여 0x10200으로 이동하게끔 변경하겠다.
C 커널 코드로 이동하게 수정하는 일은 아주 간단하다.
리얼 모드에서 보호 모드로 전환할때처럼 CS세그먼트 셀렉터와 이동할 선형 주소를 jmp 명령에 같이 지정해주면 된다.
아래는 C커널 코드로 이동하기 위해 수정한 코드이다.

``` asm
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 보호 모드로 진입
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
[BITS 32]           ; 이하의 코드는 32비트 코드로 설정
PROTECTEDMODE:
    mov ax, 0x10    ; 보호 모드 커널용 데이터 세그먼트 디스크립터를 AX 레지스터에 저장
    mov ds, ax      ; DS 세그먼트 셀렉터에 설정
    mov es, ax      ; ES 세그먼트 셀렉터에 설정
    mov fs, ax      ; FS 세그먼트 셀렉터에 설정
    mov gs, ax      ; GS 세그먼트 셀렉터에 설정

    ; 스택을 0x00000000 ~ 0x0000FFFF 영역에 64KB 크기로 생성
    mov ss, ax      ; SS 세그먼트 셀렉터에 설정
    mov esp, 0xFFFE ; ESP 레지스터의 어드레스를 0xFFFE로 설정
    mov ebp, 0xFFFE ; EBP 레지스터의 어드레스를 0xFFFE로 설정

    ; 화면에 보호 모드로 전환되었다는 메세지 보여주기
    push ( SWITCHSUCCESSMESSAGE - $$ + 0x10000 )    ; 출력할 메시지의 어드레스를 스택에 삽입
    push 2              ; 화면 Y 좌표 2를 스택에 삽입
    push 0              ; 화면 X 좌표 0를 스택에 삽입
    call PRINTMESSAGE   ; PRINTMESSAGE 함수 호출
    add esp, 12         ; 삽입한 파라미터 제거
	
    ; 수정할 부분
    jmp dword 0x08: 0x10200 ; C언어 커널이 존재하는 0x10200 어드레스로 이동하여 C언어 커널 수행

```

### makefile 수정
다수의 파일을 컴파일하고 링크해야하므로 makefile이 좀 더 편리하게 수정할 필요가 있다.
따라서 make의 몇 가지 유용한 기능을 사용하여 Source 디렉터리에 .c 확장자의 파일만 추가하면 자동으로 포함하여 빌드하게 수정할 것이다.

.c 파일을 자동으로 빌드 목록에 추가려면, 매번 빌드 때마다 Source 디렉터리에 있는 *.c파일을 검색하여 소스 파일 목록에 추가해야한다.
make에서 이러한 작업을 위해 디렉터리에 있는 파일을 검색하는 와일드 카드 기능을 제공한다.
Source 디렉터리에 있는 *.c 파일을 모두 검색해서 CSOURCEFILES이라는 변수에 넣고 싶다면 와잉ㄹ드 카드를 사용하여 다음과 같이 입력한다.

``` makefile
CSOURCEFILES = $(wildcard Source/*.c)
```

디렉터리에 있는 모든 C파일을 검색했으니, 이제 이 파일들에 대한 빌드 룰만 정해주면 자동으로 빌드할 수 있다.
지금까지의 makefile은 각 파일에 대해 빌드 룰을 개별적으로 기술했다.
하지만 빌드에 필요한 파일이 수백개쯤 된다면 관리하기 힘들것이다.
또한 파일이 추가되고 삭제될때마다 룰을 변경해야하는데 실수하면 오류나 실행 도중 예기치 못한 오류가 발생할 수 있다.

이런한 문제는 파일 패턴에 대해 동일한 룰을 적용함으로써, 간단히 처리할 수 잇다.
가령 모든 .c 파일은 `gcc -c`라는 컴파일 과정을 통해 .o 파일로 변환된다면, 다음과 같이 써서 모든 .c 파일을 .o파일로 컴파일 할 수 있다.

``` makefile
%.o : %.c
	gcc -c $<
```

와일드 카드와 패턴 룰 기능을 이용하면 Source 디렉터리 내의 모든 C파일을 자동으로 컴파일 할 수 있다.
그럼 이제 검색된 C파일을 이용하여 링크할 파일 목록을 생성해 보도록 하겠다.
일반적으로 오브젝트 파일은 소스 파일과 같은 이름이며 확장자만 .o로 변경되므로 소스 파일 목록에 포함된 파일의 확장자를 .c에서 .o로 수정하면 된다.
특정 문자를 치환하려면 patsubst 기능을 사용하면 되고, patsubst는 `$(patsubst 수정할 패턴, 교체할 패넡, 입력 문자열)` 의 형식으로 사용한다.
CSOURCEFILES의 내용에서 확장자 .c를 .o로 수정하고 싶다면 다음과 같이 사용하면 된다.

``` makefile
COBJECTFILES = $(patsubst %.c, %.o, $(CSOURCEFILES))
```

이게 끝이 아니다.
우리는 C 커널 엔트리 포인트 함수를 가장 앞쪽에 배치하려면 엔트리 포인트 파일을 COBJECTFILES의 맨 앞에 둬야 한다.
만일 C 커널의 엔틜 포인트를 포함하는 오브젝트 파일 이름이 Main.o 라고 가정한다면, Main.o 파일을 COBJECTFILES에서 맨 앞에 두려면 다음과 같이 subst를 사용한다.

