# $Header$
#
# To be run with perl.  This builds Makefile from Makefile.in,
# substituting things whose location varies on different systems, and
# has to be looked for.
#
# There is no #! at the top because perl isn't installed in the same
# place on all systems.
#
# COPYRIGHT


sub check_wordlists {
    my ($dir) = @_;
    if ((-r $dir."/gismu") && (-r $dir."/lujvo-list") && (-r $dir."/cmavo") && (-r $dir."/oblique.key") && (-r $dir."/rafsi")) {
        print "Found word lists in directory $dir, using that.\n";
        return 1;
    } else {
        return 0;
    }
}


# Try to find the directory containing the files 'gismu', 'cmavo', 'oblique.key' etc
if (&check_wordlists(".")) {
    $word_list_dir = ".";
} else {
    # Try all directories below the parent directory
    print "Searching to find directory containing wordlists ...\n";
    @dirs = qx=find .. -type d -print=;
    for $dir (@dirs) {
        chop $dir;
        if (&check_wordlists($dir)) {
            $word_list_dir = $dir;
            last;
        }
    }

    # Otherwise, try everything below the user's home directory
    unless ($word_list_dir) {
        @dirs = qx=find ~ -type d -print=;
        for $dir (@dirs) {
            chop $dir;
            if (&check_wordlists($dir)) {
                $word_list_dir = $dir;
                last;
            }
        }
    }   

    unless ($word_list_dir) {
        die "Can't find word lists gismu, cmavo etc in your directory structure.\n";
    }
}

$prefix="/usr/local";
$install="ginstall";
$debug=0;

while ($_ = shift @ARGV) {
    if (/^--prefix=(.*)$/) {
        $prefix = $1;
    } elsif (/^-p/) {
        $prefix = shift @ARGV;
    } elsif (/^--installprog=(.*)$/) {
        $install = $1;
    } elsif (/^--debug$/) {
		$debug = 1;
	}
}

$optdebug = $debug ? "-g" : "-O2";

open(IN, "<Makefile.in");
open(OUT, ">Makefile");
while (<IN>) {
    s/\@\@WORD-LISTS\@\@/$word_list_dir/eg;
    s/\@\@PREFIX\@\@/$prefix/eg;
    s/\@\@INSTALLPROG\@\@/$install/eg;
	s/\@\@OPTDEBUG\@\@/$optdebug/eg;
    print OUT;
}
close(IN);
close(OUT);

