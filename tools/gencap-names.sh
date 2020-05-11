#!/bin/sh
#/ Usage: gencaps-names.sh [<url>]
#/ Generates terminfo capability name array constants for all terminfo
#/ capabilities using the latest ncurses Caps database published to the
#/ ThomasDickey/ncurses-snapshots repository on GitHub, or a custom URL
#/ if provided.
set -eu

# Show usage
if [ $# -gt 0 ] && [ "$1" = "--help" ]; then
    grep <"$0" '^#/' | cut -c4-
    exit
fi

# URL of raw ncurses Caps file
url="${1:-https://raw.githubusercontent.com/ThomasDickey/ncurses-snapshots/master/include/Caps}"

# Use local Caps file if it exists
if [ $# -eq 0 -a -f "Caps" ]; then
    fetch="cat Caps"
else
    fetch="curl -sS '$url'"
fi

# Fetch file and do some light preformatting
db=$(
    $fetch |
    grep -v -e "^#" |
    awk '{o=$3" "$2" "$1; for(i=8;i<=NF;i++){o=o" "$i}; print o}')


# Reformat lines into literal string array elements
def='{o="\""$3"\",\t//";for(i=5;i<=NF;i++){o=o" "$i};print o}'
format_names() {
    nl -s ' ' -w 1 -v 0 |
        awk '{ print "\t\"" $3 "\"," }'
}

echo "/*"
echo " * Capability names generated with $0 at $(date '+%Y-%m-%dT%H:%M:%S %Z')"
echo " *"
echo " */"
echo
echo "// Boolean capability names"
echo "static const char * const ti_boolnames[] = {"
echo "$db" | grep '^bool ' | format_names | fmt
echo "};"

echo
echo "// Numeric capability names"
echo "static const char * const ti_numnames[] = {"
echo "$db" | grep '^num ' | format_names | fmt
echo "};"

echo
echo "// String capability names"
echo "static const char * const ti_strnames[] = {"
echo "$db" | grep '^str ' | format_names | fmt
echo "};"
