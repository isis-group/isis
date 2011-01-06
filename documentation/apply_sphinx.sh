#!/bin/sh

ISIS_ROOT_DIR=$(cd $1 && pwd);

if [ $# -gt 1 ];then
	ISIS_BUILD_DIR=$2;
fi

VERSION=$3

SPHINX_BUILD=$(which sphinx-build)
if [ ! ${SPHINX_BUILD} ];then
	echo -e "sphinx-build binary not found. Abort.\nPlease install python-sphinx package.";
	exit -1;
fi

echo "Building documentation from $ISIS_ROOT_DIR/documentation for isis version $VERSION"

$SPHINX_BUILD -b html -D release=$VERSION -D version=$VERSION $ISIS_ROOT_DIR/documentation $ISIS_BUILD_DIR/documentation

echo "Done."
