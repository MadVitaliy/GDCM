IF(WIN32)
  INCLUDE(${DotNETFrameworkSDK_USE_FILE})
  # remap
  SET(CMAKE_CSHARP1_COMPILER ${CSC_v1_EXECUTABLE})
  SET(CMAKE_CSHARP2_COMPILER ${CSC_v2_EXECUTABLE})
  SET(CMAKE_CSHARP3_COMPILER ${CSC_v3_EXECUTABLE})

  #SET(CMAKE_CSHARP3_INTERPRETER ${MONO_EXECUTABLE})
ELSE(WIN32)
  INCLUDE(${MONO_USE_FILE})
  SET(CMAKE_CSHARP1_COMPILER ${MCS_EXECUTABLE})
  SET(CMAKE_CSHARP2_COMPILER ${GMCS_EXECUTABLE})
  SET(CMAKE_CSHARP3_COMPILER ${SMCS_EXECUTABLE})

  SET(CMAKE_CSHARP3_INTERPRETER ${MONO_EXECUTABLE})
ENDIF(WIN32)

# default to v1:
SET(CMAKE_CSHARP_COMPILER ${CMAKE_CSHARP1_COMPILER})

