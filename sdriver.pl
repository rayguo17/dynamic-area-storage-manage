#!/usr/bin/perl
#!/usr/local/bin/perl
use FileHandle;
use Getopt::Std;
use IPC::Open2;

sub usage
{
    printf STDERR "$_[0]\n";
    printf STDERR "Usage: $0 [-hv] -t <trace> -s <shellprog> -a <args>\n";
    printf STDERR "Options:\n";
    printf STDERR "  -h            Print this message\n";
    printf STDERR "  -v            Be more verbose\n";
    printf STDERR "  -t <trace>    Trace file\n";
    printf STDERR "  -p <program>  Program to test\n";
    die "\n" ;
}

getopts('ht:p:a:');
if($opt_h){
    usage();
}
if(!$opt_t){
    usage("Missing required -t argument");
}
if(!$opt_p){
    usage("Missing required -p argument");
}
$verbose = $opt_v;
$infile = $opt_t;
$prog = $opt_p;
$grade = $opt_g;
# Make sure the input script exists and is readable
-e $infile
    or die "$0: ERROR: $infile not found\n";
-r $infile
    or die "$0: ERROR: $infile is not readable\n";

# Make sure the shell program exists and is executable
-e $prog
    or die "$0: ERROR: $prog not found\n";
-x $prog
    or die "$0: ERROR: $prog is not executable\n";

# Open the input script
open INFILE, $infile
    or die "$0: ERROR: Couldn't open input file $infile: $!\n";

$pid = open2(\*Reader, \*Writer, "$prog");
Writer->autoflush();

while(<INFILE>){
    $line = $_;
    chomp($line);

    # Comment line
    if ($line =~ /^#/) {  
	print "$line\n";
    }

    # Blank line
    elsif ($line =~ /^\s*$/) { 
	if ($verbose) {
	    print "$0: Ignoring blank line\n";
	}
    }
    # Close pipe (sends EOF notification to child)
    elsif ($line =~ /CLOSE/) {
	if ($verbose) {
	    print "$0: Closing output end of pipe to child $pid\n";
	}
	close Writer;
    }

    # Wait for child to terminate
    elsif ($line =~ /WAIT/) {
	if ($verbose) {
	    print "$0: Waiting for child $pid\n";
	}
	wait;
	if ($verbose) {
	    print "$0: Child $pid reaped\n";
	}
    }
    else {
	if ($verbose) {
	    print "$0: Sending :$line: to child $pid\n";
	}
	print Writer "$line\n";
    }
}

close Writer;
if ($verbose) {
    print "$0: Reading data from child $pid\n";
}
while ($line = <Reader>) {
    print $line;
}
close Reader;

# Finally, parent reaps child
wait;

if ($verbose) {
    print "$0: Shell terminated\n";
}

exit;