#!/bin/sh

# Copyright 2012 Jesse Weaver
# 
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
# 
#        http://www.apache.org/licenses/LICENSE-2.0
# 
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
#    implied. See the License for the specific language governing
#    permissions and limitations under the License.

echo '+-------------------+'
echo '| X     XXXXX  XXX  |'
echo '| X        X  X   X |'
echo '| X       X   X   X |'
echo '| X      X    X   X |'
echo '| XXXXX XXXXX  XXX  |'
echo '+-------------------+'
echo
if [ -d "lzo" ]; then
	echo "A directory already exists called 'lzo'."
	echo "Leaving the current 'lzo' directly in place."
	exit
fi
echo
echo 'The Primary Source Code is distributed under the permissive Apache 2.0'
echo 'license.'
echo
echo 'This script downloads and installs LZO source code from'
echo 'http://www.oberhumer.com/opensource/lzo/download/lzo-2.06.tar.gz .'
echo
echo 'At the time that this script was written, the LZO source code was'
echo 'licensed under GNU General Public License version 2 or later.'
echo
echo 'The GNU General Public Licenses have been incompatible with Apache 2.0'
echo 'according to http://www.apache.org/licenses/GPL-compatibility.html .'
echo
echo 'In essence, the Primary Source Code has been distributed to you under'
echo 'the Apache 2.0 license, but once you download and incorporate the LZO'
echo 'source code, YOU ARE CHANGING THE LICENSING TERMS OF THE OVERALL'
echo 'SOURCE CODE (the Primary Source Code in conjunction with the LZO source'
echo 'code).'
echo
echo 'Please view the following resources to determine your rights.'
echo '   * http://www.oberhumer.com/opensource/gpl.html'
echo '   * http://www.apache.org/licenses/LICENSE-2.0'
echo
echo '(hit enter to continue)'
read Y
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
if [ "$Y" != "y" ]; then
	echo "NOT getting the LZO source code.  We're done here."
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
read Y
echo
