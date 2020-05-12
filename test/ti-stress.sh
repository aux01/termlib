#!/bin/sh
#/ Usage: test/ti-stress.sh
#/ Loads all terminfo files on system in an attempt to surface bugs.
set -eu

wd="$(cd "$(dirname "$0")" && pwd)"

terminfo_dirs="
    /etc/terminfo
    /usr/lib/terminfo
    /usr/share/terminfo
    /usr/local/share/terminfo
    $HOME/.terminfo
"

# loop through all terminfo files and try to load via the capdump utility
oks=0 fails=0
for f in $(find 2>/dev/null $terminfo_dirs -type f | grep -v README); do
    term="$(basename "$f")"
    if output="$(TERM=$term "$wd/../demo/capdump" 2>&1 1>/dev/null)"; then
        oks=$(( oks+1 ))
    else
        fails=$(( fails+1 ))
        printf "FAILED: %s\n" "$term"
        echo "$output" | sed 's/^/    /'
    fi
done

# report
printf "%d terminfo files loaded: %d ok, %d failed.\n" \
    $((oks+fails)) $oks $fails

# exit non-zero if any failures
test $fails -eq 0
