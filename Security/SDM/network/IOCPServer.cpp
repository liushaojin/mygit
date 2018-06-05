#include "StdAfx.h"
#include "IOCPServer.h"
#include "../SDMDlg.h"

#define WORKER_THREADS_PER_PROCESSOR 2
#define MAX_POST_ACCEPT              10
#define EXIT_CODE                    NULL

#define RELEASE(x)                      {if(x != NULL ){delete x;x=NULL;}}
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}
#define RELEASE_SOCKET(x)               {if(x !=INVALID_SOCKET) { closesocket(x);x=INVALID_SOCKET;}}

CIOCPServer::CIOCPServer(void):
    m_nThreads(0),
    m_hShutdownEvent(NULL),
    m_hIOCompletionPort(NULL),
    m_phWorkerThreads(NULL),
    m_strIP(DEFAULT_IP),
    m_nPort(DEFAULT_PORT),
    m_pMain(NULL),
    m_lpfnAcceptEx(NULL),
    m_pListenContext(NULL),
    m_start(FALSE)
{
}

CIOCPServer::~CIOCPServer(void)
{
    this->Stop();
}


// 工作者线程：为IOCP请求服务的工作者线程
// 也就是每当完成端口上出现了完成数据包，就将之取出来进行处理的线程
DWORD WINAPI CIOCPServer::WorkerThread(LPVOID lpParam)
{
    WorkThreadParams* pParam = (WorkThreadParams*)lpParam;
    CIOCPServer* pIOCPServer = (CIOCPServer*)pParam->pIOCPServer;
    int threadNo = (int)pParam->threadNo;
    pIOCPServer->ShowMessage(_T("工作者线程启动，ID: %d."), threadNo);
    OVERLAPPED           *pOverlapped = NULL;
    PerSocketContext     *pSocketContext = NULL;
    DWORD                dwBytesTransfered = 0;
    
    while(WAIT_OBJECT_0 != WaitForSingleObject(pIOCPServer->m_hShutdownEvent, 0))
    {
        BOOL bReturn = GetQueuedCompletionStatus(
                           pIOCPServer->m_hIOCompletionPort,
                           &dwBytesTransfered,
                           (PULONG_PTR)&pSocketContext,
                           &pOverlapped,
                           INFINITE);
                           
        if(EXIT_CODE == (DWORD)pSocketContext)
        {
            break;
        }
        
        if(!bReturn)
        {
            DWORD dwErr = GetLastError();
            
            if(!pIOCPServer->HandleError(pSocketContext, dwErr))
            {
                break;
            }
            
            continue;
        }
        else
        {
            PerIOContext* pIoContext = CONTAINING_RECORD(pOverlapped, PerIOContext, overLapped);
            
			if (0 == dwBytesTransfered)
			{
				continue;
			}

            if((0 == dwBytesTransfered) && (RecvPosted == pIoContext->opType || SendPosted == pIoContext->opType))
            {
                pIOCPServer->ShowMessage(_T("客户端 %s:%d 断开连接."), inet_ntoa(pSocketContext->m_clientAddr.sin_addr), ntohs(pSocketContext->m_clientAddr.sin_port));
                pIOCPServer->RemoveContext(pSocketContext);
                continue;
            }
            else
            {
                switch(pIoContext->opType)
                {
                    // Accept
                    case AcceptPosted:
                        {
                            // 为了增加代码可读性，这里用专门的_DoAccept函数进行处理连入请求
                            pIOCPServer->DoAccpet(pSocketContext, pIoContext);
                        }
                        break;
                        
                    // RECV
                    case RecvPosted:
                        {
                            // 为了增加代码可读性，这里用专门的DoRecv函数进行处理接收请求
                            pIOCPServer->DoRecv(pSocketContext, pIoContext);
                        }
                        break;
                        
                    // SEND
                    // 这里只是为了检查一下发送是否成功，当然可以在这里判断数据是否需要重发
                    case SendPosted:
                        {
                            pIOCPServer->DoSend(pSocketContext, pIoContext);
                        }
                        break;
                        
                    default:
                        // 不应该执行到这里
                        TRACE(_T("_WorkThread中的 pIoContext->opType 参数异常.\n"));
                        break;
                } //switch
            }//if
        }//if
    }//while
    
    TRACE(_T("工作者线程 %d 号退出.\n"), threadNo);
    // 释放线程参数
    RELEASE(lpParam);
    return 0;
}

bool CIOCPServer::LoadSocketLib()
{
    WSADATA wsaData;
    int nResult;
    nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    if(NO_ERROR != nResult)
    {
        this->ShowMessage(_T("初始化WinSock 2.2失败！\n"));
        return false;
    }
    
    return true;
}

bool CIOCPServer::Start()
{
    InitializeCriticalSection(&m_csContextList);	//初始化临界区
    // 建立系统退出的事件通知
    m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    // 初始化IOCP
    if(false == InitializeIOCP())
    {
        this->ShowMessage(_T("初始化IOCP失败！\n"));
        return false;
    }
    else
    {
        this->ShowMessage(_T("\nIOCP初始化完毕\n."));
    }
    
    // 初始化Socket
    if(false == InitializeListenSocket())
    {
        this->ShowMessage(_T("Listen Socket初始化失败！\n"));
        this->DeInitialize();
        return false;
    }
    else
    {
        this->ShowMessage(_T("Listen Socket初始化完毕."));
    }
    
    this->ShowMessage(_T("系统准备就绪，等候连接....\n"));
    //标志服务器开启了
    this->m_start = TRUE;
    return true;
}


//	开始发送系统退出消息，退出完成端口和线程资源
void CIOCPServer::Stop()
{
    if(m_pListenContext != NULL && m_pListenContext->m_Socket != INVALID_SOCKET)
    {
        // 激活关闭消息通知
        SetEvent(m_hShutdownEvent);
        
        for(int i = 0; i < m_nThreads; i++)
        {
            // 通知所有的完成端口操作退出
            PostQueuedCompletionStatus(m_hIOCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
        }
        
        // 等待所有的客户端资源退出
        WaitForMultipleObjects(m_nThreads, m_phWorkerThreads, TRUE, INFINITE);
        // 清除客户端列表信息
        this->ClearContextList();
        // 释放其他资源
        this->DeInitialize();
        this->ShowMessage(_T("停止监听\n"));
    }
}

// 初始化完成端口
bool CIOCPServer::InitializeIOCP()
{
    // 建立第一个完成端口
    m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    
    if(NULL == m_hIOCompletionPort)
    {
        this->ShowMessage(_T("建立完成端口失败！错误代码: %d!\n"), WSAGetLastError());
        return false;
    }
    
    // 根据本机中的处理器数量，建立对应的线程数，CPU核心数*2 + 2.
    m_nThreads = WORKER_THREADS_PER_PROCESSOR * GetNoOfProcessors() + 2;
    // 为工作者线程初始化句柄
    m_phWorkerThreads = new HANDLE[m_nThreads];
    // 根据计算出来的数量建立工作者线程
    DWORD nThreadID;
    
    for(int i = 0; i < m_nThreads; i++)
    {
        WorkThreadParams* pThreadParams = new WorkThreadParams;
        pThreadParams->pIOCPServer = this;
        pThreadParams->threadNo  = i + 1;
        m_phWorkerThreads[i] = ::CreateThread(0, 0, WorkerThread, (void *)pThreadParams, 0, &nThreadID);
    }
    
    TRACE(" 建立 WorkerThread %d 个.\n", m_nThreads);
    return true;
}

// 初始化Socket
bool CIOCPServer::InitializeListenSocket()
{
    // AcceptEx 和 GetAcceptExSockaddrs 的GUID，用于导出函数指针
    GUID GuidAcceptEx = WSAID_ACCEPTEX;
    GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
    // 服务器地址信息，用于绑定Socket
    struct sockaddr_in ServerAddress;
    // 生成用于监听的Socket的信息
    m_pListenContext = new PerSocketContext;
    // 需要使用重叠IO，必须得使用WSASocket来建立Socket，才可以支持重叠IO操作
    m_pListenContext->m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    
    if(INVALID_SOCKET == m_pListenContext->m_Socket)
    {
        this->ShowMessage(_T("初始化Socket失败，错误代码: %d.\n"), WSAGetLastError());
        return false;
    }
    else
    {
        TRACE("WSASocket() 完成.\n");
    }
    
    // 将Listen Socket绑定至完成端口中
    if(NULL == CreateIoCompletionPort((HANDLE)m_pListenContext->m_Socket, m_hIOCompletionPort, (DWORD)m_pListenContext, 0))
    {
        this->ShowMessage(_T("绑定 Listen Socket至完成端口失败！错误代码: %d/n"), WSAGetLastError());
        RELEASE_SOCKET(m_pListenContext->m_Socket);
        return false;
    }
    else
    {
        TRACE("Listen Socket绑定完成端口 完成.\n");
    }
    
    ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    //ServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    ServerAddress.sin_addr.s_addr = inet_addr(m_strIP.GetString());
    ServerAddress.sin_port = htons(m_nPort);
    
    if(SOCKET_ERROR == bind(m_pListenContext->m_Socket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)))
    {
        this->ShowMessage(_T("bind()函数执行错误.\n"));
        return false;
    }
    else
    {
        TRACE("bind() 完成.\n");
    }
    
    // 开始进行监听
    if(SOCKET_ERROR == listen(m_pListenContext->m_Socket, SOMAXCONN))
    {
        this->ShowMessage(_T("Listen()函数执行出现错误.\n"));
        return false;
    }
    else
    {
        TRACE("Listen() 完成.\n");
    }
    
    // 使用AcceptEx函数，因为这个是属于WinSock2规范之外的微软另外提供的扩展函数
    // 所以需要额外获取一下函数的指针，
    // 获取AcceptEx函数指针
    DWORD dwBytes = 0;
    
    if(SOCKET_ERROR == WSAIoctl(
                m_pListenContext->m_Socket,
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                &GuidAcceptEx,
                sizeof(GuidAcceptEx),
                &m_lpfnAcceptEx,
                sizeof(m_lpfnAcceptEx),
                &dwBytes,
                NULL,
                NULL))
    {
        this->ShowMessage(_T("WSAIoctl 未能获取AcceptEx函数指针。错误代码: %d\n"), WSAGetLastError());
        this->DeInitialize();
        return false;
    }
    
    // 获取GetAcceptExSockAddrs函数指针，也是同理
    if(SOCKET_ERROR == WSAIoctl(
                m_pListenContext->m_Socket,
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                &GuidGetAcceptExSockAddrs,
                sizeof(GuidGetAcceptExSockAddrs),
                &m_lpfnGetAcceptExSockAddrs,
                sizeof(m_lpfnGetAcceptExSockAddrs),
                &dwBytes,
                NULL,
                NULL))
    {
        this->ShowMessage(_T("WSAIoctl 未能获取GuidGetAcceptExSockAddrs函数指针。错误代码: %d\n"), WSAGetLastError());
        this->DeInitialize();
        return false;
    }
    
    // 为AcceptEx 准备参数，然后投递AcceptEx I/O请求
    for(int i = 0; i < MAX_POST_ACCEPT; i++)
    {
        // 新建一个IO_CONTEXT
        PerIOContext* pAcceptIoContext = m_pListenContext->GetNewIoContext();
        
        if(false == this->PostAccept(pAcceptIoContext))
        {
            m_pListenContext->RemoveContext(pAcceptIoContext);
            return false;
        }
    }
    
    this->ShowMessage(_T("投递 %d 个AcceptEx请求完毕"), MAX_POST_ACCEPT);
    return true;
}

void CIOCPServer::DeInitialize()
{
    DeleteCriticalSection(&m_csContextList);
    RELEASE_HANDLE(m_hShutdownEvent);
    
    for(int i = 0; i < m_nThreads; i++)
    {
        RELEASE_HANDLE(m_phWorkerThreads[i]);
    }
    
    RELEASE(m_phWorkerThreads);
    RELEASE_HANDLE(m_hIOCompletionPort);
    RELEASE(m_pListenContext);
    this->ShowMessage(_T("释放资源完毕.\n"));
}

bool CIOCPServer::PostAccept(PerIOContext* pAcceptIoContext)
{
    ASSERT(INVALID_SOCKET != m_pListenContext->m_Socket);
    // 准备参数
    DWORD dwBytes = 0;
    pAcceptIoContext->opType = AcceptPosted;
    WSABUF *p_wbuf   = &pAcceptIoContext->wsaBuf;
    OVERLAPPED *p_ol = &pAcceptIoContext->overLapped;
    // 为以后新连入的客户端先准备好Socket( 这个是与传统accept最大的区别 )
    pAcceptIoContext->sockAccept  = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    
    if(INVALID_SOCKET == pAcceptIoContext->sockAccept)
    {
        ShowMessage(_T("创建用于Accept的Socket失败！错误代码: %d"), WSAGetLastError());
        return false;
    }
    
    // 投递AcceptEx
    if(FALSE == m_lpfnAcceptEx(m_pListenContext->m_Socket, pAcceptIoContext->sockAccept, p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN) + 16) * 2),
                               sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, p_ol))
    {
        int n = (sizeof(SOCKADDR_IN) + 16) * 2;
        
        if(WSA_IO_PENDING != WSAGetLastError())
        {
            ShowMessage(_T("投递 AcceptEx 请求失败，错误代码: %d"), WSAGetLastError());
            return false;
        }
    }
    
    return true;
}


// 在有客户端连入的时候，进行处理
// 流程有点复杂，你要是看不懂的话，就看配套的文档吧....
// 如果能理解这里的话，完成端口的机制你就消化了一大半了

// 总之你要知道，传入的是ListenSocket的Context，我们需要复制一份出来给新连入的Socket用
// 原来的Context还是要在上面继续投递下一个Accept请求
bool CIOCPServer::DoAccpet(PerSocketContext* pSocketContext, PerIOContext* pIoContext)
{
    SOCKADDR_IN* clientAddr = NULL;
    SOCKADDR_IN* LocalAddr = NULL;
    int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);
    // 1. 首先取得连入客户端的地址信息
    // 这个 m_lpfnGetAcceptExSockAddrs 不得了啊~~~~~~
    // 不但可以取得客户端和本地端的地址信息，还能顺便取出客户端发来的第一组数据，老强大了...
    this->m_lpfnGetAcceptExSockAddrs(pIoContext->wsaBuf.buf, pIoContext->wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
                                     sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&clientAddr, &remoteLen);
    this->ShowMessage(_T("客户端 %s:%d 连入."), inet_ntoa(clientAddr->sin_addr), ntohs(clientAddr->sin_port));
    this->ShowMessage(_T("客户额 %s:%d 信息：%s."), inet_ntoa(clientAddr->sin_addr), ntohs(clientAddr->sin_port), pIoContext->wsaBuf.buf);
    // 2. 这里需要注意，这里传入的这个是ListenSocket上的Context，这个Context我们还需要用于监听下一个连接
    // 所以我还得要将ListenSocket上的Context复制出来一份为新连入的Socket新建一个SocketContext
    PerSocketContext* pNewSocketContext = new PerSocketContext;
    pNewSocketContext->m_Socket = pIoContext->sockAccept;
    memcpy(&(pNewSocketContext->m_clientAddr), clientAddr, sizeof(SOCKADDR_IN));
    
    // 参数设置完毕，将这个Socket和完成端口绑定(这也是一个关键步骤)
    if(false == this->AssociateWithIOCP(pNewSocketContext))
    {
        RELEASE(pNewSocketContext);
        return false;
    }
    
    // 3. 继续，建立其下的IoContext，用于在这个Socket上投递第一个Recv数据请求
    PerIOContext* pNewIoContext = pNewSocketContext->GetNewIoContext();
    pNewIoContext->opType       = RecvPosted;
    pNewIoContext->sockAccept   = pNewSocketContext->m_Socket;
    // 如果Buffer需要保留，就自己拷贝一份出来
    //memcpy( pNewIoContext->szBuffer,pIoContext->szBuffer,MAX_BUFFER_LEN );
    
    // 绑定完毕之后，就可以开始在这个Socket上投递完成请求了
    if(false == this->PostRecv(pNewIoContext))
    {
        pNewSocketContext->RemoveContext(pNewIoContext);
        return false;
    }
    
    // 4. 如果投递成功，那么就把这个有效的客户端信息，加入到ContextList中去(需要统一管理，方便释放资源)
    this->AddToContextList(pNewSocketContext, pIoContext->wsaBuf);

    // 5. 使用完毕之后，把Listen Socket的那个IoContext重置，然后准备投递新的AcceptEx
    pIoContext->ResetBuffer();
    return this->PostAccept(pIoContext);
}


// 投递接收数据请求
bool CIOCPServer::PostRecv(PerIOContext* pIoContext)
{
    // 初始化变量
    DWORD dwFlags = 0;
    DWORD dwBytes = 0;
    WSABUF *p_wbuf   = &pIoContext->wsaBuf;
    OVERLAPPED *p_ol = &pIoContext->overLapped;
    pIoContext->ResetBuffer();
    pIoContext->opType = RecvPosted;
    // 初始化完成后，，投递WSARecv请求
    int nBytesRecv = WSARecv(pIoContext->sockAccept, p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL);
    
    // 如果返回值错误，并且错误的代码并非是Pending的话，那就说明这个重叠请求失败了
    if((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
    {
        this->ShowMessage(_T("投递第一个WSARecv失败！"));
        return false;
    }
    
    return true;
}

//投递发送请求
bool CIOCPServer::PostSend(PerIOContext* pIoContext)
{
    DWORD dwFlags = 0;
    DWORD dwBytes = 0;
    WSABUF *p_wbuf = &pIoContext->wsaBuf;
    OVERLAPPED *p_ol = &pIoContext->overLapped;
    //该IO上下文变为忙绿状态
    pIoContext->busy = true;
    //使用完成端口发送
    int nBytesSend = WSASend(pIoContext->sockAccept, p_wbuf, 1, &dwBytes, dwFlags, p_ol, NULL);
    int nError = WSAGetLastError();
    
    if((SOCKET_ERROR == nBytesSend) && (WSA_IO_PENDING != nError))
    {
        ShowMessage(TEXT("在完成端口上提交SEND请求失败, code: %d"), nError);
        //_RELEASE_IO_DATA(pIO);//释放PER_IO_DATA内存
        return false;
    }
    
    return true;
}

// 在有接收的数据到达的时候，进行处理
bool CIOCPServer::DoRecv(PerSocketContext* pSocketContext, PerIOContext* pIoContext)
{
    // 先把上一次的数据显示出现，然后就重置状态，发出下一个Recv请求
    SOCKADDR_IN* clientAddr = &pSocketContext->m_clientAddr;
    this->ShowMessage(_T("收到  %s:%d 信息：%s"), inet_ntoa(clientAddr->sin_addr), ntohs(clientAddr->sin_port), pIoContext->wsaBuf.buf);
    ClientInfo *cliect = new ClientInfo;
    strcpy_s(cliect->ipAddress, inet_ntoa(clientAddr->sin_addr));
    cliect->nPort = ntohs(clientAddr->sin_port);
    char *str = pIoContext->wsaBuf.buf;
    theApp.m_SDCommData.RecvData(cliect, str);
    // 然后开始投递下一个WSARecv请求
    return PostRecv(pIoContext);
}
//在发送数据后进行检测
bool CIOCPServer::DoSend(PerSocketContext* pSocketContext, PerIOContext* pIoContext)
{
    DWORD nSendByte = 0;
    DWORD dwFlag = 0;
    //获得发送的结果
    BOOL result = WSAGetOverlappedResult(pSocketContext->m_Socket, &pIoContext->overLapped, &nSendByte, TRUE, &dwFlag);
    
    if(result) ////////////////////////////////dont check send bytes here
    {
        ShowMessage(_T("发送数据成功！"));
        //该IO操作上下文变为可用状态
        pIoContext->busy = false;
    }
    else
    {
        ShowMessage(TEXT("发送数据失败, 错误码为 %d"), WSAGetLastError());
    }
    
    return (result != 0);
}


// 将句柄(Socket)绑定到完成端口中
bool CIOCPServer::AssociateWithIOCP(PerSocketContext *pContext)
{
    // 将用于和客户端通信的SOCKET绑定到完成端口中
    HANDLE hTemp = CreateIoCompletionPort((HANDLE)pContext->m_Socket, m_hIOCompletionPort, (DWORD)pContext, 0);
    
    if(NULL == hTemp)
    {
        this->ShowMessage(_T("执行CreateIoCompletionPort()出现错误.错误代码：%d"), GetLastError());
        return false;
    }
    
    return true;
}

// 将客户端的相关信息存储到数组中
void CIOCPServer::AddToContextList(PerSocketContext *pHandleData, WSABUF wsabuf)
{
    //把客户端信息放入
    EnterCriticalSection(&m_csContextList);
    m_arrayClientContext.Add(pHandleData);
    LeaveCriticalSection(&m_csContextList);
    //把客户端信息存放在界面的容器中
    ClientInfo *cliect = new ClientInfo;
    strcpy_s(cliect->ipAddress, inet_ntoa(pHandleData->m_clientAddr.sin_addr));
    int len = strlen(cliect->ipAddress);
    cliect->ipAddress[len] = 0;
    cliect->nPort = ntohs(pHandleData->m_clientAddr.sin_port);
    cliect->wsaBuf = wsabuf;
    cliect->wsaBuf.len = 4;
    cliect->doorID = ((unsigned int) wsabuf.buf[0]) & 0xff;
	cliect->cntOfOnlineJudge = 0;	//是否连接的判断次数初始为0
	cliect->isOfflineFlag = 0;
    theApp.m_SDCommData.AddClient(cliect);
	//第一组数据接收处理
	theApp.m_SDCommData.RecvData(cliect, wsabuf.buf);
}

//	移除某个特定的Context
void CIOCPServer::RemoveContext(PerSocketContext *pSocketContext)
{
    //删除界面的客户端信息
    theApp.m_SDCommData.DeleteClient(inet_ntoa(pSocketContext->m_clientAddr.sin_addr), ntohs(pSocketContext->m_clientAddr.sin_port));
    EnterCriticalSection(&m_csContextList);
    
    for(int i = 0; i < m_arrayClientContext.GetCount(); i++)
    {
        if(pSocketContext == m_arrayClientContext.GetAt(i))
        {
            RELEASE(pSocketContext);
            m_arrayClientContext.RemoveAt(i);
            break;
        }
    }
    
    LeaveCriticalSection(&m_csContextList);
}

// 清空客户端信息
void CIOCPServer::ClearContextList()
{
    EnterCriticalSection(&m_csContextList);
    
    for(int i = 0; i < m_arrayClientContext.GetCount(); i++)
    {
        delete m_arrayClientContext.GetAt(i);
    }
    
    m_arrayClientContext.RemoveAll();
    LeaveCriticalSection(&m_csContextList);
}

//给客户端发送数据
bool CIOCPServer::SendData(char* dwAddr, int nPort, WSABUF wsaBuf, WORD buflen)
{
    _ASSERTE(NULL != dwAddr);
    
    if(buflen > PACK_SIZE) //确认包的大小不超过最大长度
    {
        _ASSERTE(!"Package is too large.");
        return false;
    }
    
    //判断服务器有没有启动
    if(!m_start)
    {
        return false;
    }
    
    //找到是否有此IP地址的客户端
    PerIOContext* pContext = NULL ;
    PerSocketContext *pSendSock = NULL ;
    //根据IP获取客户端
    pSendSock = GetClientSocktContext(dwAddr, nPort);
    
    //检查该客户端是否存在
    if(!pSendSock)
    {
        return false;
    }
    
    pContext = GetCliectIOContext(pSendSock);
    pContext->wsaBuf = wsaBuf;
    PostSend(pContext);
    return true;
}

//查找客户端中的发送上下文是否存在，不存在则创建一个
PerIOContext* CIOCPServer::GetCliectIOContext(PerSocketContext * sendSock)
{
    PerIOContext* pSendContext = NULL;
    
    for(int i = 0; i < sendSock->m_arrayIoContext.GetCount(); i++)
    {
        if(sendSock->m_arrayIoContext.GetAt(i)->opType == SendPosted && (!sendSock->m_arrayIoContext.GetAt(i)->busy))
        {
            pSendContext = sendSock->m_arrayIoContext.GetAt(i);
            break;
        }
    }
    
    //如果发送数据的上下文操作还不存在，则重新创建一个
    if(!pSendContext)
    {
        pSendContext = sendSock->GetNewIoContext();
        pSendContext->sockAccept = sendSock->m_Socket;
        pSendContext->opType = SendPosted;
    }
    
    return pSendContext;
}

// 获得本机的IP地址
CString CIOCPServer::GetLocalIP()
{
    // 获得本机主机名
    char hostname[MAX_PATH] = {0};
    gethostname(hostname, MAX_PATH);
    struct hostent FAR* lpHostEnt = gethostbyname(hostname);
    
    if(lpHostEnt == NULL)
    {
        return DEFAULT_IP;
    }
    
    // 取得IP地址列表中的第一个为返回的IP(因为一台主机可能会绑定多个IP)
    LPSTR lpAddr = lpHostEnt->h_addr_list[0];
    // 将IP地址转化成字符串形式
    struct in_addr inAddr;
    memmove(&inAddr, lpAddr, 4);
    m_strIP = CString(inet_ntoa(inAddr));
    return m_strIP;
}

void CIOCPServer::SetServerIpPort(CString ip, int port)
{
	m_strIP = ip;
	m_nPort = port;
}

// 获得本机中处理器的数量
int CIOCPServer::GetNoOfProcessors()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
}

// 在主界面中显示提示信息
void CIOCPServer::ShowMessage(const CString szFormat, ...) const
{
    // 根据传入的参数格式化字符串
    CString   strMessage;
    va_list   arglist;
    // 处理变长参数
    va_start(arglist, szFormat);
    strMessage.FormatV(szFormat, arglist);
    va_end(arglist);
    // 在主界面中显示
    //CSDMDlg* pMain = (CSDMDlg*)m_pMain;
    //
    //if(m_pMain != NULL)
    //{
    //    //pMain->AddInformation(strMessage);
    //    pMain->m_LogListBox.AddString(strMessage);
    //    TRACE(strMessage + _T("\n"));
    //}
}

// 判断客户端Socket是否已经断开，否则在一个无效的Socket上投递WSARecv操作会出现异常
// 使用的方法是尝试向这个socket发送数据，判断这个socket调用的返回值
// 因为如果客户端网络异常断开(例如客户端崩溃或者拔掉网线等)的时候，服务器端是无法收到客户端断开的通知的
bool CIOCPServer::IsSocketAlive(SOCKET s)
{
    int nByteSent = send(s, "", 0, 0);
    
    if(-1 == nByteSent)
    {
        return false;
    }
    
    return true;
}

// 显示并处理完成端口上的错误
bool CIOCPServer::HandleError(PerSocketContext *pContext, const DWORD& dwErr)
{
    // 如果是超时了，就再继续等吧
    if(WAIT_TIMEOUT == dwErr)
    {
        // 确认客户端是否还活着...
        if(!IsSocketAlive(pContext->m_Socket))
        {
            this->ShowMessage(_T("检测到客户端异常退出！"));
            this->RemoveContext(pContext);
            return true;
        }
        else
        {
            this->ShowMessage(_T("网络操作超时！重试中..."));
            return true;
        }
    }
    // 可能是客户端异常退出了
    else if(ERROR_NETNAME_DELETED == dwErr)
    {
        this->ShowMessage(_T("检测到客户端异常退出！"));
        this->RemoveContext(pContext);
        return true;
    }
    else
    {
        this->ShowMessage(_T("完成端口操作出现错误，线程退出。错误代码：%d"), dwErr);
        return false;
    }
}

PerSocketContext* CIOCPServer::GetClientSocktContext(char* dwAddr, int nPort)
{
    PerSocketContext *pSendSocketContext = NULL;
    EnterCriticalSection(&m_csContextList);
    
    for(int i = 0; i < m_arrayClientContext.GetCount(); i++)
    {
        if(strcmp(inet_ntoa(m_arrayClientContext.GetAt(i)->m_clientAddr.sin_addr), dwAddr) == 0 && ntohs(m_arrayClientContext.GetAt(i)->m_clientAddr.sin_port) == nPort)
        {
            pSendSocketContext = m_arrayClientContext.GetAt(i);
            break;
        }
    }
    
    LeaveCriticalSection(&m_csContextList);
    return pSendSocketContext;
}

/*
BOOL CIOCPServer::Startselfserver(UINT uListenPort)
{
	if (!WinSockInit())
	{
		return FALSE;
	}
	m_ListenSocket = WSASocket(AF_INET , SOCK_STREAM , IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if ( m_ListenSocket == INVALID_SOCKET )
	{
		goto __Error_End;
	}
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(uListenPort);
	if ( bind(m_ListenSocket, (sockaddr*)&service, sizeof(sockaddr_in)) == SOCKET_ERROR )
	{
		goto __Error_End;
	}
	if( listen(m_ListenSocket, SOMAXCONN) == SOCKET_ERROR )
	{
		SetLastErrorMsg(_T("监听失败！"));
		goto __Error_End;
	}
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if ( m_hCompletionPort == NULL )
	{
		SetLastErrorMsg(_T("完成端口创建失败！"));
		goto __Error_End;
	}
	CreateIoCompletionPort((HANDLE)m_ListenSocket, m_hCompletionPort,(ULONG_PTR)m_ListenSocket, 0);
	m_wsaapi.LoadAllFun(m_ListenSocket);
	::InitializeCriticalSection(&m_getsktpoll);
	SYSTEM_INFO  systeminfo;
	GetSystemInfo(&systeminfo);
	pThreadpool = CreateThreadpool(NULL);
	SetThreadpoolThreadMinimum(pThreadpool, 2);
	SetThreadpoolThreadMaximum(pThreadpool,(int)systeminfo.dwNumberOfProcessors *2);
	InitializeThreadpoolEnvironment(&tcbe);
	SetThreadpoolCallbackPool(&tcbe, pThreadpool);
	pTpcg= CreateThreadpoolCleanupGroup();
	SetThreadpoolCallbackCleanupGroup(&tcbe, pTpcg, NULL);
	pTpWork= CreateThreadpoolWork(allMyWorkCallback,this,&tcbe);
	SubmitThreadpoolWork(pTpWork);
	///先投递100的等连接
	PostCreateSocket(m_ListenSocket);//创建一个SOCKET池
	return TRUE;
}

BOOL CIOCPServer::PostCreateSocket(SOCKET cListSKT)
{
    SOCKET cClientSKT;
    CSoketpool *cspl=NULL;
    int lierr=0;
    int li=0;
    for (li=0;li<m_ConnFreeMax;li++)
    {
cClientSKT=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
        if (INVALID_SOCKET == cClientSKT)
        {
            lierr=WSAGetLastError();
            break;
        }
        CreateIoCompletionPort((HANDLE)cClientSKT,m_hCompletionPort,(ULONG_PTR)cListSKT,0);
        COverLappedEx *m_pOverLap = new COverLappedEx(IO_TYPE_ACCEPT,cClientSKT);
        BOOL bRet = AcceptEx(cListSKT,cClientSKT, m_pOverLap->m_szBuf, 0,
            sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16, &m_pOverLap->dwBytesRecv, &m_pOverLap->m_OLap);
        if (!bRet)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                AfxMessageBox(_T("111"));
                break;
            }
        }
        InterlockedIncrement(&m_ConnSKTCount);
        InterlockedIncrement(&m_ConnNowCount);
    }
    BOOL lspw=SetProcessWorkingSetSize(GetCurrentProcess(),1024*1024*1024,2*1024*1024*1024);
    return TRUE;
}

VOID CALLBACK allMyWorkCallback(PTP_CALLBACK_INSTANCE Instance,PVOID Parameter,PTP_WORK Work)
{
    //接收DLL的主线程
    CIOCPServer *pThis = (CIOCPServer *)Parameter;
    COverLappedEx *pOverLaps = NULL;
    SOCKET cListenSKT;
    BOOL bIORet=FALSE;
    BOOL bSendt=FALSE;
    DWORD dwTrans = 0;
    char lctype=NULL;
    char revc[2048]={0};
    CString csket;
    char szx[10] = {'\0'};
    PTP_WORK pTpWorks;
    DWORD dwFlags=0;

    InterlockedIncrement(&pThis->m_ThreadNums);
    while (TRUE)
    {
        bIORet = GetQueuedCompletionStatus(pThis->m_hCompletionPort, &dwTrans, (PULONG_PTR)&cListenSKT, (LPOVERLAPPED *)&pOverLaps, INFINITE);
        if (!bIORet) continue;
        if(bIORet && pOverLaps)
        {
            switch (pOverLaps->m_IOType)
            {
                case IO_TYPE_READ:
                    {
                        memset(revc,0,2048);
                        WSABUF wsaBuf = {0};
                        wsaBuf.buf = revc;
                        wsaBuf.len = 2048;

                        WSARecv(pOverLaps->m_ClientSKT, &wsaBuf, 1, &(pOverLaps->dwBytesRecv), &(pOverLaps->dwFlags), NULL, NULL);
                        if ((strcmp(revc,"")==0) || (revc[0]==NULL))
                        {//接收到空数据，断开连接
                            pThis->PostCloseSocket(pOverLaps->m_ClientSKT,pOverLaps);
                            pThis->Postaccept(pOverLaps);//断开连接后，重新投递为连接请求
                            continue;
                        }
                        memset(revc,0,2048);
                        pThis->PostRecv(pOverLaps);
                        continue;
                    }
                case IO_TYPE_SEND:
                    {
                        continue;
                    }
                case IO_TYPE_ACCEPT:
                    {    InterlockedDecrement(&pThis->m_ConnSKTCount);

                        pThis->PostRecv(pOverLaps);
                        pTpWorks= CreateThreadpoolWork(allMyWorkCallback,pThis,&pThis->tcbe);// 创建一个工作项
                        SubmitThreadpoolWork(pTpWorks);
                        if (pThis->m_ConnSKTCount<pThis->m_ConnFreeMax)
                        {
                            pThis->PostCreateSocket(cListenSKT);
                        }
                        continue;
                    }
                case IO_TYPE_CLOSE:
                    {
                        continue;
                    }
                default:
                    {
                        continue;
                    }
            }
        }
        else if (!pOverLaps )
        {
            break;
        }
    }
    InterlockedDecrement(&pThis->m_ThreadNums);
    return ;
}

BOOL CIOCPServer::PostRecv(COverLappedEx *m_pOverLap)
{
    ZeroMemory(&(m_pOverLap->m_OLap), sizeof(OVERLAPPED));
    m_pOverLap->m_IOType=IO_TYPE_READ;
    m_pOverLap->dwBytesRecv=0;
    WSABUF wsaBuf={NULL,0};
    int iRet = WSARecv(m_pOverLap->m_ClientSKT,&wsaBuf, 1, &(m_pOverLap->dwBytesRecv), &(m_pOverLap->dwFlags), &(m_pOverLap->m_OLap), NULL);
    if (iRet != NO_ERROR)
    {
        int lierr=WSAGetLastError() ;
        if (lierr != WSA_IO_PENDING)
        {
            delete m_pOverLap;
            return FALSE;
        }
    }
    return TRUE;
}
*/