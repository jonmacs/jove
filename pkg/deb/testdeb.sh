#!/bin/sh
# Mark Moraes, 20260804

# Target architecture passed from the GitHub workflow (defaults to host arch)
BUILD_ARCH="$(dpkg --print-architecture)"
UBUNAME=noble

set -eux

case "$#" in
1) tar="$1";;
2) tar="$1"; TARGET_ARCH="$2";;
*) echo "Usage: $0 TARBALL [TARGET_ARCH] where TARBALL can be https://github.com/jonmacs/jove/archive/refs/tags/VERSION.tar.gz or .../jove-VERSION.tgz and TARGET_ARCH is one of amd64, i386, arm64 or armhf (default is host arch)" >&2; exit 1;;
esac

: ${TARGET_ARCH="$(dpkg --print-architecture)"}
case "$TARGET_ARCH" in
amd64|i386) urlsub=archive; urlbase="/ubuntu";;
arm64|armhf) urlsub=ports; urlbase="";;
*) echo "$0: unknown TARGET_ARCH \"$TARGET_ARCH\"" >&2; exit 1;;
esac


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

if ! grep -s ${ver} "$sdir"/debian/changelog; then
        (echo "$0: prepend something like this to $sdir/debian/changelog and 'make tgz' and re-run";
	cat <<- EOF
		jove (${ver}-1) unstable; urgency=low
		  * New upstream release.

		 -- $DEBJOVEDEV  $(date +'%a, %d %b %Y %H:%M:%S %z')
		EOF
	) >&2
	exit 1
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
tar -C "$odir" -xf "$tar"

cd "$odir"
mv jove-${ver}/pkg/deb/debian jove-${ver}

echo "==> Setting up an isolated APT sandbox directory"
mkdir -p var/apt-cache-lists/partial var/apt-cache-files/archives/partial var/apt-logs

cat << EOF > "$odir/var/apt-sandbox.conf"
Dir::Etc::SourceList "$odir/var/sandbox_sources.list";
Dir::Etc::SourceParts "/dev/null";
Dir::State::Lists "$odir/var/apt-cache-lists/";
Dir::Cache "$odir/var/apt-cache-files/";
Dir::Log "$odir/var/apt-logs/";
APT::Sandbox::User "root";
EOF

# 2. Write out a custom multi-architecture source list matching your target mirror requirements.
# This explicitly allows amd64, i386, arm64, and armhf to coexist cleanly.

cat << EOF > "$odir/var/sandbox_sources.list"
deb [arch=${TARGET_ARCH}] http://$urlsub.ubuntu.com$urlbase $UBUNAME main universe restricted multiverse
deb [arch=${TARGET_ARCH}] http://$urlsub.ubuntu.com$urlbase $UBUNAME-updates main universe restricted multiverse
deb [arch=${TARGET_ARCH}] http://$urlsub.ubuntu.com$urlbase $UBUNAME-security main universe restricted multiverse
deb-src [arch=${TARGET_ARCH}] http://$urlsub.ubuntu.com$urlbase $UBUNAME main universe restricted multiverse
EOF

export APT_CONFIG="$odir/var/apt-sandbox.conf"

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
Build-Depends: debhelper, po-debconf, libncurses-dev:any, groff, pkgconf
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
) | gpg --no-random-seed-file --clearsign > "$odir/jove-${ver}/debian/jove_${ver}-1.dsc"


echo "==> Configuring environment for target architecture: ${TARGET_ARCH}"
$SUDO apt-get update

# Install general Debian packaging toolchains
$SUDO apt-get install -y devscripts equivs build-essential

if [ "${TARGET_ARCH}" != "${BUILD_ARCH}" ]; then
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
    
fi

echo "==> Generating local build-deps package meta-structure"
mk-build-deps -a "${TARGET_ARCH}" jove-${ver}/debian/control

# Grab the freshly created file name dynamically
DEPS_PKG=$(ls jove-build-deps_*.deb 2>/dev/null || ls *-build-deps_*.deb)

echo "==> Installing package dependencies via local sandbox path..."
# Let standard APT parse and fulfill the dependencies tracked inside the .deb archive
$SUDO apt-get install -y --no-install-recommends "./${DEPS_PKG}"

cd jove-${ver}
echo "==> Compiling"
dpkg-buildpackage -a"${TARGET_ARCH}" -b -us -uc

echo "==> Organizing build artifacts"
: ${DISTDIR="$sdir/../../DIST"}
mkdir -p "$DISTDIR/${TARGET_ARCH}"
mv ../*.deb ../*.changes ../*.buildinfo "$DISTDIR/${TARGET_ARCH}/"
cd "$sdir"
rm -rf "$odir"

echo "==> Build complete for ${TARGET_ARCH}"
