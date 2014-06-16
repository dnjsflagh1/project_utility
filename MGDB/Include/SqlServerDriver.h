/******************************************************************************/
#ifndef  _SQLSERVERDRIVER_H_
#define  _SQLSERVERDRIVER_H_
#include "DBDriver.h"
/******************************************************************************/
#pragma once
#import "c:\program files\common files\system\ado\msado15.dll" no_namespace rename("EOF","adoEOF")

/******************************************************************************/
namespace MG
{
	enum SQL_RESULT
	{
		SQL_OK,
		SQL_FALSE,
		SQL_FAIL = 0x80004005,
		SQL_MAX
	};
	/******************************************************************************/
	//SqlServerDriver 数据操作基础类
	/******************************************************************************/
	class SqlServerDriver
	{
	public:
		SqlServerDriver(void);
		virtual ~SqlServerDriver(void);

		Bool			initObject();
		Bool			uninitObject();

		I32				connectDB(Char8* sqlStr);
		I32				connectDB(Char8* sqlStr, CChar8* szDBName, CChar8* szDBIP, CChar8* szDBUser, CChar8* szDBPassWord);
		
		I32				executeNoRs(Char8 *szFormat,...);
		I32				executeNoRsEx(Char8 *szFormat,...);
		_RecordsetPtr	executeRs(Char8 *szFormat,...);
		_RecordsetPtr	executeRsEx(Char8 *szFormat,...);
		_RecordsetPtr	executeSP(Char8 *szFormat,...);

		// With affected row count
		I32				executeNoRs(I32 &affectRecord, Char8 *szFormat,...);
		_RecordsetPtr	executeRs(I32 &affectRecord, Char8 *szFormat,...);

		void			addColume(Char8 * tableName,Colume * columes,I32 count);
		virtual void	messageOut(Char8 * msg,...);
		I32				reConnectDB(Char8* sqlStr);

	public:
		Char8				mConnectionString[MAX_PATH];
		_CommandPtr			mCommand;
		_ConnectionPtr		mConnection;
		Bool				mConnected;
		void				(*mMsgout)(Char8 * msg);
	};

	/******************************************************************************/
	//数据库宏
	/******************************************************************************/
	#define _DBGETREC(Rs, FieldName) (Rs->Fields->Item[_variant_t(FieldName)]->Value)

	#define SET_RECORD_BINARY_FIELD(lpRecordSet, valName, pbin)					\
		{																		\
		Char8* pFlag = NULL;													\
		_variant_t chunk = lpRecordSet->Fields->Item[valName]->Value;			\
		if(SQL_OK == SafeArrayAccessData(chunk.parray, (void**)&pFlag))			\
			{																	\
			memcpy(pFlag, pbin, (long)chunk.parray->rgsabound->cElements);		\
			SafeArrayUnaccessData(chunk.parray);								\
			lpRecordSet->Fields->Item[valName]->Value = chunk;					\
			}																	\
		}				

	#define GET_RECORD_BINARY_FIELD(lpRecordSet, valName, pbin)					\
		{																		\
		Char8* pFlag = NULL;													\
		_variant_t chunk = lpRecordSet->Fields->Item[valName]->Value;			\
		if(SQL_OK == SafeArrayAccessData(chunk.parray, (Ptr*)&pFlag))			\
			{																	\
			memcpy(pbin, pFlag, (I32)chunk.parray->rgsabound->cElements);		\
			SafeArrayUnaccessData(chunk.parray);								\
			}																	\
		}

}
#endif