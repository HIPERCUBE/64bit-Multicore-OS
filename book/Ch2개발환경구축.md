## 2장 개발환경 구축

### 크로스 컴파일러 만들기
이 책에서는 window 기반으로 작성되어있다.

맥용 크로스 컴파일러는 [util/CrossCompiler](https://github.com/HIPERCUBE/64bit-Multicore-OS/tree/master/util/CrossCompiler)에 있으니 이걸 이용해서 컴파일 하면 된다.

### NASM 설치
NASM은 [util/nasm-2.11.08](https://github.com/HIPERCUBE/64bit-Multicore-OS/tree/master/util/nasm-2.11.08)에 들어있다.
제대로 작동하는지 확인해보자
```
> nasm -version
NASM version 0.98.40 (Apple Computer, Inc. build 11) compiled on Nov 11 2015
```

### 이클립스 설치 & 자바 런타임 환경 구축하기
이건 패스하도록 하겠다.
JDK 8, Intellij 15 를 사용해서 진행한다.

### QEMU 설치
Mac쓰는 개발자라면 당연히 Homebrew가 설치되어있으니 설치과정을 넘어가도록하겠다.
```
brew install qemu
```
간단하다.

잘 설치되었는지 확인해보도록 하자.
```
> qemu-img --v
qemu-img version 2.5.0, Copyright (c) 2004-2008 Fabrice Bellard
```
