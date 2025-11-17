cmake_minimum_required (VERSION 3.25)
project ("FlowerKnightGirlPlayer-deps")

set(SPINE_VERSION "3.8")
set(SPINE_RUNTIME_LANGUAGE "spine-cpp")
set(SPINE_RUNTIME_DIR ${CMAKE_CURRENT_LIST_DIR}/${SPINE_RUNTIME_LANGUAGE}-${SPINE_VERSION})

set(DXLIB_DIR ${CMAKE_CURRENT_LIST_DIR}/dxlib)

set(AVIR_VERSION "3.1")
set(AVIR_DIR ${CMAKE_CURRENT_LIST_DIR}/avir-${AVIR_VERSION})

set(TEMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/temp)

function(download_and_extract HOST_URL ARCHIVE_NAME)
	if (NOT EXISTS "${TEMP_DIR}/${ARCHIVE_NAME}.zip")
		file(DOWNLOAD "${HOST_URL}" "${TEMP_DIR}/${ARCHIVE_NAME}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${TEMP_DIR}/${ARCHIVE_NAME}")
		file(ARCHIVE_EXTRACT INPUT "${TEMP_DIR}/${ARCHIVE_NAME}.zip" DESTINATION "${TEMP_DIR}")
	endif()
endfunction()

# Download DxLib for VC.
if (NOT EXISTS "${DXLIB_DIR}")
	set (DXLIB_VC_URL "https://dxlib.xsrv.jp/temp/DxLibVCTest.zip")
	set(DXLIB_VC_FOLDER_NAME "DxLibVCTest")
	download_and_extract("${DXLIB_VC_URL}" "${DXLIB_VC_FOLDER_NAME}")

	file(GLOB DXLIB_LIBS "${TEMP_DIR}/${DXLIB_VC_FOLDER_NAME}/*.lib")
	foreach(FILE IN LISTS DXLIB_LIBS)
		if(FILE MATCHES "_vs2015_x64_MDd.lib" OR FILE MATCHES "_vs2015_x64_MD.lib")
			file(COPY ${FILE} DESTINATION "${DXLIB_DIR}")
		endif()
	endforeach()

	file(GLOB INCLUDE_FILES "${TEMP_DIR}/${DXLIB_VC_FOLDER_NAME}/*.h")
	foreach(FILE IN LISTS INCLUDE_FILES)
		file(COPY ${FILE} DESTINATION "${DXLIB_DIR}")
	endforeach()
endif()

# Download Spine runtime
if (NOT EXISTS "${SPINE_RUNTIME_DIR}/lib")
	set (SPINE_RUNTIME_URL "https://github.com/EsotericSoftware/spine-runtimes/archive/refs/heads/${SPINE_VERSION}.zip")
	set (SPINE_FOLDER_NAME "spine-runtimes-${SPINE_VERSION}")
	download_and_extract("${SPINE_RUNTIME_URL}" "${SPINE_FOLDER_NAME}")

	file(COPY "${TEMP_DIR}/${SPINE_FOLDER_NAME}/${SPINE_RUNTIME_LANGUAGE}/${SPINE_RUNTIME_LANGUAGE}/src" DESTINATION "${SPINE_RUNTIME_DIR}")
	file(COPY "${TEMP_DIR}/${SPINE_FOLDER_NAME}/${SPINE_RUNTIME_LANGUAGE}/${SPINE_RUNTIME_LANGUAGE}/include" DESTINATION "${SPINE_RUNTIME_DIR}")
endif()

# Download avir
if (NOT EXISTS "${AVIR_DIR}")
	set(AVIR_URL "https://github.com/avaneev/avir/archive/refs/tags/${AVIR_VERSION}.zip")
	set (AVIR_FOLDER_NAME "avir-${AVIR_VERSION}")
	download_and_extract("${AVIR_URL}" "${AVIR_FOLDER_NAME}")

	# Not interested in binary files
	file(GLOB INCLUDE_FILES "${TEMP_DIR}/${AVIR_FOLDER_NAME}/*.h")
	foreach(FILE IN LISTS INCLUDE_FILES)
		file(COPY ${FILE} DESTINATION "${AVIR_DIR}")
	endforeach()
endif()

# Delete Spine runtimes for other languages than C++,
# and DxLib static libraries for other compilers than MSVC.
file(REMOVE_RECURSE "${TEMP_DIR}")

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>,$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>>")
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/d1trimfile:${CMAKE_SOURCE_DIR}\\>")
endif()

# Build Spine runtime as static library
add_subdirectory ("${SPINE_RUNTIME_LANGUAGE}-${SPINE_VERSION}")
