#include "StdAfx.h"
#include "SDCommData.h"
#include "../SDMDlg.h"
#include "../system/SystemSetDlg.h"

static BYTE addrssNum;
extern int g_currentDoorNum;
extern BYTE g_address[];
extern ClientInfo *g_CurClientInfo;
extern CDataBaseT*  g_pDBT;

CSDCommData::CSDCommData(void)
{
    InitializeCriticalSection(&m_CliectCs);
    InitializeCriticalSection(&m_RecvCs);
    m_CountIndex = 1;
}

CSDCommData::~CSDCommData(void)
{
    DeleteCriticalSection(&m_CliectCs);
    DeleteCriticalSection(&m_RecvCs);
    this->ClearVec();
}

bool CSDCommData::AddClient(ClientInfo* c)
{
    CSDMDlg *pView = (CSDMDlg*)m_pUI;
    EnterCriticalSection(&m_CliectCs);
    m_CliectVec.push_back(c);
    pView->AddClient();
    this->UpdateUI();
    //SendToClient(c);
    LeaveCriticalSection(&m_CliectCs);
    return true;
}

vector<ClientInfo*> CSDCommData::GetAllClients()
{
    return m_CliectVec;
}

bool CSDCommData::DeleteClient(char* IpAdress, int nPort)
{
    CSDMDlg *pView = (CSDMDlg*)m_pUI;
    EnterCriticalSection(&m_CliectCs);
    vector<ClientInfo*>::iterator iter = m_CliectVec.begin();
    
    while (iter != m_CliectVec.end())
    {
        if (strcmp((*iter)->ipAddress, IpAdress) == 0 && (*iter)->nPort == nPort)
        {
            m_CliectVec.erase(iter);
            pView->DecreaseCliect();
            break;
        }
        else
        {
            iter++;
        }
    }
    
    this->UpdateUI();
    //SendToClient();
    LeaveCriticalSection(&m_CliectCs);
    return true;
}

bool CSDCommData::UpdateUI()
{
    CSDMDlg *pView = (CSDMDlg*)m_pUI;
    pView->m_DevTree.DeleteAllItems();
    
    if (m_CliectVec.size() < 1)
    {
        return false;
    }
    
    CString  Port;
    CTreeCtrl* pTreeCtl = &pView->m_DevTree;
    ASSERT(pTreeCtl);
    HTREEITEM hItem;
    hItem = pTreeCtl->InsertItem("当前连接客户端", 0, 0, TVI_ROOT);
    vector<ClientInfo*>::iterator iter = m_CliectVec.begin();
    int i = 1;
    
    while (iter != m_CliectVec.end())
    {
        if (*iter)
        {
            CString showData;
            CString IP((*iter)->ipAddress);
            Port.Format(_T("%d"), (*iter)->nPort);
            CString doorID;
            doorID.Format(_T(",门ID:%d"), ((*iter)->doorID) & 0x7f);
            showData = IP + doorID;
            pTreeCtl->InsertItem(showData, i, i, hItem);
            i++;
        }
        
        iter++;
    }
    
    /*pTreeCtl->SelectItem(pTreeCtl->GetChildItem(hItem));*/
    pTreeCtl->Expand(hItem, TVE_EXPAND);
    return true;
}

void CSDCommData::RecvData(ClientInfo* sender, char* message)
{
    EnterCriticalSection(&m_RecvCs);
    this->AnalysisData(sender, message);
    LeaveCriticalSection(&m_RecvCs);
}

bool CSDCommData::AnalysisData(ClientInfo* sender, char* message)
{
    bool res = false;
    char *recvData = message;
    int dateLen = recvData[1];
    int lastIndex = dateLen - 1;
    unsigned int dataType = (unsigned int)recvData[2];
    unsigned int lastDat = (unsigned int)recvData[lastIndex];
    
    switch ((unsigned int)recvData[2])
    {
        case M0:
            if (lastDat == 0x7f)
            {
                //BYTE* data = new BYTE[dateLen];
                //memset(data, 0, sizeof(data));
                //memcpy(data, recvData, (dateLen)*sizeof(BYTE));
                BYTE* data = new BYTE[dateLen + 25];
                memset(data, 0, sizeof(data));
                memcpy(data, recvData, (dateLen)*sizeof(BYTE));
                memcpy(data + dateLen, sender->ipAddress, (25)*sizeof(BYTE));
                g_currentDoorNum += 1;
                g_address[addrssNum] = data[0]; //id记录保存
                
                if (addrssNum >= 127)
                {
                    addrssNum = 0;
                    g_currentDoorNum = 127;
                }
                

				int clientNum = m_CliectVec.size();
				if (clientNum <= 0)
				{
					return 0L; //判断当前是否有连接客户端，没有则直接返回，没必要再走轮询逻辑
				}
				vector<ClientInfo*>::iterator iter = m_CliectVec.begin();
				while (iter != m_CliectVec.end())
				{
					ClientInfo *client = *iter;

					if (strcmp(client->ipAddress, sender->ipAddress) == 0)
					{
						client->doorID = data[0] & 0xff;
						break;
					}

					iter++;
				}
                
                if (g_CurClientInfo != NULL)
                {
                    if ((g_CurClientInfo->doorID & 0x7f) == 0 && (strcmp(sender->ipAddress, g_CurClientInfo->ipAddress) == 0))
                    {
                        g_CurClientInfo->doorID = data[0] & 0xff;
                    }
                }
                
                UpdateUI();
                CString id;
                CString name;
                CString remark;
                id.Format("%d", data[0] & 0x7F);
                g_pDBT->AddNewDev(id, name, remark);
                addrssNum += 1;
                //::SendMessage(this->m_curCtrlWnd, WM_DEV_INFO_RECEIVED, WPARAM(data), 0);
                //::PostMessage(this->m_curCtrlWnd, WM_DEV_INFO_RECEIVED, WPARAM(data), 0);
            }
            
            break;
            
        case M10:   //无报警数据不作处理
            if (lastDat == 0x7f)
            {
                //根据当前接收到的客户端ID,更新复位缓存中已有的连接设备断线判断次数
                int curId = recvData[0] & 0xff;
                vector<ClientInfo*> allCliect = theApp.m_SDCommData.GetAllClients();
                vector<ClientInfo*>::iterator iter = allCliect.begin();
                
                while (iter != allCliect.end())
                {
                    ClientInfo *client = *iter;
                    
                    if (curId == client->doorID)
                    {
                        client->cntOfOnlineJudge = 0;
                        break;
                    }
                    
                    iter++;
                }
            }
            
            res = true;
            break;
            
        case M11:   //有报警数据
        
            //CSDMDlg *pView = (CSDMDlg*)m_pUI;
            
            //判断接收数据长度是否符合要求，
            if (lastDat == 0x7f)
            {
                BYTE* data = new BYTE[dateLen];
                memset(data, 0, sizeof(data));
                memcpy(data, recvData, (dateLen)*sizeof(BYTE));
                ::PostMessage(this->m_curCtrlWnd, WM_ALARM_DATA_RECEIVED, WPARAM(data), 0); //pView->m_hWnd
            }
            
            res = true;
            break;
            
        case M12:   //参数返回
        
            //CSystemSetDlg *pView = (CSystemSetDlg*)m_pUI;
            
            //判断接收数据长度是否符合要求，
            if (lastDat == 0x7f)
            {
                BYTE* data = new BYTE[dateLen];
                memset(data, 0, sizeof(data));
                memcpy(data, recvData, (dateLen)*sizeof(BYTE));
                ::PostMessage(this->m_curCtrlWnd, WM_PARAM_DATA_RECEIVED, WPARAM(data), 0); //pView->m_hWnd
            }
            
            res = true;
            break;
            
        case M16:
            res = true;
            break;
            
        default:
            break;
    }
    
    return res;
}

void CSDCommData::InitUI(CDialog* p)
{
    m_pUI = p;
}

void CSDCommData::SetCurCtrlWnd(HWND hCtrlWnd)
{
    m_preCtrlWnd = m_curCtrlWnd;
    m_curCtrlWnd = hCtrlWnd;
}

void CSDCommData::RecoverPreCtrlWnd()
{
    m_curCtrlWnd = m_preCtrlWnd;
}

void CSDCommData::LoginOutInform()
{
    CSDMDlg *pView = (CSDMDlg*)m_pUI;
    vector<ClientInfo*>::iterator iter = m_CliectVec.begin();
    
    while (iter != m_CliectVec.end())
    {
        //  pView->m_IOCPServer.SendData((*iter)->ipAddress, (*iter)->nPort, "L#", 3);
        iter++;
    }
}

void CSDCommData::ClearVec()
{
    vector<ClientInfo*>::iterator iter = m_CliectVec.begin();
    
    while (iter != m_CliectVec.end())
    {
        if (*iter)
        {
            delete (*iter);
        }
        
        iter++;
    }
    
    m_CliectVec.clear();
}

void CSDCommData::SendToClient(ClientInfo* c)
{
    CSDMDlg *pView = (CSDMDlg*)m_pUI;
    ClientInfo* curClient = c;
    //char  data[4] ={0x04,0x20,0x01,0x7F};
    WSABUF wsaBuf = curClient->wsaBuf;
    pView->m_IOCPServer.SendData(curClient->ipAddress, curClient->nPort, curClient->wsaBuf, 4);
}

bool CSDCommData::SendToAllClient(ClientInfo* sender, char* message)
{
    if (message[0] == '#' && message[1] == '#')
    {
        char IP[32];
        char Port[10];
        char *sendM;
        int nPort, p = 0, n = 0, i, j;
        int len = strlen(message);
        
        for (i = 3; i < len; i++)
        {
            if (message[i] == ' ')
            {
                break;
            }
            
            IP[p++] = message[i];
        }
        
        IP[p] = 0;
        
        for (j = i + 1; j < len; j++)
        {
            if (message[j] == ' ')
            {
                break;
            }
            
            Port[n++] = message[j];
        }
        
        Port[n] = 0;
        nPort = atoi(Port);
        sendM = message + 1 + 1;
        char send[1024];
        sprintf_s(send, "%s %d %s", sender->ipAddress, sender->nPort, sendM);
        CSDMDlg *pView = (CSDMDlg*)m_pUI;
        vector<ClientInfo*>::iterator iter = m_CliectVec.begin();
        
        while (iter != m_CliectVec.end())
        {
            //  pView->m_IOCPServer.SendData((*iter)->ipAddress, (*iter)->nPort,send, strlen(send)+1);
            iter++;
        }
        
        return true;
    }
    
    return false;
}

ClientInfo* CSDCommData::GetCurClientInfo(BYTE doorID)
{
    vector<ClientInfo*>::iterator iter = m_CliectVec.begin();
    
    while (iter != m_CliectVec.end())
    {
        if (doorID == (*iter)->doorID)
        {
            return *iter;
        }
        
        iter++;
    }
}

ClientInfo* CSDCommData::GetCurClientInfo(int doorID)
{
    vector<ClientInfo*>::iterator iter = m_CliectVec.begin();
    
    while (iter != m_CliectVec.end())
    {
        if (doorID == (*iter)->doorID)
        {
            return *iter;
        }
        
        iter++;
    }
}

ClientInfo* CSDCommData::GetCurClientInfo(char *ip)
{
    vector<ClientInfo*>::iterator iter = m_CliectVec.begin();
    
    while (iter != m_CliectVec.end())
    {
        if (strcmp((*iter)->ipAddress, ip) == 0)
        {
            return *iter;
        }
        
        iter++;
    }
}