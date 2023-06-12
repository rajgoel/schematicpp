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

#ifndef _CLASS_H
#define _CLASS_H

#include <string>
#include <map>
#include <list>
#include <set>
#include <limits.h>

#define UNBOUNDED INT_MAX

typedef std::string NamespaceName;
typedef std::string ClassName;
typedef std::pair<NamespaceName, ClassName> FullName;

extern const std::string variablePostfix;

//commonly used temp variable names
extern const std::string nodeWithPostfix;       //"node" + variablePostfix
extern const std::string tempWithPostfix;       //"temp" + variablePostfix
extern const std::string convertedWithPostfix;  //"converted" + variablePostfix
extern const std::string ssWithPostfix;         //"ss" + variablePostfix

class Class {
public:
    class Member {
    public:
        std::string name;
        FullName type;
        Class *cl;          //NULL is class is unknown (only allowed for optionals and vectors)
        std::string defaultStr;
        int minOccurs;
        int maxOccurs;
        bool isAttribute;   //true if this member is an attribute rather than an element
        bool isArray() const;
        bool isOptional() const;    //returns true if this member is optional (not an array)
        bool isRequired() const;
    };

private:
    //classes that we should friend so they can access our default constructor
    std::set<std::string> friends;

public:
    enum ClassType {
        SIMPLE_TYPE,
        COMPLEX_TYPE,
    };

    bool isSimple() const;
    virtual bool isBuiltIn() const;

    const FullName name;
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
     * Should return the name with which to refer to this Class.
     */
    virtual std::string getClassname() const;

    /**
     * Returns name of header wherein the base class is defined.
     */
    virtual std::string getBaseHeader() const;

    /**
     * Returns whether this type has a header when it is the base of another class.
     * Examples include normal complex types and certain built-in types like std::string.
     */
    virtual bool hasHeader() const;

    /**
     * Returns whether the constructor should take const references to this class or not.
     * Counter cases include xs:int, xs:byte etc.
     */
    virtual bool shouldUseConstReferences() const;

    std::set<std::string> getIncludedClasses() const;
    std::set<std::string> getPrototypeClasses() const;

    /**
     * Returns a list of the required elements of this Class.
     * Also include vector elements if vectors == true.
     * Also include optional elements if optionals == true.
     * Also includes those of its base if includingBase == true.
     */
    std::list<Member> getElements(bool includeBase, bool vectors, bool optionals) const;

    void writeImplementation(std::ostream& os) const;
    void writeHeader(std::ostream& os) const;
};

#endif /* _CLASS_H */

