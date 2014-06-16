/******************************************************************************/
#include "stdafx.h"
#include "XmlHelper.h"
#include "MGStrOp.h"
/******************************************************************************/

    using namespace tinyxml2;

namespace MG
{
    //-----------------------------------------------------------------------------
    UInt XmlHelper::getChildCount(XMLNode *elm)
    {
        XMLNode*  currNode            = elm->FirstChild();
        XMLNode*  firstSiblingNode    = currNode;
        XMLNode*  nextNode            = NULL;
        UInt        count               = 0;

        while( currNode )
        {
            //@ parallel 
            nextNode    = currNode->NextSibling();
            if (nextNode&&nextNode==firstSiblingNode)
            {
                break;
            }
            currNode = nextNode;

            count ++;
        }

        return count;
    }

    //-----------------------------------------------------------------------------
    XMLElement* XmlHelper::getChild(XMLNode *elm, Str8 tagName)
    {
        XMLNode* child                = NULL;
        XMLElement* child_elm         = NULL;
        if (elm)
        {
            for( child = elm->FirstChild(); child; child = child->NextSibling() )
            {
                child_elm = child->ToElement();
                if ( tagName==XML_GET_VALUE(child_elm) )
                {
                    return child_elm;
                }
            }
        }
        return NULL;
    }
    //-----------------------------------------------------------------------------
    XMLElement* XmlHelper::getChild(XMLNode *elm, Str8 tagName, Str8 attr, Str8 attrName)
    {
        XMLNode* child                = NULL;
        XMLElement* child_elm         = NULL;
        Str8 childAttr;
        if (elm)
        {
            for( child = elm->FirstChild(); child; child = child->NextSibling() )
            {
                child_elm = child->ToElement();
                if ( tagName==XML_GET_VALUE(child_elm) )
                {
                    getAttribute(child_elm,childAttr,attrName);
                    if ( childAttr == attr )
                        return child_elm;
                }
            }
        }
        return NULL;
    }

	//-----------------------------------------------------------------------------
	const XMLElement* XmlHelper::getChild(const XMLNode *elm, Str8 tagName)
	{
		const XMLNode* child                = NULL;
		const XMLElement* child_elm         = NULL;
		if (elm)
		{
			for( child = elm->FirstChild(); child; child = child->NextSibling() )
			{
				child_elm = child->ToElement();
				if ( tagName==XML_GET_VALUE(child_elm) )
				{
					return child_elm;
				}
			}
		}
		return NULL;
	}
	//-----------------------------------------------------------------------------
	const XMLElement* XmlHelper::getChild(const XMLNode *elm, Str8 tagName, Str8 attr, Str8 attrName)
	{
		const XMLNode* child                = NULL;
		const XMLElement* child_elm         = NULL;
		Str8 childAttr;
		if (elm)
		{
			for( child = elm->FirstChild(); child; child = child->NextSibling() )
			{
				child_elm = child->ToElement();
				if ( tagName==XML_GET_VALUE(child_elm) )
				{
					getAttribute(child_elm,childAttr,attrName);
					if ( childAttr == attr )
						return child_elm;
				}
			}
		}
		return NULL;
	}

    //-----------------------------------------------------------------------------
    XMLElement* XmlHelper::addChild(XMLNode *elm, Str8 tagName,  Bool isUnique)
    {
        XMLElement* child_elm = NULL;
        if ( isUnique )
            child_elm = getChild(elm,tagName);
        if (!child_elm)
        {
            tinyxml2::XMLDocument* doc = elm->GetDocument();
            DYNAMIC_ASSERT(doc!=NULL);
            if (doc)
            {
                XMLElement* new_elm = doc->NewElement( tagName.c_str() );
                child_elm = elm->InsertEndChild(new_elm)->ToElement();
            }
        }
        return child_elm;
    }
    //-----------------------------------------------------------------------------
    XMLElement* XmlHelper::addChild(XMLNode *elm, Str8 tagName, Str8 attr, Str8 attrName)
    {
        XMLElement* child_elm = getChild(elm,tagName,attr,attrName);
        if (!child_elm)
        {
            tinyxml2::XMLDocument* doc = elm->GetDocument();
            DYNAMIC_ASSERT(doc!=NULL);
            if (doc)
            {
                XMLElement* new_elm = doc->NewElement( tagName.c_str() );
                child_elm = elm->InsertEndChild(new_elm)->ToElement();
            }
        }
        return child_elm;
    }
    //-----------------------------------------------------------------------------
    void XmlHelper::removeChild(XMLNode *elm, Str8 tagName)
    {
        XMLElement* child_elm = getChild(elm, tagName);
        if (child_elm)
        {
            elm->DeleteChild(child_elm);
        }
    }
    //-----------------------------------------------------------------------------
    void XmlHelper::removeChild(XMLNode *elm, Str8 tagName, Str8 attr, Str8 attrName)
    {
        XMLElement* child_elm = getChild(elm, tagName, attr, attrName);
        if (child_elm)
        {
            elm->DeleteChild(child_elm);
        }
    }
    //-----------------------------------------------------------------------------
    Bool XmlHelper::hasAttribute(const XMLElement *elm, Str8 attrName)
    {
        CChar8* str = elm->Attribute(attrName.c_str());
        if (!str)
            return false;

        return true;
    }
    //-----------------------------------------------------------------------------
    Bool XmlHelper::getAttribute(const XMLElement *elm, Str8& attr,  Str8 attrName)
    {
        if (!elm)
            return false;

        CChar8* str = elm->Attribute(attrName.c_str());
        if (!str)
            return false;

        attr = str;

        return true;
    }
    //-----------------------------------------------------------------------------
    Bool XmlHelper::getAttribute(const XMLElement *elm, Int& attr, Str8 attrName)
    {
        Str8 str_value;
        if (getAttribute(elm, str_value, attrName)==false)
            return false;

        MGStrOp::toI32(str_value.c_str(), attr);

        return true;
    }
	//-----------------------------------------------------------------------------
	Bool XmlHelper::getAttribute(const XMLElement *elm, U32& attr,	Str8 attrName)
	{
		Str8 str_value;
		if (getAttribute(elm, str_value, attrName)==false)
			return false;

		MGStrOp::toU32(str_value.c_str(), attr);

		return true;
	}
    //-----------------------------------------------------------------------------
    Bool XmlHelper::getAttribute(const XMLElement *elm, Byte& attr, Str8 attrName)
    {
        std::string str_value;
        if (getAttribute(elm,str_value,attrName)==false)
            return false;

        Int numInt;
        MGStrOp::toI32(str_value.c_str(), numInt);
        attr = (Byte)numInt;

        return true;
    }
    //-----------------------------------------------------------------------------
    Bool XmlHelper::getAttribute(const XMLElement *elm, Flt& attr, Str8 attrName)
    {
        std::string str_value;
        if (getAttribute(elm, str_value, attrName)==false)
            return false;

        MGStrOp::toFlt(str_value.c_str(), attr);
      
        return true;
    }
	//-----------------------------------------------------------------------------
    void XmlHelper::setAttribute(XMLElement *elm, Str8 attr, Str8 attrName)
    {
        if (elm)
        {
            elm->SetAttribute(attrName.c_str(), attr.c_str());
        }
    }
    //-----------------------------------------------------------------------------
    void XmlHelper::setAttribute(XMLElement *elm, Int attr, Str8 attrName)
    {
        Str8 str;
        MGStrOp::toString(attr, str);
        setAttribute(elm, str, attrName);
    }
	//-----------------------------------------------------------------------------
	void XmlHelper::setAttribute(XMLElement *elm, U32 attr, Str8 attrName)
	{
		Str8 str;
		MGStrOp::toString(attr, str);
		setAttribute(elm, str, attrName);
	}
    //-----------------------------------------------------------------------------
    void XmlHelper::setAttribute(XMLElement *elm, Byte attr, Str8 attrName)
    {
        Str8 str;
        MGStrOp::toString((Int)attr, str);
        setAttribute(elm, str, attrName);
    }
    //-----------------------------------------------------------------------------
    void XmlHelper::setAttribute(XMLElement *elm, Flt attr, Str8 attrName)
    {
        Str8 str;
        MGStrOp::toString(attr, str);
        setAttribute(elm, str, attrName);
    }
	//-----------------------------------------------------------------------------
	const XMLElement* XmlHelper::IterateChildElements( const XMLElement* parentElement, const XMLElement* childElement ) 
	{
		if ( parentElement )
		{
			if ( childElement == NULL )
			{
				return parentElement->FirstChildElement();
			}
			else
			{
				return childElement->NextSiblingElement();
			}
		}
		return NULL;
	}
	//-----------------------------------------------------------------------------
	XMLElement* XmlHelper::IterateChildElements( XMLElement* parentElement, XMLElement* childElement )
	{
		if ( parentElement )
		{
			if ( childElement == NULL )
			{
				return parentElement->FirstChildElement();
			}
			else
			{
				return childElement->NextSiblingElement();
			}
		}
		return NULL;
	}
	/******************************************************************************/
    //Color XML FORMAT
    /******************************************************************************/
    Bool ColorXmlFormat::load(XMLElement*parent_elm, Str8 child_name, Color& color)
    {
        if (parent_elm)
        {
            XMLElement* elm = XmlHelper::getChild(parent_elm,child_name);
            if (elm)
            {
                if ( XmlHelper::getAttribute(elm,color.r,"R") == false )
                    return false;
                if ( XmlHelper::getAttribute(elm,color.g,"G") == false )
                    return false;
                if ( XmlHelper::getAttribute(elm,color.b,"B") == false )
                    return false;
                if ( XmlHelper::getAttribute(elm,color.a,"A") == false )
                    return false;
                return true;
            }
        }

        return false;
    }
    //-----------------------------------------------------------------------------
    Bool ColorXmlFormat::save(XMLElement*parent_elm, Str8 child_name ,Color& color)
    {
        if (parent_elm)
        {
            XMLElement* elm = XmlHelper::addChild(parent_elm,child_name);
            if (elm)
            {
                XmlHelper::setAttribute(elm,color.r,"R");
                XmlHelper::setAttribute(elm,color.g,"G");
                XmlHelper::setAttribute(elm,color.b,"B");
                XmlHelper::setAttribute(elm,color.a,"A");

                return true;
            }
        }
        return false;
    }
   
    /******************************************************************************/
    //Vec3XmlFormat XML FORMAT
    /******************************************************************************/
	Bool Vec3XmlFormat::load(XMLElement*parent_elm, Str8 child_name, Vec3& vec3)
	{
		if (parent_elm)
		{
			XMLElement* elm = XmlHelper::getChild(parent_elm,child_name);
			if (elm)
			{
				if ( XmlHelper::getAttribute(elm,vec3.x,"X") == false )
                    return false;
				if ( XmlHelper::getAttribute(elm,vec3.y,"Y") == false )
                    return false;
				if ( XmlHelper::getAttribute(elm,vec3.z,"Z") == false )
                    return false;
                return true;
			}
		}

        return false;
	}


	//-----------------------------------------------------------------------------
	Bool Vec3XmlFormat::save(XMLElement*parent_elm, Str8 child_name ,Vec3& vec3)
	{
		if (parent_elm)
		{
			XMLElement* elm = XmlHelper::addChild(parent_elm,child_name);
			if (elm)
			{
				XmlHelper::setAttribute(elm,vec3.x,"X");
				XmlHelper::setAttribute(elm,vec3.y,"Y");
				XmlHelper::setAttribute(elm,vec3.z,"Z");

                return true;
			}
		}
        return false;
	}

	/******************************************************************************/
	//Vec3XmlFormat2 XML FORMAT
	/******************************************************************************/
	MG::Bool Vec3XmlFormat2::load( XMLElement*parent_elm, Str8 child_name, Vec3& vec3 )
	{
		if (parent_elm)
		{
			XMLElement* elm = XmlHelper::getChild(parent_elm,child_name);
			if (elm)
			{
				if ( XmlHelper::getAttribute(elm,vec3.x,"x") == false )
					return false;
				if ( XmlHelper::getAttribute(elm,vec3.y,"y") == false )
					return false;
				if ( XmlHelper::getAttribute(elm,vec3.z,"z") == false )
					return false;
				return true;
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------------
	MG::Bool Vec3XmlFormat2::save( XMLElement*parent_elm, Str8 child_name ,Vec3& vec3 )
	{
		if (parent_elm)
		{
			XMLElement* elm = XmlHelper::addChild(parent_elm,child_name);
			if (elm)
			{
				XmlHelper::setAttribute(elm,vec3.x,"x");
				XmlHelper::setAttribute(elm,vec3.y,"y");
				XmlHelper::setAttribute(elm,vec3.z,"z");

				return true;
			}
		}
		return false;
	}

    /******************************************************************************/
    //Vec4XmlFormat XML FORMAT
    /******************************************************************************/
	Bool Vec4XmlFormat::load(XMLElement*parent_elm, Str8 child_name, Vec4& vec4)
	{
		if (parent_elm)
		{
			XMLElement* elm = XmlHelper::getChild(parent_elm,child_name);
			if (elm)
			{
				if ( XmlHelper::getAttribute(elm,vec4.w,"W") == false )
                    return false;
				if ( XmlHelper::getAttribute(elm,vec4.x,"X") == false )
                    return false;
				if ( XmlHelper::getAttribute(elm,vec4.y,"Y") == false )
                    return false;
				if ( XmlHelper::getAttribute(elm,vec4.z,"Z") == false )
                    return false;

                return true;
			}
		}
        return false;
	}
	//-----------------------------------------------------------------------------
	Bool Vec4XmlFormat::save(XMLElement*parent_elm, Str8 child_name ,Vec4& vec4)
	{
		if (parent_elm)
		{
			XMLElement* elm = XmlHelper::addChild(parent_elm,child_name);
			if (elm)
			{
				XmlHelper::setAttribute(elm,vec4.w,"W");
				XmlHelper::setAttribute(elm,vec4.x,"X");
				XmlHelper::setAttribute(elm,vec4.y,"Y");
				XmlHelper::setAttribute(elm,vec4.z,"Z");

                return true;
			}
		}

        return false;
	}
	/******************************************************************************/
	//QuaXmlFormat XML FORMAT
	/******************************************************************************/
	Bool QuaXmlFormat::load(XMLElement*parent_elm, Str8 child_name, Qua& qua)
	{
		if (parent_elm)
		{
			XMLElement* elm = XmlHelper::getChild(parent_elm,child_name);
			if (elm)
			{
				if ( XmlHelper::getAttribute(elm,qua.w,"qw") == false )
					return false;
				if ( XmlHelper::getAttribute(elm,qua.x,"qx") == false )
					return false;
				if ( XmlHelper::getAttribute(elm,qua.y,"qy") == false )
					return false;
				if ( XmlHelper::getAttribute(elm,qua.z,"qz") == false )
					return false;

				return true;
			}
		}
		return false;
	}
	//-----------------------------------------------------------------------------
	Bool QuaXmlFormat::save(XMLElement*parent_elm, Str8 child_name ,Qua& qua)
	{
		if (parent_elm)
		{
			XMLElement* elm = XmlHelper::addChild(parent_elm,child_name);
			if (elm)
			{
				XmlHelper::setAttribute(elm,qua.w,"qw");
				XmlHelper::setAttribute(elm,qua.x,"qx");
				XmlHelper::setAttribute(elm,qua.y,"qy");
				XmlHelper::setAttribute(elm,qua.z,"qz");

				return true;
			}
		}

		return false;
	}
}


