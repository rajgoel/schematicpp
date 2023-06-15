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

#ifndef SCHEMATICPP_BUILTINCLASSES_H
#define SCHEMATICPP_BUILTINCLASSES_H

#include "Class.h"

class BuiltInClass : public Class {
public:
    BuiltInClass(std::string name);
    virtual ~BuiltInClass();

    bool isBuiltIn() const;
};

//same as GENERATE_BUILTIN, except header-less and shouldUseConstReferences() is made to return false
#define GENERATE_BUILTIN(name, xslName, classname)\
class name : public BuiltInClass {\
public:\
    name() : BuiltInClass(xslName) {}\
    name(std::string xslOverride) : BuiltInClass(xslOverride) {}\
    std::string getClassname() const {return classname;}\
    bool hasHeader() const { return false; }\
};

#define GENERATE_BUILTIN_ALIAS(name, base, override)\
class name : public base {\
public:\
    name() : base(override) {}\
};

GENERATE_BUILTIN(StringClass, "string", "std::string")
GENERATE_BUILTIN(IntegerClass, "integer", "int")
GENERATE_BUILTIN(DecimalClass, "decimal", "double")
GENERATE_BUILTIN(BooleanClass, "boolean", "bool")

//aliases
GENERATE_BUILTIN_ALIAS(IntClass, IntegerClass, "int")
GENERATE_BUILTIN_ALIAS(FloatClass, DecimalClass, "float")
GENERATE_BUILTIN_ALIAS(DoubleClass, DecimalClass, "double")
GENERATE_BUILTIN_ALIAS(AnyURIClass, StringClass, "anyURI")
GENERATE_BUILTIN_ALIAS(TimeClass, StringClass, "time")
GENERATE_BUILTIN_ALIAS(DateClass, StringClass, "date")
GENERATE_BUILTIN_ALIAS(DateTimeClass, StringClass, "dateTime")
GENERATE_BUILTIN_ALIAS(QualifiedNameClass, StringClass, "QName")
GENERATE_BUILTIN_ALIAS(IdClass, StringClass, "ID")
GENERATE_BUILTIN_ALIAS(IdRefClass, StringClass, "IDREF")

#endif /* _BUILTINCLASSES_H */

