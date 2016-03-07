## 2장 개발환경 구축

### 크로스 컴파일러 만들기
이 책에서는 windows 기반으로 작성되어있다.

맥용 크로스 컴파일러는 [util/CrossCompiler](https://github.com/HIPERCUBE/64bit-Multicore-OS/tree/master/util/CrossCompiler)에 있으니 이걸 이용해서 컴파일 하면 된다.

[Chris Ohk](https://github.com/utilForever)님께서 윈도우 [윈도우 개발환경 구축](https://github.com/HIPERCUBE/64bit-Multicore-OS/blob/master/book/Windows.pdf) 방법을 공유해주셨다.
윈도우 사용자들은 위 문서를 참고해서 개발환경을 구축하면 된다.

### NASM 설치
NASM은 [util/nasm-2.11.08](https://github.com/HIPERCUBE/64bit-Multicore-OS/tree/master/util/nasm-2.11.08)에 들어있다.
제대로 작동하는지 확인해보자
```
> util/nasm-2.11.08/nasm -version
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


다른사람에 맥에서 구동시키는 방법을 정리해놓은 문서가 있어서 첨부 : [OS X 개발환경 구축](http://nayuta.net/64비트_멀티코어_OS_원리와_구조/OS_X에서_개발환경_구축하기)
