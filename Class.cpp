/*
 * File:   main.cpp
 * Authors: Asvin Goel (rajgoel), Tomas Härdin (tjoppen)
 *
 * Forked from: https://github.com/Tjoppen/james (June 6, 2023)
 *
 * LICENSE:
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Class.h"
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace std;

extern bool verbose;

const string variablePostfix = "_schematicpp";
const string nodeWithPostfix = "node" + variablePostfix;
const string tempWithPostfix = "temp" + variablePostfix;
const string convertedWithPostfix = "converted" + variablePostfix;
const string ssWithPostfix = "ss" + variablePostfix;

Class::Class(FullName name, ClassType type) : name(name), type(type), 
        isDocument(false), base(NULL)  {
}

Class::Class(FullName name, ClassType type, FullName baseType) : name(name),
        type(type), isDocument(false), baseType(baseType), base(NULL) {
}

Class::~Class() {
}

bool Class::isSimple() const {
    return type == SIMPLE_TYPE;
}

bool Class::isBuiltIn() const {
    return false;
}

bool Class::hasBase() const {
    return baseType.second.length() > 0;
}


void Class::doPostResolveInit() {
    //make sure members classes add us as their friend
    for (std::list<Member>::iterator it = members.begin(); it != members.end(); it++) {
        //there's no need to befriend ourselves
        if (it->cl && it->cl != this) {
            it->cl->friends.insert(getClassname());
        }
    }
}

std::list<Class::Member>::iterator Class::findMember(std::string name) {
    for (std::list<Member>::iterator it = members.begin(); it != members.end(); it++) {
        if (it->name == name) {
            return it;
        }
    }

    return members.end();
}

void Class::addMember(Member memberInfo) {
    if (findMember(memberInfo.name) != members.end()) {
        throw runtime_error("Member " + memberInfo.name + " defined more than once in " + this->name.second);
    }

    if (verbose) cerr << this->name.second << " got " << memberInfo.type.first << ":" << memberInfo.type.second << " " << memberInfo.name << ". Occurance: ";

    if (memberInfo.maxOccurs == UNBOUNDED) {
        if (verbose) cerr << "at least " << memberInfo.minOccurs;
    } else if (memberInfo.minOccurs == memberInfo.maxOccurs) {
        if (verbose) cerr << "exactly " << memberInfo.minOccurs;
    } else {
        if (verbose) cerr << "between " << memberInfo.minOccurs << "-" << memberInfo.maxOccurs;
    }

    if (verbose) cerr << endl;

    members.push_back(memberInfo);
}


string Class::getClassname() const {
    return name.second;
}

string Class::getBaseHeader() const {
    if (base->isSimple()) {
        return base->getBaseHeader();
    }

    return "\"" + base->getClassname() + ".h\"";
}

bool Class::hasHeader() const {
    return true;
}

void Class::writeImplementation(ostream& os) const {
    ClassName className = name.second;

    os << "#include \"" << className << ".h\"" << endl;
    os << endl;
    os << "using namespace XML;" << endl;
    os << endl;

    if (!isSimple()) {
      os << className << "::" << className << "(const ClassName& className, const xercesc::DOMElement* element, XMLObject* parent) :" << endl;
      if (base) {
        os << "\t" << base->getClassname() << "(className, element, parent)" << endl;
      }
      else {
        os << "\tXMLObject(className, element, parent)" << endl;
      }
      //member initialization
      for (list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        if (!it->cl) {
          continue;
        }

        if ( it->isAttribute ) {
          if (it->isOptional() ) {
            os << "\t, " << it->name << "(std::nullopt) // use placeholder because defaults are not available yet" << endl;
          }
          else {
            os << "\t, " << it->name << "(_attribute_) // use placeholder because defaults are not available yet" << endl;
          }
        }
        else {
          if (it->isArray()) {
            os << "\t, " << it->name << "(getChildren<" << it->cl->getClassname() << ">())" << endl;
          }
          else if (it->isOptional() ) {
            os << "\t, " << it->name << "(getOptionalChild<" << it->cl->getClassname() << ">())" << endl;
          }
          else {
            os << "\t, " << it->name << "(getRequiredChild<" << it->cl->getClassname() << ">())" << endl;
          }
        }
      }
      os << "{" << endl;

      os << "\t// add defaults for missing attributes" << endl;
      os << "\tfor ( auto& defaultAttribute : defaults ) {" << endl;
//      os << "\t\tif ( getOptionalAttributeByName(defaultAttribute.name) == std::nullopt ) {" << endl;
      os << "\t\tif ( !getOptionalAttributeByName(defaultAttribute.name) ) {" << endl;
      os << "\t\t\tattributes.push_back(defaultAttribute);" << endl;
      os << "\t\t}" << endl;
      os << "\t}" << endl;
      os << endl;

      os << "\t// set references for attribute members" << endl;
      //member manipulation
      for (list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        //elements of unknown types are ignored
        if (!it->cl) {
          continue;
        }

//        os << "\n\t// " << it->name << " (" << it->cl->getClassname() << ")" << endl;

        if (it->isAttribute) {
          if ( !it->cl->isSimple() ) {
            throw runtime_error("Complex data type illegal for attribute: " + it->cl->getClassname());
          }
          if (it->isOptional()) {
            os << "\t" << it->name << " = getOptionalAttributeByName(\"" << it->name << "\");" << endl;
          }
          else {
            os << "\t" << it->name << " = getRequiredAttributeByName(\"" << it->name << "\");" << endl;
          }
        }
      }

      os << "}" << endl;
    }
}

set<string> Class::getIncludedClasses() const {
    set<string> classesToInclude;

    //return classes of any simple non-builtin elements and any required non-simple elements
    for (list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        if (it->cl && ((!it->cl->isBuiltIn() && it->cl->isSimple()) || (it->isRequired() && !it->cl->isSimple()))) {
            classesToInclude.insert(it->cl->getClassname());
        }
    }

    return classesToInclude;
}

set<string> Class::getPrototypeClasses() const {
    set<string> classesToInclude = getIncludedClasses();
    set<string> classesToPrototype;

    //return the classes of any non-simple non-required elements
    for (list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        if (it->cl && classesToInclude.find(it->cl->getClassname()) == classesToInclude.end() && !it->cl->isSimple() && !it->isRequired()) {
            classesToPrototype.insert(it->cl->getClassname());
        }
    }

    return classesToPrototype;
}

void Class::writeHeader(ostream& os) const {
    ClassName className = name.second;

    os << "#ifndef XML_" << className << "_H" << endl;
    os << "#define XML_" << className << "_H" << endl;

    os << "#include <memory>" << endl;
    os << "#include <optional>" << endl;
    os << "#include <vector>" << endl;
    os << endl;
    os << "#include \"XMLObject.h\"" << endl;

    //simple types only need a typedef
    if (isSimple()) {
      os << endl;
      os << "typedef " << base->getClassname() << " " << name.second << ";" << endl;
    } else {
        if (base && base->hasHeader()) {
            os << "#include " << getBaseHeader() << endl;
        }


        //include member classes that we can't prototype
        set<string> classesToInclude = getIncludedClasses();

        for (set<string>::const_iterator it = classesToInclude.begin(); it != classesToInclude.end(); it++) {
            os << "#include \"" << *it << ".h\"" << endl;
        }

        os << endl;
        os << "using namespace std;" << endl;
        os << endl;
        os << "namespace XML {" << endl;
        os << endl;

        set<string> classesToPrototype = getPrototypeClasses();

        //member class prototypes, but only for classes that we haven't already included
        for (set<string>::const_iterator it = classesToPrototype.begin(); it != classesToPrototype.end(); it++) {
            os << "class " << *it << ";" << endl;
        }

        if (classesToPrototype.size() > 0) {
            os << endl;
        }

        if (base && !base->isSimple()) {
            os << "class " << className << " : public " << base->getClassname();
        }
        else {
            os << "class " << className << " : public XMLObject";
        }

        os << " {" << endl;

        os << "\ttemplate<typename T> friend XMLObject* createInstance(const ClassName& className, const xercesc::DOMElement* element, XMLObject* parent);" << endl; 

        os << "private:" << endl;

        os << "\tstatic bool registerClass() {" << endl;
        os << "\t\tXMLObject::factory[\"" << className << "\"] = &createInstance<" << className << ">; // register function in factory" << endl;
        os << "\t\treturn true;" << endl;
        os << "\t};" << endl;
        os << "\tinline static bool registered = registerClass();" << endl;
        os << "protected:" << endl;
        os << "\t" << className << "(const ClassName& className, const xercesc::DOMElement* element, XMLObject* parent);" << endl;
        os << endl;

        if (friends.size()) {
            //add friends
            for (set<string>::const_iterator it = friends.begin(); it != friends.end(); it++) {
                os << "\tfriend class " << *it << ";" << endl;
            }

            os << endl;
        }

        os << "public:" << endl;

        os << "\t/// default attributes to be used if they are not explicitly provided" << endl;
        os << "\tinline static const vector<Attribute> defaults = {";
        bool first = true;
        for (list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
          if ( !it->defaultStr.empty() ) {
            if (!first) os << ",";
            os << endl;
            os << "\t\t{.name = \"" << it->name  << "\", .value = \"" << it->defaultStr << "\"}";
            first = false; 
          }
        }
        os << endl;
        os << "\t};" << endl; 
        os << endl;


        //simpleContent
        if (base && base->isSimple()) {
            os << "\t" << base->getClassname() << " content;" << endl;
        }

        //members
        for (list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
            if (!it->cl) {
              os << "\t//" << it->name << " (" << it->type.first << ":" << it->type.second << ") is undefined" << endl;
              continue;
            }

            os << "\t";

            //elements of unknown types are shown commented out
            if (!it->cl) {
                os << "//";
            }

            if ( it->isAttribute ) {
				      if (it->isOptional()) {
                os << "optional< reference_wrapper<Attribute> > " << it->name << "; ";
              }
              else {
                os << "Attribute& " << it->name << "; ";
              }
              os << "///< Attribute value can be expected to be of type '" << (it->cl->isBuiltIn() ? it->cl->getClassname() : it->cl->base->getClassname()) << "'" << endl;
            }
            else {
              if (it->isArray()) {
                os << "vector< reference_wrapper<" << it->cl->getClassname() << "> > " << it->name << ";" << endl;
              }
              else if (it->isOptional()) {
                os << "optional< reference_wrapper<" << it->cl->getClassname() << "> > " << it->name << ";" << endl;
              }
							else {
                os << it->cl->getClassname() << "& " << it->name << ";" << endl;
              }
            }
        }

        os << "};" << endl;
        os << endl;
        os << "} // namespace XML" << endl;

        //include classes that we prototyped earlier
        for (set<string>::const_iterator it = classesToPrototype.begin(); it != classesToPrototype.end(); it++) {
            os << "#include \"" << *it << ".h\"" << endl;
        }

        if (classesToPrototype.size() > 0) {
            os << endl;
        }
    }

    os << endl;
    os << "#endif // XML_" << className << "_H" << endl;
}

bool Class::Member::isArray() const {
    return maxOccurs > 1 || maxOccurs == UNBOUNDED;
}

bool Class::Member::isOptional() const {
    return minOccurs == 0 && maxOccurs == 1;
}

bool Class::Member::isRequired() const {
    return !isArray() && !isOptional();
}


