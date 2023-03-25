#!/bin/bash
#
# removes trailing whitespace from source code files
# from @r-barnes
#

find . -iname "*.c" -type f -print0 | xargs -0 perl -pi -e 's/ +$//'
find . -iname "*.cpp" -type f -print0 | xargs -0 perl -pi -e 's/ +$//'
find . -iname "*.h" -type f -print0 | xargs -0 perl -pi -e 's/ +$//'
find . -iname "*.hpp" -type f -print0 | xargs -0 perl -pi -e 's/ +$//'

