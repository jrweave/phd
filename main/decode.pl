#!/usr/bin/perl

open CNF, "<$ARGV[0]";
while (<CNF>) {
	if ($_ =~ m/^c\s+(\d+)\s+(.*)$/) {
		$dict{$1} = $2;
	}
}
close CNF;
while (<STDIN>) {
	if ($_ =~ m/^\s*Solution (\d+): (.*)$/) {
		$has_problem = 0;
		$solnum = $1;
		print "=====Solution$solnum =====\n";
		@nums = split(/\s+/, $2);
		foreach $num (@nums) {
			$str = $dict{$num} . "\n";
			if ($str =~ m/^PROBLEM/) {
				$has_problem = 1;
			}
			print $str;
		}
		if ($has_problem == 0) {
			push(@noprobs, $solnum);
		}
	}
}
#print "SOLUTIONS WITHOUT PROBLEMS: @noprobs\n";
