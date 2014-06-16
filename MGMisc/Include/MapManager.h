//////////////////////////////////////////////////////////////
//
// Copyright (C), 2011, MG Software Co., Ltd. 
//
//      FileName: e:\MGProgram\Trunk\Projects\Common\Utility\MGUtility\Include\MapManager.h
//        Author: yuanlinhu
//          Date: 2012-3-13
//          Time: 17:39
//   Description: 
//			����ָ�룬 ֻ���� ��ӣ����ң� ɾ����
//			�ڴ�Ĵ������ͷ����ⲿ����
//
// modify History:
//      <author>    <time>        <descript>
//      yuanlinhu  2012-3-13      add
//////////////////////////////////////////////////////////////

/******************************************************************************/
#ifndef _MAP_MANAGER_H_
#define _MAP_MANAGER_H_
/******************************************************************************/

#include <map>

/******************************************************************************/
namespace MG
{
	template<typename TYPE_KEY, typename TYPE_CLASS>
	class MapManager
	{
	public:	
		void			addData(TYPE_KEY key, TYPE_CLASS object);
		void			eraseData(TYPE_KEY key);
		Bool			hasData(TYPE_KEY key);
		TYPE_CLASS		getSecondData(TYPE_KEY key);
		void			clearData();

		std::map<TYPE_KEY, TYPE_CLASS>*		getDataList();

		//typedef std::map<TYPE_KEY, TYPE_CLASS*>						DataSetMap;
		//typedef typename std::map<TYPE_KEY, TYPE_CLASS*>::iterator	DataSetMapIter;

	private:
		

		std::map<TYPE_KEY, TYPE_CLASS>			mDataList;
	};

	/******************************************************************************/
	// SimpleMapDataSetManager Definition
	/******************************************************************************/
	template<typename TYPE_KEY, typename TYPE_CLASS>
	void MapManager<TYPE_KEY,TYPE_CLASS>::addData(TYPE_KEY key, TYPE_CLASS object)
	{
		assert(NULL != object);

		if (true == hasData(key))
		{
			assert(0);
			return;
		}

		mDataList[key] = object;
			
	}

	//-----------------------------------------------------------------------------
	template<typename TYPE_KEY, typename TYPE_CLASS>
	void MapManager<TYPE_KEY,TYPE_CLASS>::eraseData(TYPE_KEY key)
	{
		std::map<TYPE_KEY, TYPE_CLASS>::iterator iter = mDataList.find(key);
		if (iter != mDataList.end())
		{
			mDataList.erase(iter);
		}
	}

	//-----------------------------------------------------------------------------
	template<typename TYPE_KEY, typename TYPE_CLASS>
	Bool MapManager<TYPE_KEY,TYPE_CLASS>::hasData(TYPE_KEY key)
	{
		return Bool(NULL != getSecondData(key));
	}

	//-----------------------------------------------------------------------------
	template<typename TYPE_KEY, typename TYPE_CLASS>
	TYPE_CLASS MapManager<TYPE_KEY,TYPE_CLASS>::getSecondData(TYPE_KEY key)
	{
		TYPE_CLASS object = NULL;
		
		std::map<TYPE_KEY, TYPE_CLASS>::iterator it = mDataList.find(key);
		if ( it != mDataList.end() )
		{
			object = it->second;
		}

		return object;
	}

	//-----------------------------------------------------------------------------
	template<typename TYPE_KEY, typename TYPE_CLASS>
	void MapManager<TYPE_KEY,TYPE_CLASS>::clearData()
	{
		mDataList.clear();
	}

	//-----------------------------------------------------------------------------
	template<typename TYPE_KEY, typename TYPE_CLASS>
	std::map<TYPE_KEY, TYPE_CLASS>* MapManager<TYPE_KEY,TYPE_CLASS>::getDataList()
	{
		return &mDataList;
	}
}

#endif	//_MAP_MANAGER_H_