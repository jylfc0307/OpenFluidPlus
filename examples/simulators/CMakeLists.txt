
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/src" "${PROJECT_BINARY_DIR}/src")                   


OPNFLD_ADD_SIMULATOR(examples.primitives.unitsA.prod ${SIMULATORS_OUTPUT_PATH})

OPNFLD_ADD_SIMULATOR(examples.primitives.unitsA.up ${SIMULATORS_OUTPUT_PATH})

OPNFLD_ADD_SIMULATOR(examples.primitives.unitsB.prod ${SIMULATORS_OUTPUT_PATH})


OPNFLD_ADD_SIMULATOR(examples.trafficlight.state ${SIMULATORS_OUTPUT_PATH})

OPNFLD_ADD_SIMULATOR(examples.road.traffic ${SIMULATORS_OUTPUT_PATH})


OPNFLD_ADD_SIMULATOR(examples.surf.wind-fire.connections ${SIMULATORS_OUTPUT_PATH})

OPNFLD_ADD_SIMULATOR(examples.surf.fire.prod-spread ${SIMULATORS_OUTPUT_PATH})

