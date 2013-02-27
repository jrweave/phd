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

sub setsoln {
	$soln = $s;
	@solncounts = @counts;
}

open CNF, "<$ARGV[0]";
while (<CNF>) {
	if ($_ =~ m/^c\s+(\d+)\s+(.*)$/) {
		$dict{$1} = $2;
	}
}
close CNF;

@solncounts = (0, 0, 0, 0);
$soln = undef;
while (<STDIN>) {
	if ($_ =~ m/^\s*Solution \d+: (.*)$/) {
		$s = $1;
		@nums = split(/\s+/, $s);
		@mapped = map { ($_-1) & 3 } @nums;
		@counts = (0, 0, 0, 0);
		foreach $i (@mapped) {
			++$counts[$i];
		}
		if (!defined($soln)) {
			&setsoln();
		} elsif ($counts[0] < $solncounts[0]) {
			&setsoln();
		} elsif ($counts[0] == $solncounts[0]) {
			if ($counts[1] < $solncounts[1]) {
				&setsoln();
			} elsif ($counts[1] == $solncounts[1]) {
				if ($counts[2] > $solncounts[2]) {
					&setsoln();
				} else {
					print STDERR "[WARNING] Arbitrarily breaking tie.\n";
					&setsoln();
				}
			}
		}
	} elsif ($_ =~ m/^c|UNSAT/) {
		print STDERR $_;
	}
}

@nums = split(/\s+/, $soln);
foreach $n (@nums) {
	print $dict{$n} . "\n";
}
