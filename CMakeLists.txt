CMAKE_MINIMUM_REQUIRED( VERSION 3.0 )

###################### <Version> ######################
PROJECT( ISIS )
SET(PROJECT_PREFIX isis)
SET(PROJECT_VERSION "0.2.0")
ADD_DEFINITIONS(-DPROJECT_VERSION="${PROJECT_VERSION}")
ADD_DEFINITIONS(-DPROJECT_PREFIX="${PROJECT_PREFIX}")
###################### </Version> ######################

INCLUDE_DIRECTORIES (
	"${PROJECT_SOURCE_DIR}/includes" 
    "${PROJECT_SOURCE_DIR}/ui/includes"
)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/usr/local/include/boost")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/usr/local/lib")
SET(CMAKE_CXX_FLAGS "-std=c++0x")
#SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

###################### <INSTALL QT5 & BOOST> ######################
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    EXECUTE_PROCESS(
        COMMAND "brew install qt5 boost"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/local/Cellar/qt5/5.3.0/lib/cmake/Qt5Widgets/")  #"~/Applications/Qt5.3.0/5.3/clang_64/lib/cmake/Qt5Widgets")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    #TODO : Add linux specifics here
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
###################### </INSTALL QT5 & BOOST> ######################

###################### <UI INCLUDES> ######################
SET(ISIS_UI_RESOURCES
    ui/resources.qrc
)
SET(ISIS_UI_FORMS     
    ui/layouts/mainwindow.ui
    ui/layouts/aboutDialog.ui
)
SET(ISIS_UI_CPP 	
    ui/mainwindow.cpp
    ui/pollerthread.cpp
    ui/AboutDialog.cpp
    ui/QtOpenCV.cpp
)
SET(ISIS_UI_HEADERS
    ui/includes/mainwindow.h
    ui/includes/pollerthread.h
    ui/includes/AboutDialog.h
)
###################### </UI INCLUDES> #####################

###################### <Regular Src> #####################
SET(ISIS_SOURCE 
    main.cpp
    AI.cpp
    CamInstance.cpp
    VisionManager.cpp
    Isis.cpp
    Config.cpp
    speech.cpp
    COM.cpp
    trace.cpp
    Utility.cpp
)
###################### </Regular Src> #####################


###################### <Find The Reqd Packages> #####################
FIND_PACKAGE( OpenCV REQUIRED )
FIND_PACKAGE( Qt5Widgets REQUIRED )
FIND_PACKAGE( Boost COMPONENTS thread system filesystem chrono log program_options)
###################### </Find The Reqd Packages> ####################


###################### <BUILD CAFFE> #####################

ADD_CUSTOM_TARGET(
   caffe_lib
   COMMAND make
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/caffe
   #RESULT_VARIABLE rv
)
#MESSAGE("rv='${rv}'")

###################### </BUILD CAFFE> ####################


###################### <BUILD MITIE> #####################
SET(MITIE_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libs/MITIE/mitielib/libmitie.a)
ADD_CUSTOM_TARGET(
   build_mitie ALL
   COMMAND ${CMAKE_MAKE_PROGRAM}
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/MITIE
)
ADD_LIBRARY(mitie STATIC IMPORTED)
SET_PROPERTY(TARGET mitie APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
SET_TARGET_PROPERTIES(mitie PROPERTIES IMPORTED_LOCATION_NOCONFIG "${MITIE_LIBRARY}")
ADD_DEPENDENCIES(mitie build_mitie)
###################### </BUILD MITIE> ####################


###################### <BUILD MOE> #####################
#SET(MOE_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libs/MOE/mitielib/libmitie.a)
#ADD_CUSTOM_TARGET(
#   moe_lib ALL
#   COMMAND ${CMAKE_MAKE_PROGRAM}
#   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/MOE
#   #RESULT_VARIABLE rv
#)
#MESSAGE("rv='${rv}'")

###################### </BUILD MOE> ####################


###################### DO QT WRAPPING #####################
QT5_WRAP_UI(ISIS_FORMS_HEADERS ${ISIS_UI_FORMS})
QT5_WRAP_CPP(ISIS_HEADERS_MOC ${ISIS_UI_HEADERS})
QT5_ADD_RESOURCES(ISIS_UI_RES ${ISIS_UI_RESOURCES})
###################### /DO QT WRAPPING #####################

IF(Boost_FOUND)
        SET(Boost_USE_MULTITHREADED ON)
        INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS})
        ADD_DEFINITIONS( "-DHAS_BOOST" "-DBOOST_ALL_DYN_LINK")
ENDIF()

ADD_EXECUTABLE( ISIS.out  
	${ISIS_SOURCE} 
	${ISIS_UI_CPP} 
	${ISIS_HEADERS_MOC} 
	${ISIS_FORMS_HEADERS} 
	${ISIS_UI_RES}
)

TARGET_LINK_LIBRARIES(ISIS.out ${OpenCV_LIBS} ${Boost_LIBRARIES} ${OPENCL_LIBRARIES} mitie)
QT5_USE_MODULES( ISIS.out Widgets PrintSupport)
