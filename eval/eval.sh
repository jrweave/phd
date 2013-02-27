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

if [ $# -gt 1 ]; then
	echo "USAGE: $0 [optional-host-name]"
	exit 1
fi

if [ $# -gt 0 ]; then
	host=$1
else
	host=`hostname`
fi

case "$host" in
	q.ccni.rpi.edu)				script="eval-q.sh"
												;;
	mastiff.cs.rpi.edu)		script="eval-mastiff.sh"
												;;
	*)										echo "Unrecognized hostname: $host" 1>&2
												exit 2
												;;
esac

echo "$script"
echo "Do it? (y/n)"
read Y
if [ "$Y" != "y" ]; then
	echo "Aborted (you didn't enter 'y')." 1>&2
	exit 3
fi
./$script
