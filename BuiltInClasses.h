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

#ifndef _BUILTINCLASSES_H
#define _BUILTINCLASSES_H

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
    bool shouldUseConstReferences() const {return false;}\
};

#define GENERATE_BUILTIN_ALIAS(name, base, override)\
class name : public base {\
public:\
    name() : base(override) {}\
};

GENERATE_BUILTIN(StringClass, "string", "std::string")

GENERATE_BUILTIN(ByteClass, "byte", "char")
GENERATE_BUILTIN(UnsignedByteClass, "unsignedByte", "unsigned char")
GENERATE_BUILTIN(ShortClass, "short", "short")
GENERATE_BUILTIN(UnsignedShortClass, "unsignedShort", "unsigned short")
GENERATE_BUILTIN(IntClass, "int", "int")
GENERATE_BUILTIN(UnsignedIntClass, "unsignedInt", "unsigned int")
GENERATE_BUILTIN(LongClass, "long", "long long")
GENERATE_BUILTIN(UnsignedLongClass, "unsignedLong", "unsigned long long")
GENERATE_BUILTIN(FloatClass, "float", "float")
GENERATE_BUILTIN(DoubleClass, "double", "double")
GENERATE_BUILTIN(BooleanClass, "boolean", "bool")

//aliases
GENERATE_BUILTIN_ALIAS(IntegerClass, IntClass, "integer")
GENERATE_BUILTIN_ALIAS(DecimalClass, DoubleClass, "decimal")
GENERATE_BUILTIN_ALIAS(AnyURIClass, StringClass, "anyURI")
GENERATE_BUILTIN_ALIAS(TimeClass, StringClass, "time")
GENERATE_BUILTIN_ALIAS(DateClass, StringClass, "date")
GENERATE_BUILTIN_ALIAS(DateTimeClass, StringClass, "dateTime")
GENERATE_BUILTIN_ALIAS(LanguageClass, StringClass, "language")
GENERATE_BUILTIN_ALIAS(QualifiedNameClass, StringClass, "QName")
GENERATE_BUILTIN_ALIAS(IdClass, StringClass, "ID")
GENERATE_BUILTIN_ALIAS(IdRefClass, StringClass, "IDREF")

#endif /* _BUILTINCLASSES_H */

