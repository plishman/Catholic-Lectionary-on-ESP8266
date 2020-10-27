#!/usr/bin/perl -s

# note the use of -s for switch processing. Under NT/2000, you will need to
# call this script explicitly with -s (i.e., perl -s script) if you do not
# have perl file associations in place. 
# -s is also considered 'retro', many programmers prefer to load
# a separate module (from the Getopt:: family) for switch parsing.

use utf8;
use Cwd; # module for finding the current working directory
use List::Util 1.33 'any';
use List::Util 'first';  
use File::Basename qw( fileparse );
use File::Path qw( make_path );
use File::Spec;
use Time::Piece;
use Time::Seconds;
use List::MoreUtils 'true';

$t = localtime;

$inputrootdir = "H:/divinum-officium/divinum-officium-master/web/output";
$inputrootdir_escaped = qr /H:\/divinum-officium\/divinum-officium-master\/web\/output/;
$outputrootdir = "H:/output_consolidated";

$dirs_processed = 0;

# This subroutine takes the name of a directory and recursively scans 
# down the filesystem from that point looking for files named "core"
sub ScanDirectory{
    my ($workdir) = shift; 
    my ($outputdirroot) = shift;

    my ($startdir) = &cwd; # keep track of where we began

    chdir($workdir) or die "Unable to enter dir $workdir:$!\n";
    opendir(DIR, ".") or die "Unable to open $workdir:$!\n";
    my @names = readdir(DIR) or die "Unable to read $workdir:$!\n";
    closedir(DIR);

    foreach my $name (@names){
        next if ($name eq "."); 
        next if ($name eq "..");

        if (-d $name){                  # is this a directory?
            &ScanDirectory($name, $outputdirroot);
            $dirs_processed++;
            next;
        }

        #if ($name eq "core") {          # is this a file named "core"?
        #    # if -r specified on command line, actually delete the file
        #    if (defined $r){
	    #         unlink($name) or die "Unable to delete $name:$!\n";
        #    }
        #    else {
        #        print "found one in $workdir!\n"; 
        #    }
        #}
        
        #chdir($startdir) or die "Unable to change to dir $startdir:$!\n";
    }

    my $fnn = 0;
    my $hnn = 0;
    my $match_found = 0;
    my $outputindexfilename = "index.txt";
    my $outputdatafilename = "propers.txt";
    my $relative_input_dir = getcwd();
    #my $relative_input_dir_regex = qr $inputrootdir;
    $relative_input_dir =~ s/$inputrootdir_escaped//;
    my $outputdir = $outputdirroot . "/" . $relative_input_dir . "/";
    $outputdir =~ s/\/\/\//\//g;
    $outputdir =~ s/\/\//\//g;

    checkdir($outputdir);

    my $oifh;
    my $odfh;

    my $match_found = 1;
    my $outputfilesopen = 0;

    my $output_fileptr = 0;

    my $recordcount;
    my $subrecordcount;

    my $filename_count_re = qr /^\d{1,2}\_0\_.{2}\.txt/;
    my $recordcount = true { /$filename_count_re/ } @names;    
    my $outputindexcsvline = "";

    my $heading_missing = 0;

    while ($match_found) {
        my $filename_re = qr /^$fnn\_$hnn\_.{2}\.txt/;
        #$match_found = any { /$filename_re/ } @names;
        my $filename = first { /$filename_re/ } @names;

        if ($filename) {
            $match_found = 1;
        }
        else {
            $match_found = 0;
            if ($fnn == 3) {
                $fnn++;
                $match_found = 1;
                next;   # try 4 if 3 not found, since with Tridentine 1570 I skipped 3, where the lesson is in the other versions, so that they align
            }

            if ($fnn == 0 && $hnn == 0) {
                # Good Friday has no "Introit" heading, so starts at 0_1 rather than 0_0 (no heading)
                # Check if the 0_1 filename exists, to be sure not in a subdir without any records (should be in all valid records)
                my $filename_test_re = qr /^0\_1\_.{2}\.txt/;
                my $filename_test = first { /$filename_test_re/ } @names;

                if ($filename_test) {
                    $match_found = 1;
                    $heading_missing = 1;
                }
            }
        }

        if (!$match_found) {
            last;
        }
        
        my $filename_subpartcount_re;
        my $subrecordcount;

        if ($match_found) {
            if (!$outputfilesopen) {               
                open($oifh, '+>>:encoding(UTF-8)', $outputdir . $outputindexfilename) or die "Could not open output index file '$outputdir$outputindexfilename' $!";
                open($odfh, '+>>:encoding(UTF-8)', $outputdir . $outputdatafilename) or die "Could not open output data file '$outputdir$outputdatafilename' $!";
                $outputfilesopen = 1;

                my $head = "";
                my $name = "";
                my $comm = "";
                my $rank = "";
                my $colour = "";
                
                if (open(my $h2fh, '<:encoding(UTF-8)', "head.txt")) {
                    $head = <$h2fh>;
                    chomp($head);
                    $head =~ s/^\s+|\s+$//g; # trim leading and trailing spaces from heading
                    close $h2fh;
                }

                if (open(my $nfh, '<:encoding(UTF-8)', "name.txt")) {
                    $name = <$nfh>;
                    chomp($name);
                    $name =~ s/^\s+|\s+$//g; # trim leading and trailing spaces from heading
                    close $nfh;
                }

                if (open(my $c2fh, '<:encoding(UTF-8)', "comm.txt")) {
                    $comm = <$c2fh>;
                    chomp($comm);
                    $comm =~ s/^\s+|\s+$//g; # trim leading and trailing spaces from heading
                    close $c2fh;
                }

                if (open(my $rfh, '<:encoding(UTF-8)', "rank.txt")) {
                    $rank = <$rfh>;
                    chomp($rank);
                    $rank =~ s/^\s+|\s+$//g; # trim leading and trailing spaces from heading
                    close $rfh;
                }

                if (open(my $cfh, '<:encoding(UTF-8)', "col.txt")) {
                    $colour = <$cfh>;
                    chomp($colour);
                    $colour =~ s/^\s+|\s+$//g; # trim leading and trailing spaces from heading
                    close $cfh;
                }

                $head =~ s/\"/\"\"/g;
                $name =~ s/\"/\"\"/g;
                $comm =~ s/\"/\"\"/g;
                $rank =~ s/\"/\"\"/g;
                $colour =~ s/\"/\"\"/g;

                $outputindexcsvline = "\"" . $head . "\",\"" . $name . "\",\"" . $comm . "\",\"" . $rank . "\",\"" . $colour . "\"\n";
                print {$oifh} $outputindexcsvline;
            }
            
            my $heading = " ";

            if ($heading_missing == 0) {
                open(my $hfh, '<:encoding(UTF-8)', $filename) or die "Could not open header file '$filename' $!";
                $heading = <$hfh>;
                chomp($heading);
                $heading =~ s/^\s+|\s+$//g; # trim leading and trailing spaces from heading
                $heading =~  s/\"/\"\"/g;   # escape quotes for csv (needed when FONT COLOR="red" appears in the heading, eg on Good Friday)
                close $hfh;
            }

            my $datafile_found = 1;

            $filename_subpartcount_re = qr /^$fnn\_[1-9]\d{0,1}\_.{2}\.txt/;    # count subparts for this heading
            $subrecordcount = true { /$filename_subpartcount_re/ } @names;    

            while ($datafile_found) {
                $hnn++;

                my $datafilename_re = qr /^$fnn\_$hnn\_.{2}\.txt/;
                my $datafilename = first { /$datafilename_re/ } @names;

                if ($datafilename) {
                    $datafile_found = 1;
                }
                else {
                    $datafile_found = 0;
                }

                if (!$datafile_found) {
                    last;
                }

                open(my $idfh, '<:encoding(UTF-8)', $datafilename) or die "Could not open propers file '$datafilename' $!";

                while ( my $line = <$idfh> ) {
                    print {$odfh} $line;
                }

                close $idfh;

                my $subpartnum = $hnn - 1;

                $outputindexcsvline = "\"" . $heading . "\",\"" . $fnn . "\",\"" . $recordcount . "\",\"" . $subpartnum . "\",\"" . $subrecordcount . "\",\"" . $output_fileptr . "\",\"" . tell($odfh) . "\"\n";
                print {$oifh} $outputindexcsvline;
                $output_fileptr = tell($odfh);
            }
            
            $fnn++;
            $hnn = 0;
            $heading_missing = 0;
        }
    } 

    if ($outputfilesopen) {
        close $oifh;
        close $odfh;
    }
    
    chdir($startdir) or die "Unable to change to dir $startdir:$!\n";

    my $t2 = localtime;
    my $s = $t2 - $t;
    if ($s->seconds > 10) {
        print "Processed $dirs_processed dirs\n";
        $t = localtime;
    }
}

sub checkdir {
  $directories = shift;

  if ( !-d $directories ) {
      make_path $directories or die "Failed to create path: $directories";
  }
}

&ScanDirectory("H:/divinum-officium/divinum-officium-master/web/output", "H:/output_consolidated");