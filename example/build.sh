#!/bin/bash

# Generate classes from XML schema definitions
../schematicpp -p BPMN -o BPMNParser -i DC.xsd DI.xsd BPMNDI.xsd Semantic.xsd BPMN20.xsd 

# Change to output directory
cd BPMNParser

# Build library and executable
mkdir build
cd build
cmake -DSRC=../main.cpp -DEXE=bpmnParser ..
make
cd ..
echo "bpmnParser created in folder 'BPMNParser'"

# Change back to original directory
cd ..

# Run executable
./BPMNParser/bpmnParser < diagram.bpmn

