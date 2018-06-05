#pragma once

//自定义消息
#define WM_DEV_INFO_RECEIVED  (WM_USER + 0x040)		//设备信息刷新消息
#define WM_ALARM_DATA_RECEIVED  (WM_USER + 0x041)	//报警数据刷新消息
#define WM_PARAM_DATA_RECEIVED  (WM_USER + 0x042)	//参数数据刷新消息

class CSDCommData
{
    public:
        CSDCommData(void);
        ~CSDCommData(void);
        
        ClientInfo* GetCurClientInfo(int doorID);
        ClientInfo* GetCurClientInfo(BYTE doorID);	//根据安检门编号获取对应的客户端信息
		ClientInfo* GetCurClientInfo(char *ip);
        bool AddClient(ClientInfo* c);	//添加客户端
        bool DeleteClient(char* IpAdress, int nPort);
        bool UpdateUI();
        void InitUI(CDialog* p);
        void RecvData(ClientInfo* sender, char* message);
        void LoginOutInform();
        void SetCurCtrlWnd(HWND hCtrlWnd);	//设置当前活动窗口的句柄
        void RecoverPreCtrlWnd();			//子窗口关闭后恢复到主窗口句柄
        vector<ClientInfo*> GetAllClients();//获取所有的客户端
        
        enum Command
        {
            M0 = 0x0,		//同步命令
            M10 = 0x0A,		//无报警正常通过
            M11 = 0x0B,		//通过且报警
            M12 = 0x0C,		//安检门参数
            M16 = 0x10,		//安检门关机
        };
        
    private:
        bool AnalysisData(ClientInfo* sender, char* message);
        bool SendToAllClient(ClientInfo* sender, char* message);
        void SendToClient(ClientInfo* c);
        void ClearVec();
        
        vector<ClientInfo*> m_CliectVec;
        CRITICAL_SECTION  m_CliectCs;
        CRITICAL_SECTION  m_RecvCs;
        CDialog* m_pUI;
        
        HWND m_curCtrlWnd;
        HWND m_preCtrlWnd;
        
        int m_CountIndex;
};

