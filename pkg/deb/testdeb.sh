#!/bin/sh
# Prereq: apt install pbuilder debian-keyring debian-archive-keyring
# Mark Moraes, 20230108
#
: ${DEBVER=buster}
: ${DEBARCHS="amd64 armhf arm64"}
set -eux
case "$#" in
1) tar="$1";;
*) echo "Usage: gendsc TARBALL where TARBALL can be https://github.com/jonmacs/jove/archive/refs/tags/VERSION.tar.gz or .../jove-VERSION.tgz" >&2; exit 1;;
esac
if test ! -r debian/changelog; then
	echo "$0: must run in a parent directory of debian/" >&2
	exit 1
fi
# fragile regexp for ver
ver=$(expr "$tar" : ".*/.*[-/]\([0-9.]*\)\..*gz")
case "$ver" in
[0-9]*)	echo "$0: version $ver tarball $tar";;
*)	echo "$0: failure to extract version from tarball name" >&2
	exit 1
	;;
esac
if ! grep -s ${ver} debian/changelog; then
	cat - debian/changelog <<- EOF > debian/changelog.new
		jove (${ver}-1) unstable; urgency=low
		  * New upstream release.

		 -- Cord Beermann <cord@debian.org>  $(date +'%a, %d %b %Y %H:%M:%S %z')
		EOF
	rm debian/changelog && mv debian/changelog.new debian/changelog
fi
odir=$(mktemp -d)
tar -C "$odir" -xvf gpg-testhome.tar
GNUPGHOME="$odir/gpg"
export GNUPGHOME
test -e $odir/gpg/secring.gpg
tar -Jcf "$odir/jove_${ver}-1.debian.tar.xz" debian
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
while read hdr cmd; do
	echo "$hdr"
	for f in "$odir/jove_${ver}.orig.tar.gz" "$odir/jove_${ver}-1.debian.tar.xz"; do
		sz=$(stat --format=%s "$f")
		echo '' $("$cmd" < "$f" | awk '{print $1}') "$sz" $(basename "$f")
	done
done << EOF
Checksums-Sha1: sha1sum
Checksums-Sha256: sha256sum
Files: md5sum
EOF
) | gpg --no-random-seed-file --clearsign > "$odir/jove_${ver}-1.dsc"
: ${SUDO=}
case "$(stat --format=%u $ofile)" in
0)	;;
*)	: ${SUDO=sudo};;
esac

for arch in $DEBARCHS; do
	echo "Packaging for $DEBVER $arch"
	base="/var/cache/pbuilder/$DEBVER-$arch-base.tgz"
	if test ! -f "$base"; then
		pbuilder create --distribution "$DEBVER" --host-arch "$arch" --mirror http://ftp.us.debian.org/debian/ --basetgz "$base" debootstrapopts "--keyring=/usr/share/keyrings/debian-archive-keyring.gpg"
	fi
	pbuilder build --host-arch "$arch"  --basetgz "$base" "$odir/jove_${ver}-1.dsc"
done
rm -r "$odir"
