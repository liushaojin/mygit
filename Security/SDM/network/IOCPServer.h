#pragma once
#include <winsock2.h>
#include <MSWSock.h>
#pragma comment(lib,"ws2_32.lib")


#define MAX_BUFFER_LEN        8192				// 缓冲区长度 (1024*8) 如果确实客户端发来的每组数据都比较少，那么就设置得小一些，省内存
#define DEFAULT_PORT          12345				// 默认端口
#define DEFAULT_IP            _T("127.0.0.1")	// 默认IP地址
#define PACK_SIZE            1024

// 在完成端口上投递的I/O操作的类型
typedef enum OpType
{
    AcceptPosted,                     // 标志投递的Accept操作
    SendPosted,                       // 标志投递的是发送操作
    RecvPosted,                       // 标志投递的是接收操作
    NullPosted                        // 用于初始化，无意义
} OperateType;

typedef struct PerIOContext
{
    OVERLAPPED     overLapped;						// 每一个重叠网络操作的重叠结构(针对每一个Socket的每一个操作，都要有一个)
    SOCKET         sockAccept;						// 这个网络操作所使用的Socket
    WSABUF         wsaBuf;							// WSA类型的缓冲区，用于给重叠操作传参数的
    char           szBuffer[MAX_BUFFER_LEN];		// 这个是WSABUF里具体存字符的缓冲区
    OperateType	   opType;							// 标识网络操作的类型(对应上面的枚举)
    bool           busy;							// 标识该IO操作是否处于忙碌中，true为忙绿中。主要用于发送数据的判断
    
    // 初始化
    PerIOContext()
    {
        ZeroMemory(&overLapped, sizeof(overLapped));
        ZeroMemory(szBuffer, MAX_BUFFER_LEN);
        sockAccept = INVALID_SOCKET;
        wsaBuf.buf = szBuffer;
        wsaBuf.len = MAX_BUFFER_LEN;
        opType     = NullPosted;
        busy       = false;
    }
    // 释放掉Socket
    ~PerIOContext()
    {
        if(sockAccept != INVALID_SOCKET)
        {
            closesocket(sockAccept);
            sockAccept = INVALID_SOCKET;
        }
    }
    // 重置缓冲区内容
    void ResetBuffer()
    {
        ZeroMemory(szBuffer, MAX_BUFFER_LEN);
    }
    
} PerIOContext, *PPerIOContext;

typedef struct PerSocketContext
{
    SOCKET      m_Socket;						// 每一个客户端连接的Socket
    SOCKADDR_IN m_clientAddr;					// 客户端的地址
    CArray<PerIOContext*> m_arrayIoContext;		// 客户端网络操作的上下文数据，
    // 也就是说对于每一个客户端Socket，是可以在上面同时投递多个IO请求的
    
    // 初始化
    PerSocketContext()
    {
        m_Socket = INVALID_SOCKET;
        memset(&m_clientAddr, 0, sizeof(m_clientAddr));
    }
    
    // 释放资源
    ~PerSocketContext()
    {
        if(m_Socket != INVALID_SOCKET)
        {
            closesocket(m_Socket);
            m_Socket = INVALID_SOCKET;
        }
        
        // 释放掉所有的IO上下文数据//关掉所有的IO请求
        for(int i = 0; i < m_arrayIoContext.GetCount(); i++)
        {
            delete m_arrayIoContext.GetAt(i);
        }
        
        m_arrayIoContext.RemoveAll();
    }
    
    // 获取一个新的IoContext
    PerIOContext* GetNewIoContext()
    {
        PerIOContext* p = new PerIOContext;
        m_arrayIoContext.Add(p);
        return p;
    }
    
    // 从数组中移除一个指定的IoContext
    void RemoveContext(PerIOContext* pContext)
    {
        //断言
        ASSERT(pContext != NULL);
        
        for(int i = 0; i < m_arrayIoContext.GetCount(); i++)
        {
            if(pContext == m_arrayIoContext.GetAt(i))
            {
                delete pContext;
                pContext = NULL;
                m_arrayIoContext.RemoveAt(i);
                break;
            }
        }
    }
    
} PerSocketContext, *PPerSocketContext;

// 工作者线程的线程参数
class CIOCPServer;
//工作线程都是静态函数（线程函数是静态函数）
typedef struct WorkerThreadParams
{
    CIOCPServer* pIOCPServer;                                   // 类指针，用于调用类中的函数
    int         threadNo;                                    // 线程编号
    
} WorkThreadParams, *PWorkThreadParams;

class CIOCPServer
{
    public:
        CIOCPServer(void);
        ~CIOCPServer(void);
        
    public:
        bool Start();	//开启网络服务器
        void Stop();	//停止或关闭服务器
        bool LoadSocketLib();	//加载套接字库信息
        void UnloadSocketLib() { WSACleanup(); }	//卸载套接字库信息
        void SetPort(const int& nPort) { m_nPort = nPort; }
        void SetMainDlg(CDialog* p) { m_pMain = p; }
        bool SendData(char* dwAddr, int nPort, WSABUF wsaBuf, WORD buflen);
        
        CString GetLocalIP();
		void SetServerIpPort(CString ip, int port);
        
    protected:
        bool InitializeIOCP();	//初始化端口
        bool InitializeListenSocket();	//初始化监听套接字
        bool PostAccept(PerIOContext* pAcceptIoContext);   //端口请求受理
        bool PostRecv(PerIOContext* pIoContext);
        bool PostSend(PerIOContext* pIoContext);
        bool DoAccpet(PerSocketContext* pSocketContext, PerIOContext* pIoContext);
        bool DoRecv(PerSocketContext* pSocketContext, PerIOContext* pIoContext);
        bool DoSend(PerSocketContext* pSocketContext, PerIOContext* pIoContext);
        bool HandleError(PerSocketContext *pContext, const DWORD& dwErr);
        bool AssociateWithIOCP(PerSocketContext *pContext);
        bool IsSocketAlive(SOCKET s);
        
        void AddToContextList(PerSocketContext *pSocketContext, WSABUF wsabuf);
        void RemoveContext(PerSocketContext *pSocketContext);
        void ShowMessage(const CString szFormat, ...) const;
        void ClearContextList();
        void DeInitialize();	//服务端启动错误时的，释放资源
        
        static DWORD WINAPI WorkerThread(LPVOID lpParam);
        int GetNoOfProcessors();
        
        PerIOContext* GetCliectIOContext(PerSocketContext * sendSock);
        PerSocketContext* GetClientSocktContext(char* dwAddr, int nPort);
        
    private:
    
        HANDLE                       m_hShutdownEvent;              // 用来通知线程系统退出的事件，为了能够更好的退出线程
        HANDLE                       m_hIOCompletionPort;           // 完成端口的句柄
        HANDLE*                      m_phWorkerThreads;             // 工作者线程的句柄指针数组
        CString                      m_strIP;                       // 服务器端的IP地址
        
        CDialog*                     m_pMain;                       // 主界面的界面指针，用于在主界面中显示消息
        CRITICAL_SECTION             m_csContextList;               // 用于Worker线程同步的互斥量
        CArray<PerSocketContext*>	 m_arrayClientContext;          // 客户端Socket的Context信息
        PerSocketContext*            m_pListenContext;              // 用于监听的Socket的Context信息
        
        LPFN_ACCEPTEX                m_lpfnAcceptEx;                // AcceptEx 和 GetAcceptExSockaddrs 的函数指针，用于调用这两个扩展函数
        LPFN_GETACCEPTEXSOCKADDRS    m_lpfnGetAcceptExSockAddrs;
        int		                     m_nThreads;                    // 生成的线程数量
        int                          m_nPort;                       // 服务器端的监听端口
        bool                         m_start;
};

