#pragma once

#include "../stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <String.h>


#define  TITLE   "DiskId32"


//  Required to ensure correct PhysicalDrive IOCTL structure setup
#pragma pack(1)

//  Max number of drives assuming primary/secondary, master/slave topology
#define  MAX_IDE_DRIVES  4
#define  IDENTIFY_BUFFER_SIZE  512
//
#define  m_cVxDFunctionIdesDInfo  1
#define  SENDIDLENGTH  sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE


//  IOCTL commands
#define  DFP_GET_VERSION          0x00074080
#define  DFP_SEND_DRIVE_COMMAND   0x0007c084
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088

#define  FILE_DEVICE_SCSI              0x0000001b
#define  IOCTL_SCSI_MINIPORT_IDENTIFY  ((FILE_DEVICE_SCSI << 16) + 0x0501)
#define  IOCTL_SCSI_MINIPORT 0x0004D008  //  see NTDDSCSI.H for definition



//  GETVERSIONOUTPARAMS contains the data returned from the
//  Get Driver Version function.
typedef struct _GETVERSIONOUTPARAMS
{
    BYTE bVersion;      // Binary driver version.
    BYTE bRevision;     // Binary driver revision.
    BYTE bReserved;     // Not used.
    BYTE bIDEDeviceMap; // Bit map of IDE devices.
    DWORD fCapabilities; // Bit mask of driver capabilities.
    DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;


//  Bits returned in the fCapabilities member of GETVERSIONOUTPARAMS
#define  CAP_IDE_ID_FUNCTION             1  // ATA ID command supported
#define  CAP_IDE_ATAPI_ID                2  // ATAPI ID command supported
#define  CAP_IDE_EXECUTE_SMART_FUNCTION  4  // SMART commannds supported


//  IDE registers
typedef struct _IDEREGS
{
    BYTE bFeaturesReg;       // Used for specifying SMART "commands".
    BYTE bSectorCountReg;    // IDE sector count register
    BYTE bSectorNumberReg;   // IDE sector number register
    BYTE bCylLowReg;         // IDE low order cylinder value
    BYTE bCylHighReg;        // IDE high order cylinder value
    BYTE bDriveHeadReg;      // IDE drive/head register
    BYTE bCommandReg;        // Actual IDE command.
    BYTE bReserved;          // reserved for future use.  Must be zero.
} IDEREGS, *PIDEREGS, *LPIDEREGS;


//  SENDCMDINPARAMS contains the input parameters for the
//  Send Command to Drive function.
typedef struct _SENDCMDINPARAMS
{
    DWORD     cBufferSize;   //  Buffer size in bytes
    IDEREGS   irDriveRegs;   //  Structure with drive register values.
    BYTE bDriveNumber;       //  Physical drive number to send
    //  command to (0,1,2,3).
    BYTE bReserved[3];       //  Reserved for future expansion.
    DWORD     dwReserved[4]; //  For future use.
    BYTE      bBuffer[1];    //  Input buffer.
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;


//  Valid values for the bCommandReg member of IDEREGS.
#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.


// Status returned from driver
typedef struct _DRIVERSTATUS
{
    BYTE  bDriverError;  //  Error code from driver, or 0 if no error.
    BYTE  bIDEStatus;    //  Contents of IDE Error register.
    //  Only valid when bDriverError is SMART_IDE_ERROR.
    BYTE  bReserved[2];  //  Reserved for future expansion.
    DWORD  dwReserved[2];  //  Reserved for future expansion.
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;


// Structure returned by PhysicalDrive IOCTL for several commands
typedef struct _SENDCMDOUTPARAMS
{
    DWORD         cBufferSize;   //  Size of bBuffer in bytes
    DRIVERSTATUS  DriverStatus;  //  Driver status structure.
    BYTE          bBuffer[1];    //  Buffer of arbitrary length in which to store the data read from the                                                       // drive.
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;


// The following struct defines the interesting part of the IDENTIFY
// buffer:
typedef struct _IDSECTOR
{
    USHORT  wGenConfig;
    USHORT  wNumCyls;
    USHORT  wReserved;
    USHORT  wNumHeads;
    USHORT  wBytesPerTrack;
    USHORT  wBytesPerSector;
    USHORT  wSectorsPerTrack;
    USHORT  wVendorUnique[3];
    CHAR    sSerialNumber[20];
    USHORT  wBufferType;
    USHORT  wBufferSize;
    USHORT  wECCSize;
    CHAR    sFirmwareRev[8];
    CHAR    sModelNumber[40];
    USHORT  wMoreVendorUnique;
    USHORT  wDoubleWordIO;
    USHORT  wCapabilities;
    USHORT  wReserved1;
    USHORT  wPIOTiming;
    USHORT  wDMATiming;
    USHORT  wBS;
    USHORT  wNumCurrentCyls;
    USHORT  wNumCurrentHeads;
    USHORT  wNumCurrentSectorsPerTrack;
    ULONG   ulCurrentSectorCapacity;
    USHORT  wMultSectorStuff;
    ULONG   ulTotalAddressableSectors;
    USHORT  wSingleWordDMA;
    USHORT  wMultiWordDMA;
    BYTE    bReserved[128];
} IDSECTOR, *PIDSECTOR;


typedef struct _SRB_IO_CONTROL
{
    ULONG HeaderLength;
    UCHAR Signature[8];
    ULONG Timeout;
    ULONG ControlCode;
    ULONG ReturnCode;
    ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;

//  ---------------------------------------------------

// (* Output Bbuffer for the VxD (rt_IdeDinfo record) *)
typedef struct _rt_IdeDInfo_
{
    BYTE IDEExists[4];
    BYTE DiskExists[8];
    WORD DisksRawInfo[8 * 256];
} rt_IdeDInfo, *pt_IdeDInfo;


// (* IdeDinfo "data fields" *)
typedef struct _rt_DiskInfo_
{
    BOOL DiskExists;
    BOOL ATAdevice;
    BOOL RemovableDevice;
    WORD TotLogCyl;
    WORD TotLogHeads;
    WORD TotLogSPT;
    char SerialNumber[20];
    char FirmwareRevision[8];
    char ModelNumber[40];
    WORD CurLogCyl;
    WORD CurLogHeads;
    WORD CurLogSPT;
} rt_DiskInfo;

class CGetHardDiskSN
{
    public:
        CGetHardDiskSN(void);
        ~CGetHardDiskSN(void);

        char HardDriveSerialNumber [1024];
        CString str_DN_Modol;
        CString str_DN_Serial;
        CString str_DN_ControllerRevision;
        CString str_HardDeskSize;
        CString str_HardDeskBufferSize;
        CString str_HardDeskType;
        CString str_HardDesk_Form;
        CString str_Controller;
        
    protected:
        //conversion to char string
        char *ConvertToString(DWORD diskdata [256], int firstIndex, int lastIndex);
        
        BOOL DoIDENTIFY(HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
                        PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum,
                        PDWORD lpcbBytesReturned);
        int ReadPhysicalDriveInNT(void);
        int ReadDrivePortsInWin9X(void);
        int ReadIdeDriveAsScsiDriveInNT(void);
        BOOL ReturnInfo(int drive, DWORD diskdata [256]);
        
};

// IDE的ID命令返回的数据
// 共512字节(256个WORD)，这里仅定义了一些感兴趣的项(基本上依据ATA/ATAPI-4)
typedef struct _IDINFO
{
    USHORT  wGenConfig;                 // WORD 0: 基本信息字
    USHORT  wNumCyls;                   // WORD 1: 柱面数
    USHORT  wReserved2;                 // WORD 2: 保留
    USHORT  wNumHeads;                  // WORD 3: 磁头数
    USHORT  wReserved4;                 // WORD 4: 保留
    USHORT  wReserved5;                 // WORD 5: 保留
    USHORT  wNumSectorsPerTrack;        // WORD 6: 每磁道扇区数
    USHORT  wVendorUnique[3];           // WORD 7-9: 厂家设定值
    CHAR    sSerialNumber[20];          // WORD 10-19:序列号
    USHORT  wBufferType;                // WORD 20: 缓冲类型
    USHORT  wBufferSize;                // WORD 21: 缓冲大小
    USHORT  wECCSize;                   // WORD 22: ECC校验大小
    CHAR    sFirmwareRev[8];            // WORD 23-26: 固件版本
    CHAR    sModelNumber[40];           // WORD 27-46: 内部型号
    USHORT  wMoreVendorUnique;          // WORD 47: 厂家设定值
    USHORT  wReserved48;                // WORD 48: 保留
    struct
    {
        USHORT  reserved1: 8;
        USHORT  DMA: 1;                 // 1=支持DMA
        USHORT  LBA: 1;                 // 1=支持LBA
        USHORT  DisIORDY: 1;            // 1=可不使用IORDY
        USHORT  IORDY: 1;               // 1=支持IORDY
        USHORT  SoftReset: 1;           // 1=需要ATA软启动
        USHORT  Overlap: 1;             // 1=支持重叠操作
        USHORT  Queue: 1;               // 1=支持命令队列
        USHORT  InlDMA: 1;              // 1=支持交叉存取DMA
    } wCapabilities;                    // WORD 49: 一般能力
    USHORT  wReserved1;                 // WORD 50: 保留
    USHORT  wPIOTiming;                 // WORD 51: PIO时序
    USHORT  wDMATiming;                 // WORD 52: DMA时序
    struct
    {
        USHORT  CHSNumber: 1;           // 1=WORD 54-58有效
        USHORT  CycleNumber: 1;         // 1=WORD 64-70有效
        USHORT  UnltraDMA: 1;           // 1=WORD 88有效
        USHORT  reserved: 13;
    } wFieldValidity;                   // WORD 53: 后续字段有效性标志
    USHORT  wNumCurCyls;                // WORD 54: CHS可寻址的柱面数
    USHORT  wNumCurHeads;               // WORD 55: CHS可寻址的磁头数
    USHORT  wNumCurSectorsPerTrack;     // WORD 56: CHS可寻址每磁道扇区数
    USHORT  wCurSectorsLow;             // WORD 57: CHS可寻址的扇区数低位字
    USHORT  wCurSectorsHigh;            // WORD 58: CHS可寻址的扇区数高位字
    struct
    {
        USHORT  CurNumber: 8;           // 当前一次性可读写扇区数
        USHORT  Multi: 1;               // 1=已选择多扇区读写
        USHORT  reserved1: 7;
    } wMultSectorStuff;                 // WORD 59: 多扇区读写设定
    
    ULONG  dwTotalSectors;              // WORD 60-61: LBA可寻址的扇区数
    USHORT  wSingleWordDMA;             // WORD 62: 单字节DMA支持能力
    struct
    {
        USHORT  Mode0: 1;               // 1=支持模式0 (4.17Mb/s)
        USHORT  Mode1: 1;               // 1=支持模式1 (13.3Mb/s)
        USHORT  Mode2: 1;               // 1=支持模式2 (16.7Mb/s)
        USHORT  Reserved1: 5;
        USHORT  Mode0Sel: 1;            // 1=已选择模式0
        USHORT  Mode1Sel: 1;            // 1=已选择模式1
        USHORT  Mode2Sel: 1;            // 1=已选择模式2
        USHORT  Reserved2: 5;
    } wMultiWordDMA;                    // WORD 63: 多字节DMA支持能力
    struct
    {
        USHORT  AdvPOIModes: 8;         // 支持高级POI模式数
        USHORT  reserved: 8;
    } wPIOCapacity;                     // WORD 64: 高级PIO支持能力
    USHORT  wMinMultiWordDMACycle;      // WORD 65: 多字节DMA传输周期的最小值
    USHORT  wRecMultiWordDMACycle;      // WORD 66: 多字节DMA传输周期的建议值
    USHORT  wMinPIONoFlowCycle;         // WORD 67: 无流控制时PIO传输周期的最小值
    USHORT  wMinPOIFlowCycle;           // WORD 68: 有流控制时PIO传输周期的最小值
    USHORT  wReserved69[11];            // WORD 69-79: 保留
    struct
    {
        USHORT  Reserved1: 1;
        USHORT  ATA1: 1;                // 1=支持ATA-1
        USHORT  ATA2: 1;                // 1=支持ATA-2
        USHORT  ATA3: 1;                // 1=支持ATA-3
        USHORT  ATA4: 1;                // 1=支持ATA/ATAPI-4
        USHORT  ATA5: 1;                // 1=支持ATA/ATAPI-5
        USHORT  ATA6: 1;                // 1=支持ATA/ATAPI-6
        USHORT  ATA7: 1;                // 1=支持ATA/ATAPI-7
        USHORT  ATA8: 1;                // 1=支持ATA/ATAPI-8
        USHORT  ATA9: 1;                // 1=支持ATA/ATAPI-9
        USHORT  ATA10: 1;               // 1=支持ATA/ATAPI-10
        USHORT  ATA11: 1;               // 1=支持ATA/ATAPI-11
        USHORT  ATA12: 1;               // 1=支持ATA/ATAPI-12
        USHORT  ATA13: 1;               // 1=支持ATA/ATAPI-13
        USHORT  ATA14: 1;               // 1=支持ATA/ATAPI-14
        USHORT  Reserved2: 1;
    } wMajorVersion;                    // WORD 80: 主版本
    USHORT  wMinorVersion;              // WORD 81: 副版本
    USHORT  wReserved82[6];             // WORD 82-87: 保留
    struct
    {
        USHORT  Mode0: 1;               // 1=支持模式0 (16.7Mb/s)
        USHORT  Mode1: 1;               // 1=支持模式1 (25Mb/s)
        USHORT  Mode2: 1;               // 1=支持模式2 (33Mb/s)
        USHORT  Mode3: 1;               // 1=支持模式3 (44Mb/s)
        USHORT  Mode4: 1;               // 1=支持模式4 (66Mb/s)
        USHORT  Mode5: 1;               // 1=支持模式5 (100Mb/s)
        USHORT  Mode6: 1;               // 1=支持模式6 (133Mb/s)
        USHORT  Mode7: 1;               // 1=支持模式7 (166Mb/s) ???
        USHORT  Mode0Sel: 1;            // 1=已选择模式0
        USHORT  Mode1Sel: 1;            // 1=已选择模式1
        USHORT  Mode2Sel: 1;            // 1=已选择模式2
        USHORT  Mode3Sel: 1;            // 1=已选择模式3
        USHORT  Mode4Sel: 1;            // 1=已选择模式4
        USHORT  Mode5Sel: 1;            // 1=已选择模式5
        USHORT  Mode6Sel: 1;            // 1=已选择模式6
        USHORT  Mode7Sel: 1;            // 1=已选择模式7
    } wUltraDMA;                        // WORD 88:  Ultra DMA支持能力
    USHORT    wReserved89[167];         // WORD 89-255
} IDINFO, *PIDINFO;

BOOL GetIdeDriveAsScsiInfoInNT(int nDrive, PIDINFO pIdInfo);
BOOL GetPhysicalDriveInfoInNT(int nDrive, PIDINFO pIdInfo);