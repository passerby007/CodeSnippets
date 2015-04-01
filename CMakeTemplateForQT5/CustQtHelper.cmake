# ======================================
#   original Qt5 Macros for CMake suck
# ======================================
#
# all Qt generated files ( ui.h, moc.cpp ) are located in the ${CUST_QT5_GENERATED_FILE_DIR}
# user cmake file should manually include_directories(${CUST_QT5_GENERATED_FILE_DIR})
#

include(CMakeParseArguments)

set(_gen_file_root_name QtGenerated)
set(CUST_QT5_GENERATED_FILE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${_gen_file_root_name})
file(MAKE_DIRECTORY ${CUST_QT5_GENERATED_FILE_DIR})

#
#   QUI wrapper
#
function(CUST_QT5_WRAP_UI outfiles )
    set(options)
    set(oneValueArgs)
    set(multiValueArgs OPTIONS)

    cmake_parse_arguments(_WRAP_UI "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(ui_files ${_WRAP_UI_UNPARSED_ARGUMENTS})
    set(ui_options ${_WRAP_UI_OPTIONS})

    foreach(it ${ui_files})
        get_filename_component(outfile ${it} NAME_WE)
        get_filename_component(infile ${it} ABSOLUTE)
        set(outfile "${CUST_QT5_GENERATED_FILE_DIR}/ui_${outfile}.h")
        add_custom_command(OUTPUT ${outfile}
          COMMAND ${Qt5Widgets_UIC_EXECUTABLE}
          ARGS ${ui_options} -o ${outfile} ${infile}
          MAIN_DEPENDENCY ${infile} VERBATIM)
        list(APPEND ${outfiles} ${outfile})
    endforeach()
    set(${outfiles} ${${outfiles}} PARENT_SCOPE)
endfunction()

#
#   MOC wrapper
#
macro(CUST_QT5_MAKE_MOC_OUTPUT baseroot infile outfile)
    # setup moc.cpp file path relative to the CustGeneratedDir
    # ensure the relative path exists
	file(RELATIVE_PATH ret ${baseroot} ${infile})
	set(_outfile "${CUST_QT5_GENERATED_FILE_DIR}/${ret}")
    get_filename_component(outpath ${_outfile} PATH)
    get_filename_component(_outfile ${_outfile} NAME_WE)
    file(MAKE_DIRECTORY ${outpath})
	#set_property(DIRECTORY ${outpath} APPEND PROPERTY EXCLUDE_FROM_ALL)
    set(${outfile} ${outpath}/moc_${_outfile}.cpp)
endmacro()

# helper macro to set up a moc rule
macro(CUST_QT5_CREATE_MOC_COMMAND infile outfile moc_flags moc_options moc_target)
    get_filename_component(_moc_outfile_name "${outfile}" NAME)
    get_filename_component(_moc_outfile_dir "${outfile}" PATH)
    if(_moc_outfile_dir)
        set(_moc_working_dir WORKING_DIRECTORY ${_moc_outfile_dir})
    endif()
    set (_moc_parameters_file ${outfile}.param)
    set (_moc_parameters ${moc_flags} ${moc_options} -o "${outfile}" "${infile}")

   string (REPLACE ";" "\n" _moc_parameters "${_moc_parameters}")

    file(WRITE ${_moc_parameters_file} "${_moc_parameters}\n")

    set(_moc_extra_parameters_file @${_moc_parameters_file})

    add_custom_command(OUTPUT ${outfile}
                       COMMAND ${Qt5Core_MOC_EXECUTABLE} ${_moc_extra_parameters_file}
					   MAIN_DEPENDENCY ${infile}
                       DEPENDS ${infile}
                       ${_moc_working_dir}
                       VERBATIM)
endmacro()

macro(CUST_QT5_GET_MOC_FLAGS _moc_flags)
    set(${_moc_flags})

    ## Tips:
    ##  'get_directory_property' retrieves specific parameter,
    ##  and store results in a user-provided variable.
    ##  it's NOT restricted to 'directory' stuff
    get_directory_property(_inc_DIRS INCLUDE_DIRECTORIES)

    if(CMAKE_INCLUDE_CURRENT_DIR)
        list(APPEND _inc_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    foreach(_current ${_inc_DIRS})
        if("${_current}" MATCHES "\\.framework/?$") # works on MacOS
            string(REGEX REPLACE "/[^/]+\\.framework" "" framework_path "${_current}")
            set(${_moc_flags} ${${_moc_flags}} "-F${framework_path}")
        else()  # for other platforms (linux/win)
            set(${_moc_flags} ${${_moc_flags}} "-I${_current}")
        endif()
    endforeach()

    get_directory_property(_defines COMPILE_DEFINITIONS)
    foreach(_current ${_defines})
        set(${_moc_flags} ${${_moc_flags}} "-D${_current}")
    endforeach()

    if(WIN32)
        set(${_moc_flags} ${${_moc_flags}} -DWIN32)
    endif()
endmacro()

# the 'baseroot' param is to help understand the hierarchy of source directories
# so the corresponding moc*.cpp files will be generated and placed
# in the ${CUST_QT5_GENERATED_DIR} with the same hierarchy.
function(CUST_QT5_WRAP_CPP baseroot outfiles)
    # get include dirs, and other compile options
    cust_qt5_get_moc_flags(moc_flags)

    set(options)
    set(oneValueArgs TARGET)
    set(multiValueArgs OPTIONS)

    cmake_parse_arguments(_WRAP_CPP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(moc_files ${_WRAP_CPP_UNPARSED_ARGUMENTS})
    set(moc_options ${_WRAP_CPP_OPTIONS})
    set(moc_target ${_WRAP_CPP_TARGET})

    if (moc_target AND CMAKE_VERSION VERSION_LESS 2.8.12)
        message(FATAL_ERROR "The TARGET parameter to CUST_QT5_WRAP_CPP is only available when using CMake 2.8.12 or later.")
    endif()
    foreach(it ${moc_files})
        get_filename_component(it ${it} ABSOLUTE)
        cust_qt5_make_moc_output(${baseroot} ${it} outfile)
        cust_qt5_create_moc_command(${it} ${outfile} "${moc_flags}" "${moc_options}" "${moc_target}")
        list(APPEND ${outfiles} ${outfile})
    endforeach()
    set(${outfiles} ${${outfiles}} PARENT_SCOPE)
endfunction()

#
#   QRC wrapper
#
function(CUST_QT5_WRAP_RC outfiles )

    set(options)
    set(oneValueArgs)
    set(multiValueArgs OPTIONS)

    cmake_parse_arguments(_RCC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(rcc_files ${_RCC_UNPARSED_ARGUMENTS})
    set(rcc_options ${_RCC_OPTIONS})

    foreach(it ${rcc_files})
        get_filename_component(outfilename ${it} NAME_WE)
        get_filename_component(infile ${it} ABSOLUTE)
        get_filename_component(rc_path ${infile} PATH)
        set(outfile "${CUST_QT5_GENERATED_FILE_DIR}/qrc_${outfilename}.cpp")

        #set(_RC_DEPENDS)
        #if(EXISTS "${infile}")
        #    #  parse file for dependencies
        #    #  all files are absolute paths or relative to the location of the qrc file
        #    file(READ "${infile}" _RC_FILE_CONTENTS)
        #    string(REGEX MATCHALL "<file[^<]+" _RC_FILES "${_RC_FILE_CONTENTS}")
        #    foreach(_RC_FILE ${_RC_FILES})
        #        string(REGEX REPLACE "^<file[^>]*>" "" _RC_FILE "${_RC_FILE}")
        #        if(NOT IS_ABSOLUTE "${_RC_FILE}")
        #            set(_RC_FILE "${rc_path}/${_RC_FILE}")
        #        endif()
        #        set(_RC_DEPENDS ${_RC_DEPENDS} "${_RC_FILE}")
        #    endforeach()
        #    # Since this cmake macro is doing the dependency scanning for these files,
        #    # let's make a configured file and add it as a dependency so cmake is run
        #    # again when dependencies need to be recomputed.
        #    qt5_make_output_file("${infile}" "" "qrc.depends" out_depends)
        #    configure_file("${infile}" "${out_depends}" COPY_ONLY)
        #else()
        #    # The .qrc file does not exist (yet). Let's add a dependency and hope
        #    # that it will be generated later
        #    set(out_depends)
        #endif()

        add_custom_command(OUTPUT ${outfile}
                           COMMAND ${Qt5Core_RCC_EXECUTABLE}
                           ARGS ${rcc_options} -name ${outfilename} -o ${outfile} ${infile}
                           MAIN_DEPENDENCY ${infile}
                           DEPENDS ${_RC_DEPENDS} "${out_depends}" VERBATIM)
        list(APPEND ${outfiles} ${outfile})
    endforeach()
    set(${outfiles} ${${outfiles}} PARENT_SCOPE)
endfunction()
