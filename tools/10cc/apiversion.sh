#!/bin/sh

API_SPEC=${1:?}
LIB_DIR=${2:?}

. ${API_SPEC}

SONAME=lib${API_NAME:?}.so

if [ "x$API_MAJOR" != "x" ]
then
	ln -sf ${SONAME}.${API_MAJOR} ${LIB_DIR}/${SONAME}
	SONAME=${SONAME}.${API_MAJOR}

	if [ "x$API_MINOR" != "x" ]
	then
		ln -sf ${SONAME}.${API_MINOR} ${LIB_DIR}/${SONAME}
		SONAME=${SONAME}.${API_MINOR}

		if [ "x$API_PATCH" != "x" ]
		then
			ln -sf ${SONAME}.${API_PATCH} ${LIB_DIR}/${SONAME}
			SONAME=${SONAME}.${API_PATCH}
		fi
	fi
fi

echo "SONAME=${SONAME}"
