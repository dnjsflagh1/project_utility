/******************************************************************************/
#include "stdafx.h"
#include "SqlServerDriver.h"

/******************************************************************************/
namespace MG
{
	//-----------------------------------------------------------------------------
	SqlServerDriver::SqlServerDriver(void)
	{
		CoInitialize(NULL);
		mCommand = NULL;
		mConnection = NULL;
		mMsgout = NULL;
	}
	//-----------------------------------------------------------------------------
	SqlServerDriver::~SqlServerDriver(void)
	{
		uninitObject();
		CoUninitialize();
	}
	//-----------------------------------------------------------------------------
	Bool SqlServerDriver::initObject()
	{
		try
		{
			mCommand.CreateInstance(__uuidof(Command));
			mConnection.CreateInstance(__uuidof(Connection));	
			mConnected = false;
			return TRUE;
		}
		catch(_com_error &e)
		{
			messageOut("InitObject:Description = %s", (Char8*) e.Description());
			uninitObject();
			return FALSE;
		}
		catch(...)
		{
			messageOut("InitObject:Init DB Object failed");
			uninitObject();
			return FALSE;
		}
		return TRUE;
	}
	//-----------------------------------------------------------------------------
	Bool SqlServerDriver::uninitObject()
	{
		try
		{
			if (mConnection)
			{
				if(mConnection->State == adStateOpen)
				{
					mConnection->Close();
					mConnection.Release();			
				}
				mConnection = NULL;
			}
			if (mCommand)
			{
				mCommand.Release();
				mCommand = NULL;
			}
			return true;
		}
		catch(_com_error &e)
		{
			messageOut("UninitObject:Description = %s", (Char8*) e.Description());
			return false;
		}
		catch(...)
		{
			messageOut("UninitObject:Release DB Object failed");
			return false;
		}
		return true;
	}
	//-----------------------------------------------------------------------------
	I32 SqlServerDriver::reConnectDB(Char8* sqlStr)
	{
		try
		{
			//RESET CONNECTION//		
			uninitObject();
			CoUninitialize();
			CoInitialize(NULL);
			mCommand = NULL;
			mConnection = NULL;
		}
		catch(_com_error &e)
		{
			messageOut("ConnectDB:Description = %s", (Char8*) e.Description());
		}

		return connectDB(sqlStr);
	}
	//-----------------------------------------------------------------------------
	I32 SqlServerDriver::connectDB(Char8* sqlStr)
	{
		try
		{
			initObject();

			if (mConnection == NULL)
				mConnection.CreateInstance(__uuidof(Connection));

			if (0 == _stricmp(sqlStr, mConnectionString) && 
				mConnection->State == adStateOpen && 
				mConnected)
				return SQL_OK;
			else
			{
				if (mConnection->State == adStateOpen)
				{
					mConnection->Close();		

					mCommand = NULL;
					mConnection = NULL;

					mCommand.CreateInstance(__uuidof(Command));
					mConnection.CreateInstance(__uuidof(Connection));	
				}
				_snprintf_s(mConnectionString, MAX_PATH, MAX_PATH - 1, sqlStr);
				mConnectionString[MAX_PATH-1]=0;
			}
			if (SQL_OK == mConnection->Open((Char8*)mConnectionString, L"", L"", adModeUnknown))			
			{
				mConnected = true;
				return SQL_OK;
			}
		}
		catch(_com_error &e)
		{
			messageOut("ConnectDB:Description = %s", (Char8*) e.Description());
		}
		catch(...)
		{
			messageOut("ConnectDB:error");
		}
		return SQL_FAIL;
	}
	//-----------------------------------------------------------------------------
	I32 SqlServerDriver::connectDB(Char8* sqlStr, CChar8* szDBName, CChar8* szDBIP, CChar8* szDBUser, CChar8* szDBPassWord)
	{
		I32 hr = connectDB(sqlStr);
		if ( hr == SQL_OK )
			return hr;
		else
		{
			if ( !szDBName || !szDBIP || !szDBUser || !szDBPassWord )
				return E_FAIL;

			// 先连接系统数据库
			Char8 szTemp[MAX_PATH];
			MGStrOp::sprintf(szTemp, _countof(szTemp), "Provider=sqloledb;Data Source=%s;Initial Catalog=%s;User Id=%s;Password=%s;",
				szDBIP, "master", szDBUser, szDBPassWord );
			hr = connectDB( szTemp ); 
			if ( hr != SQL_OK )
				return hr;

			// 创建相应的数据库
			MGStrOp::sprintf( szTemp, _countof(szTemp), "CREATE DATABASE %s", szDBName );
			try
			{
				mConnection->Execute( szTemp, NULL, adExecuteNoRecords );

				MG::MGStrOp::sprintf(szTemp, _countof(szTemp), "Provider=sqloledb;Data Source=%s;Initial Catalog=%s;User Id=%s;Password=%s;",
					szDBIP, szDBName, szDBUser, szDBPassWord );

				// 连接新的数据库
				hr = reConnectDB( szTemp );

				return hr;
			}
			catch (_com_error &e)
			{
				messageOut("ExecuteNoRs:Description = %s,SQL=%s", (char*) e.Description(),szTemp);
				return SQL_FAIL;
			}
		}

	}
	//-----------------------------------------------------------------------------
	I32 SqlServerDriver::executeNoRs(char *szFormat,...)
	{
		char	szSQL[1024*10];
		try
		{
			int maxbufsize = sizeof(szSQL) - 1;
			_vsnprintf_s(szSQL, maxbufsize,maxbufsize, szFormat, (char*) (&szFormat + 1));
			szSQL[maxbufsize]=0;
			int len = strlen(szSQL);
			if (len >= maxbufsize)
			{
				messageOut("ExecuteNoRs: %s", szSQL);
				mConnected = false;
				return SQL_FAIL;
			}
			if (mConnection == NULL || mConnection->State == adStateClosed)		
				reConnectDB(mConnectionString);		
			mConnection->Execute(szSQL, NULL, adExecuteNoRecords );
			return SQL_OK;
		}
		catch(_com_error &e)
		{
			reConnectDB(mConnectionString);
			try		
			{
				mConnection->Execute(szSQL, NULL, adExecuteNoRecords );		
			}
			catch(...)		
			{
				messageOut("ExecuteNoRs:Description = %s,SQL=%s", (char*) e.Description(),szSQL);
				return SQL_FAIL;
			}
			return SQL_OK;
		}
		catch(...)
		{
			messageOut("ExecuteNoRs:%s",szSQL);
		}
		mConnected = false;
		return SQL_FAIL;
	}
	//-----------------------------------------------------------------------------
	I32 SqlServerDriver::executeNoRsEx(char *szFormat,...)
	{
		char	szSQL[8000];
		try
		{
			int maxbufsize = sizeof(szSQL) - 1;
			_vsnprintf_s(szSQL, maxbufsize, szFormat, (char*) (&szFormat + 1));
			szSQL[maxbufsize]=0;
			int len = strlen(szSQL);
			if (len >= maxbufsize)
			{
				messageOut("ExecuteNoRsEx: %s", szSQL);
				mConnected = false;
				return SQL_FAIL;
			}
			if (mConnection == NULL || mConnection->State == adStateClosed)
				connectDB(mConnectionString);
			mConnection->Execute(szSQL, NULL, adExecuteNoRecords );
			return SQL_OK;
		}
		catch(_com_error &e)
		{
			connectDB(mConnectionString);
			messageOut("ExecuteNoRs:Description = %s,SQL=%s", (char*) e.Description(),szSQL);		
		}
		catch(...)
		{
			messageOut("ExecuteNoRs:%s",szSQL);
		}
		mConnected = false;
		return SQL_FAIL;
	}
	//-----------------------------------------------------------------------------
	_RecordsetPtr SqlServerDriver::executeRs(char *szFormat,...)
	{
		char	szSQL[4096];
		try
		{			
			int maxbufsize = sizeof(szSQL) - 1;
			_vsnprintf_s(szSQL, maxbufsize, szFormat, (char*) (&szFormat + 1));
			szSQL[maxbufsize]=0;
			int len = strlen(szSQL);
			if (len >= maxbufsize)
			{
				messageOut("ExecuteRs: %s", szSQL);
				mConnected = false;
				return NULL;
			}
			if (mConnection == NULL || mConnection->State == adStateClosed)
				reConnectDB(mConnectionString);
			_RecordsetPtr pRecordset;
			pRecordset.CreateInstance(__uuidof(Recordset));
			//		pRecordset->Open(szSQL,m_Connection.GetInterfacePtr(),adOpenStatic,adLockOptimistic,adCmdText);
			mCommand->ActiveConnection = mConnection;
			mCommand->CommandType = adCmdText;
			mCommand->CommandText = szSQL;
			_variant_t vNull;
			vNull.vt = VT_ERROR;
			vNull.scode = DISP_E_PARAMNOTFOUND;
			pRecordset = mCommand->Execute( &vNull, &vNull, adCmdText );
			return pRecordset;
		}
		catch(_com_error &e)
		{
			reConnectDB(mConnectionString);
			messageOut("ExecuteRs:Description = %s,sql=%s", (char*) e.Description(),szSQL);		
		}
		catch(...)
		{
			messageOut("ExecuteRs:%s",szSQL);
		}
		mConnected = false;
		return NULL;
	}
	//-----------------------------------------------------------------------------
	_RecordsetPtr SqlServerDriver::executeRsEx(char *szFormat,...)
	{
		char	szSQL[4096];
		try
		{			
			int maxbufsize = sizeof(szSQL) - 1;
			_vsnprintf_s(szSQL, maxbufsize, szFormat, (char*) (&szFormat + 1));
			szSQL[maxbufsize]=0;
			int len = strlen(szSQL);
			if (len >= maxbufsize)
			{
				messageOut("ExecuteRsEx: %s", szSQL);
				mConnected = false;
				return NULL;
			}
			if (mConnection == NULL || mConnection->State == adStateClosed)
				reConnectDB(mConnectionString);
			_RecordsetPtr pRecordset;
			pRecordset.CreateInstance(__uuidof(Recordset));
			pRecordset->Open(szSQL,mConnection.GetInterfacePtr(),adOpenStatic,adLockOptimistic,adCmdText);
			return pRecordset;
		}
		catch(_com_error &e)
		{
			reConnectDB(mConnectionString);
			messageOut("ExecuteRsEx:Description = %s", (char*) e.Description());		
		}
		catch(...)
		{
			messageOut("ExecuteRsEx:%s",szSQL);
		}
		mConnected = false;
		return NULL;
	}
	//-----------------------------------------------------------------------------
	_RecordsetPtr SqlServerDriver::executeSP(char *szFormat,...)
	{
		//char	szSQL[4096];
		//try
		//{			
		//	int maxbufsize = sizeof(szSQL) - 1;
		//	_vsnprintf_s(szSQL, maxbufsize, szFormat, (char*) (&szFormat + 1));
		//	szSQL[maxbufsize]=0;
		//	int len = strlen(szSQL);
		//	if (len >= maxbufsize)
		//	{
		//		messageOut("ExecuteRs: %s", szSQL);
		//		mConnected = FALSE;
		//		return NULL;
		//	}
		//	if (mConnection == NULL || mConnection->State == adStateClosed)
		//		reConnectDB(mConnectionString);
		//	_RecordsetPtr pRecordset;
		//	pRecordset.CreateInstance(__uuidof(Recordset));
		//	//		pRecordset->Open(szSQL,m_Connection.GetInterfacePtr(),adOpenStatic,adLockOptimistic,adCmdText);
		//	mCommand->ActiveConnection = mConnection;
		//	mCommand->CommandType = adCmdDBdProc;
		//	mCommand->CommandText = szSQL;
		//	_variant_t vNull;
		//	vNull.vt = VT_ERROR;
		//	vNull.scode = DISP_E_PARAMNOTFOUND;
		//	pRecordset = mCommand->Execute( &vNull, &vNull, adCmdDBdProc );
		//	return pRecordset;
		//}
		//catch(_com_error &e)
		//{
		//	reConnectDB(mConnectionString);
		//	messageOut("ExecuteRs:Description = %s,sql=%s", (char*) e.Description(),szSQL);		
		//}
		//catch(...)
		//{
		//	messageOut("ExecuteRs:%s",szSQL);
		//}
		//mConnected = false;
		return NULL;
	}
	//-----------------------------------------------------------------------------
	I32 SqlServerDriver::executeNoRs(int &affectRecord, char *szFormat,...)
	{
		affectRecord = 0;
		char	szSQL[1024*10];
		try
		{
			int maxbufsize = sizeof(szSQL);
			_vsnprintf_s(szSQL, maxbufsize, szFormat, (char*) (&szFormat + 1));
			szSQL[maxbufsize]=0;
			int len = strlen(szSQL);
			if (len >= maxbufsize)
			{
				messageOut("ExecuteNoRs: %s", szSQL);
				mConnected = FALSE;
				return E_FAIL;
			}
			if (mConnection == NULL || mConnection->State == adStateClosed)		
				reConnectDB(mConnectionString);
			_variant_t vNull;
			vNull.vt = VT_ERROR;
			vNull.scode = DISP_E_PARAMNOTFOUND;
			mConnection->Execute(szSQL, &vNull, adExecuteNoRecords );
			affectRecord = vNull.intVal;
			return SQL_OK;
		}
		catch(_com_error &e)
		{
			reConnectDB(mConnectionString);
			try		
			{
				mConnection->Execute(szSQL, NULL, adExecuteNoRecords );		
			}
			catch(...)		
			{
				messageOut("ExecuteNoRs:Description = %s,SQL=%s", (char*) e.Description(),szSQL);
				return SQL_FAIL;
			}
			return SQL_OK;
		}
		catch(...)
		{
			messageOut("ExecuteNoRs:%s",szSQL);
		}
		mConnected = false;
		return SQL_FAIL;
	}
	//-----------------------------------------------------------------------------
	_RecordsetPtr SqlServerDriver::executeRs(int &affectRecord, char *szFormat,...)
	{
		affectRecord = 0;
		char	szSQL[4096];
		try
		{			
			int maxbufsize = sizeof(szSQL) - 1;
			_vsnprintf_s(szSQL, maxbufsize, szFormat, (char*) (&szFormat + 1));
			szSQL[maxbufsize]=0;
			int len = strlen(szSQL);
			if (len >= maxbufsize)
			{
				messageOut("ExecuteRs: %s", szSQL);
				mConnected = FALSE;
				return NULL;
			}
			if (mConnection == NULL || mConnection->State == adStateClosed)
				reConnectDB(mConnectionString);
			_RecordsetPtr pRecordset;
			pRecordset.CreateInstance(__uuidof(Recordset));
			//		pRecordset->Open(szSQL,m_Connection.GetInterfacePtr(),adOpenStatic,adLockOptimistic,adCmdText);
			mCommand->ActiveConnection = mConnection;
			mCommand->CommandType = adCmdText;
			mCommand->CommandText = szSQL;
			_variant_t vNull;
			vNull.vt = VT_ERROR;
			vNull.scode = DISP_E_PARAMNOTFOUND;
			pRecordset = mCommand->Execute( &vNull, &vNull, adCmdText );
			affectRecord = vNull.intVal;
			return pRecordset;
		}
		catch(_com_error &e)
		{
			reConnectDB(mConnectionString);
			messageOut("ExecuteRs:Description = %s,sql=%s", (char*) e.Description(),szSQL);		
		}
		catch(...)
		{
			messageOut("ExecuteRs:%s",szSQL);
		}
		mConnected = false;
		return NULL;
	}
	//-----------------------------------------------------------------------------
	void SqlServerDriver::addColume(char * tableName,Colume * columes,int count)
	{
		for (int i=0;i<count;i++)
			executeNoRsEx("ALTER TABLE %s ADD %s %s NOT NULL DEFAULT %s",tableName,columes[i].name,columes[i].type,columes[i].dValue);
	}
	//-----------------------------------------------------------------------------
	void SqlServerDriver::messageOut(char * msg,...)
	{
		if (mMsgout)
		{
			char	szSQL[8192];
			int maxbufsize = sizeof(szSQL) - 1;
			_vsnprintf_s(szSQL, maxbufsize, msg, (char*) (&msg + 1));
			szSQL[maxbufsize]=0;
			int len = strlen(szSQL);
			mMsgout(szSQL);
		}
	}
	//-----------------------------------------------------------------------------
}