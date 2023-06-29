#ifndef XMLObject_H
#define XMLObject_H
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

#include <xercesc/dom/DOM.hpp>

namespace XML {

class XMLObject;

typedef std::string ClassName;
typedef std::string ElementName;
typedef std::string TextContent;
typedef std::string Namespace;
typedef std::string AttributeName;
typedef std::string AttributeValue;

/**
 * The `Attribute` struct stores information about the namespace, prefix, name, and
 * value of the attribute. It provides implicit conversion and assignment operators
 * to facilitate easy conversion between different types and convenient assignment
 * of attribute values.
 *
 * Example usage:
 * Attribute attribute;
 * attribute = "a_string";              // Assignment using a std::string.
 * attribute = true;                    // Assignment using a bool.
 * attribute = 42;                      // Assignment using an int.
 * attribute = 3.14;                    // Assignment using a double.
 *
 * std::string stringValue = attribute; // Implicit conversion to std::string.
 * bool booleanValue = attribute;       // Implicit conversion to bool.
 * int integerValue = attribute;        // Implicit conversion to int.
 * double realValue = attribute;        // Implicit conversion to double.
 *
 */
struct Attribute {
  Namespace xmlns;
  Namespace prefix;
  AttributeName name;
  AttributeValue value;
  operator std::string() const { return value; };
  operator bool() const { return (value == "true"); };
  operator int() const { return std::stoi(value); };
  operator double() const  { return std::stod(value); };
  Attribute& operator=(const std::string& s) { value = s; return *this; };
  Attribute& operator=(const bool& b) { value = (b ? "true" : "false"); return *this; };
  Attribute& operator=(const int& i) { value = std::to_string(i); return *this; };
  Attribute& operator=(const double& d) { value = std::to_string(d); return *this; };
};
typedef std::vector<Attribute> Attributes;
typedef std::vector<std::unique_ptr<XMLObject>> Children;

/// Template function used to store in factory
template<typename T> XMLObject* createInstance(const Namespace& xmlns, const ClassName& className, const xercesc::DOMElement* element) { return new T(xmlns, className, element, T::defaults); }

/// Factory used to create instance depending on element name
typedef std::unordered_map<ElementName, XMLObject* (*)(const Namespace& xmlns, const ClassName& className, const xercesc::DOMElement* element)> Factory;


/**
 * XMLObject
 *
 * A class allowing to read and store an XML-tree. The root element can be created using
 * - XMLObject::createFromStream(xmlStream)
 * - XMLObject::createFromString(xmlString)
 * - XMLObject::createFromFile(filename)
 *
 * Each instance has a
 * - xmlns: refers to the XML namespace
 * - className: refers to the class it belong to
 * - elementName: refers to the name used in the XML
 * - prefix (optional): refers to the namespace prefix in the XML
 * - textContent: textual content of XML element without children
 * - attributes: a list of attributes containing the namespace, prefix, attribute name, and attribute value
 * - children: a list of child elements
 *
 * Derived classes providing dedicated members for attributes and children belonging to the respective XML element according to an XML schema defintion are automatically generated by schematicpp.
 */
class XMLObject {

public:
  /**
   * Create an XMLObject from the input stream.
   *
   * @param xmlStream The input stream containing the XML data.
   * @return A pointer to the created XMLObject.
   * @throws std::runtime_error if parsing the XML fails.
   */
	static XMLObject* createFromStream(std::istream& xmlStream);

  /**
   * Create an XMLObject from a string representation of XML.
   *
   * @param xmlString The string containing the XML data.
   * @return A pointer to the created XMLObject.
   * @throws std::runtime_error if parsing the XML fails.
   */
	static XMLObject* createFromString(const std::string& xmlString);

  /**
   * Create an XMLObject from an XML file.
   *
   * @param filename The path to the XML file.
   * @return A pointer to the created XMLObject.
   * @throws std::runtime_error if parsing the XML fails.
   */
	static XMLObject* createFromFile(const std::string& filename);

  virtual ~XMLObject() {};

protected:
  static XMLObject* createObject(const xercesc::DOMElement* element);

template<typename T> friend XMLObject* createInstance(const Namespace& xmlns, const ClassName& className, const xercesc::DOMElement* element);

protected:
  XMLObject(const Namespace& xmlns, const ClassName& className, const xercesc::DOMElement* element, const Attributes& defaultAttributes);

  inline static Factory factory;
public:
  /// Returns a pointer of type T of the object.
  template<typename T> inline T* is() {
    return dynamic_cast<T*>(this);
  }

  /**
   * Attempt to cast the current instance to the specified type T.
   * If the cast is successful, returns a pointer to the casted object.
   * If the cast fails, throws a std::runtime_error with an error message
   * indicating an illegal cast operation.
   */
  template<typename T> inline T* get() {
    T* ptr = dynamic_cast<T*>(this);
    if ( ptr == nullptr ) {
      throw std::runtime_error("XMLObject: Illegal cast");
    }
    return ptr;
  };

protected:
  template<typename T>
  void findRecursive(std::vector<std::reference_wrapper<T> >& result, const Children& descendants)
  {
    for (auto& descendant : descendants) {
      if (descendant->is<T>()) {
        result.push_back(*descendant->get<T>());
      }
      findRecursive(result, descendant->children );
    }
  }

public:
  /**
   * Find all descendants of type T.
   *
   * @return A vector of references to decendants of type T.
   */
  template<typename T>
  std::vector<std::reference_wrapper<T> > find()
  {
    std::vector<std::reference_wrapper<T> > result;
    findRecursive(result, children);
    return result;
  }

  Namespace xmlns;
  const ClassName className;
  Namespace prefix;
  ElementName elementName;

  TextContent textContent; ///< Textual content of XML element without children
  Children children; ///< Child nodes of the XML element
  Attributes attributes; /// Attributes of the XML element
	inline static const Attributes defaults = {};

  /**
   * Convert the XMLObject and its children to a string representation.
   *
   * @return The string representation of the XMLObject.
   */
  std::string stringify() const;

  /**
   * Get a required child of type T.
   *
   * @return A reference to the required child.
   * @throws std::runtime_error if the required child is not found.
   */
  template<typename T> T& getRequiredChild() {
    for ( auto& child : children ) {
      if ( child->is<T>() ) {
        return *child->get<T>();
      }
    }
    throw std::runtime_error("Failed to get required child of element '" + elementName + "'");
  }

  /**
   * Get an optional child of type T.
   *
   * @return An optional containing a reference to the optional child if found,
   *         or std::nullopt if the optional child is not found.
   */
  template<typename T> std::optional< std::reference_wrapper<T> > getOptionalChild() {
    for ( auto& child : children ) {
      if ( child->is<T>() ) {
        return *child->get<T>();
      }
    }
    return std::nullopt;
  }

  /**
   * Get all children of type T.
   *
   * @return A vector of references to the children of type T.
   */
  template<typename T> std::vector< std::reference_wrapper<T> > getChildren() {
    std::vector< std::reference_wrapper<T> > result;
    for ( auto& child : children ) {
      if ( child->is<T>() ) {
        result.push_back(*child->get<T>());
      }
    }
    return result;
  }

  /**
   * Get a required child with the specified element name.
   *
   * @param elementName The name of the child element without namespace prefix.
   * @return A reference to the required child.
   * @throws std::runtime_error if the required child is not found.
   */
  XMLObject& getRequiredChildByName(const ElementName& elementName);

  /**
   * Get the optional child with the specified element name.
   *
   * @param elementName The name of the child element without namespace prefix.
   * @return An optional containing a reference to the optional child if found,
   *         or std::nullopt if the optional child is not found.
   */
  std::optional< std::reference_wrapper<XMLObject> > getOptionalChildByName(const ElementName& elementName);

  /**
   * Get all children with the specified element name.
   *
   * @param elementName The name of the child elements without namespace prefix.
   * @return A vector of references to the children with the specified element name.
   */
  std::vector< std::reference_wrapper<XMLObject> > getChildrenByName(const ElementName& elementName);

  /**
   * Get a required attribute with the specified attribute name.
   *
   * @param attributeName The name of the attribute without namespace prefix.
   * @return A reference to the required attribute.
   * @throws std::runtime_error if the required attribute is not found.
   */
  Attribute& getRequiredAttributeByName(const AttributeName& attributeName);

  /**
   * Get an optional attribute with the specified attribute name.
   *
   * @param attributeName The name of the attribute without namespace prefix.
   * @return An optional containing a reference to the optional attribute if found,
   *         or std::nullopt if the optional attribute is not found.
   */
  std::optional< std::reference_wrapper<Attribute> > getOptionalAttributeByName(const AttributeName& attributeName);


};

std::ostream& operator<< (std::ostream& os, const XMLObject* obj);
std::ostream& operator<< (std::ostream& os, const XMLObject& obj);

} // end namespace XML

#endif // XML_H
