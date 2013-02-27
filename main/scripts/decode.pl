#!/usr/bin/perl

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

open CNF, "<$ARGV[0]";
while (<CNF>) {
	if ($_ =~ m/^c\s+(\d+)\s+(.*)$/) {
		$dict{$1} = $2;
	}
}
close CNF;
while (<STDIN>) {
	if ($_ =~ m/^((?:\s*Solution \d+: )?)(-?\d+(?:\s+-?\d+)*)\s*$/) {
	#if ($_ =~ m/^(\s*Solution \d+: )(.*?)\s*$/) {
		if ($1 ne "") {
			print "===== " . $1 . " =====\n";
		}
		$numline = $2;
		while ($numline =~ m/(-?\d+)/g) {
			print "" . ($1 < 0 ? "NOT " : "    ") . $dict{abs($1)} . "\n";
		}
	}
}
