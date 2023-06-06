# This file is in the public domain.
add_library(libschematicpp
    Exceptions.cpp
    HexBinary.cpp
    XercesString.cpp
    XMLObject.cpp
)

set_target_properties(libschematicpp PROPERTIES OUTPUT_NAME schematicpp)
install(TARGETS libschematicpp DESTINATION lib)

install(FILES
    Exceptions.h
    HexBinary.h
    version.h
    XercesString.h
    XMLDocument.h
    XMLObject.h
    optional.h
    DESTINATION include/libschematicpp
)

