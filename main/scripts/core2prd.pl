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

$rule = "";
while (<STDIN>) {
	chomp;
	$line = $_;
	while ($line =~ m/^([^\{]*)\{([^\s\}]+)\}(.*)$/) {
		if (!defined($defns{$2})) {
			die "[ERROR] Undefined replacement string: $2 .  Giving up.\n";
		}
		$line = $1 . $defns{$2} . $3;
	}
	if ($line =~ m/^\s*Prefix\s*\(\s*(\w+)\s+<([^\s]+)>\s*\)\s*$/) {
		$prefix{$1} = $2;
	} elsif ($line =~ m/^\s*$/) {
		if ($rule =~ m/^\s*Forall\s*(?:\?\w+\s*)*\(\s*(.*?)\s*\)\s*$/) {
			$impl = $1;
			if ($impl =~ m/^(.*?)\s*:-\s*(.*?)$/) {
				$newrule = "If " . $2 . " Then Do(Assert(" . $1 . "))";
				$newrule =~ s/\b(\d+)\b/"$1"^^xsd:integer/g;
				$newrule =~ s/\b(\w*):([\w-]+)\b/"$prefix{$1}$2"^^<http:\/\/www.w3.org\/2007\/rif#iri>/g;
				$newrule =~ s/\^\^"([^"]*)"\^\^<[^\s]+>/^^<$1>/g;
				$newrule =~ s/\b_(\w+)\(/"$1"^^<http:\/\/www.w3.org\/2007\/rif#local>(/g;
				print $newrule . "\n";
			} else {
				die "[ERROR] Unexpected implication: " . $impl . "\n";
			}
		} elsif ($rule eq "") {
			# ignore
		} elsif ($rule =~ m/^\s*Forall/) {
			die "[ERROR] Cannot parse: " . $rule . "\n";
		} else {
			$newrule = "If And() Then Do(Assert(" . $rule . "))";
			$newrule =~ s/\b(\d+)\b/"$1"^^xsd:integer/g;
			$newrule =~ s/\b(\w*):([\w-]+)\b/"$prefix{$1}$2"^^<http:\/\/www.w3.org\/2007\/rif#iri>/g;
			$newrule =~ s/\^\^"([^"]*)"\^\^<[^\s]+>/^^<$1>/g;
			$newrule =~ s/\b_(\w+)\(/"$1"^^<http:\/\/www.w3.org\/2007\/rif#local>(/g;
			print $newrule . "\n";
		}
		$rule = "";
	} elsif ($line =~ m/^\s*\(\*.*\*\)\s*$/) {
		# ignore
	} elsif ($line =~ m/^\s*#DEFINE\s+([^\s]+)\s+(.*)$/) {
		$defns{$1} = $2;
	} elsif ($line =~ m/^\s*(#PRAGMA\s+.*)$/) {
		$p = $1;
		$p =~ s/\b(\d+)\b/"$1"^^xsd:integer/g;
		$p =~ s/\b(\w*):([\w-]+)\b/"$prefix{$1}$2"^^<http:\/\/www.w3.org\/2007\/rif#iri>/g;
		$p =~ s/\^\^"([^"]*)"\^\^<[^\s]+>/^^<$1>/g;
		$p =~ s/\b_(\w+)\(/"$1"^^<http:\/\/www.w3.org\/2007\/rif#local>(/g;
		print $p . "\n"; # pass on expanded pragma
	} elsif ($line =~ m/^\s*(#.*)$/) {
		print $1 . "\n"; # pass comment through
	} else {
		$line =~ s/\s*(.*)\s*/$1/;
		if ($rule ne "") {
			$rule = $rule . " ";
		}
		$rule = $rule . $line;
	}
}
