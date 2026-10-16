#!/bin/bash
#
# DMX Total Control - the tests of the application (build/ already built):
# the groups (zone) and the backend. The engine suites inherited from
# upstream are not run here: 8 of them only pass on Linux (paths, fonts,
# a test plugin).
#
# Licensed under the Apache License, Version 2.0

SRCDIR="$(cd "$(dirname "$0")/.." && pwd)"
export PATH="$SRCDIR/build/engine/src:$SRCDIR/build/engine/audio/src:$PATH"
export QT_QPA_PLATFORM=offscreen
cd "$SRCDIR/build/dtc/test" || exit 1

result=0
for suite in dtc_zone_test dtc_backend_test; do
    echo "::group::$suite"
    ./$suite.exe -maxwarnings 0 -o "$suite.txt,txt"
    code=$?
    grep -aE "^(FAIL|XPASS)|Loc: |Totals" "$suite.txt"
    echo "::endgroup::"
    if [ $code -ne 0 ]; then
        # one annotation with the failures, readable without the logs
        echo "::error title=$suite::$(grep -aE '^FAIL|Loc: ' "$suite.txt" | head -20 | tr '\n' ' ')"
        result=1
    fi
done
exit $result
