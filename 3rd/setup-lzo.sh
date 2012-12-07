#!/bin/sh

echo '+-------------------+'
echo '| X     ZZZZZ  OOO  |'
echo '| X        Z  O   O |'
echo '| X       Z   O   O |'
echo '| X      Z    O   O |'
echo '| XXXXX ZZZZZ  OOO  |'
echo '+-------------------+'
echo
if [ -d "lzo" ]; then
	echo "A directory already exists called 'lzo'.  Do you want to newly install"
	echo "lzo in place of the current lzo directly? (y/n)"
	echo
	read Y
	echo
	if [ "$Y" != "y" ]; then
		echo "Leaving the current 'lzo' directly in place and aborting download and"
		echo "new installation of lzo."
		exit
	fi
fi
echo
echo 'The Primary Source Code is distributed under the permissive Apache 2.0'
echo 'license.'
echo
echo '(hit enter to continue)'
read
echo
echo 'This script downloads and installs LZO source code from'
echo 'http://www.oberhumer.com/opensource/lzo/download/lzo-2.06.tar.gz .'
echo
echo '(hit enter to continue)'
read
echo
echo 'At the time that this script was written, the LZO source code was'
echo 'licensed under GNU General Public License version 2 or later.'
echo
echo '(hit enter to continue)'
read
echo
echo 'The GNU General Public Licenses have been incompatible with Apache 2.0'
echo 'according to http://www.apache.org/licenses/GPL-compatibility.html .'
echo
echo '(hit enter to continue)'
read
echo
echo 'In essence, the Primary Source Code has been distributed to you under'
echo 'the Apache 2.0 license, but once you download and incorporate the LZO'
echo 'source code, YOU ARE CHANGING THE LICENSING TERMS OF THE OVERALL'
echo 'SOURCE CODE (the Primary Source Code in conjunction with the LZO source'
echo 'code).'
echo
echo '(hit enter to continue)'
read
echo
echo 'Please view the following resources to determine your rights.'
echo '   * http://www.oberhumer.com/opensource/gpl.html'
echo '   * http://www.apache.org/licenses/LICENSE-2.0'
echo
echo '(hit enter to continue)'
read
echo
echo 'Would you like to download the LZO source code and unpack it by executing'
echo 'the following commands? (y/n)'
echo
echo 'rm -fr lzo'
echo 'curl http://www.oberhumer.com/opensource/lzo/download/lzo-2.06.tar.gz > lzo-2.06.tar.gz'
echo 'tar xvfz lzo-2.06.tar.gz'
echo 'mv lzo-2.06 lzo'
echo
read Y
echo
if [ "$Y" != "y" ] && [ "$Y" != "Y" ] && [ "$Y" != "yes" ] && [ "$Y" != "YES" ] && [ "$Y" != "Yes" ]; then
	echo 'Aborting.'
	echo
	exit
fi
rm -fr lzo
curl http://www.oberhumer.com/opensource/lzo/download/lzo-2.06.tar.gz > lzo-2.06.tar.gz
tar xvfz lzo-2.06.tar.gz
mv lzo-2.06 lzo
echo
echo "Please see the appropriate documentation in the 'lzo' directory for"
echo "licensing information for the LZO source code."
echo
echo '(hit enter to end)'
read
echo
