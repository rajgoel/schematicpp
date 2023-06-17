#include "XMLObject.h"
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/sax/InputSource.hpp>

namespace XML {

// Utility class for parsing directly from an std::istream.
class IStreamInputSource : public xercesc::InputSource {
protected:
  std::istream &is;
  xercesc::BinInputStream* makeStream() const {
    return new IStreamBinInputStream(is);
  }

  class IStreamBinInputStream : public xercesc::BinInputStream {
    std::istream &is;
  public:
    IStreamBinInputStream(std::istream &is) : xercesc::BinInputStream(), is(is) {};
    XMLFilePos curPos(void) const { return is.tellg(); };
    XMLSize_t readBytes(XMLByte* const buf, const XMLSize_t max) {
      is.read((char*)buf, max);
      return is.gcount();
    };
    const XMLCh* getContentType() const {
      //TODO: return application/xml
      return NULL;
    };
  };
public:
  IStreamInputSource(std::istream &is) : InputSource(), is(is) {};
};



XMLObject* XMLObject::createFromStream(std::istream& xmlStream) {
  // std::cout << "Create XML object from input stream" << std::endl;
  xercesc::XMLPlatformUtils::Initialize();
  std::unique_ptr<xercesc::XercesDOMParser> parser = std::make_unique<xercesc::XercesDOMParser>();
  parser->setDoNamespaces(true);
  parser->parse(IStreamInputSource(xmlStream));

  xercesc::DOMDocument* document = parser->getDocument();
  if(!document) throw std::runtime_error("Failed to parse XML");

  xercesc::DOMElement* rootElement = document->getDocumentElement();
  if(!rootElement) throw std::runtime_error("Failed to get root element of XML");

  std::string rootName =  xercesc::XMLString::transcode(rootElement->getLocalName());
  XMLObject* object = createObject(rootElement, nullptr);
  parser.reset(); // delete unique_ptr to parser before calling Terminate
  xercesc::XMLPlatformUtils::Terminate();
  return object;
};

XMLObject* XMLObject::createFromString(std::string& xmlString) {
  // std::cout << "Create XML object from string" << std::endl;
  std::istringstream iss(xmlString);
  return createFromStream(iss);
}

XMLObject* XMLObject::createFromFile(std::string& filename) {
  // std::cout << "Create XML object from file" << std::endl;
  throw std::runtime_error("Not yet implemented!");
  return nullptr;
}


XMLObject* XMLObject::createObject(const xercesc::DOMElement* element, XMLObject* parent) {
  ElementName elementName = xercesc::XMLString::transcode(element->getLocalName());
  if ( auto it = factory.find(elementName); it != factory.end() ) { 
    return it->second(elementName, element, parent); 
  }
  // std::cout << "Unknown element '" << elementName << "' using 'XMLObject' instead" << std::endl;
  return createInstance<XMLObject>("XMLObject", element, parent);
}

XMLObject::XMLObject(const ClassName& className, const xercesc::DOMElement* element, XMLObject* parent, const Attributes& defaultAttributes) : className(className), parent(parent) {

  elementName = xercesc::XMLString::transcode(element->getLocalName());
  prefix = xercesc::XMLString::transcode(element->getPrefix());

  // set attributes
  xercesc::DOMNamedNodeMap* elementAttributes = element->getAttributes();
  for (XMLSize_t i = 0; i < elementAttributes->getLength(); i++) {
    xercesc::DOMNode* item = elementAttributes->item(i);
    Namespace attributePrefix = "";
    AttributeName attributeName = xercesc::XMLString::transcode(item->getNodeName());
    if ( auto pos = attributeName.find(":"); pos != std::string::npos ) {
      attributePrefix = attributeName.substr(0, pos);
      attributeName.erase(0, pos + 1);
    }
    AttributeValue attributeValue = xercesc::XMLString::transcode(item->getNodeValue());
    attributes.push_back( { attributePrefix, attributeName, attributeValue } );
  }

  // add defaults for missing attributes
  for ( auto& defaultAttribute : defaultAttributes ) {
    if ( !getOptionalAttributeByName(defaultAttribute.name) ) {
      attributes.push_back(defaultAttribute);
    }
  }

  // set children
  for (xercesc::DOMElement *childElement = element->getFirstElementChild(); childElement; childElement = childElement->getNextElementSibling()) {
    ElementName childName = xercesc::XMLString::transcode(childElement->getLocalName());
    children.push_back(std::unique_ptr<XMLObject>(createObject(childElement,this)));
  }

  if ( children.empty() ) {
    textContent = xercesc::XMLString::transcode(element->getTextContent());
  }
}

  XMLObject& XMLObject::getRequiredChildByName(const ElementName& elementName) {
    for ( auto& child : children ) {
      if ( child->elementName == elementName ) {
        return *child;
      }
    }
    throw std::runtime_error("Failed to get required child of element '" + elementName + "'");
  }

  std::optional< std::reference_wrapper<XMLObject> > XMLObject::getOptionalChildByName(const ElementName& elementName) {
    for ( auto& child : children ) {
      if ( child->elementName == elementName ) {
        return *child;
      }
    }
    return std::nullopt;
  }

  std::vector< std::reference_wrapper<XMLObject> > XMLObject::getChildrenByName(const ElementName& elementName) {
    std::vector< std::reference_wrapper<XMLObject> > result;
    for ( auto& child : children ) {
      if ( child->elementName == elementName ) {
        result.push_back(*child);
      }
    }
    return result;
  }

Attribute& XMLObject::getRequiredAttributeByName(const AttributeName& attributeName) {
  auto it = std::find_if(attributes.begin(), attributes.end(), 
                         [attributeName](Attribute& attribute) { return attribute.name == attributeName; }
  );
  if (it != attributes.end()) {
    return *it;
  }
  throw std::runtime_error("Failed to get required attribute '" +  attributeName + "' of element '" + elementName + "'");
}

std::optional< std::reference_wrapper<Attribute> > XMLObject::getOptionalAttributeByName(const AttributeName& attributeName) {
  auto it = std::find_if(attributes.begin(), attributes.end(), 
                         [attributeName](Attribute& attribute) { return attribute.name == attributeName; }
  );
  if (it != attributes.end()) {
    return *it;
  }
  return std::nullopt;
}

std::string XMLObject::stringify() const {
  std::string xmlString = std::string("<") + (!prefix.empty() ? prefix + ":" : "") + elementName;
  for ( auto& attribute : attributes ) {
    xmlString += std::string(" ") + (!attribute.prefix.empty() ? attribute.prefix + ":" : "") + attribute.name + "=\"" + attribute.value +"\""; 
  }
  xmlString += ">";

  for ( auto& child : children ) {
    xmlString += child->stringify();
  }
  xmlString += textContent;
  xmlString += std::string("</") + (!prefix.empty() ? prefix + ":" : "") + elementName + ">";
  return xmlString;
}

std::ostream& operator<< (std::ostream& os, const XMLObject* obj) {
  os << obj->stringify();
	return os;
}

std::ostream& operator<< (std::ostream& os, const XMLObject& obj) {
  os << obj.stringify();
	return os;
}

} // end namespace XML

