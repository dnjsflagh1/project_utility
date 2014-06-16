#ifndef _LOGSYSTEM_H_
#define _LOGSYSTEM_H_

#include "Singleton.h"
#include "LogConsole.h"

namespace MG
{
	class LogConsole;
	enum LOG_FILENAME_STATE
	{
		LOG_FILENAME_STATE_UPDATE_NO,
		LOG_FILENAME_STATE_UPDATE_SYSTEM,
		LOG_FILENAME_STATE_UPDATE_LOGIC,
		LOG_FILENAME_STATE_UPDATE_SYSTEM_AND_LOGIC,
	};
	class LogSystem
	{
	public:
		LogSystem(CChar* path = NULL);
		~LogSystem();
		 SINGLETON_INSTANCE(LogSystem);

	public:
		void setSysLogPath(CChar* path);
		void setGameLogicLogPath(CChar* path);
		void setLogLevel(U32 logLevel);
		void setPartName(CChar* partName);
		void setEnableConsole(Bool enable = false);
		void setSaveToFile(Bool isSave = false);
		void log(LOG_TYPE outType, CChar* fmt, ...);

	private:
		void _log(LOG_TYPE outType, CChar* msg);
		LOG_FILENAME_STATE _updateFileName(SYSTEMTIME date);

		LogConsole* mLogConsole;

		Char mSysLogPath[MG_FILE_PATH];
		Char mGameLogicLogPath[MG_FILE_PATH];

		Char mSysLogName[MG_FILE_PATH];
		Char mGameLogicLogName[MG_FILE_PATH];

		Str8 mPartName;
		U32	 mRandName;

		U32	 mLogLevel;
		Bool mEnableConsole;

		Bool mIsSaveToFile;

		FILE* mLogicFileHandle;
		FILE* mSystemFileHandle;
		
	};

    /******************************************************************************/
    //ÏûÏ¢ºê
    /******************************************************************************/
#define MG_LOG MG::LogSystem::getInstance().log

}
#endif