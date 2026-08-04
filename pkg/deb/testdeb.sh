#!/bin/sh
# Mark Moraes, 20260804

# Target architecture passed from the GitHub workflow (defaults to host arch)
HOST_ARCH="$(dpkg --print-architecture)"
UBUNAME=noble

set -eux

case "$#" in
1) tar="$1";;
2) tar="$1"; TARGET_ARCH="$2";;
*) echo "Usage: $0 TARBALL [TARGET_ARCH] where TARBALL can be https://github.com/jonmacs/jove/archive/refs/tags/VERSION.tar.gz or .../jove-VERSION.tgz" >&2; exit 1;;
esac

: ${TARGET_ARCH="$(dpkg --print-architecture)"}

# fragile regexp for ver
tar=$(realpath -e "$tar")
ver=$(expr "$tar" : ".*/.*[-/]\([0-9.]*\)\..*gz")
case "$ver" in
[0-9]*)	echo "$0: version $ver tarball $tar";;
*)	echo "$0: failure to extract version from tarball name" >&2
	exit 1
	;;
esac

sdir="$(realpath -e $(dirname $0))"
if test ! -r "$sdir"/debian/changelog; then
	echo "$0: could not find $sdir/debian/changelog" >&2
	exit 1
fi

# one thing we write into srcdir, which means there is a race condition
# if more than one simultaneous incantation of this script runs in same tree
if ! grep -s ${ver} "$sdir"/debian/changelog; then
	echo "==> Updating $sdir/debian/changelog"
	cat - "$sdir"/debian/changelog <<- EOF > "$sdir"/debian/changelog.new
		jove (${ver}-1) unstable; urgency=low
		  * New upstream release.

		 -- $DEBJOVEDEV  $(date +'%a, %d %b %Y %H:%M:%S %z')
		EOF
	rm debian/changelog && mv debian/changelog.new debian/changelog
fi

odir=$(mktemp -d)
: ${SUDO="sudo --preserve-env=APT_CONFIG"}
ouid=$(stat --format=%u $odir)
case "$ouid" in
0)	SUDO=;;
esac
echo "==> UID=$ouid SUDO=\"$SUDO\" ODIR=\"$odir\""

tar -C "$odir" -xvf "$sdir"/gpg-testhome.tar
GNUPGHOME="$odir/gpg"
export GNUPGHOME

ofile="$odir/jove_${ver}.orig.tar.gz"
case "$tar" in
http:*|https:*|ftp:*)
	wget -O "$ofile" "$tar"
	;;
*)	if test ! -r "$tar"; then
		echo "$0: tarball $tar does not exist" >&2
		exit 1;
	fi
	cp -av "$tar" "$ofile"
	;;
esac

cd "$odir"
echo "==> Setting up an isolated APT sandbox directory"
mkdir -p pkg/debian/apt-cache-lists/partial pkg/debian/apt-cache-files/archives/partial
cat << EOF > "$odir/pkg/debian/apt-sandbox.conf"
Dir::Etc::SourceList "$odir/pkg/debian/sandbox_sources.list";
Dir::Etc::SourceParts "/dev/null";
Dir::State::Lists "$odir/pkg/debian/apt-cache-lists/";
Dir::Cache "$odir/pkg/debian/apt-cache-files/";
APT::Sandbox::User "root";
EOF

# 2. Write out a custom multi-architecture source list matching your target mirror requirements.
# This explicitly allows amd64, i386, arm64, and armhf to coexist cleanly.
cat << EOF > "$odir/pkg/debian/sandbox_sources.list"
deb [arch=amd64,i386] http://archive.ubuntu.com/ubuntu $UBUNAME main universe restricted multiverse
deb [arch=amd64,i386] http://archive.ubuntu.com/ubuntu $UBUNAME-updates main universe restricted multiverse
deb [arch=amd64,i386] http://archive.ubuntu.com/ubuntu $UBUNAME-security main universe restricted multiverse

deb [arch=arm64,armhf] http://ports.ubuntu.com $UBUNAME main universe restricted multiverse
deb [arch=arm64,armhf] http://ports.ubuntu.com $UBUNAME-updates main universe restricted multiverse
deb [arch=arm64,armhf] http://ports.ubuntu.com $UBUNAME-security main universe restricted multiverse
EOF

export APT_CONFIG="$odir/pkg/debian/apt-sandbox.conf"

# generate dsc file and sign with gpg
(
cat << EOF
Format: 3.0 (quilt)
Source: jove
Binary: jove
Architecture: any
Version: ${ver}-1
Maintainer: Cord Beermann <cord@debian.org>
Homepage: https://github.com/jonmacs/jove
Standards-Version: 4.6.1
Build-Depends: debhelper, po-debconf, libncurses-dev, groff, pkgconf
Package-List:
 jove deb editors optional arch=any
EOF

# generate checksums for dsc file
while read hdr cmd; do
	echo "$hdr"
	for f in "jove_${ver}.orig.tar.gz" "jove_${ver}-1.debian.tar.xz"; do
		if test -e $f; then
			sz=$(stat --format=%s "$f")
			echo '' $("$cmd" < "$f" | awk '{print $1}') "$sz" "$f"
		fi
	done
done << EOF
Checksums-Sha1: sha1sum
Checksums-Sha256: sha256sum
Files: md5sum
EOF
) | gpg --no-random-seed-file --clearsign > "$odir/jove_${ver}-1.dsc"


echo "==> Configuring environment for target architecture: ${TARGET_ARCH}"
$SUDO apt-get update

# Install general Debian packaging toolchains
$SUDO apt-get install -y devscripts equivs build-essential

if [ "${TARGET_ARCH}" != "${HOST_ARCH}" ]; then
    echo "==> Enabling multiarch and cross-compiler for ${TARGET_ARCH}"
    $SUDO dpkg --add-architecture "${TARGET_ARCH}"
    $SUDO apt-get update
    
    if [ "${TARGET_ARCH}" = "i386" ]; then
        # i386 runs natively on x86_64 but needs the 32-bit gcc toolchain
        $SUDO apt-get install -y gcc-multilib
    elif [ "${TARGET_ARCH}" = "arm64" ]; then
        $SUDO apt-get install -y gcc-aarch64-linux-gnu
    elif [ "${TARGET_ARCH}" = "armhf" ]; then
        $SUDO apt-get install -y gcc-arm-linux-gnueabihf
    fi
    
    echo "==> Installing cross-build dependencies"
    # mk-build-deps creates a dummy package satisfying debian/control dependencies for the target architecture
    $SUDO mk-build-deps -a "${TARGET_ARCH}" -i -r -t "apt-get -y --no-install-recommends" debian/control
    
    echo "==> Building cross-compiled Debian package"
    # -a specifies target architecture, -Pcross,nocheck bypasses running tests meant for the wrong CPU
    dpkg-buildpackage -a"${TARGET_ARCH}" -Pcross,nocheck -b -us -uc
else
    echo "==> Installing native build dependencies"
    $SUDO mk-build-deps -i -r -t "apt-get -y --no-install-recommends" debian/control
    
    echo "==> Building native Debian package"
    dpkg-buildpackage -b -us -uc
fi

echo "==> Organizing build artifacts"
mkdir -p "$sdir/DIST/${TARGET_ARCH}"
mv ../*.deb ../*.changes ../*.buildinfo "$sdir/DIST/${TARGET_ARCH}/"
rm -rf "$odir"

echo "==> Build complete for ${TARGET_ARCH}"
