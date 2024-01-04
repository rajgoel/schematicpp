/*
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
#include <vector>
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace std;

extern bool verbose;
extern string schemaName;
extern vector<string> schemaNames;
extern string cppNamespace;

Class::Class(FullName name, ClassType type) : name(name), cppName(sanitize(name.second)), type(type), 
        isDocument(false), base(NULL), schema(schemaName)  {
}

Class::Class(FullName name, ClassType type, FullName baseType) : name(name), cppName(sanitize(name.second)),
        type(type), isDocument(false), baseType(baseType), base(NULL), schema(schemaName) {
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

    memberInfo.cppName = sanitize(memberInfo.name);
    members.push_back(memberInfo);
}


string Class::getClassname() const {
    return name.second;
}

string Class::getCppClassname() const {
    return cppName;
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
    os << "using namespace XML::" << cppNamespace << ";" << endl;
    os << endl;

    if (!isSimple()) {
      os << getCppClassname() << "::" << getCppClassname() << "(const Namespace& xmlns, const ClassName& className, const xercesc::DOMElement* element, const Attributes& defaultAttributes) :" << endl;
      if (base) {
        os << "\t" << base->getCppClassname() << "(xmlns, className, element, defaultAttributes)" << endl;
      }
      else {
        os << "\tXMLObject(xmlns, className, element, defaultAttributes)" << endl;
      }
      //member initialization
      for (list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
        if (!it->cl) {
          continue;
        }

        if ( it->isAttribute ) {
          if (it->isOptional() ) {
            os << "\t, " << it->cppName << "(getOptionalAttributeByName(\"" << it->name<< "\"))" << endl;
          }
          else {
            os << "\t, " << it->cppName << "(getRequiredAttributeByName(\"" << it->name<< "\"))" << endl;
          }
        }
        else if (!it->cl->isBuiltIn()) {
          if (it->isArray()) {
            os << "\t, " << it->cppName << "(getChildren<" << it->cl->getCppClassname() << ">())" << endl;
          }
          else if (it->isOptional() ) {
            os << "\t, " << it->cppName << "(getOptionalChild<" << it->cl->getCppClassname() << ">())" << endl;
          }
          else {
            os << "\t, " << it->cppName << "(getRequiredChild<" << it->cl->getCppClassname() << ">())" << endl;
          }
        }
        else {
          if (it->isArray()) {
            os << "\t, " << it->cppName << "(getChildrenByName(\"" << it->name << "\"))" << endl;
          }
          else if (it->isOptional() ) {
            os << "\t, " << it->cppName << "(getOptionalChildByName(\"" << it->name << "\"))" << endl;
          }
          else {
            os << "\t, " << it->cppName << "(getRequiredChildByName(\"" << it->name << "\"))" << endl;
          }
        }
      }
      os << "{" << endl;
      os << "}" << endl;
    }
}

void Class::writeHeader(ostream& os) const {
    ClassName className = getClassname();
    ClassName cppName = getCppClassname();

    os << "#ifndef XML_" << cppNamespace << "_" << className << "_H" << endl;
    os << "#define XML_" << cppNamespace << "_" << className << "_H" << endl;

    os << "#include <memory>" << endl;
    os << "#include <optional>" << endl;
    os << "#include <vector>" << endl;
    os << endl;
    os << "#include \"../XMLObject.h\"" << endl;

    //simple types only need a typedef
    if (isSimple()) {
      os << endl;
      os << "typedef " << base->getCppClassname() << " " << name.second << ";" << endl;
    } else {
        if (base && base->hasHeader()) {
            os << "#include " << getBaseHeader() << endl;
        }

        //include non-builtin member classes and non-simple member classes
        for (list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
          if (it->cl && (!it->cl->isBuiltIn() || !it->cl->isSimple())) {
            os << "#include \"" << it->cl->getClassname() << ".h\"" << endl;
          }
        }

        os << endl;
        os << "/**" << endl;
        os << " * @brief The `XML::" << cppNamespace << "` namespace contains classes from the following XML-schema(s): ";
        for (auto schema : schemaNames ) {
          if ( schema != schemaNames.front() ) os << ", ";
          os << "@ref " << schema;
        }
        os << "." << endl;
        os << " */" << endl;
        os << "namespace XML::" << cppNamespace << " {" << endl;

        os << endl;
        bool memberClass = false;
        //declare non-builtin member classes and non-simple member classes
        for (list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
          if (it->cl && (!it->cl->isBuiltIn() || !it->cl->isSimple())) {
            os << "class " << it->cl->getCppClassname() << ";" << endl;
            memberClass = true;
          }
        }
        if ( memberClass ) os << endl;

        os << "/**" << endl;
        os << " * Overview:" << endl;
        os << " * - Element name:  " << cppName << endl;
        os << " * - XML-Schema:    " << schema << endl;
        os << " * - XML-Namespace: " << name.first << endl;
        os << " *" << endl;
        os << " * Members:" << endl;
        // obtain all defaults including those from base classes
        const Class* c = this;
        while ( true ) {
          for (list<Member>::const_iterator it = c->members.begin(); it != c->members.end(); it++) {
            os << " * - " << it->name << " : " << it->type.second << " [" << it->minOccurs << ".." << (it->maxOccurs == UNBOUNDED ? "*" : to_string(it->maxOccurs)) << "]";
            if ( c != this ) {
              os << " (from: " << c->cppName << ")";
            }
            os << endl;
          }
          if ( c->base ) { 
            c = c->base;
          }
          else { 
            break;
          }
        }
        os << " *" << endl;
        os << " * Automatically generated by schematic++ v" << VERSION << " (https://github.com/rajgoel/schematicpp)" << endl;
        os << " */" << endl;

        if (base && !base->isSimple()) {
            os << "class " << cppName << " : public " << base->getCppClassname();
        }
        else {
            os << "class " << cppName << " : public XMLObject";
        }

        os << " {" << endl;

        os << "\ttemplate<typename T> friend XMLObject* ::XML::createInstance(const Namespace& xmlns, const ClassName& className, const xercesc::DOMElement* element);" << endl; 

        os << "private:" << endl;

        os << "\tstatic bool registerClass() {" << endl;
        os << "\t\tXMLObject::factory[\"" << name.first << ":" << className << "\"] = &createInstance<" << cppName << ">; // register function in factory" << endl;
        os << "\t\treturn true;" << endl;
        os << "\t};" << endl;
        os << "\tinline static bool registered = registerClass();" << endl;
        os << "protected:" << endl;
        os << "\t" << cppName << "(const Namespace& xmlns, const ClassName& className, const xercesc::DOMElement* element, const Attributes& defaultAttributes);" << endl;
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
        os << "\tinline static const Attributes defaults = {";
        bool first = true;

        // obtain all defaults including those from base classes
        c = this;
        while ( true ) {
          for (list<Member>::const_iterator it = c->members.begin(); it != c->members.end(); it++) {
            if ( !it->defaultStr.empty() ) {
              if (!first) os << ",";
              os << endl;
              os << "\t\t{ .xmlns = \"" << name.first << "\", .prefix = \"\" , .name = \"" << it->name  << "\", .value = Value(std::string(\"" << it->defaultStr << "\"))}";
              first = false; 
            }
          }
          if ( c->base ) { 
            c = c->base;
          }
          else { 
            break;
          }
        }

        os << endl;
        os << "\t};" << endl; 
        os << endl;

        //members
        for (list<Member>::const_iterator it = members.begin(); it != members.end(); it++) {
            if (!it->cl) {
              os << "\t//" << it->cppName << " (" << it->type.first << ":" << it->type.second << ") is undefined" << endl;
              continue;
            }

            os << "\t";
            if ( it->isAttribute ) {
				      if (it->isOptional()) {
                os << "std::optional< std::reference_wrapper<Attribute> > " << it->cppName << "; ";
              }
              else {
                os << "Attribute& " << it->cppName << "; ";
              }
              os << "///< Attribute value can be expected to be of type '" << (it->cl->isBuiltIn() ? it->cl->getClassname() : it->cl->base->getCppClassname()) << "'" << endl;
            }
            else if (!it->cl->isBuiltIn()) {
              std::string cppMember = (it->cl->isBuiltIn() ? it->cl->getClassname() : it->cl->getCppClassname());
              if (it->isArray()) {
                os << "std::vector< std::reference_wrapper<" << it->cl->getCppClassname() << "> > " << it->cppName << ";" << endl;
              }
              else if (it->isOptional()) {
                os << "std::optional< std::reference_wrapper<" << it->cl->getCppClassname() << "> > " << it->cppName << ";" << endl;
              }
							else {
                os << it->cl->getCppClassname() << "& " << it->cppName << ";" << endl;
              }
            }
            else {
              if (it->isArray()) {
                os << "std::vector< std::reference_wrapper<XMLObject> > " << it->cppName << ";" << endl;
              }
              else if (it->isOptional()) {
                os << "std::optional< std::reference_wrapper<XMLObject> > " << it->cppName << ";" << endl;
              }
							else {
                os << "XMLObject& " << it->cppName << ";" << endl;
              }
            }
        }

        os << "};" << endl;
        os << endl;
        os << "} // namespace XML::" << cppNamespace << endl;

    }

    os << endl;
    os << "#endif // XML_" << cppNamespace << "_" << className << "_H" << endl;

}

bool Class::Member::isArray() const {
    return maxOccurs > 1 || maxOccurs == UNBOUNDED;
}

bool Class::Member::isOptional() const {
    return minOccurs == 0 && maxOccurs == 1;
}

// https://en.cppreference.com/w/cpp/keyword
std::set<std::string_view> Class::keywordSet = {
    "alignas",
    "alignof",
    "and",
    "and_eq",
    "asm",
    "atomic_cancel",
    "atomic_commit",
    "atomic_noexcept",
    "auto",
    "bitand",
    "bitor",
    "bool",
    "break",
    "case",
    "catch",
    "char",
    "char8_t",
    "char16_t",
    "char32_t",
    "class",
    "compl",
    "concept",
    "const",
    "consteval",
    "constexpr",
    "constinit",
    "const_cast",
    "continue",
    "co_await",
    "co_return",
    "co_yield",
    "decltype",
    "default",
    "delete",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "export",
    "extern",
    "false",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "mutable",
    "namespace",
    "new",
    "not",
    "not_eq",
    "nullptr",
    "operator",
    "or",
    "or_eq",
    "private",
    "protected",
    "public",
    "reflexpr",
    "register",
    "reinterpret_cast",
    "requires",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_assert",
    "static_cast",
    "struct",
    "switch",
    "synchronized",
    "template",
    "this",
    "thread_local",
    "throw",
    "true",
    "try",
    "typedef",
    "typeid",
    "typename",
    "union",
    "unsigned",
    "using",
    "virtual",
    "void",
    "volatile",
    "wchar_t",
    "while",
    "xor",
    "xor_eq",
};

