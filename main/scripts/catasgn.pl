#!/usr/bin/perl

$go = 0;
while (<STDIN>) {
	if ($_ =~ /Solution (\d+)/) {
		if ($go == 1) {
			last;
		} elsif ($1 eq $ARGV[0]) {
			print $_;
			$go = 1;
		}
	} elsif ($go == 1) {
		print $_;
	}
}
