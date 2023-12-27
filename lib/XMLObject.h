#ifndef XMLObject_H
#define XMLObject_H
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
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

/**
 * The `Value` struct stores a value and provides implicit conversion and assignment operators
 * to facilitate easy conversion between different types and convenient assignment
 * of values.
 *
 * Example usage:
 * ```
 * Value value;
 * value = std::to_string("a_string");  // Assignment using a std::string.
 * value = true;                        // Assignment using a bool.
 * value = 42;                          // Assignment using an int.
 * value = 3.14;                        // Assignment using a double.
 *
 * std::string stringValue = value;     // Implicit conversion to std::string.
 * bool booleanValue = value;           // Implicit conversion to bool.
 * int integerValue = value;            // Implicit conversion to int.
 * double realValue = value;            // Implicit conversion to double.
 * ```
 */
struct Value {
  std::string value;
  operator std::string_view() const { return value; };
  operator std::string() const { return value; };
  operator bool() const { return (value == True); };
  operator int() const { return std::stoi(value); };
  operator double() const  { return std::stod(value); };
  Value& operator=(const std::string& s) { value = s; return *this; };
  Value& operator=(bool b) { value = (b ? True : False); return *this; };
  Value& operator=(int i) { value = std::to_string(i); return *this; };
  Value& operator=(double d) { value = std::to_string(d); return *this; };
  Value(const std::string& s) : value(s) {};
  Value(bool b) : value(b ? True : False) {};
  Value(int i) : value(std::to_string(i)) {};
  Value(double d) : value(std::to_string(d)) {};
  inline static std::string True = "true";
  inline static std::string False = "false";
};

/**
 * The `Attribute` struct stores information about the namespace, prefix, name, and
 * value of the attribute. 
 */
struct Attribute {
  Namespace xmlns;
  Namespace prefix;
  AttributeName name;
  Value value;
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
 * - `XMLObject::createFromStream(xmlStream)`
 * - `XMLObject::createFromString(xmlString)`
 * - `XMLObject::createFromFile(filename)`
 *
 * Each instance has a
 * - `xmlns`: refers to the XML namespace
 * - `className`: refers to the class it belong to
 * - `elementName`: refers to the name used in the XML
 * - `prefix` (optional): refers to the namespace prefix in the XML
 * - `textContent`: textual content of XML element without children
 * - `attributes`: a list of attributes containing the namespace, prefix, attribute name, and attribute value
 * - `children: a list of child elements
 *
 * Derived classes providing dedicated members for attributes and children belonging to the respective XML element according to an XML schema definition are automatically generated by schematicpp.
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

  template<typename T> inline const T* is() const {
    return dynamic_cast<const T*>(this);
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
  }

  template<typename T> inline const T* get() const {
    const T* ptr = dynamic_cast<const T*>(this);
    if ( ptr == nullptr ) {
      throw std::runtime_error("XMLObject: Illegal cast");
    }
    return ptr;
  }

private:
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

  template<typename T>
  void findRecursive(std::vector<std::reference_wrapper<const T> >& result, const Children& descendants) const
  {
    for (auto& descendant : descendants) {
      if (descendant->is<const T>()) {
        result.push_back(*descendant->get<const T>());
      }
      findRecursive(result, descendant->children );
    }
  }

public:
  /**
   * Find all descendants of type T.
   *
   * @return A vector of references to descendants of type T.
   */
  template<typename T>
  std::vector<std::reference_wrapper<T> > find()
  {
    std::vector<std::reference_wrapper<T> > result;
    findRecursive(result, children);
    return result;
  }

  /**
   * Find all descendants of type T.
   *
   * @return A vector of const references to descendants of type T.
   */
  template<typename T>
  std::vector<std::reference_wrapper<const T> > find() const
  {
    std::vector<std::reference_wrapper<const T> > result;
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

std::ostream& operator<< (std::ostream& os, const XMLObject* obj); ///< Allows printing of stringified XML object
std::ostream& operator<< (std::ostream& os, const XMLObject& obj); ///< Allows printing of stringified XML object

} // end namespace XML

#endif // XML_H
