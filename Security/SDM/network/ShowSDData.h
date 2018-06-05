#pragma once

#define    BUFFER_LITTLE_SIZE        512//缓存512
#define    BUFFER_SMALL_SIZE         1024//缓存1k
#define    BUFFER_BIG_SIZE           4096//缓存4k

class CShowSDData
{
    public:
        CShowSDData(void);
        virtual ~CShowSDData(void);
        CShowSDData(CListCtrl* pL, CImageList* pI);
        
        static CShowSDData* Instance(CListCtrl* pL = NULL, CImageList* pI = NULL);
        
        void ShowSDData(CString IP, CString Name, int TheState, CTime ConnTime, CTime ReTime, CString ReData);
        void DisplaySDData();
        
    private:
        //存储界面显示信息的队列
        typedef struct ShowSdData
        {    
            char IP[30];//ip:Port  ip地址和端口
            char Name[BUFFER_LITTLE_SIZE];//连接名称
            int  TheState;//当前状态:0连接/1断开
            CTime ConnTime;//连接时间
            CTime ReTime;//上次接收数据时间
            char ReData[BUFFER_SMALL_SIZE];//上次接收数据
        } ShowSdcData;

        typedef struct ShowSdDataQueue
        {
            ShowSdcData DataQueue[BUFFER_BIG_SIZE];
            DWORD Head;
            DWORD Tail;
            ShowSdDataQueue()
            {
                Head = 0;
                Tail = 0;
            }
        } ShowSdcDataQueue, *PShowSdcDataQueue;
        
        //成员变量
        PShowSdcDataQueue m_pShowQueue;//数据存储队列
        CListCtrl* m_pListCtrl;//显示列表
        CImageList* m_pImageList;//图像列表
        
        static CShowSDData* _instance;
        
        CRITICAL_SECTION m_csShowBuf;//显示内容到显示队列用临界区
        //解决单件模式对象自动释放的问题
        class Cleaner
        {
            public:
                Cleaner() {}
                ~Cleaner()
                {
                    if(CShowSDData::Instance())
                    {
                        delete CShowSDData::Instance();
                    }
                }
        };
};

