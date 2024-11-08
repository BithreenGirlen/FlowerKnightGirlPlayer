cmake_minimum_required (VERSION 3.8)


set(DIR_DXLIB ${CMAKE_CURRENT_LIST_DIR}/dxlib)
set(DIR_SPINE_CPP ${CMAKE_CURRENT_LIST_DIR}/spine-cpp)

set(DIR_TEMP ${CMAKE_CURRENT_BINARY_DIR}/temp)

function(download_and_extract HOST_URL ARCHIVE_NAME)
	if (NOT EXISTS "${DIR_TEMP}/${ARCHIVE_NAME}.zip")
		file(DOWNLOAD "${HOST_URL}" "${DIR_TEMP}/${ARCHIVE_NAME}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${DIR_TEMP}/${ARCHIVE_NAME}")
		file(ARCHIVE_EXTRACT INPUT "${DIR_TEMP}/${ARCHIVE_NAME}.zip" DESTINATION "${DIR_TEMP}")
	endif()
endfunction()

# Fetch DxLib for VC
if (NOT EXISTS "${DIR_DXLIB}")
	set(DXLIB_ARCHIVE_NAME "DxLibVCTest")
	set(DXLIB_VC_URL "https://dxlib.xsrv.jp/temp/${DXLIB_ARCHIVE_NAME}.zip")
	download_and_extract("${DXLIB_VC_URL}" "${DXLIB_ARCHIVE_NAME}")

	file(GLOB DXLIB_LIBS "${DIR_TEMP}/${DXLIB_ARCHIVE_NAME}/*.lib")
	foreach(FILE IN LISTS DXLIB_LIBS)
		if(FILE MATCHES "_vs2015_x64_MDd.lib" OR FILE MATCHES "_vs2015_x64_MD.lib")
			file(COPY ${FILE} DESTINATION "${DIR_DXLIB}")
		endif()
	endforeach()

	file(GLOB INCLUDE_FILES "${DIR_TEMP}/${DXLIB_ARCHIVE_NAME}/*.h")
	foreach(FILE IN LISTS INCLUDE_FILES)
		file(COPY ${FILE} DESTINATION "${DIR_DXLIB}")
	endforeach()
endif()

#Fetch spine-cpp 3.8
if (NOT EXISTS "${DIR_SPINE_CPP}")
	set (SPINE_RUNTIME_URL "https://github.com/EsotericSoftware/spine-runtimes/archive/refs/heads/3.8.zip")
	download_and_extract("${SPINE_RUNTIME_URL}" "spine-runtimes-3.8")

	file(COPY "${DIR_TEMP}/spine-runtimes-3.8/spine-cpp/spine-cpp/src" DESTINATION "${DIR_SPINE_CPP}")
	file(COPY "${DIR_TEMP}/spine-runtimes-3.8/spine-cpp/spine-cpp/include" DESTINATION "${DIR_SPINE_CPP}")
endif()