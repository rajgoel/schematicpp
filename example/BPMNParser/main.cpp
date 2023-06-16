#include "lib/BPMN.h"
#include <iostream>
#include <sstream>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <libschematicpp/XercesString.h>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/PlatformUtils.hpp>

using namespace xercesc;
using namespace std;
using namespace XML;

int main(void) {
  unique_ptr<XML::XMLObject> root(XML::XMLObject::createFromStream(std::cin));
  
  vector< reference_wrapper<tProcess> > processes = root->getChildren<tProcess>();
  
  cout << "Model has " << processes.size() << " processes:" << endl;
  for ( tProcess& process : processes ) {
    auto processId = process.getOptionalAttributeByName("id");
    cout << "- Process";
    if ( processId.has_value() ) {
      cout << " with id '" << (string)processId->get() << "'";
    }
    else {
      cout << " without id";
    }
    if ( process.isExecutable.has_value() && process.isExecutable->get() ) {
      cout << " is executable and"; 
    }
    auto flowNodes = process.getChildren<tFlowNode>();
    cout << " has " << flowNodes.size() << " node(s)." << endl;  
    for ( tFlowNode& node : flowNodes ) {
      optional< reference_wrapper<Attribute> > nodeId = node.id;
      cout << "  - " << node.className;
      if ( nodeId.has_value() ) {
        cout << " with id '" << (string)nodeId->get() << "'";
      }
      else {
        cout << " without id";
      }
      cout << ": " << &node << endl;
    }
  }

  return 0;
}

