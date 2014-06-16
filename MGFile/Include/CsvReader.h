/******************************************************************************/
#ifndef _CSVREADER_H_
#define _CSVREADER_H_
/******************************************************************************/

           
/******************************************************************************/
namespace MG
{
	/******************************************************************************/
	//CSV ÎÄ¼þ¶ÁÈ¡
	/******************************************************************************/

	#pragma pack (push)
	#pragma pack (1)
	//row data
	struct CsvData					
	{
		std::vector< std::wstring >	mData;
		U32							mID;
	};
	#pragma pack (pop)

	// MG_CN_MODIFY
	class IMGExternalPackManager;

	class CsvReader
	{
	public:
		enum CSV_TYPE
		{
			CSV_COMMA = 0,
			CSV_TAB,
			CSV_ENCODE,
			CSV_MAX
		};
		enum ERR
		{
			INDEX_ERR = -1
		};

	public:
		CsvReader();
		~CsvReader();

	public:
		// MG_CN_MODIFY
		Bool			create( const std::wstring filename, IMGExternalPackManager* packManager);
		Bool			create( const std::wstring fileName);	//Open csv by filename
		Bool			create(Char16* memory);		//Open csv by memory, filename use for display error when file is not in unicode
		Bool			saveToFile(std::wstring filename, Char8* memory, I32 size);	//Save the file memory to file
		Bool			saveToFile(std::wstring filename);
		void			destroy();													//Clear data
		void			setType(CSV_TYPE type);										//Set the sperator value, default is comma ','
		CSV_TYPE		getType();													//Get the current sperator use

		std::wstring	findValue(std::wstring rowKey, std::wstring colKey);				//Find value by row and col, using string. The 2nd row of csv is the col's key
		std::wstring	findValue(I32 rowIndex, std::wstring colKey);						//Find value by row and col, row using index, col using string.
		std::wstring	findValue(I32 rowIndex, I32 colIndex);								//Find value by row and col, using index.
		CsvData*		findValue(std::wstring rowKey);								//Find value by key. The 1st column of csv is the key value. Must be unique!
		CsvData*		findValue(I32 rowIndex);											//Find value by row's index. Return the row of csv by row number.

		std::wstring	getColName(I32 index);										//Get Column Key Name by col index
		I32				getColIndex(std::wstring colName);									//Get Column Index Name by col key
		I32				count(){return (I32)mTableVector.size();}					//Total record in csv
		I32				getColumnSize(){return mColnum;}							//Total column of csv, just calculate the 1st header column size.
		Bool			addValue(std::wstring key, CsvData* data);
	
	protected:
		typedef std::map< std::wstring, CsvData* >::iterator	TabMapIter;
		typedef	std::vector< CsvData* >::iterator				TabVecIter;
		typedef std::map< std::wstring, I32 >::iterator			ColMapIter;
		typedef std::vector< std::wstring >::iterator			CsvRowDataIter;

	protected:
		Char8*			readFile(const Char16* fileName, I32& size);
		/*Bool			addValue(std::wstring key, CsvData* data);*/
		void			getValue(std::wstring& rowKey, const I32& keyLen, CsvData* csvData);
		Bool			newAddValue(CsvData* csvData);
	
	private:
		std::wstring	_findValue(CsvData* rowData,std::wstring colKey);
	
	private:
		//key	rowData
		std::map< std::wstring, CsvData* >		mTableMap;
		//count	rowData
		std::vector< CsvData* >					mTableVector;
		//col	map
		std::map< std::wstring, I32 >			mColMap;

		I32										mColnum;

		CSV_TYPE								mType;
		Char16									mTypeValue[CSV_MAX];

		FILE									*mFileStream;
		I32										mReadRowCnt;

	};

}

/******************************************************************************/

#endif