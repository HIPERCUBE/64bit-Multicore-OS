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