#!/bin/sh
# Builds libyottaslack.so. cgo needs the Sailfish SDK's compiler and sysroot, so
# that the result doesn't depend on a newer glibc than the device has.
#
# yottaslack.pri runs this during the app build, inside the build target:
#
#   ./build.sh <QT_ARCH> <absolute output path>
#
# That compiles with the target's gcc if go is installed there, otherwise it
# copies prebuilds/<QT_ARCH>/libyottaslack.so.
#
# To fill prebuilds/ from the host instead, for the architectures whose C
# compiler is set (compiler flags such as --sysroot go in the same variable):
#
#   CC_ARM=... CC_ARM64=... CC_I386=... ./build.sh
set -eu

cd "$(dirname "$0")"

compile() {
    arch=$1
    compiler=$2
    output=$3

    case "$arch" in
        arm) target="GOARCH=arm GOARM=7" ;;
        arm64) target="GOARCH=arm64" ;;
        i386) target="GOARCH=386 GO386=softfloat" ;;
        *)
            echo "yottaslack: unknown architecture $arch" >&2
            exit 1
            ;;
    esac

    mkdir -p "$(dirname "$output")"
    # shellcheck disable=SC2086 # target holds several assignments
    env CGO_ENABLED=1 GOOS=linux GOTOOLCHAIN=local CC="$compiler" $target \
        go build -v -buildmode=c-shared -trimpath -buildvcs=false -ldflags="-s -w" \
        -o "$output" .
    rm -f "${output%.so}.h"
    echo "yottaslack: built $output"
}

if [ $# -eq 2 ]; then
    prebuilt="prebuilds/$1/libyottaslack.so"

    # Sailfish's go isn't on PATH
    for dir in /usr/lib/golang-*/bin; do
        if [ -x "$dir/go" ]; then
            PATH="$PATH:$dir"
        fi
    done

    if command -v go >/dev/null 2>&1; then
        # Under the target's qemu, go's garbage collector crashes
        GOGC=off GOMAXPROCS=1 GOFLAGS=-p=1
        export GOGC GOMAXPROCS GOFLAGS
        compile "$1" "${CC:-gcc}" "$2"
    elif [ -f "$prebuilt" ]; then
        mkdir -p "$(dirname "$2")"
        cp "$prebuilt" "$2"
        echo "yottaslack: go not found, using $prebuilt"
    else
        echo "yottaslack: go not found and there is no $prebuilt." >&2
        echo "Install go in the build target, or fill prebuilds/, see README.md." >&2
        exit 1
    fi
    exit 0
fi

build_prebuilt() {
    if [ -z "$2" ]; then
        echo "yottaslack: skipping $1, no compiler set"
        return
    fi

    compile "$1" "$2" "prebuilds/$1/libyottaslack.so"
}

build_prebuilt arm "${CC_ARM:-}"
build_prebuilt arm64 "${CC_ARM64:-}"
build_prebuilt i386 "${CC_I386:-}"
