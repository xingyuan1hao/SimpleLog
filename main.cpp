#include "LogTask.h"


DWORD  WINAPI ThreadFun(LPVOID pVoid)
{
	LogTask::LOG_PRINT(LEVEL_ERROR,"ThreadFun This is error test.");
	LogTask::LOG_PRINT(LEVEL_INFOR,"ThreadFun This is info test.");
	LogTask::LOG_PRINT(LEVEL_DEBUG,"ThreadFun This is debug test.");
	return 0;
}

int main(int argc, char* argv[])
{
	LogTask *LTask = LogTask::GetInstance();
	LTask->SetLogLevel(LEVEL_DEBUG);
	LTask->start();

	HANDLE handle = CreateThread(NULL, 0, ThreadFun, 0, 0, NULL);
	if ( handle == NULL)
	{
		printf("TaskBase::start CreateThread error:%d\n", GetLastError());
	}

	LogTask::LOG_PRINT(LEVEL_ERROR,"This is error test.");
	LogTask::LOG_PRINT(LEVEL_INFOR,"This is info test.");
	LogTask::LOG_PRINT(LEVEL_DEBUG,"This is debug test.");

	system( "PAUSE ");

	WaitForSingleObject(handle, INFINITE);
	LTask->RemoveInstance();
}