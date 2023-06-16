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

#ifndef SCHEMATICPP_CLASS_H
#define SCHEMATICPP_CLASS_H

#include <string>
#include <string_view>
#include <map>
#include <list>
#include <set>
#include <limits.h>

#define UNBOUNDED INT_MAX
#define XSL "http://www.w3.org/2001/XMLSchema"

typedef std::string NamespaceName;
typedef std::string ClassName;
typedef std::pair<NamespaceName, ClassName> FullName;

class Class {
public:
    class Member {
    public:
        std::string name;
        std::string cppName;
        FullName type;
        Class *cl;          //NULL is class is unknown (only allowed for optionals and vectors)
        std::string defaultStr;
        int minOccurs;
        int maxOccurs;
        bool isAttribute;   //true if this member is an attribute rather than an element
        bool isArray() const;
        bool isOptional() const;    //returns true if this member is optional (not an array)
    };

private:
    //classes that we should friend so they can access our default constructor
    std::set<std::string> friends;

    //set of C++ keywords
    static std::set<std::string_view> keywordSet;

public:
    static std::string sanitize(std::string str) {
      //strip any bad characters such as dots, colons, semicolons..
      std::string ret;

      for (size_t x = 0; x < str.size(); x++) {
        char c = str[x];

        if ((c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                (c >= 'A' && c <= 'Z') ||
                c == '_') {
            ret += c;
        }
        else {
            ret += "_";
        }
      }

      //check if identifier is a reserved C++ keyword, and append an underscore if so
      if (keywordSet.find(ret) != keywordSet.end()) {
        ret += "_";
      }

      return ret;
    }

    enum ClassType {
        SIMPLE_TYPE,
        COMPLEX_TYPE,
    };

    bool isSimple() const;
    virtual bool isBuiltIn() const;

    const FullName name;
    const std::string cppName;
    const ClassType type;

    bool isDocument;            //true if this is a document class

    FullName baseType;
    Class *base;

    bool hasBase() const;
    
    std::list<Member> members;
    std::list<FullName> groups; //attributeGroups to add to this class

    Class(FullName name, ClassType type);
    Class(FullName name, ClassType type, FullName baseType);
    virtual ~Class();

    std::list<Member>::iterator findMember(std::string name);
    void addMember(Member memberInfo);

    /**
     * Do work needed before writeImplementation() or writeHeader() are called.
     * This method is called after the classes of each member has been resolved.
     */
    void doPostResolveInit();

    /**
     * Returns the name with which to refer to this class.
     */
    virtual std::string getClassname() const;

    /**
     * Returns the sanitized name which to use in the generated code.
     */
    virtual std::string getCppClassname() const;

    /**
     * Returns name of header wherein the base class is defined.
     */
    virtual std::string getBaseHeader() const;

    /**
     * Returns whether this type has a header when it is the base of another class.
     * Examples include normal complex types and certain built-in types like std::string.
     */
    virtual bool hasHeader() const;

    void writeImplementation(std::ostream& os) const;
    void writeHeader(std::ostream& os) const;
};

#endif /* SCHEMATICPP_CLASS_H */

