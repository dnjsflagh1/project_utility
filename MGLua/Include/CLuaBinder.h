/******************************************************************************/
#ifndef _CLUABINDER_H
#define _CLUABINDER_H
/******************************************************************************/

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
#ifdef __cplusplus
}
#endif

/******************************************************************************/
namespace MG
{
    
    #define LUA_RETRIEVE_OBJECT( C_CLASS, LUA_CLASS, LUA_STATE ) static_cast<C_CLASS*>( LUA_CLASS::retrieveObject(LUA_STATE, LUA_CLASS::LUA_TYPE_NAME.c_str()) );

	#define LUA_INDEX_RETRIEVE_OBJECT( C_CLASS, LUA_CLASS, LUA_STATE, INDEX ) static_cast<C_CLASS*>( LUA_CLASS::retrieveObject(LUA_STATE, LUA_CLASS::LUA_TYPE_NAME.c_str(),INDEX) );

    #define LUA_RETURN_OBJECT( C_OBJECT, LUA_CLASS, LUA_STATE ) LUA_CLASS::createUserData(LUA_STATE, C_OBJECT, LUA_CLASS::LUA_TYPE_NAME.c_str());
    
    /**
    -----------------------------------------------------------------------------
    CLuaBinder C和Lua的数据绑定Reference对象的基类
    -----------------------------------------------------------------------------
    */
    struct CLuaReference
    {
    public:
        CLuaReference(void):m_isGc(false){};
        CLuaReference(bool isGc):m_isGc(isGc){};
        virtual ~CLuaReference()
        { 
            releaseLuaReference();
        };
        
        //--------------------------------------
        void setLuaReference(lua_State* L)
        {	
            int ref = luaL_ref(L, LUA_REGISTRYINDEX);
            mLuaReferenceMap[L] = ref;
        };

        //--------------------------------------
        bool getLuaReference(lua_State* L)
        {
            std::map<lua_State*, int>::iterator mi = mLuaReferenceMap.find(L);
            if ( mi != mLuaReferenceMap.end() )
            {
                lua_rawgeti(L, LUA_REGISTRYINDEX, mLuaReferenceMap[L]);
                return true;
            }
            return false;
        };

        //--------------------------------------
        void releaseLuaReference(void)
        {
            for( std::map<lua_State*, int>::iterator mi = mLuaReferenceMap.begin(); mi!=mLuaReferenceMap.end(); ++mi )
            {
                luaL_unref(mi->first, LUA_REGISTRYINDEX, mi->second); 
            }
        };

        //--------------------------------------
        void setGcEnable(bool isGc)
        {
            m_isGc = isGc;
        };

        //--------------------------------------
        bool isGc(void)
        {
            return m_isGc;
        };
    private:
        bool m_isGc;
        std::map<lua_State*, int> mLuaReferenceMap;
    };

    /**
    -----------------------------------------------------------------------------
    CLuaBinder C和Lua的数据绑定对象的基类
    -----------------------------------------------------------------------------
    */
    template<class _T>
    class CLuaBinder 
    {
    public:
        CLuaBinder(){};
        virtual ~CLuaBinder(){};

        virtual const Str& getLuaTypeName() const = 0;

        /**  注册Lua  */
        void LuaRegister(lua_State* L)
        {
            Str name = getLuaTypeName();
            createLuaMethodMetaTable(L, name);
        }

        /**  创建数据MetaTable  */
        static bool createLuaDataMetaTable(lua_State* L, const Str& metatablename, bool isGc = false)
        {
            const char* luaTypeName = metatablename.c_str();  //lua_pushstring(L,luaTypeName);
            //创建MetaTable
            if (luaL_newmetatable(L,luaTypeName) != 0) {

                /* metatable.__index = metatable */
                lua_pushstring(L, "__index");
                lua_pushvalue(L, -2);
                lua_rawset(L, -3);  

                /* metatable.__newindex = metatable */
                lua_pushstring(L, "__newindex");
                lua_pushvalue(L, -2);
                lua_rawset(L, -3);  

                if ( isGc )
                {
                    lua_pushstring(L, "__gc");
                    lua_pushcfunction(L, delTarget);
                    lua_rawset(L, -3);  
                }

                //MG_LOG( out_info, ("create new LuaDataMetaTable <%s> \n"), metatablename.c_str() );

                //delete registry info
                lua_pushstring(L, luaTypeName);
                lua_pushnil( L );  
                lua_settable(L, LUA_REGISTRYINDEX);

                return true;
            }else
                return false;
        }

        /**  创建方法MetaTable  */
        void createLuaMethodMetaTable(lua_State* L, const Str& metatablename)
        {
            RegisterMethods();
            RegisterMembers();

            const char* luaTypeName = metatablename.c_str();
            //创建MetaTable
            if (luaL_newmetatable(L,luaTypeName) != 0) {

                //暂时把lib放在metatable里
                /* metatable.__index = metatable */
                lua_pushstring(L, "__index");
                lua_pushvalue(L, -2);
                lua_rawset(L, -3);  

                struct luaL_reg* pMethodBuffer = MG_NEW luaL_reg[mMethod.size()+1];
                for (unsigned int i=0;i<mMethod.size();++i) 
                    pMethodBuffer[i] = mMethod[i];
                pMethodBuffer[mMethod.size()] = make_luaL_reg(0,0);

                //加载pMethodBuffer存放的函数到栈顶的表
                luaL_openlib(L, NULL, pMethodBuffer, 0); // = luaL_register in lua5.1

                MG_DELETE_ARRAY(pMethodBuffer);
            }

        }

        struct luaL_reg make_luaL_reg(const char *name, lua_CFunction func)
        {
            struct luaL_reg s;
            s.name = name;
            s.func = func;
            return s;
        }

        /** 检测Lua栈中index位置元素是否为userdata,
        如果是则再检测userdata的MetaTable名字是否为luaTypename,
        如果正确则返回_T类型指针
        如果错误则返回0
        */
        static _T* retrieveObject( lua_State* L, const Str& luaTypename, int index = 1 )
        {
            udata* o;
            o = (udata*)lua_touserdata(L, index); //(udata*)luaL_checkudata(L, index, luaTypename.c_str());

            if ( !o )
            {
                //MG_LOG( out_error, ("no userData from stack of index Or Wrong userData Type <%s> \n"), luaTypename.c_str() );
            }

            if (!o->ptr) 
            {
                //MG_LOG( out_error, ("Null udata->ptr, CLuaBinder<class _T>::retrieveObject() \n"), luaTypename.c_str() );
                return 0;
            }
            else
                return (o->ptr);
        }

        /** 
        gc时释放lua_userdata中的c对象
        */
        static int delTarget(lua_State *L)
        {
            udata* o;
            o = (udata*)lua_touserdata(L, 1); 

            MG_DELETE_ARRAY(o->ptr);
            return 0;
        }

        /** 
        创建Lua userData( 用作 struct udata) 并压到Lua栈顶,
        Para: luaTypename: userData的MetaTable名
        如果正确则返回1
        如果错误则返回0
        */
        static int createUserData( lua_State* L, _T* target, Str methodMetaTableName, Str dataMetaTablename = "" )
        {
            if ( target )
            {
                if ( ! target->getLuaReference(L) )
                {
                    udata* o = (udata*)lua_newuserdata( L, sizeof(udata) );
                    o->ptr = target;

                    if ( !target->isGc() )
                    {
                        target->setLuaReference(L);
                        target->getLuaReference(L);
                    }

                    if (dataMetaTablename == "")
                    {
                        dataMetaTablename = methodMetaTableName + "_DATAMetaTable";
                    }

                    if ( createLuaDataMetaTable(L, dataMetaTablename, target->isGc()) )
                    {
                        luaL_getmetatable(L, methodMetaTableName.c_str());
                        lua_setmetatable(L, -2);
                        lua_setmetatable(L, -2);
                    }else
                    {
                        lua_setmetatable(L, -2);
                    }
                }
                return 1;
            }else
            {
                return 0;
            }
        }

    protected:

        virtual void RegisterMethods(){};
        virtual void RegisterMembers(){};

    public:

        /** 用于 Lua userdata  */
        struct udata 
        { 
            _T* ptr; 
        };

        /** userdata 的 MetaTable 方法  */
        std::vector<struct luaL_reg> mMethod;
#define REGISTER_METHOD_CCLASS(methodname, CClass) mMethod.push_back(make_luaL_reg(#methodname,&CClass::methodname));
    };

      
}


#endif