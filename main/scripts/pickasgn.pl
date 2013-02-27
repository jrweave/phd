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
