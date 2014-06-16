/******************************************************************************/
#include "stdafx.h"
#include "CsvReader.h"
#include "MGStrOp.h"

// MG_CN_MODIFY
#include "IMGPackExterInterface.h"
/******************************************************************************/


namespace MG
{

	//-----------------------------------------------------------------------------
	CsvReader::CsvReader()
	{
		mFileStream = NULL;
		mType = CSV_COMMA;
		mColnum = 0;
		mTypeValue[CSV_COMMA] = L',';
		mTypeValue[CSV_TAB] = L'	';
		mTypeValue[CSV_ENCODE] = L' ';
		mReadRowCnt = 0;
	}
	//-----------------------------------------------------------------------------
	CsvReader::~CsvReader()
	{
		destroy();
	}
	//-----------------------------------------------------------------------------
	// MG_CN_MODIFY
	MG::Bool CsvReader::create( const std::wstring fileName, IMGExternalPackManager* packManager)
	{
		if (create(fileName))
			return true;
		else if(packManager)
		{
			Char16* buf = NULL;
			Str fileNametemp;
			MGStrOp::toString(fileName.c_str(), fileNametemp);
			packManager->extractPackFile(fileNametemp, buf);
			if (buf && create(buf))
				return true;
			else
				return false;
		}
		else
			return false;
	}
	//-----------------------------------------------------------------------------
	Bool CsvReader::create( const std::wstring fileName )
	{

		Str16	 szFileFath;
		Char16   szFileName[256]; 
		GetModuleFileNameW(NULL,szFileName,256); 
		Char16   szDrive[256]; 
		Char16   szDir[256]; 
		Char16   szPath[256]; 
		_wsplitpath(szFileName,szDrive,szDir,NULL,NULL); 
		swprintf(szPath, L"%s%s",szDrive,szDir);
		szFileFath = szPath;                       //当前应用程序所在路径
		szFileFath += fileName;                     //CSV文件所在路径

		if (_wfopen_s(&mFileStream,szFileFath.c_str(),L"r+b") == 0)
		{	
			I32 size = 0;
			fseek(mFileStream,0L,SEEK_END);
			size = ftell(mFileStream);
			fseek(mFileStream,0L,SEEK_SET);
			if (size != 0)
			{
				I32	uSize = size / 2;
				Char16* buf = new Char16[uSize + 1];
				buf[uSize] = '\0';
				fread(buf,sizeof(Char16),uSize,mFileStream);
				fclose(mFileStream);
				mFileStream = NULL;
				//文件不是Unicode格式
				
				if (buf[0] != 0xFEFF)
				{	
					DYNAMIC_ASSERT(false);
					Char16 errorStr[512];
					MGStrOp::sprintf(errorStr,512,L"%s不是Unicode",szFileFath.c_str());
					MessageOut(errorStr);
					delete[] buf;
					buf = NULL;
					return false;
				}

				std::wstring textValue = buf;
				delete[] buf;

				I32 rowOldPos = -1;
				I32 rowNewPos = -1;

				//行数据
				rowNewPos = textValue.find(L"\r\n");
				for(I32 i = 0; rowNewPos != rowOldPos; i++)
				{
					I32 colOldPos = -1;
					I32 colNewPos = -1;
					
					std::wstring row;
					if (rowNewPos != -1)
					{
						textValue.replace(rowNewPos,2,1,L'@');
						row = textValue.substr(rowOldPos + 1,rowNewPos - rowOldPos - 1);
					}
					else
					{
						row = textValue.substr(rowOldPos + 1,textValue.size() - 1);
					}
					
					if (i == 0)//忽略第一行 中文注释
					{
						rowOldPos = rowNewPos;
						rowNewPos = textValue.find(L"\r\n");
					}
					else if (i == 1)//第二行 列键值（英文）
					{
						//col
						I32 colNewPos = row.find(mTypeValue[mType]);
						I32 j = 0;
						for (; colNewPos != -1; j++)
						{
							row.replace(colNewPos ,1,1,L'@');
							std::wstring col = row.substr(colOldPos + 1,colNewPos - colOldPos - 1);
							mColMap[col] = j;
							colOldPos = colNewPos;
							colNewPos = row.find(mTypeValue[mType]);
						}
						std::wstring col = row.substr(colOldPos + 1,row.size() - 1);
						mColMap[col] = j;
						rowOldPos = rowNewPos;
						rowNewPos = textValue.find(L"\r\n");
						mColnum = j + 1;
					}
					else//其余行 数据内容
					{
						I32 colNewPos = row.find(mTypeValue[mType]);
						if ( colNewPos != colOldPos)
						{
							std::wstring col = row.substr(colOldPos + 1,colNewPos - colOldPos - 1);
							I32 id = 0;
							if (MGStrOp::toI32(col.c_str(),id))
							{

								CsvData* data = new CsvData();
								data->mID = id;
								mTableMap[col] = data;
								I32 i = 0;
								for ( i = 0;colNewPos != colOldPos; i++)
								{
									std::wstring col;
									if (colNewPos != -1)
									{
										row.replace(colNewPos,1,1,L'@');
										col = row.substr(colOldPos + 1,colNewPos - colOldPos - 1);
									}
									else
									{
										col = row.substr(colOldPos + 1, row.size() - colOldPos - 1);
									}

									data->mData.push_back(col);
									colOldPos = colNewPos;
									colNewPos = row.find(mTypeValue[mType]);
								}
								//文件内容格式不正确
								DYNAMIC_ASSERT(i == mColnum);
								if (i != mColnum)
								{
									destroy();
									return false;
								}
								mTableVector.push_back(data);

							}
							else
							{
								//id 不是数字
								DYNAMIC_ASSERT(false);
								destroy();
								return false;
							}
						}
						rowOldPos = rowNewPos;
						rowNewPos = textValue.find(L"\r\n");
					}
					
				}

				mReadRowCnt = count();
			}
			return true;
		}
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool CsvReader::create( Char16* memory)
	{
		//文件不是Unicode格式
		DYNAMIC_ASSERT(memory[0] == 0xFEFF);
		if (memory[0] != 0xFEFF)
		{
			delete[] memory;
			memory = NULL;
			return false;
		}

		std::wstring textValue = memory;
		delete[] memory;

		I32 rowOldPos = -1;
		I32 rowNewPos = -1;

		//行数据
		rowNewPos = textValue.find(L"\r\n");
		for(I32 i = 0; rowNewPos != rowOldPos; i++)
		{
			I32 colOldPos = -1;
			I32 colNewPos = -1;

			std::wstring row;
			if (rowNewPos != -1)
			{
				textValue.replace(rowNewPos,2,1,L'@');
				row = textValue.substr(rowOldPos + 1,rowNewPos - rowOldPos - 1);
			}
			else
			{
				row = textValue.substr(rowOldPos + 1,textValue.size() - 1);
			}

			if (i == 0)//忽略第一行 中文注释
			{
				rowOldPos = rowNewPos;
				rowNewPos = textValue.find(L"\r\n");
			}
			else if (i == 1)//第二行 列键值（英文）
			{
				//col
				I32 colNewPos = row.find(mTypeValue[mType]);
				I32 j = 0;
				for (; colNewPos != -1; j++)
				{
					row.replace(colNewPos ,1,1,L'@');
					std::wstring col = row.substr(colOldPos + 1,colNewPos - colOldPos - 1);
					mColMap[col] = j;
					colOldPos = colNewPos;
					colNewPos = row.find(mTypeValue[mType]);
				}
				std::wstring col = row.substr(colOldPos + 1,row.size() - 1);
				mColMap[col] = j;
				rowOldPos = rowNewPos;
				rowNewPos = textValue.find(L"\r\n");
				mColnum = j + 1;
			}
			else//其余行 数据内容
			{
				I32 colNewPos = row.find(mTypeValue[mType]);
				if ( colNewPos != colOldPos)
				{
					std::wstring col = row.substr(colOldPos + 1,colNewPos - colOldPos - 1);
					I32 id = 0;
					if (MGStrOp::toI32(col.c_str(),id))
					{

						CsvData* data = new CsvData();
						data->mID = id;
						mTableMap[col] = data;
						I32 i = 0;
						for ( i = 0;colNewPos != colOldPos; i++)
						{
							std::wstring col;
							if (colNewPos != -1)
							{
								row.replace(colNewPos,1,1,L'@');
								col = row.substr(colOldPos + 1,colNewPos - colOldPos - 1);
							}
							else
							{
								col = row.substr(colOldPos + 1, row.size() - colOldPos - 1);
							}

							data->mData.push_back(col);
							colOldPos = colNewPos;
							colNewPos = row.find(mTypeValue[mType]);
						}
						//文件内容格式不正确
						DYNAMIC_ASSERT(i == mColnum);
						if (i != mColnum)
						{
							destroy();
							return false;
						}
						mTableVector.push_back(data);

					}
					else
					{
						//id 不是数字
						DYNAMIC_ASSERT(false);
						destroy();
						return false;
					}
				}
				rowOldPos = rowNewPos;
				rowNewPos = textValue.find(L"\r\n");
			}

		}

		mReadRowCnt = count();
		return true;
	}
	//-----------------------------------------------------------------------------
	Bool CsvReader::saveToFile( std::wstring filename, Char8* memory, I32 size )
	{
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool CsvReader::saveToFile(std::wstring filename)
	{
		Char16 colnote[1024] = {};
		Char16 colname[1024] = {};

		if(_wfopen_s(&mFileStream,filename.c_str(),L"r+b") == 0)
		{
			fgetws(colnote, 1024, mFileStream);
			fgetws(colname, 1024, mFileStream);

			fclose(mFileStream);
		}

		if(_wfopen_s(&mFileStream,filename.c_str(),L"w+b") == 0)
		{
			CsvData* csvdata = NULL;

			Char8 uni[2] = {(char)(0xff),(char)(0xfe)};     

			fwrite(uni, 2, 1, mFileStream);

			fwrite((void*)(&colnote[1]), sizeof(Char16), wcslen(colnote) - 1, mFileStream);
			fwrite((void*)colname, sizeof(Char16), wcslen(colname), mFileStream);

			for(UInt i = 0/*mReadRowCnt*/; i < mTableVector.size(); ++i)
			{
				std::wstring buf;
				csvdata = mTableVector[i];
				if(csvdata)
				{
					for(UInt j = 0; j < csvdata->mData.size(); ++j)
					{
						buf += csvdata->mData[j];
						if(j == csvdata->mData.size() - 1)
						{
							buf += L"\r\n";
						}
						else
						{
							buf += L",";
						}
					}	
				}

				fwrite((void*)(buf.c_str()), sizeof(Char16), buf.size(), mFileStream);
			}
			
			fclose(mFileStream);
		}

		return true;
	}
	//-----------------------------------------------------------------------------
	void CsvReader::destroy()
	{
		if (mFileStream != NULL)
		{
			fclose(mFileStream);
			mFileStream = NULL;
		}

		TabMapIter it = mTableMap.begin();
		for (; it != mTableMap.end(); it++)
		{
			delete it->second;
			it->second = NULL;
		}
		mTableMap.clear();
		mTableVector.clear();
		mColMap.clear();
	}
	//-----------------------------------------------------------------------------
	void CsvReader::setType( CSV_TYPE type )
	{
		mType = type;
	}
	//-----------------------------------------------------------------------------
	CsvReader::CSV_TYPE CsvReader::getType()
	{
		return mType;
	}
	//-----------------------------------------------------------------------------
	std::wstring CsvReader::findValue( std::wstring rowKey, std::wstring colKey )
	{
		TabMapIter it = mTableMap.find(rowKey);
		TabMapIter itEnd = mTableMap.end();
		if (it != itEnd && it->second != NULL)
		{
			return _findValue(it->second,colKey);
		}
		return L"";
	}
	//-----------------------------------------------------------------------------
	std::wstring CsvReader::findValue( I32 row, std::wstring colKey )
	{
		if (row < count())
		{
			return _findValue(mTableVector[row],colKey);
		}
		return L"";
	}
	//-----------------------------------------------------------------------------
	std::wstring CsvReader::findValue( I32 row, I32 col )
	{
		if (row < count() && col < getColumnSize())
		{
			return mTableVector[row]->mData[col];
		}
		return L"";
	}
	//-----------------------------------------------------------------------------
	CsvData* CsvReader::findValue( std::wstring rowKey )
	{
		TabMapIter it = mTableMap.find(rowKey);
		TabMapIter itEnd = mTableMap.end();
		if (it != itEnd)
		{
			return it->second;
		}
		return NULL;
	}
	//-----------------------------------------------------------------------------
	CsvData* CsvReader::findValue( I32 row )
	{
		if (row < count())
		{
			return mTableVector[row];
		}
		return NULL;
	}
	//-----------------------------------------------------------------------------
	std::wstring CsvReader::getColName( I32 index )
	{
		for (ColMapIter it = mColMap.begin(); it != mColMap.end(); it++)
		{
			if (it->second == index)
			{
				return it->first;
			}
		}
		return L"";
	}
	//-----------------------------------------------------------------------------
	I32 CsvReader::getColIndex( std::wstring colKey )
	{
		ColMapIter it = mColMap.find(colKey);
		ColMapIter itEnd = mColMap.end();
		if (it != itEnd)
		{
			return it->second;
		}
		return INDEX_ERR;
	}
	//-----------------------------------------------------------------------------
	Char8* CsvReader::readFile( const Char16* filename, I32& size )
	{
		return NULL;
	}
	//-----------------------------------------------------------------------------
	Bool CsvReader::addValue( std::wstring rowKey, CsvData* data )
	{
		TabMapIter it = mTableMap.find(rowKey);
		TabMapIter itEnd = mTableMap.end();
		if (data != NULL && it == itEnd)
		{
			mTableMap[rowKey] = data;
			mTableVector.push_back(data);
			return true;
		}
		return false;
	}
	//-----------------------------------------------------------------------------
	void CsvReader::getValue( std::wstring& rowKey, const I32& keyLen, CsvData* csvData )
	{

	}
	//-----------------------------------------------------------------------------
	Bool CsvReader::newAddValue( CsvData* csvData )
	{
		return false;
	}

	std::wstring CsvReader::_findValue( CsvData* rowData,std::wstring colKey )
	{
		if (rowData != NULL)
		{
			I32 index = getColIndex(colKey.c_str());
			if (index != INDEX_ERR && index < getColumnSize())
			{
				return rowData->mData[index];
			}
		}
		return L"";
	}


	//-----------------------------------------------------------------------------
}