#include "LogTask.h"


LogTask *LogTask::m_Task = NULL;

LogTask *LogTask::GetInstance()
{
	if (m_Task == NULL)
	{
		m_Task = new LogTask();
	}
	return m_Task;
}

void LogTask::RemoveInstance()
{
	if (m_Task != NULL)
	{
		delete m_Task;
	}
}

LogTask::LogTask()
{
	InitializeCriticalSection(&m_section);
	m_event = CreateEvent(NULL,true,false,NULL);
	m_level = LEVEL_ERROR;
	m_fLog = NULL;
	m_bExit = false;
}

LogTask::LogTask(const LogTask&)
{

}

LogTask& LogTask::operator=(const LogTask&)
{
	return *m_Task;
}

LogTask::~LogTask()
{
	m_bExit = true;
	SetEvent(m_event);
	
	if (m_fLog)
	{
		fclose(m_fLog);
	}
	join();
	CloseHandle(m_event);
	DeleteCriticalSection(&m_section);
}

void LogTask::SetLogLevel(int level)
{
	m_level = level;

	if (m_level > 1)
	{
		char filepath[MAX_PATH+1] = {0};
		if (GetModuleFileName(NULL,filepath,MAX_PATH) == 0)
		{
			printf("LogTask GetModuleFileName err %d.\n",GetLastError());
		}
		else
		{
			if (strcmp(filepath, "") != 0)
			{
				string strpath = filepath;
				int pos = strpath.rfind("\\");
				strpath = strpath.substr(0,pos+1);
				strpath += "log.txt";
				m_fLog = fopen(strpath.c_str(),"a+");
			}	
		}
		
	}
}

void LogTask::LOG_PRINT(int level, char* cFormat, ...)
{
	va_list args = NULL;
	va_start(args, cFormat);
	m_Task->PushLog(level,cFormat,args);
	va_end(args);
}

int LogTask::PushLog(int level, char* cFormat, va_list vlist)
{
	if (level <= m_level)
	{
		char levelstr[20];
		switch(level)
		{
		case LEVEL_DEBUG:
			strcpy(levelstr,"DEBUG");
			break;
		case LEVEL_ERROR:
			strcpy(levelstr,"ERROR");
			break;
		case LEVEL_INFOR:
		default:
			strcpy(levelstr,"INFOR");
			break;
		}
		char logbuf[2048];

		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		sprintf(logbuf,"%4d/%02d/%02d %02d:%02d:%02d.%03d [%s]",sysTime.wYear,sysTime.wMonth,
			sysTime.wDay,sysTime.wHour,sysTime.wMinute,sysTime.wSecond,sysTime.wMilliseconds,levelstr);
		int lens = strlen(logbuf);
		vsprintf_s(logbuf+lens,2047-lens,cFormat,vlist);
		string LogString(logbuf);

		//ÁÙ½çÇø
		EnterCriticalSection(&m_section);
		m_LogQue.push(LogString);
		SetEvent(m_event);
		LeaveCriticalSection(&m_section);
	}
	return 0;
}

void LogTask::svc()
{
	while(!m_bExit)
	{
		WaitForSingleObject(m_event,INFINITE);
		string LogString;
		EnterCriticalSection(&m_section);
		if (!m_LogQue.empty())
		{
			LogString = m_LogQue.front();
			m_LogQue.pop();
		}
		if (m_LogQue.empty())
		{
			ResetEvent(m_event);
		}
		LeaveCriticalSection(&m_section);

		if (m_fLog && !LogString.empty())
		{
			fprintf(m_fLog,"%s\n",LogString.c_str());
			fflush(m_fLog);
			printf("%s\n",LogString.c_str());
		}
	}
}