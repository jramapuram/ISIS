CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

###################### <Version> ######################
PROJECT( ISIS )
SET(PROJECT_PREFIX isis)
SET(PROJECT_VERSION "0.2.0")
SET(USE_GPU 1)
ADD_DEFINITIONS(-DPROJECT_VERSION="${PROJECT_VERSION}")
ADD_DEFINITIONS(-DPROJECT_PREFIX="${PROJECT_PREFIX}")
###################### </Version> ######################

INCLUDE_DIRECTORIES (
	"${PROJECT_SOURCE_DIR}/includes" 
        "${PROJECT_SOURCE_DIR}/libs/caffe/include"
        "${PROJECT_SOURCE_DIR}/libs/MITIE/mitielib/include"
        "${PROJECT_SOURCE_DIR}/ui/includes"
)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/usr/local/include/boost")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/usr/local/lib")
SET(CMAKE_CXX_FLAGS "-std=c++11")
#SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

###################### <ADD QT5 & BOOST PARAMS> ######################
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/local/Cellar/qt5/5.3.0/lib/cmake/Qt5Widgets/")  #"~/Applications/Qt5.3.0/5.3/clang_64/lib/cmake/Qt5Widgets")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/local/qt5/5.3/gcc_64/lib/cmake/Qt5Widgets")
    #TODO : Add linux specifics here
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
###################### </ADD QT5 & BOOST PARAMS> ######################

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
IF(${USE_GPU})
    ADD_CUSTOM_TARGET(caffe_conf
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/conf/caffe/Makefile.config.gpu
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/caffe/Makefile.config)
ELSE(${USE_GPU})
    ADD_CUSTOM_TARGET(caffe_conf
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/conf/caffe/Makefile.config.cpu
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/caffe/Makefile.config)
ENDIF(${USE_GPU})

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    ADD_CUSTOM_TARGET(caffe_fix
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libs/conf/caffe/make_osx_compatible.rb
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/conf/caffe
)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    ADD_CUSTOM_TARGET(caffe_fix
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libs/conf/caffe/make_linux_compatible.rb
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/conf/caffe
)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

SET(CAFFE_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libs/caffe/build/lib/libcaffe.so)

ADD_CUSTOM_TARGET(
   build_caffe ALL
   COMMAND ${CMAKE_MAKE_PROGRAM}
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/caffe
)
ADD_LIBRARY(caffe SHARED IMPORTED)
SET_PROPERTY(TARGET caffe APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
SET_TARGET_PROPERTIES(caffe PROPERTIES IMPORTED_LOCATION_NOCONFIG "${CAFFE_LIBRARY}")
ADD_DEPENDENCIES(caffe caffe_conf build_caffe caffe_fix)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/libs/caffe/build/src)
###################### </BUILD CAFFE> ####################

###################### <MODELS> #####################
SET(REQD_MODELS
    isis.proto
    isis_solver.proto
    imagenet_deploy.prototxt
    caffe_reference_imagenet_model
    synset_words.txt
)
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/models)
FOREACH(f ${REQD_MODELS})
    ADD_CUSTOM_TARGET(model_${f} ALL
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       ${CMAKE_CURRENT_SOURCE_DIR}/models/${f}
       ${CMAKE_BINARY_DIR}/models/${f}
       #DEPENDS get_imagenet_model get_imagenet_extras
     )
ENDFOREACH(f)
###################### </MODELS> #####################


###################### <BUILD MITIE> #####################
SET(MITIE_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libs/MITIE/mitielib/libmitie.so)
ADD_CUSTOM_TARGET(
   build_mitie ALL
   COMMAND ${CMAKE_MAKE_PROGRAM}
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/MITIE
)
ADD_LIBRARY(mitie SHARED IMPORTED)
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

IF(CUDA_FOUND)
       INCLUDE_DIRECTORIES(${CUDA_INCLUDE_DIRS})
ENDIF()

ADD_EXECUTABLE( ISIS.out  
	${ISIS_SOURCE} 
	${ISIS_UI_CPP} 
	${ISIS_HEADERS_MOC} 
	${ISIS_FORMS_HEADERS} 
	${ISIS_UI_RES}
        ${CUDA_INCLUDE_DIRS}
)

TARGET_LINK_LIBRARIES(ISIS.out ${OpenCV_LIBS} ${Boost_LIBRARIES} mitie caffe gflags)
QT5_USE_MODULES( ISIS.out Widgets PrintSupport)
