#!/usr/bin/perl

sub pickbest {
	if ($last[0] == 0) {
	  @last = @cur;
	} elsif ($last[1] != $cur[1]) {
		if ($cur[1] < $last[1]) {
			@last = @cur;
		}
	} elsif ($last[2] != $cur[2]) {
		if ($cur[2] < $last[2]) {
			@last = @cur;
		}
	} elsif ($last[3] != $cur[3]) {
		if ($cur[3] > $last[3]) {
			@last = @cur;
		}
	} else {
		print STDERR "[TIE] " . $last[0] . " " . $cur[0] . "\n";
	}
}

@last = (0, 0, 0, 0);
@cur = (0, 0, 0, 0);
while (<STDIN>) {
	if ($_ =~ m/^=====\s*Solution\s*(\d+):?\s*=====$/) {
		if ($cur[0] != 0) {
			&pickbest();
		}
		@cur = ($1, 0, 0, 0);
	} elsif ($_ =~ m/ABANDON/) {
		++$cur[1];
	} elsif ($_ =~ m/REPLICATE/) {
		++$cur[2];
	} elsif ($_ =~ m/ARBITRARY/) {
		++$cur[3];
	}
}
&pickbest();
print "@last\n";
