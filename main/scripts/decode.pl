#!/usr/bin/perl

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
