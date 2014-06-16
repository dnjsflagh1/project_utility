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

        ///  ��ʹ��
        virtual Bool        initialize();
        virtual Bool        unInitialize();

        /// �����ļ�
                Bool        dofile( const char *filename );

        ///  ��ʹ��lua_Stateָ��
        void                restore(void);

        ///  �õ���lua_Stateָ��
        lua_State*          getMainLuaState(void);
        ///  �õ���ǰ�����lua_Stateָ�� 
        lua_State*          getActiveLuaState(void);

        ///  ���c,lua����ջ�Ŀռ�
        void                clearStack(void);
        /// �õ�c,lua����ջ�Ŀռ�����Ԫ�ص�����
        Int                 getStackElementCount(void);

        /** �õ�c,lua����ջ�Ŀռ�����ָ��Ԫ�ص����� 
        @index : ����->   ջ��Ϊ1,  ջ��Ϊlength
        ����->   ջ��Ϊ-length,  ջ��Ϊ-1
        */
        Bool                isString(Int index);
        Bool                isNumber(Int index);
        Bool                isBoolean(Int index);
        Bool                isTable(Int index);

        ///  �õ�c,lua����ջ�Ŀռ�����ָ��Ԫ�ص�ֵ 
        Str                 getString(Int index);
        Int                 getNumber(Int index);
        Bool                getBoolean(Int index);

        ///  ѹ��Ԫ�ص�c,lua����ջ
        void                setString(const Str& value, Int index);
        void                setNumber(Int value, Int index);

        ///  ɾ��c,lua����ջ�Ŀռ�ָ��λ��Ԫ��
        void                removeStackElement(Int index);

        ///  ��ӡ��ջ
        static void         printStack(lua_State *L);

    public:

        /// �����Լ��ļ���
        void                registerMyFileLoader();
        
    public:
        /**  �ӽ���ջ��ָ����ʽ���еõ�String�б�
        table = { string, string, ...}	
        */
        void                getStringListFromTable( std::vector<Str>& slt, Int index );

		/**  �ӽ���ջ��ָ����ʽ���еõ�Int�б�
        table = { int, int, ...}
		*/
		void                getIntListFromTable( std::vector<Int>& slt, Int index );

        /**  �ӽ���ջ��ָ����ʽ���еõ�Vector3�б�
        table = { {x,y,z},{x,y,z} }
        */
        void                getVector3ListFromTable( std::vector<Vec3>& pointList, Int index );

        /**  �ӽ���ջ��ָ����ʽ���еõ�Vector3
        table = {x,y,z}
        */
        Vec3                getVector3FromTable(Int index);


        /**  �ӽ���ջ��ָ����ʽ���еõ�Vector2
        table = {x,y}
        */
        Vec2                getVector2FromTable(Int index);

        /**  �ӽ���ջ��ָ����ʽ���еõ�Orientation
        table = {w,x,y,z}
        */
        Vec4                getQuaternionFromTable(Int index);

        /**  �ӽ���ջ��ָ����ʽ���еõ�ColourValue
        table = {r,g,b,a}
        */
        Color               getColourValueFromTable(Int index);

    public:

        /**  �ӽ���ջ�����еõ�ָ��Keyֵ����ֵ�ͱ���
        return table[name]
        */
        Flt                 getNumberField (const Str& key, Int tableindex, Int defaultindex);
		Flt					getNumberField (const Str& key, Int tableindex, Bool& isError);

        /**  �ӽ���ջ�����еõ�ָ��Keyֵ���ַ�������
        return table[name]
        */
        Str                 getStringField (const Str& key, Int tableindex, Int defaultindex);
		Str					getStringField (const Str& key, Int tableindex);

        /**  ��������ջ��ָ����ʽ��ΪString�б�
        table = { string, string, ...}	
        */
        void                createStringListTable(const std::vector<Str>& slt );

        /**  ��������ջ��ָ����ʽ��ΪQuadrangle�б�
        table = { {v1,v2,v3,v4},{v1,v2,v3,v4},...}	
        */
        void                createQuadrangleListTable(const std::list<Vec4>& qmd );

        /**  ��������ջ��ָ����ʽ��ΪVector3�б�
        table = { {x,y,z},{x,y,z} }
        */
        void                createVector3ListTable(const std::vector<Vec3>& pointList);

        /**  ��������ջ��ָ����ʽ��ΪVector3
        table = {x,y,z}
        */
        void                createVector3Table(const Vec3& vec);

        /**  ��������ջ��ָ����ʽ��ΪQuaternion
        table = {w,x,y,z}
        */
        void                createQuaternionTable(const Vec4& quaternion);

        /**  ��������ջ��ָ����ʽ��ΪColourValue
        table = {r,g,b,a}
        */
        void                createColourValueTable(const Color& colourValue);

		/**  ���ý���ջ������ָ��Keyֵ��Ӧvalue
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

        /// ִ��lua_pcall
        Bool                luaPcall( int nargs, int nresults );
        /// ִ��StringChunk 
        void                doStringChunk( const Str& chunk );

        /// ת��luaȫ�ֱ����������� 
        void                convertGlobalVariableToPool( const Str& luaVariable );
        /// �õ����������� 
        size_t              getVariableCountInPool(void);
        /// �õ�������ָ���������� 
        const Str&          getVariableInPool( size_t index );
        /// �õ�������ָ������������ʶ 
        const Str&          getMarkInPool( size_t index );
        /// �õ�������ָ����ʾ���� 
        const Str&          getVariableInPool( const Str& mark );

    private:

        /// ת��ָ����ջ�ϵ�(�ַ��������֣��߼�)����Ϊ�ַ���
        static void         convertLuaStackValueToString( lua_State* L, Int stackindex, Str& convertstring );

        /// ת��ָ����ջ�ϵı�����������
        void                convertLuaStackValueToPool( Int stackindex);

        /// ��ӱ�����������
        void                pushValueToPool( const Str& value, const Str& mark = "");
        /// ��ձ����� 
        void                clearVariablePool(void);

    public:

        void                startAssertStackBalance(void);
        void                endAssertStackBalance(void);

		// MG_CN_MODIFY
		void				setPackManager(IMGExternalPackManager* packManager);
		static IMGExternalPackManager*		getPackManager();
    private:
		static IMGExternalPackManager*		mPackManager;

        //lua�����
        lua_State* mLuaState;

        //�����ر���ֵ
        std::vector<Str> mPoolVariables;
        //�����ر���ֵ��ʶ
        std::vector<Str> mPoolVariableMarks;

        //��ջ���Ե�ǰǶ�״���
        Int mAssertStackCount;
        //��ջ��С
        Int mStackNumber;
    };

}


#endif