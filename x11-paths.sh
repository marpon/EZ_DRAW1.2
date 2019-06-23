#! /bin/bash
#
# x11-paths.sh - Edouard.Thiel@lif.univ-mrs.fr - 03/04/2014
#
# This program finds the X11 paths to set in the Makefile
#
# This program is free software under the terms of the
# GNU Lesser General Public License (LGPL) version 2.1.

# Directories to search - order matters
DIRS_H="/Developer /usr /opt"
DIRS_L="/Developer /usr /opt"

# Compiler
CC="gcc"
CFLAGS="-Wall -W -std=c99 -pedantic -O2 -g"
LIBS="-lX11 -lXext"

# Temporary files
TMP1="/tmp/x11-paths-list1-$$.txt"
TMP2="/tmp/x11-paths-list2-$$.txt"
TMP3="/tmp/x11-paths-list3-$$.txt"

clean_files ()
{
    test -f "$TMP1" && rm -f "$TMP1"
    test -f "$TMP2" && rm -f "$TMP2"
    test -f "$TMP3" && rm -f "$TMP3"
}

trap clean_files 0 1 2 3 15

indent ()
{
    while read -r line ; do
        echo "  $line"
    done
}

is_macos ()
{
    [[ $(uname) = *Darwin* ]]
}

sort_tab ()
{
    >| "$TMP1"
    >| "$TMP2"
    for h in "${tab[@]}" ; do
        case "$h" in
           $1*) echo "$h" >> "$TMP1" ;;
           *)   echo "$h" >> "$TMP2" ;;
        esac
    done
    sort -r < "$TMP1" >| "$TMP3"
    tab=()
    while read -r line ; do tab+=($line) ; done < "$TMP3"
    while read -r line ; do tab+=($line) ; done < "$TMP2"
}

echo
echo "Searching for Xlib.h - this may take a while ..."
find $DIRS_H -name "Xlib.h" -print 2> /dev/null | tee "$TMP1" | indent
test -f "$TMP1" || exit 1

tab_h=()
while read -r line ; do
    path=${line%/X11/Xlib.h}
    tab_h+=($path)
done < "$TMP1"

if ((${#tab_h[*]} == 0)); then
    echo "Can't find Xlib.h. Please install the X11 development packages." >&2
    exit 1
fi

echo "Checking for Xdbe.h ..."
tab=()
for h in "${tab_h[@]}" ; do
    path="$h/X11/extensions/Xdbe.h"
    if test -f "$path" ; then
        echo "  $path"
        tab+=($h)
    fi
done
tab_h=("${tab[@]}")

if ((${#tab_h[*]} == 0)); then
    echo "Can't find Xdbe.h. Please install the X11 extension development packages." >&2
    exit 1
fi

if is_macos ; then
    echo "Sorting by version ..."
    tab=("${tab_h[@]}")
    sort_tab "/Developer"
    tab_h=("${tab[@]}")
fi

echo "Trying to compile ez-draw.c ..."
tab=()
for h in "${tab_h[@]}" ; do
    if $CC $CFLAGS -c ez-draw.c -I"$h" 2> /dev/null ; then
        echo "  -I$h : OK"
        tab+=($h)
    else 
        echo "  -I$h : not working"
    fi
    rm -f ez-draw.o 2> /dev/null
done
tab_h=("${tab[@]}")

if ((${#tab_h[*]} == 0)); then
    echo "Could not compile ez-draw.c, sorry." >&2
    exit 1
fi

if is_macos ; then ext="dylib" ; else ext="so" ; fi

echo "Searching for libX11.$ext - this may take a while ..."
find $DIRS_L -name "libX11.$ext" -print 2> /dev/null | tee "$TMP1" | indent
test -f "$TMP1" || exit 1

tab_l=()
while read -r line ; do
    path=${line%/libX11.$ext}
    tab_l+=($path)
done < "$TMP1"

if ((${#tab_l[*]} == 0)); then
    echo "Can't find libX11.$ext. Please install the X11 libraries." >&2
    exit 1
fi

echo "Checking for libXext.$ext ..."
tab=()
for l in "${tab_l[@]}" ; do
    path="$l/libXext.$ext"
    if test -f "$path" ; then
        echo "  $path"
        tab+=($l)
    fi
done
tab_l=("${tab[@]}")

if ((${#tab_l[*]} == 0)); then
    echo "Can't find libXext.$ext. Please install the Xext library." >&2
    exit 1
fi

if is_macos ; then
    echo "Sorting by version ..."
    tab=("${tab_l[@]}")
    sort_tab "/Developer"
    tab_l=("${tab[@]}")
fi

echo "Trying to compile demo-01 ..."
tab1=() ; tab2=() ; n=0
for h in "${tab_h[@]}" ; do
    for l in "${tab_l[@]}" ; do
        if $CC $CFLAGS demo-01.c ez-draw.c -o demo-01 -I"$h" $LIBS -L"$l" 2> /dev/null
        then
            echo "  -I$h -L$l : OK"
            tab1+=($h) ; tab2+=($l) ; ((n++))
        else 
            echo "  -I$h -L$l : not working"
        fi
        rm -f demo-01 2> /dev/null
    done
done

if ((n == 0)); then
    echo "Could not compile demo-01, sorry." >&2
    exit 1
fi

cat << EOF

** Good news! You will be able to compile EZ-Draw. **

Please edit the Makefile, go to "section A", add "-I<path_to_h>" at the end of
the line "CFLAGS =", and add "-L<path_to_lib> at the end of the line "LIBS =".
You should obtain this:

  CFLAGS = $CFLAGS -I${tab1[0]}
  LIBS   = $LIBS -L${tab2[0]}

Next, save the Makefile and type in your terminal: make clean all

EOF

if ((n > 1)); then
    echo "If you get errors or warnings, try another couple of paths,"
    echo "or replace '-std=c99' by '-std=gnu99'"
fi

exit 0

