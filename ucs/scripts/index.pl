#!/usr/bin/perl

$CODE_VALUE = 0;
$CHAR_NAME = 1;
$GEN_CATEGORY = 2;
$CANON_COMB_CLASS = 3;
$BIDIR_CATEGORY = 4;
$CHAR_DECOM_MAPPING = 5;
$DEC_DIGIT_VALUE = 6;
$DIGIT_VALUE = 7;
$NUMERIC_VALUE = 8;
$MIRRORED = 9;
$UNICODE_1_0_NAME = 10;
$COMMENT_10646 = 11;
$UC_MAPPING = 12;
$LC_MAPPING = 13;
$TC_MAPPING = 14;

$first_bound = 0;
$last_codepoint = 0;
$ranges[0] = 0;
$line_count = 0;
$range_count = 1;
$index[0] = 0;
while (<STDIN>) {
	@chardata = split(/;/, $_);
	$codepoint = hex($chardata[$CODE_VALUE]);
	if ($chardata[$CHAR_NAME] ne "" || $chardata[$UNICODE_1_0_NAME] ne "") {
		if ($chardata[$CHAR_NAME] =~ m/<(.*), Last>$/) {
			for ($last_codepoint++; $last_codepoint < $codepoint; $last_codepoint++) {
				push(@data, [ split(/;/, sprintf("%X;<$1>;;;;;;;;;;;;;;", $last_codepoint)) ]);
				$line_count++;
			}
		}
		$names[$codepoint] = $chardata[$CHAR_NAME] . ";" . $chardata[$UNICODE_1_0_NAME];
	} else {
		$names[$codepoint] = "";
	}
	if ($codepoint > $last_codepoint + 1) {
		if ($first_bound == 0) {
			$first_bound = $last_codepoint + 1;
		}
		push(@ranges, $last_codepoint + 1, $codepoint);
		push(@index, $line_count);
		$range_count++;
	}
	push(@data, [ @chardata ]);
	$last_codepoint = $codepoint;
	$line_count++;
}
push(@ranges, $last_codepoint + 1);
$range_count++;

print "#ifndef UINT32_C(c)\n";
print "#define UINT32_C(c) ((unsigned int) c)\n";
print "#endif /* UINT32_C */\n";
print "\n";

print "#define UCS_NUM_CHARS UINT32_C($line_count)\n";
print "#define UCS_RANGE_INDEX_LEN UINT32_C($range_count)\n";
print "#define UCS_RANGES_LEN (UCS_RANGE_INDEX_LEN<<1)\n";
print "#define UCS_FIRST_BOUND UINT32_C($first_bound)\n";
print "\n";

print "struct ucs_char_data {\n";
print "\tconst uint32_t codepoint;\n";
print "\tconst uint32_t combining_class;\n";
print "\tconst uint32_t decomplen;\n";
print "\tconst uint32_t *decomposition;\n";
print "};\n";
print "\n";

print "const uint32_t UCS_RANGES[UCS_RANGES_LEN] = {\n";
foreach (@ranges) {
	print sprintf("\tUINT32_C(0x%X),", $_);
	if ($names[$_] ne "") {
		print "    // " . $names[$_];
	}
	print "\n";
}
print "};\n";
print "\n";

print "const uint32_t UCS_RANGE_INDEX[UCS_RANGE_INDEX_LEN] = {\n";
foreach (@index) {
	print "\tUINT32_C($_),\n";
}
print "};\n";
print "\n";

print "const uint32_t UCS_CHAR_EMPTY_DECOMP[0] = {};\n";
foreach (@data) {
	@d = @{$_};
	if ($d[$CHAR_DECOM_MAPPING] ne "") {
		@decomp = split(/ /, $d[$CHAR_DECOM_MAPPING]);
		if ($decomp[0] =~ m/^</) {
			shift @decomp;
		}
		if (scalar(@decomp) != 0) {
			print "const uint32_t UCS_CHAR_" . $d[$CODEPOINT] . "_DECOMP[" . scalar(@decomp) . "] = { ";
			foreach $c (@decomp) {
				print "UINT32_C(0x$c), ";
			}
			print "};\n";
		} else {
			print "#define UCS_CHAR_" . $d[$CODEPOINT] . "_DECOMP UCS_CHAR_EMPTY_DECOMP\n";
		}
	} else {
		print "#define UCS_CHAR_" . $d[$CODEPOINT] . "_DECOMP UCS_CHAR_EMPTY_DECOMP\n";
	}
}

print "const ucs_char_data UCS_CHAR_DATA[UCS_NUM_CHARS] = {\n";
foreach (@data) {
	@d = @{$_};
	print "\t{\n";

	# codepoint
	print sprintf("\t\tUINT32_C(0x%s),", $d[$CODEPOINT]);
	if ($names[hex($d[$CODEPOINT])] ne "") {
		print " // " . $names[hex($d[$CODEPOINT])];
	}
	print "\n";

	# combining_class
	if ($d[$CANON_COMB_CLASS] eq "") {
		print "\t\tUINT32_C(0),\n";
	} else {
		print "\t\tUINT32_C(" . $d[$CANON_COMB_CLASS] . "),\n";
	}

	# decomplen
	if ($d[$CHAR_DECOM_MAPPING] eq "") {
		print "\t\tUINT32_C(0),\n";
	} else {
		@decomp = split(/ /, $d[$CHAR_DECOM_MAPPING]);
		if ($decomp[0] =~ m/^</) {
			shift @decomp;
		}
		print "\t\tUINT32_C(" . scalar(@decomp) . "),\n";
	}

	# decomposition
	print "\t\tUCS_CHAR_" . $d[$CODEPOINT] . "_DECOMP,\n";
	print "\t},\n";
}
print "};\n";
print "\n";
