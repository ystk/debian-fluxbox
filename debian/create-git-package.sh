#!/bin/bash

git_url=git://git.fluxbox.org/fluxbox.git
cpwd=`pwd`

current_version=`dpkg-parsechangelog|grep ^Version|awk '{print $2}'|sed 's/-[[:digit:]]\+$//'|sed 's/+git[[:digit:]]\+//'`

# version="$current_version+git`date +%Y%m%d`"
# directory="fluxbox-$version"
# orig_tgz="fluxbox_$version.orig.tar.gz"



if ! which git &>/dev/null; then
  echo "Package git-core not installed"
  exit -1
fi

if ! which autoconf &>/dev/null; then
  echo "Package autoconf not installed"
  exit -2
fi

fakeroot debian/rules clean
rm -fr fluxbox-git
git clone $git_url fluxbox-git
cd fluxbox-git

if ! test -z "$1"; then
  git reset --hard $1
fi

git_revision=`git whatchanged|head -n 1|awk '{print $2}'`
git_date=`git whatchanged|head -n 4|grep ^Date|awk '{print $6, $3, $4}'|perl -pi -e 'my %mns=(Jan=>"01",Feb=>"02",Mar=>"03",Apr=>"04",May=>"05",Jun=>"06",Jul=>"07",Aug=>"08",Sep=>"09",Oct=>10,Nov=>11,Dec=>12); s/([a-z]{3})/ $mns{$1} /egi; s/\s(\d)$/ 0$1/; s/\s+//g'`
./autogen.sh
cd $cpwd
find fluxbox-git -type d -name '.git*' -exec rm -fr '{}' +

version="$current_version+git$git_date"
directory="fluxbox-$version"
orig_tgz="fluxbox_$version.orig.tar.gz"

rm -fr $directory
mkdir $directory
tar -cjf $directory/fluxbox-git.tar.bz2 fluxbox-git
rm -fr fluxbox-git

cp -ra debian $directory
perl -pi -e 's/^DEB_TAR_SRCDIR.*/DEB_TAR_SRCDIR = fluxbox-git/' \
  $directory/debian/rules

if test -d $directory/debian/patches; then
  rm -fr $directory/debian/patches
  perl -pi -e 's/^(include.*dpatch\.mk)/#$1/' $directory/debian/rules
fi

dch --newversion "$version-1" \
  --distribution experimental \
  --changelog $directory/debian/changelog \
  "GIT snapshot $git_revision."

tar --exclude=debian -czvf ../$orig_tgz $directory
rm -fr ../$directory
mv $directory ..
cd ..
dpkg-source -b $directory
cd $cpwd

echo
echo
echo "Use:"
echo "sudo apt-get build-dep fluxbox"
echo "cd ../$directory"
echo "fakeroot debian/rules binary"
