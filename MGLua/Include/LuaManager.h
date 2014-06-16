/******************************************************************************/
#ifndef _LUAMANAGER_H
#define _LUAMANAGER_H
/******************************************************************************/

    #ifdef __cplusplus    // If used by C++ code, 
    extern "C" {          // we need to export the C interface
    #endif
    #include "lua.h"
    #include "lstate.h"
    #include "lauxlib.h"
    #include "lualib.h"
    #ifdef __cplusplus
    }
    #endif

/******************************************************************************/
namespace MG
{
    /**
    -----------------------------------------------------------------------------
    LuaManager.h
    -----------------------------------------------------------------------------
    */
	// MG_CN_MODIFY
	class IMGExternalPackManager;

    class LuaManager
    {
    public:
        LuaManager();
        virtual ~LuaManager();

        ///  初使化
        virtual Bool        initialize();
        virtual Bool        unInitialize();

        /// 加载文件
                Bool        dofile( const char *filename );

        ///  初使化lua_State指针
        void                restore(void);

        ///  得到主lua_State指针
        lua_State*          getMainLuaState(void);
        ///  得到当前激活的lua_State指针 
        lua_State*          getActiveLuaState(void);

        ///  清空c,lua交互栈的空间
        void                clearStack(void);
        /// 得到c,lua交互栈的空间里面元素的数量
        Int                 getStackElementCount(void);

        /** 得到c,lua交互栈的空间里面指定元素的类型 
        @index : 正数->   栈底为1,  栈顶为length
        负数->   栈底为-length,  栈顶为-1
        */
        Bool                isString(Int index);
        Bool                isNumber(Int index);
        Bool                isBoolean(Int index);
        Bool                isTable(Int index);

        ///  得到c,lua交互栈的空间里面指定元素的值 
        Str                 getString(Int index);
        Int                 getNumber(Int index);
        Bool                getBoolean(Int index);

        ///  压入元素到c,lua交互栈
        void                setString(const Str& value, Int index);
        void                setNumber(Int value, Int index);

        ///  删除c,lua交互栈的空间指定位置元素
        void                removeStackElement(Int index);

        ///  打印堆栈
        static void         printStack(lua_State *L);

    public:

        /// 设置自己的加载
        void                registerMyFileLoader();
        
    public:
        /**  从交互栈中指定格式表中得到String列表
        table = { string, string, ...}	
        */
        void                getStringListFromTable( std::vector<Str>& slt, Int index );

		/**  从交互栈中指定格式表中得到Int列表
        table = { int, int, ...}
		*/
		void                getIntListFromTable( std::vector<Int>& slt, Int index );

        /**  从交互栈中指定格式表中得到Vector3列表
        table = { {x,y,z},{x,y,z} }
        */
        void                getVector3ListFromTable( std::vector<Vec3>& pointList, Int index );

        /**  从交互栈中指定格式表中得到Vector3
        table = {x,y,z}
        */
        Vec3                getVector3FromTable(Int index);


        /**  从交互栈中指定格式表中得到Vector2
        table = {x,y}
        */
        Vec2                getVector2FromTable(Int index);

        /**  从交互栈中指定格式表中得到Orientation
        table = {w,x,y,z}
        */
        Vec4                getQuaternionFromTable(Int index);

        /**  从交互栈中指定格式表中得到ColourValue
        table = {r,g,b,a}
        */
        Color               getColourValueFromTable(Int index);

    public:

        /**  从交互栈顶表中得到指定Key值，数值型变量
        return table[name]
        */
        Flt                 getNumberField (const Str& key, Int tableindex, Int defaultindex);
		Flt					getNumberField (const Str& key, Int tableindex, Bool& isError);

        /**  从交互栈顶表中得到指定Key值，字符串变量
        return table[name]
        */
        Str                 getStringField (const Str& key, Int tableindex, Int defaultindex);
		Str					getStringField (const Str& key, Int tableindex);

        /**  创建交互栈中指定格式表为String列表
        table = { string, string, ...}	
        */
        void                createStringListTable(const std::vector<Str>& slt );

        /**  创建交互栈中指定格式表为Quadrangle列表
        table = { {v1,v2,v3,v4},{v1,v2,v3,v4},...}	
        */
        void                createQuadrangleListTable(const std::list<Vec4>& qmd );

        /**  创建交互栈中指定格式表为Vector3列表
        table = { {x,y,z},{x,y,z} }
        */
        void                createVector3ListTable(const std::vector<Vec3>& pointList);

        /**  创建交互栈中指定格式表为Vector3
        table = {x,y,z}
        */
        void                createVector3Table(const Vec3& vec);

        /**  创建交互栈中指定格式表为Quaternion
        table = {w,x,y,z}
        */
        void                createQuaternionTable(const Vec4& quaternion);

        /**  创建交互栈中指定格式表为ColourValue
        table = {r,g,b,a}
        */
        void                createColourValueTable(const Color& colourValue);

		/**  设置交互栈顶表中指定Key值对应value
		table[key] = value
		*/
		void                setTableFieldByValue(const Str& key, Flt value, Int tableindex);
		void                setTableFieldByValue(const Str& key, Str value, Int tableindex);
		void                setTableFieldByValue(size_t key, Str value, Int tableindex);
		void                setTableFieldByValue(size_t key, Flt value, Int tableindex);

		void                setTableFieldByIndex(const Str& key, Int valuetableindex, Int tableindex);
		void                setTableFieldByIndex(size_t key, Int valuetableindex, Int tableindex);

    protected:



    public:

        /// 执行lua_pcall
        Bool                luaPcall( int nargs, int nresults );
        /// 执行StringChunk 
        void                doStringChunk( const Str& chunk );

        /// 转变lua全局变量到变量池 
        void                convertGlobalVariableToPool( const Str& luaVariable );
        /// 得到变量池数量 
        size_t              getVariableCountInPool(void);
        /// 得到变量池指定索引变量 
        const Str&          getVariableInPool( size_t index );
        /// 得到变量池指定索引变量标识 
        const Str&          getMarkInPool( size_t index );
        /// 得到变量池指定标示变量 
        const Str&          getVariableInPool( const Str& mark );

    private:

        /// 转变指定堆栈上的(字符串，数字，逻辑)变量为字符串
        static void         convertLuaStackValueToString( lua_State* L, Int stackindex, Str& convertstring );

        /// 转变指定堆栈上的变量到变量池
        void                convertLuaStackValueToPool( Int stackindex);

        /// 添加变量到变量池
        void                pushValueToPool( const Str& value, const Str& mark = "");
        /// 清空变量池 
        void                clearVariablePool(void);

    public:

        void                startAssertStackBalance(void);
        void                endAssertStackBalance(void);

		// MG_CN_MODIFY
		void				setPackManager(IMGExternalPackManager* packManager);
		static IMGExternalPackManager*		getPackManager();
    private:
		static IMGExternalPackManager*		mPackManager;

        //lua虚拟机
        lua_State* mLuaState;

        //变量池变量值
        std::vector<Str> mPoolVariables;
        //变量池变量值标识
        std::vector<Str> mPoolVariableMarks;

        //堆栈断言当前嵌套次数
        Int mAssertStackCount;
        //堆栈大小
        Int mStackNumber;
    };

}


#endif