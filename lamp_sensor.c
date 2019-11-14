#include "ftd2xx.h"
#include "stdlib.h"
#include <stdio.h>
#include "spi_import.c"

#define CS D5
#define SCK D7
#define SDI D6
#define LDAC D0
#define CLK D1
#define Din D3
#define Dout D4
#define CS_SHDN D2

int SPI_set(int, FT_HANDLE *, unsigned char *, int);
int SPI_get(int, FT_HANDLE *, unsigned char *, int, int);

int main(int argc, char *argv[]){
    FT_HANDLE ftHandle[4];
    FT_STATUS ftStatus;
    DWORD numDevs;
    DWORD devIndex;
    DWORD WriteNum, TransNum;
    int devcnt, devlast;
    unsigned char buf[256];
    int id, i, j, ch;
    int distance;
    numDevs = 0;

    if (argc > 1){
        for (int i = 0; i < argc; i++)
        {
            printf("argv[%d] = %s argc %d\n", i, argv[i], argc);
        }
    }
    else {
        printf("Usage: %s number -exit.\n", argv[0]);
        exit(0);
    }

    id = atoi(argv[1]);
    printf("Data %d\n", id);
    ftStatus = FT_ListDevices(&numDevs, NULL, FT_LIST_NUMBER_ONLY);
    devlast = numDevs;

    for (devcnt = 0; devcnt != devlast; devcnt++) {
        devIndex = (DWORD)devcnt;
        ftStatus = FT_ListDevices((PVOID)devIndex, buf, FT_LIST_BY_INDEX | FT_OPEN_BY_SERIAL_NUMBER);
        if (ftStatus == FT_OK) {
            ftStatus = FT_OpenEx(buf, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle[devcnt]);
            printf("device open status %d devcnt %d\n", ftStatus, devcnt);
        }
    }
    devcnt = 0;

    ftStatus = FT_SetBitMode(ftHandle[devcnt], 0x00, FT_BITMODE_RESET);
    ftStatus = FT_SetBitMode(ftHandle[devcnt], 0xef, FT_BITMODE_ASYNC_BITBANG);

    printf("Status %d\n", ftStatus);
    ftStatus = FT_SetBaudRate(ftHandle[devcnt], 115200);
    printf("Status %d\n", ftStatus);

    printf("BitMode Set End\n");

    while (1){
        SPI_set(id, ftHandle, buf, devcnt);
        ch = 0;
        distance = SPI_get(id, ftHandle, buf, devcnt, ch);
        printf("Distance: %d\n", distance);
        id = distance;
    }
    for (devcnt = 0; devcnt != devlast; devcnt++){
        ftStatus = FT_Close(ftHandle[devcnt]);
    }
    return 0;
}


int SPI_set(int id, FT_HANDLE ftHandle[], unsigned char buf[], int devcnt){
    FT_STATUS ftStatus;
    DWORD WriteNum, TransNum;
    WriteNum = 1;
    buf[0] =  CS | LDAC;
    ftStatus = FT_Write(ftHandle[devcnt], buf, WriteNum, &TransNum);
    buf[0] = (~CS) & LDAC;
    ftStatus = FT_Write(ftHandle[devcnt], buf, WriteNum, &TransNum);

    buf[0] =   SCK  |  LDAC;
    buf[1] = (~SCK) &  LDAC;
    buf[2] =   SCK  |  LDAC;
    buf[3] = (~SCK) & (LDAC | SDI);
    buf[4] =   SCK  | (LDAC | SDI);
    buf[5] = (~SCK) & (LDAC | SDI);
    buf[6] =   SCK  | (LDAC | SDI);
    WriteNum = 7;
    ftStatus = FT_Write(ftHandle[devcnt], buf, WriteNum, &TransNum);

    WriteNum = 1;

    int j = 0x200;
    for (int i = 0; i < 12; i++){
        if ((id & j) == 0){
            buf[0] = (~SDI) & LDAC;
        }
        else {
            buf[0] =   SDI  | LDAC;
        }
        j = j >> 1;
        ftStatus = FT_Write(ftHandle[devcnt], buf, WriteNum, &TransNum);
        buf[0] = buf[0] | SCK;
        ftStatus = FT_Write(ftHandle[devcnt], buf, WriteNum, &TransNum);
    }
    buf[0] =   LDAC  | CS;
    buf[1] = (~LDAC) & CS;
    buf[2] =   LDAC  | CS;
    WriteNum = 3;
    ftStatus = FT_Write(ftHandle[devcnt], buf, WriteNum, &TransNum);
    return(ftStatus);
}


int SPI_get(int id, FT_HANDLE ftHandle[], unsigned char buf[], int devcnt, int ch){
    FT_STATUS ftStatus;
    DWORD WriteNum, TransNum;
    int i, j;
    unsigned char bd, bch;
    bch = 0x4 * ch;

    buf[0] = Din | CS_SHDN;
    buf[1] = Din;
    //Start the clock
    buf[2] = Din |   CLK;
    buf[3] = Din & (~CLK);
    buf[4] = Din |   CLK;
    buf[5] = 0;
    buf[6] = (~Din) & CLK;
    buf[7] = Din & (~CLK);
    buf[8] = Din |   CLK;
    buf[9] = Din & (~CLK);
    buf[10] = Din | CLK;
    WriteNum = 11;
    ftStatus = FT_Write(ftHandle[devcnt], buf, WriteNum, &TransNum);
    WriteNum = 1;
    j = 0;
    for (i = 0; i < 10; i++){
        j = j << 1;
        buf[0] = Din;
        ftStatus = FT_Write(ftHandle[devcnt], buf, WriteNum, &TransNum);
        buf[0] = Din | CLK;
        ftStatus = FT_Write(ftHandle[devcnt], buf, WriteNum, &TransNum);
        ftStatus = FT_GetBitMode(ftHandle[devcnt], &bd);
        if ((bd & Dout) == Dout) {
            j = j + 1;
        }
    }
    buf[0] = Din | CS_SHDN;
    ftStatus = FT_Write(ftHandle[devcnt], buf, WriteNum, &TransNum);
    return(j);
}