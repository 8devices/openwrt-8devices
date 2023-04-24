#!/usr/bin/perl

if ($#ARGV != 0) {
	die "Usage: bin2c <name>";
}

# Slurp all the input
undef $/;

# Create an array of unsigned chars from input
@chars = unpack "C*", <STDIN>;

print "unsigned char $ARGV[0]\[\] = {\n  ";

foreach $char (@chars) {
	printf "0x%02x", $char;
	last if $i == $#chars;
	print ((++$i % 13) ? ", " : ",\n  ");
}

print "};\n";
