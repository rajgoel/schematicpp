#include "lib/BPMNParser.h"
#include <iostream>
#include <sstream>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <libschematicpp/XercesString.h>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/PlatformUtils.hpp>

using namespace xercesc;
using namespace std;
using namespace XML;
using namespace XML::BPMN;

int main(int argc, char **argv) {
  if ( argc != 2 ) {
    cout << "Usage: " << argv[0] << " <bpmn-file>" << endl;
    return 0;
  }

  unique_ptr<XML::XMLObject> root(XML::XMLObject::createFromFile(argv[1]));
  
  vector< reference_wrapper<tProcess> > processes = root->getChildren<tProcess>();
  
  cout << "Model has " << processes.size() << " process(es):" << endl;
  for ( tProcess& process : processes ) {
    optional< reference_wrapper<Attribute> > processId = process.id;
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
      
      cout << " has " << node.incoming.size() << " incoming and " << node.outgoing.size() << " outgoing arc(s)." << endl;
      for ( XMLObject& incoming : node.incoming ) {
        cout << "    - incoming: " << incoming.textContent << endl;
      }
      for ( XMLObject& outgoing : node.outgoing ) {
        cout << "    - outgoing: " << outgoing.textContent << endl;
      }
    }
    cout << "- XML: " << process << endl;
    cout << endl;
  }


  return 0;
}

