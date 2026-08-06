#!/bin/sh
# Mark Moraes, 20260804

BUILD_ARCH="$(dpkg --print-architecture)"
: ${UBUNAME=noble}

set -eux

TARGET_ARCH="$BUILD_ARCH"
tar="-"
case $# in
0) ;;
1) TARGET_ARCH="$1";;
2) TARGET_ARCH="$1"; tar="$2";;
*) echo "Usage: $0 [TARGET_ARCH [TARBALL]] where TARBALL can be https://github.com/jonmacs/jove/archive/refs/tags/VERSION.tar.gz or .../jove-VERSION.tgz and TARGET_ARCH is one of amd64, i386, arm64 or armhf (default is host arch)" >&2; exit 1;;
esac

sdir=$(realpath -e $(dirname $0))
if test ! -r "$sdir"/debian/changelog; then
	echo "$0: could not find $sdir/debian/changelog" >&2
	exit 1
fi
jdir=$(realpath -e "$sdir/../..")
if test ! -r "$jdir"/Makefile; then
	echo "$0: could not find $jdir/Makefile" >&2
	exit 1
fi
odir=$(mktemp -d)
: ${SUDO="sudo"}
ouid=$(stat --format=%u $odir)
case "$ouid" in
0)	SUDO=;;
esac
echo "==> UID=$ouid SUDO=\"$SUDO\" ODIR=\"$odir\""

echo "==> Configuring environment for target architecture: ${TARGET_ARCH}"
$SUDO apt-get update

# Install general Debian packaging toolchains
$SUDO apt-get install -y gpg groff curl devscripts equivs build-essential debhelper fakeroot

# fragile regexp for ver
case "$tar" in
-)	(cd "$jdir" && make .version)
	verfile="$jdir"/.version
	if test ! -r "$verfile"; then
		echo "$0: could not find $verfile" >&2
		exit 1
	fi
	read ver < "$verfile"
	ofile="$odir/jove_${ver}.orig.tar.gz"
	(cd "$jdir" && JOVETAR="$ofile" make tgz)
	if test ! -r "$ofile"; then
		echo "$0: could not find $ofile ; maybe make tgz in $jdir failed" >&2
		exit 1
	fi
	;;
*)	ver=$(expr "$tar" : ".*/.*[-/]\([0-9.]*\)\..*gz")
	case "$ver" in
	[0-9]*)	echo "$0: version $ver tarball $tar";;
	*)	echo "$0: failure to extract version from tarball name" >&2
		exit 1
		;;
	esac
	ofile="$odir/jove_${ver}.orig.tar.gz"
	case "$tar" in
	http:*|https:*|ftp:*)
		curl -s -S -f -o "$ofile" "$tar"
		;;
	*)	tar=$(realpath -e "$tar")
		if test ! -r "$tar"; then
			echo "$0: tarball $tar does not exist" >&2
			exit 1;
		fi
		cp "$tar" "$ofile"
		;;
	esac
esac

cd "$odir"
tar -xf "$ofile"
mv jove-"$ver"/pkg/deb/debian jove-"$ver"/

if ! grep -s "$ver" jove-"$ver"/debian/changelog; then
        (echo "$0: prepend something like this to debian/changelog and 'make tgz' and re-run";
	cat <<- EOF
		jove (${ver}-1) unstable; urgency=low
		  * New upstream release.

		 -- $DEBJOVEDEV  $(date +'%a, %d %b %Y %H:%M:%S %z')
		EOF
	) >&2
	exit 1
fi

tar -xvf jove-"$ver"/pkg/deb/gpg-testhome.tar
GNUPGHOME="$odir/gpg"
export GNUPGHOME

# generate dsc file and sign with gpg
(
cat << EOF
Format: 3.0 (quilt)
Binary: jove
Version: ${ver}-1
Package-List:
 jove deb editors optional arch=any
EOF

sed -i "s/:any/:${TARGET_ARCH}/g" jove-"$ver"/debian/control
egrep '^(Source|Architecture|Build-Depends|Standards-Version|Homepage|Maintainer):' jove-"$ver"/debian/control

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
) | gpg --no-random-seed-file --clearsign > "jove-${ver}/debian/jove_${ver}-1.dsc"


if [ "${TARGET_ARCH}" != "${BUILD_ARCH}" ]; then
	echo "==> Fetching cross-compiler for ${TARGET_ARCH}"
	$SUDO apt-get install -y crossbuild-essential-${TARGET_ARCH}
	echo "==> Generating cross build-deps package meta-structure"
	case "$TARGET_ARCH" in
	i386|x86_64) ;;
	*)      # Ugh. Ubuntu (24.04 at least) container does not have ports in
		# apt sources.list and does not specify Architectures, fix that.
		aptdir=/etc/apt/sources.list.d/
		origfile=$aptdir/ubuntu.sources
		newfile=$aptdir/ubuntu-ports.sources
		if test ! -r "$newfile"; then
			awk '
			BEGIN { RS=""; ORS="\n\n" }
			{
				if ($0 ~ /(^|\n)Types: deb(\n|$)/ && $0 !~ /(^|\n)Architectures:/)
					$0 = $0 "\nArchitectures: amd64 i386"
				print
			}' "$origfile" > "$odir"/$$
			cat <<- EOF >> "$odir"/$$
			# Added by $0
			Types: deb
			URIs: http://ports.ubuntu.com/ubuntu-ports
			Suites: $UBUNAME $UBUNAME-updates $UBUNAME-backports $UBUNAME-security
			Components: main universe restricted multiverse
			Architectures: arm64 armhf ppc64 riscv64 s390x
			Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg
			EOF
			# might briefly be two files but better than no files if we rm first
			mv "$odir"/$$ "$newfile" && rm "$origfile"
		fi
		;;
	esac
	$SUDO dpkg --add-architecture ${TARGET_ARCH}
	$SUDO apt-get update
	mk-build-deps --host-arch="${TARGET_ARCH}" jove-"$ver"/debian/control
else
	mk-build-deps jove-"$ver"/debian/control
fi

# Grab the freshly created file name dynamically
DEPS_PKG=$(ls jove-build-deps_*.deb 2>/dev/null || ls *-build-deps_*.deb)

echo "==> Installing package dependencies"
# Let standard APT parse and fulfill the dependencies tracked inside the .deb archive
$SUDO apt-get install -y --no-install-recommends "./${DEPS_PKG}"

cd jove-"$ver"
echo "==> Compiling"
dpkg-buildpackage -a"${TARGET_ARCH}" -b -us -uc

echo "==> Organizing build artifacts"
: ${DISTDIR="$jdir/DIST"}
mkdir -p "$DISTDIR/${TARGET_ARCH}"
mv ../*.deb ../*.changes ../*.buildinfo "$DISTDIR/${TARGET_ARCH}/"
cd "$jdir"
rm -rf "$odir"

echo "==> Build complete for ${TARGET_ARCH}"
