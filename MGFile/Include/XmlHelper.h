/******************************************************************************/
#ifndef _XMLHELPER_H_
#define _XMLHELPER_H_
/******************************************************************************/

using namespace tinyxml2;

/******************************************************************************/
namespace MG
{


    /******************************************************************************/
    //TinyXml ������
    /******************************************************************************/
    #define	XML_GET_VALUE(element) ((element)->Value())
    #define	XML_SET_VALUE(element,value) ((element)->Value(value));

    #define	XML_GET_ATTRIBUTE(element,name) (((element)->Attribute(name)) ? ((element)->Attribute(name)) : "");
    #define	XML_SET_ATTRIBUTE(element,name,value) ((element)->SetAttribute((name),(value)));

	/******************************************************************************/
	//TinyXml ��������
	/******************************************************************************/
	class XmlHelper
	{
	public:

		/** �����ӽڵ� **/
		static const XMLElement* IterateChildElements(const XMLElement* xmlElement, const XMLElement* childElement);

		/** �����ӽڵ� **/
		static XMLElement*		IterateChildElements(XMLElement* xmlElement, XMLElement* childElement);
        
		/** �õ��ӽڵ����� */
        static UInt             getChildCount(XMLNode *elm);

        /** ���ݽڵ���, ��ѯ�ӽڵ�. */
		static XMLElement*      getChild(XMLNode *elm, Str8 tagName);
        /** ���ݽڵ���, ��ѯ�ӽڵ�. */
        static XMLElement*      getChild(XMLNode *elm, Str8 tagName, Str8 attr, Str8 attrName="Name");
		/** ���ݽڵ���, ��ѯ�ӽڵ�. */
		static const XMLElement*  getChild(const XMLNode *elm, Str8 tagName);
		/** ���ݽڵ���, ��ѯ�ӽڵ�. */
		static const XMLElement*  getChild(const XMLNode *elm, Str8 tagName, Str8 attr, Str8 attrName="Name");

        /** ����һ���ӽڵ�, ����������ӽڵ�, �����*/
        static XMLElement*      addChild(XMLNode *elm, Str8 tagName, Bool isUnique = true);
        /** ����һ���ӽڵ�, ����������ӽڵ�, �����*/
        static XMLElement*      addChild(XMLNode *elm, Str8 tagName, Str8 attr, Str8 attrName="Name");

        /** �Ƴ�һ���ӽڵ�*/
		static void             removeChild(XMLNode *elm, Str8 tagName);
        /** �Ƴ�һ���ӽڵ�*/
        static void             removeChild(XMLNode *elm, Str8 tagName, Str8 attr, Str8 attrName="Name");

        /** �õ�һ���ڵ������ֵ��������������ѯ 
        */
        //@{
        static Bool             hasAttribute(const XMLElement *elm, Str8 attrName="Value");

		static Bool             getAttribute(const XMLElement *elm, Str8& attr, Str8 attrName="Value");
		static Bool             getAttribute(const XMLElement *elm, Int& attr,	Str8 attrName="Value");
		static Bool             getAttribute(const XMLElement *elm, U32& attr,	Str8 attrName="Value");
		static Bool             getAttribute(const XMLElement *elm, Byte& attr,	Str8 attrName="Value");
		static Bool             getAttribute(const XMLElement *elm, Flt& attr,	Str8 attrName="Value");
		
		static void             setAttribute(XMLElement *elm, Str8 attr,	Str8 attrName="Value");
		static void             setAttribute(XMLElement *elm, Int attr,	Str8 attrName="Value");
		static void             setAttribute(XMLElement *elm, Byte attr,	Str8 attrName="Value");
		static void             setAttribute(XMLElement *elm, Flt attr,	Str8 attrName="Value");
		static void				setAttribute(XMLElement *elm, U32 attr,	Str8 attrName="Value");
        //@}
	};

    /******************************************************************************/
    //Color XML FORMAT
    /******************************************************************************/
    class ColorXmlFormat
    {
    public:
        static Bool load(XMLElement*parent_elm, Str8 child_name, Color& color);
        static Bool save(XMLElement*parent_elm, Str8 child_name ,Color& color);
    };

	/******************************************************************************/
	//Vec3 XML FORMAT
	/******************************************************************************/
	class Vec3XmlFormat
	{
	public:
		static Bool load(XMLElement*parent_elm, Str8 child_name, Vec3& vec3);
		static Bool save(XMLElement*parent_elm, Str8 child_name ,Vec3& vec3);
	};

	/******************************************************************************/
	//Vec3 XML FORMAT
	/******************************************************************************/
	class Vec3XmlFormat2
	{
	public:
		static Bool load(XMLElement*parent_elm, Str8 child_name, Vec3& vec3);
		static Bool save(XMLElement*parent_elm, Str8 child_name ,Vec3& vec3);
	};

	/******************************************************************************/
	//Vec4 XML FORMAT
	/******************************************************************************/
	class Vec4XmlFormat
	{
	public:
		static Bool load(XMLElement*parent_elm, Str8 child_name, Vec4& vec4);
		static Bool save(XMLElement*parent_elm, Str8 child_name ,Vec4& vec4);
	};

	/******************************************************************************/
	//Qua XML FORMAT
	/******************************************************************************/
	class QuaXmlFormat
	{
	public:
		static Bool load(XMLElement*parent_elm, Str8 child_name, Qua& qua);
		static Bool save(XMLElement*parent_elm, Str8 child_name ,Qua& qua);
	};


}

/******************************************************************************/

#endif