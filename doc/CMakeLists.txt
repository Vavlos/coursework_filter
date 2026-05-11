#================================================================================
cmake_minimum_required(VERSION 3.20)
message(CHECK_START "\tDoxygen documentation")

set (DOXYGEN_GENERATE_RTF YES)

find_package(Doxygen)

if(NOT Doxygen_FOUND)
    message(WARNING "Doxygen not found. Targets for building documentation are not available")
    return()
endif()


#================================================================================
if (DOXYGEN_FOUND)
    set( DOXYGEN_OUTPUT_DIRECTORY doxygen )
    set( DOXYGEN_OUTPUT_LANGUAGE russian )
    set( DOXYGEN_COLLABORATION_GRAPH YES )
    set( DOXYGEN_EXTRACT_ALL YES )
    set( DOXYGEN_CLASS_DIAGRAMS YES )
    set( DOXYGEN_HIDE_UNDOC_RELATIONS NO )
    set( DOXYGEN_HAVE_DOT YES )
    set( DOXYGEN_CLASS_GRAPH YES )
    set( DOXYGEN_CALL_GRAPH YES )
    set( DOXYGEN_CALLER_GRAPH YES )
    set( DOXYGEN_COLLABORATION_GRAPH YES )
    set( DOXYGEN_BUILTIN_STL_SUPPORT YES )
    set( DOXYGEN_EXTRACT_PRIVATE YES )
    set( DOXYGEN_EXTRACT_PACKAGE YES )
    set( DOXYGEN_EXTRACT_STATIC YES )
    set( DOXYGEN_EXTRACT_LOCALMETHODS YES )
    set( DOXYGEN_UML_LOOK YES )
    set( DOXYGEN_UML_LIMIT_NUM_FIELDS 50 )
    set( DOXYGEN_TEMPLATE_RELATIONS YES )
    set( DOXYGEN_DOT_GRAPH_MAX_NODES 100 )
    set( DOXYGEN_MAX_DOT_GRAPH_DEPTH 0 )
    set( DOXYGEN_DOT_TRANSPARENT YES )

#    Пока неясно, как использовать README в документации
#    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/README.md")

    doxygen_add_docs(${PROJECT_NAME}_docs
        ${PROJECT_SOURCE_DIR}/src
        # ALL # ALL - включить в цели, собираемые по умолчанию. Для документации обычно не требуется
        COMMENT "Generate html docs"
    )
 else()
     message( "Doxygen need to be installed to generate the doxygen documentation" )
 endif (DOXYGEN_FOUND)


message(CHECK_PASS "Done")
