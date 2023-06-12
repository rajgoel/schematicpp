#ifndef XMLObject_H
#define XMLObject_H
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <xercesc/dom/DOM.hpp>

namespace XML {

typedef std::string ClassName;
typedef std::string ElementName;
typedef std::string Namespace;
typedef std::string AttributeName;
typedef std::string AttributeValue;
typedef std::tuple<Namespace,AttributeName,AttributeValue> Attribute;

class XMLObject;

template<typename T> XMLObject* createInstance(const ClassName& className, const xercesc::DOMElement* element, XMLObject* parent) { return new T(className, element, parent); } /// Template function used to store in factory

typedef std::unordered_map<ElementName, XMLObject* (*)(const ClassName& className, const xercesc::DOMElement* element, XMLObject* parent)> Factory; /// Factory used to create instance depending on element name


/**
 * XMLObject
 *
 * A class allowing to read and store an XML-tree. The root element can be created using
 * - XMLObject::createFromStream(xmlStream)
 * - XMLObject::createFromString(xmlString)
 * - XMLObject::createFromFile(filename)
 *
 * Each instance has a
 * - className: refers to the class it belong to
 * - elementName: refers to the name used in the XML
 * - prefix (optional): refers to the namespace prefix in the XML (or empty if no namespace is given): 
 * - attributes: a list of tuples containing the namespace prefix, the attribute name, and the attribute value
 * - children: a list of child elements
 * - parent: a pointer to the parent element (or nullptr is root element) 
 *
 * Derived classes providing dedicated members for attributes and children belonging to the respective XML element according to an XML schema defintion are automatically generated by schematicpp. 
 */
class XMLObject {

public:
	static XMLObject* createFromStream(std::istream& xmlStream);
	static XMLObject* createFromString(std::string& xmlString);
	static XMLObject* createFromFile(std::string& filename);

  virtual ~XMLObject() {};

protected:
  static XMLObject* createObject(const xercesc::DOMElement* element, XMLObject* parent);



template<typename T> friend XMLObject* createInstance(const ClassName& className, const xercesc::DOMElement* element, XMLObject* parent); 

protected:

  XMLObject(const ClassName& className, const xercesc::DOMElement* element, XMLObject* parent);

  inline static Factory factory;

public:
  template<typename T> bool is() {
    return ( dynamic_cast<T*>(this) != nullptr );
  }

  template<typename T> T* get() {
    T* ptr = dynamic_cast<T*>(this); 
    if ( ptr == nullptr ) {
      throw std::runtime_error("XMLObject: Illegal cast");
    }
    return ptr; 
  };



  Namespace prefix;
  ElementName elementName;
  const ClassName className;

  XMLObject* parent;
  std::vector<Attribute> attributes;
  std::vector<std::unique_ptr<XMLObject>> children;

  std::string stringify() const;
  Attribute* getAttributeByName(const AttributeName& attributeName);
  std::vector<XMLObject*> getChildrenByName(const ElementName& elementName);
  template<typename T> std::vector<T*> getChildren() {
    std::vector<T*> result;
    for ( auto& child : children ) {
      if ( child->is<T>() ) {
        result.push_back(child->get<T>());
      }
    }
    return result;
  }


};

std::ostream& operator<< (std::ostream& os, const XMLObject* obj);


////////////////////////////////////////

class DerivedXMLObject : public XMLObject {

template<typename T> friend XMLObject* createInstance(const ClassName& className, const xercesc::DOMElement* element, XMLObject* parent); 

private:
  static bool registerClass() {
//    std::cout <<"Init-DerivedXMLObject\n";
    XMLObject::factory["DerivedXMLObject"] = &createInstance<DerivedXMLObject>; /// register function in factory
    return true;
  };
  inline static bool registered = registerClass();

  DerivedXMLObject(const ClassName& className, const xercesc::DOMElement* element, XMLObject* parent) : XMLObject(className, element, parent)
    {
//        std::cout <<"DerivedXMLObject constructor"<< std::endl;
    };
public:
};


} // end namespace XML

#endif // XML_H
