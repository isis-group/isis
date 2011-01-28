#!/bin/sh

# test first parameter
if [ $# -lt 1 ];then
  echo -e "missing parameter. Abort.\nUsage: astyle ISIS_ROOT_DIR [ISIS_BUILD_DIR]";
  exit -1;
fi

ISIS_ROOT_DIR=$(cd $1 && pwd);

if [ $# -gt 1 ];then
  ISIS_BUILD_DIR=$2;
fi

# check astyle installation
ASTYLE_BIN="/tmp/astyle/build/gcc/bin/astyle"
if [ ! ${ASTYLE_BIN} ];then
  echo -e "Artistic Style binary not found. Abort\nPlease install the 'astyle' package.";
  exit -1;
fi

# header text
echo "ISIS root dir: ${ISIS_ROOT_DIR}"
echo "Artistic Style binary found. Using: ${ASTYLE_BIN}"
echo $(${ASTYLE_BIN} --version)
echo "Converting Project ..."

# run astyle over all cpp,hpp,c,h,txx and cxx files, no backup files are created
if [ -d "$ISIS_BUILD_DIR" ];then
  FILES=$(find $ISIS_ROOT_DIR -type f -regex '.*\.\(cpp\|hpp\|c\|h\|txx\|cxx\)' | grep -v ${ISIS_BUILD_DIR})
else
  FILES=$(find $ISIS_ROOT_DIR -type f -regex '.*\.\(cpp\|hpp\|c\|h\|txx\|cxx\)')
fi

$ASTYLE_BIN --suffix=none --formatted --options=$ISIS_ROOT_DIR/design/astylerc $FILES

#trailer text
echo "... Done"
