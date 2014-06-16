#include "stdafx.h"
#include "LogSystem.h"
#include <direct.h>
#include <bitset>
#include "MGRandom.h"
namespace MG
{

	//-----------------------------------------------------------------------------
	LogSystem::LogSystem( CChar* path)
	{
		mLogicFileHandle = NULL;
		mSystemFileHandle = NULL;
		if (!path)
		{
			memset(mSysLogPath,0,sizeof(mSysLogPath));
		}
		else
		{
			MGStrOp::sprintf(mSysLogPath,128,MG_STR("%s"),path);
		}
		
		memset(mGameLogicLogPath,0,sizeof(mGameLogicLogPath));
		mPartName = "";
		mRandName = MGRandom::getInstance().rand_ab_One(0,30000);
		mLogConsole = NULL;
		mLogLevel = 1023;
		mEnableConsole = false;
		mIsSaveToFile = false;
	}
	//-----------------------------------------------------------------------------
	LogSystem::~LogSystem()
	{

		if (mEnableConsole != false)
		{	
			if (mLogConsole)
			{
				MG_SAFE_DELETE(mLogConsole);
			}
		}


		
		if (mLogicFileHandle)
		{
			fclose(mLogicFileHandle);
		}
		if (mSystemFileHandle)
		{
			fclose(mSystemFileHandle);
		}
	}
	//-----------------------------------------------------------------------------
	void LogSystem::setSysLogPath( CChar* path )
	{
		if (path)
		{
			MGStrOp::sprintf(mSysLogPath,128,MG_STR("%s"),path);
		}
	}
	//-----------------------------------------------------------------------------
	void LogSystem::setGameLogicLogPath( CChar* path )
	{
		if (path)
		{
			MGStrOp::sprintf(mGameLogicLogPath,128,MG_STR("%s"),path);
		}
	}
	//-----------------------------------------------------------------------------
	void LogSystem::log( LOG_TYPE outType, CChar* fmt, ... )
	{
		va_list vl;
		Char msg[4096] = {0};
		va_start( vl, fmt );
		vsprintf_s( msg, fmt, vl );
		va_end( vl );
		
		if ( (outType >= 0) && ( outType < out_max ) )
		{
            if ( mLogLevel == out_max )
            {
                _log(outType,msg);
            }else
            if ( outType & mLogLevel )
            {
                _log(outType,msg);
            }
		}

	}
	//-----------------------------------------------------------------------------
	void LogSystem::_log( LOG_TYPE outType, CChar* msg )
	{
	
		if (mEnableConsole != false)
		{
			if ( outType != out_game_logic )
			{
				mLogConsole->logWrite( outType, msg );
			}
		}


		
		if (mIsSaveToFile)
		{
			// 更新文件----------------------------------------------
			SYSTEMTIME date ;
			GetLocalTime(&date) ;

			switch(_updateFileName( date ))
			{
			case LOG_FILENAME_STATE_UPDATE_NO:
				{
					Int i = 0;
				}
				break;
			case LOG_FILENAME_STATE_UPDATE_SYSTEM:
				{
					if (mSystemFileHandle)
					{
						fclose(mSystemFileHandle);
						fopen_s( &mSystemFileHandle,mSysLogName,"a" );
						if ( !mSystemFileHandle )
						{
							_mkdir(mSysLogPath);
							fopen_s( &mSystemFileHandle,mSysLogName,"a" );
						}
					}
					else
					{	
						fopen_s( &mSystemFileHandle,mSysLogName,"a" );
						if ( !mSystemFileHandle )
						{
							_mkdir(mSysLogPath);
							fopen_s( &mSystemFileHandle,mSysLogName,"a" );
						}

					}
				}
				break;
			case LOG_FILENAME_STATE_UPDATE_LOGIC:
				{
					if (mLogicFileHandle)
					{
						fclose(mLogicFileHandle);
						fopen_s( &mLogicFileHandle,mGameLogicLogName,"a" );
						if ( !mLogicFileHandle )
						{
							_mkdir(mGameLogicLogPath);
							fopen_s( &mLogicFileHandle,mGameLogicLogName,"a" );
						}
					}
					else
					{	
						fopen_s( &mLogicFileHandle,mGameLogicLogName,"a" );
						if ( !mLogicFileHandle )
						{
							_mkdir(mGameLogicLogPath);
							fopen_s( &mLogicFileHandle,mGameLogicLogName,"a" );
						}

					}
				}
				break;
			case LOG_FILENAME_STATE_UPDATE_SYSTEM_AND_LOGIC:
				{
					if (mSystemFileHandle)
					{
						fclose(mSystemFileHandle);
						fopen_s( &mSystemFileHandle,mSysLogName,"a" );
						if ( !mSystemFileHandle )
						{
							_mkdir(mSysLogPath);
							fopen_s( &mSystemFileHandle,mSysLogName,"a" );
						}
					}
					else
					{	
						fopen_s( &mSystemFileHandle,mSysLogName,"a" );
						if ( !mSystemFileHandle )
						{
							_mkdir(mSysLogPath);
							fopen_s( &mSystemFileHandle,mSysLogName,"a" );
						}

					}

					if (mLogicFileHandle)
					{
						fclose(mLogicFileHandle);
						fopen_s( &mLogicFileHandle,mGameLogicLogName,"a" );
						if ( !mLogicFileHandle )
						{
							_mkdir(mGameLogicLogPath);
							fopen_s( &mLogicFileHandle,mGameLogicLogName,"a" );
						}
					}
					else
					{	
						fopen_s( &mLogicFileHandle,mGameLogicLogName,"a" );
						if ( !mLogicFileHandle )
						{
							_mkdir(mGameLogicLogPath);
							fopen_s( &mLogicFileHandle,mGameLogicLogName,"a" );
						}

					}


				}
				break;
			}
			
			// 写日志----------------------------------------------
			if ( outType == out_game_logic )
			{
				if (!mLogicFileHandle)
				{
					if (mEnableConsole != false)
					{
						if ( mLogConsole )
						{

							//mLogConsole->logWrite( out_error, MG_STR("open log file failed, file :\n") );
							//mLogConsole->logWrite( out_error, filename );
							//mLogConsole->logWrite( out_error, MG_STR("\n") );

						}
						else
						{
							//printf( "open log file failed, file : %s\n", filename );
						}
					}
					return;
				}
				else
				{
					Char8 headerStr[64];
					MGStrOp::sprintf( headerStr, 64,"%02d-%02d-%02d %02d:%02d:%02d:[%02d]",
						date.wYear % 100, 
						date.wMonth + 1, 
						date.wDay,
						date.wHour, 
						date.wMinute, 
						date.wSecond,
						date.wMilliseconds);

					Char8* typeStr = NULL;
					switch ( outType )
					{
					case out_sys:
						typeStr = " [SYS] ";
						break;
					case out_error:
						typeStr = " [ERR] ";
						break;
					case out_debug:
						typeStr = " [DBG] ";
						break;
					case out_warning:
						typeStr = " [WRN] ";
						break;
					case out_info:
						typeStr = " [INF] ";
						break;
					case out_trace:
						typeStr = " [TRA] ";
						break;
					default:
						typeStr = " ";
						break;
					}

					strncat_s( headerStr, typeStr, sizeof(headerStr) );
					fseek(mLogicFileHandle,0,SEEK_END);
					fwrite(headerStr,1,StrLen(headerStr),mLogicFileHandle);
					fseek(mLogicFileHandle,0,SEEK_END);
					fwrite(msg,1,StrLen(msg),mLogicFileHandle);
					fseek(mLogicFileHandle,0,SEEK_END);
					//fputs( headerStr, mLogicFileHandle );
					//fputs( msg, mLogicFileHandle );
				}
			}
			else
			{
				if (!mSystemFileHandle)
				{

					if ( mEnableConsole != false)
					{
						if ( mLogConsole )
						{

							//mLogConsole->logWrite( out_error, MG_STR("open log file failed, file :\n") );
							//mLogConsole->logWrite( out_error, filename );
							//mLogConsole->logWrite( out_error, MG_STR("\n") );

						}
						else
						{
							//printf( "open log file failed, file : %s\n", filename );
						}
					}

					return;
				}
				else
				{
					Char8 headerStr[64];
					MGStrOp::sprintf( headerStr, 64,"%02d-%02d-%02d %02d:%02d:%02d:[%02d]",
						date.wYear % 100, 
						date.wMonth + 1, 
						date.wDay,
						date.wHour, 
						date.wMinute, 
						date.wSecond,
						date.wMilliseconds);

					Char8* typeStr = NULL;
					switch ( outType )
					{
					case out_sys:
						typeStr = " [SYS] ";
						break;
					case out_error:
						typeStr = " [ERR] ";
						break;
					case out_debug:
						typeStr = " [DBG] ";
						break;
					case out_warning:
						typeStr = " [WRN] ";
						break;
					case out_info:
						typeStr = " [INF] ";
						break;
					case out_trace:
						typeStr = " [TRA] ";
						break;
					default:
						typeStr = " ";
						break;
					}

					strncat_s( headerStr, typeStr, sizeof(headerStr) );
					fseek(mSystemFileHandle,0,SEEK_END);
					fwrite(headerStr,1,StrLen(headerStr),mSystemFileHandle);
					fseek(mSystemFileHandle,0,SEEK_END);
					fwrite(msg,1,StrLen(msg),mSystemFileHandle);
					fseek(mSystemFileHandle,0,SEEK_END);
					//fputs( headerStr, mSystemFileHandle );
					//fputs( msg, mSystemFileHandle );

				}
			}

		}

	}
	//-----------------------------------------------------------------------------
	LOG_FILENAME_STATE LogSystem::_updateFileName( SYSTEMTIME date )
	{
		Char8 temp[MG_FILE_PATH] = {0};
		LOG_FILENAME_STATE ret = LOG_FILENAME_STATE_UPDATE_NO;
		if (mPartName.empty() || mPartName == "")
		{
			if (*mSysLogPath == 0)
			{
				MGStrOp::sprintf( temp, MG_FILE_PATH,"Sys_%02d_%02d_%02d_%d.log",date.wYear, date.wMonth, date.wDay,mRandName);
			}
			else
			{
				MGStrOp::sprintf( temp, MG_FILE_PATH,"%s\\Sys_%02d_%02d_%02d.log_%d",mSysLogPath,date.wYear, date.wMonth, date.wDay,mRandName);
			}
			if (StrCmp(temp,mSysLogName) != 0)
			{
				ret = LOG_FILENAME_STATE_UPDATE_SYSTEM;
				MGStrOp::sprintf(mSysLogName,MG_FILE_PATH,temp);
			}
			if (*mGameLogicLogPath == 0)
			{
				MGStrOp::sprintf( temp, MG_FILE_PATH,"GameLogic_%02d_%02d_%02d_%d.log",date.wYear, date.wMonth , date.wDay,mRandName);
			}
			else
			{
				MGStrOp::sprintf( temp, MG_FILE_PATH,"%s\\GameLogic_%02d_%02d_%02d_%d.log",mGameLogicLogPath,mPartName.c_str(),date.wYear, date.wMonth, date.wDay,mRandName);
			}
		}
		else
		{
			if (*mSysLogPath == 0)
			{
				MGStrOp::sprintf( temp, MG_FILE_PATH,"Sys_%s%02d_%02d_%02d.log",mPartName.c_str(),date.wYear, date.wMonth, date.wDay);
			}
			else
			{
				MGStrOp::sprintf( temp, MG_FILE_PATH,"%s\\Sys_%s%02d_%02d_%02d.log",mSysLogPath,mPartName.c_str(),date.wYear, date.wMonth, date.wDay);
			}
			if (StrCmp(temp,mSysLogName) != 0)
			{
				ret = LOG_FILENAME_STATE_UPDATE_SYSTEM;
				MGStrOp::sprintf(mSysLogName,MG_FILE_PATH,temp);
			}
			if (*mGameLogicLogPath == 0)
			{
				MGStrOp::sprintf( temp, MG_FILE_PATH,"GameLogic_%s%02d_%02d_%02d.log",mPartName.c_str(),date.wYear, date.wMonth , date.wDay);
			}
			else
			{
				MGStrOp::sprintf( temp, MG_FILE_PATH,"%s\\GameLogic_%s%02d_%02d_%02d.log",mGameLogicLogPath,mPartName.c_str(),date.wYear, date.wMonth, date.wDay);
			}
		}
		//-----------------------------------------------------------------------------
		if(StrCmp(temp,mGameLogicLogName) != 0)
		{
			if (ret == LOG_FILENAME_STATE_UPDATE_SYSTEM)
			{
				ret = LOG_FILENAME_STATE_UPDATE_SYSTEM_AND_LOGIC;
			}
			else
			{
				ret = LOG_FILENAME_STATE_UPDATE_LOGIC;
			}
			MGStrOp::sprintf(mGameLogicLogName,MG_FILE_PATH,temp);
		}
		
		return ret;
		
	}
	//-----------------------------------------------------------------------------
	void LogSystem::setLogLevel(U32 logLevel)
	{
		if (logLevel <= 1024 && logLevel >= 0)
		{
			mLogLevel = logLevel;
		}
		else
		{
			mLogLevel = 8; //err 日志
		}
	}
	//-----------------------------------------------------------------------------
	void LogSystem::setPartName( CChar* partName )
	{
		mPartName = partName;
	}
	//-----------------------------------------------------------------------------
	void LogSystem::setEnableConsole(Bool enable)
	{
		mEnableConsole = enable;
		if (mEnableConsole == false)
		{
			if (mLogConsole)
			{
				MG_DELETE mLogConsole;
			}
		}
		else
		{
			if (mLogConsole)
			{
				return;
			}
			else
			{
				mLogConsole = MG_NEW LogConsole;
			}
			
		}
	}
	//-----------------------------------------------------------------------------
	void LogSystem::setSaveToFile( Bool isSave)
	{
		mIsSaveToFile =	isSave;
	}
	//-----------------------------------------------------------------------------

}