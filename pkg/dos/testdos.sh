#!/bin/sh
# Prereq: dosbox unzip zip
: ${WATFTP=https://openwatcom.org/ftp/install}
: ${WATZIP=open-watcom-c-dos-1.9.exe}
: ${HOMECACHE=$HOME/.cache}
set -eux
d=$(dirname $0)
if test ! -f "$d/dosbox.conf"; then
	echo "$0: cannot find dosbox.conf in \"$d\"" >&2
	exit 1
fi
if test ! -f "$d/../../Makefile"; then
	echo "$0: cannot find ../../Makefile from \"$d\"" >&2
	exit 2
fi
td=$(mktemp -d)
cp "$d/dosbox.conf" "$td"/
cd "$d"/../..
jd=$(pwd)
make zip
if test ! -f .version; then
	echo "$0: cannot find .version in \"$jd\"" >&2
	exit 3
fi
ver=$(sed 's,\.,,g' .version)
jsrc=jove${ver}s.zip
if test ! -f "$jsrc"; then
	echo "$0: cannot find $jsrc in \"$jd\"" >&2
	exit 3
fi
wzip="$HOMECACHE/$WATZIP"
if test ! -d $HOMECACHE; then
	mkdir $HOMECACHE
fi
if test ! -f "$wzip"; then
	wget -o "$wzip"  "$WATFTP/$WATZIP"
fi
cp "$jsrc" "$td"/
cd "$td"
unzip "$jsrc"
mkdir watcom tmp "jove${ver}d"
cd watcom
unzip "$wzip"
cd ..
cat << EOF > build.bat
cd jove${ver}s
wmake /f makefile.wat
copy jjove.exe ..\jove${ver}d\jove.exe
copy recover.exe readme readme.dos ..\jove${ver}d
md ..\jove${ver}d\doc
copy doc\*.* ..\jove${ver}d\doc
EOF
echo "Starting dosbox in $td"
dosbox .
if test ! -d "$jd/DIST"; then mkdir "$jd/DIST"; fi
zip -r -q -k "$jd/DIST/jove500d.zip" jove500d
echo "If everything worked, please"
echo "    rm -rf $td"
