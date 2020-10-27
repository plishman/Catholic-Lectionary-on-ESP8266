#!/usr/bin/perl
use utf8;
no warnings 'utf8';

# PLL 17-06-2020
use File::Basename qw( fileparse );
use File::Path qw( make_path );
use File::Spec;
# PLL 17-06-2020


# áéíóöõúüûÁÉ  ‡
# Name : Laszlo Kiss
# Date : 01-20-08
# Divine Office
$a = 1;

#*** ordo()
# collects and prints the ordo
# first let specials to fill the chapters
# then break the text into units (separated by double newline)
# resolves the references (formatting characters, prayers hash references and subs)
#and prints the result
sub ordo {
  $tlang = ($lang1 !~ /Latin/) ? $lang1 : $lang2;

  #???%translate = %{setupstring($datafolder, $tlang, "Ordo/Translate.txt")};
  cache_prayers();
  $savesolemn = $solemn;
  if ($winner =~ /Quad6-[456]/i) { $solemn = 1; }
  $column = 1;
  if ($Ck) { $version = $version1; setmdir($version); precedence(); }
  @script1 = getordinarium($lang1, $command);
  @script1 = specials(\@script1, $lang1);
  $column = 2;
  if ($Ck) { $version = $version2; setmdir($version); precedence(); }
  @script2 = getordinarium($lang2, $command);
  @script2 = specials(\@script2, $lang2);
  $solemn = $savesolemn;
  #table_start();
  $ind1 = $ind2 = 0;
  $searchind = 0;
  #ante_post('Ante');

  if ($rule =~ /Full text/i) {
    @script1 = ();
    @script2 = ();
    $rule = 'Prelude';
  }

  if ($rule =~ /prelude/i) {
    my $str = $winner{Prelude};
    $str = norubr1($str);
    unshift(@script1, split('_', $str));
    $str = $winner2{Prelude};
    $str = norubr1($str);
    unshift(@script2, split('_', $str));
  }

  if ($rule =~ /Post Missam/i) {
    my $str = $winner{'Post Missam'};
    $str = norubr1($str);
    push(@script1, split('_', $str));
    $str = $winner2{'Post Missam'};
    $str = norubr1($str);
    push(@script2, split('_', $str));
  }

# PLL 18-06-2020
  $fnn_alt = 0; #PLL 17-06-2020 filenumber fnn
  $fnn_la = 0; #PLL 17-06-2020 filenumber fnn
  $hnn_alt = 0; #PLL 18-06-2020 heading number hnn_alt
  $hnn_la = 0; 

  $fnn_alt_last = 0;
  $hnn_alt_last = 0;
  $fnn_la_last = 0;
  $hnn_la_last = 0;
  
  $outputtext_alt = "";
  $outputtext_la = "";
  $endoutputtext = 0;
  $calldepth = 0;
  $break_early = 0; 

  $mass_heading = "";
  $last_mass_heading = "";
  $mass_heading_alt = "";

  $paragraph_loop_count = 0;

  %glorianames = (
    Cesky => 'Gloria',
    Deutsch => 'Gloria',
    English => 'Gloria',
    French => 'Gloria',
    Italiano => 'Gloria',
    Latin => 'Gloria',
    Magyar => 'Gloria',
    Polski => 'Gloria',
    Portugues => 'Gloria',
    Spanish => 'Gloria'
  );

  %credonames = (
    Cesky => 'Krédo|Credo',
    Deutsch => 'Credo',
    English => 'Creed|Credo',
    French => 'Profession|Credo',
    Italiano => 'Credo',
    Latin => 'Credo',
    Magyar => 'Creed|Credo',
    Polski => 'Credo',
    Portugues => 'Creed|Credo',
    Spanish => 'Creed|Credo'
  );

  if ($Propers) {
    $glorialine_la = -1;
    $credoline_la = -1;
    $glorialine_alt = -1;
    $credoline_alt = -1;
  }

# PLL 18-06-2020

  while ($ind1 < @script1 || $ind2 < @script2) {  
    #if ($fnn_alt == 15) {
    #  $DB::single = 1;
    #}

    $last_mass_heading = $mass_heading;
    $mass_heading = "";
    $mass_heading_alt = "";

    if (!$Propers && $break_early) {
      last;
    }

    if ($Propers && $op_ver =~ /Tridentine 1570/ && $fnn_la == 3 && $last_mass_heading =~ /Oratio/) { # try catch the case of normal 12 part propers, where the Lesson is omitted in th 1570 mass
      $fnn_la++;
      #print "tridentine 1570 skipping one la";
    }
    ($text1, $ind1) = getunit(\@script1, $ind1);
    ($text2, $ind2) = getunit(\@script2, $ind2);
    $column = 1;
    if ($Ck) { $version = $version1; }
    $text1 = resolve_refs($text1, $lang1);      #PLL 17-06-2020 (additional params $filenumber, $dn)
    $text1 =~ s/\<BR\>\s*\<BR\>/\<BR\>/g;
    if ($lang1 =~ /Latin/i) { $text1 = spell_var($text1); }
    if ($text1 && $text1 !~ /^\s+$/) { setcell($text1, $lang1); }

    if ($Propers) {
      $glorianame_la = %glorianames{$lang1};
      if ($mass_heading =~ /$glorianame_la/) {
        $glorialine_la = $fnn_la;
      }
      
      $credoname_la = %credonames{$lang1};
      if ($mass_heading =~ /$credoname_la/) {
        $credoline_la = $fnn_la;
      }
    }

    $fnn_la_last = $fnn_la;
    $hnn_la_last = $hnn_la;
    $fnn_la += 1;
    $hnn_la = 0;

    if (!$only) {
      if ($Propers && $op_ver =~ /Tridentine 1570/ && $fnn_alt == 3 && $last_mass_heading =~ /Oratio/) { # try catch the case of normal 12 part propers, where the Lesson is omitted in th 1570 mass
        $fnn_alt++;
        #print "tridentine 1570 skipping one alt";
      }
      $column = 2;
      if ($Ck) { $version = $version2; }
      $text2 = resolve_refs($text2, $lang2);      #PLL 17-06-2020 (additional params $filenumber, $dn)
      $text2 =~ s/\<BR\>\s*\<BR\>/\<BR\>/g;
      if ($lang2 =~ /Latin/i) { $text2 = spell_var($text2); }
      if ($text2 && $text2 !~ /^\s+$/) { 
        #if ($Propers) {
        #  if ($mass_heading_alt eq "" && $mass_heading ne "") {
        #    write_ordo($mass_heading, $fnn_alt, $hnn_alt, $lang2, $dnn); #PLL 17-06-2020
        #    $hnn_alt += 1; #PLL 18-06-2020         
        #  }
        #}
        setcell($text2, $lang2); 
      }

      if ($Propers) {
        $glorianame_alt = %glorianames{$lang2};
        if ($mass_heading_alt =~ /$glorianame_alt/) {
          $glorialine_alt = $fnn_alt;
        }

        $credoname_alt = %credonames{$lang2};
        if ($mass_heading_alt =~ /$credoname_alt/) {
          $credoline_alt = $fnn_alt;
        }
      }

      $fnn_alt_last = $fnn_alt;
      $hnn_alt_last = $hnn_alt;
      $fnn_alt += 1;
      $hnn_alt = 0;
    }

    $paragraph_loop_count++;

    #$fnn_alt += 1;  #PLL 17-06-2020
    #$fnn_la += 1;  #PLL 17-06-2020
  }
  #ante_post('Post');
  #table_end();
  if ($column == 1) { $searchind++; }

  ##$endoutputtext = 1;#PLL 19-06-2020
  ##outputordo($t, $lang2);  #PLL 19-06-2020
}

#*** getunits(\@s, $ind)
# break the array into units separated by double newlines
# from $ind  to the returned new $ind
sub getunit {

  my $s = shift;
  my @s = @$s;
  my $ind = shift;
  my $t = '';
  my $plen = 1;

  while ($ind < @s) {
    my $line = chompd($s[$ind]);
    $ind++;
    if ($line && !($line =~ /^\s+$/)) { $t .= "$line\n"; next; }
    if (!$t) { next; }
    last;
  }

  if ($dayname[0] !~ /Pasc/i) {
    $t =~ s/\(Alleluia.*?\)//ig;
  } else {
    $t =~ s/\((Alleluia.*?)\)/$1/ig;
  }
  return ($t, $ind);
}

#*** resolve_refs($text_of_block, $lang)
#resolves $name &name references and special characters
#retuns the to be listed text
sub resolve_refs {
  $calldepth++; # PLL 19-06-2020
  #my $fragment = "";

  my $t = shift;
  my $lang = shift;
  my @t = split("\n", $t);
  my $t = '';

  #my $filenumber = 0;
  #my $dirname = "";
  my $headingnumber = 1;
  my $filenumber = 0;
  #
  #if (scalar(@_) >= 2) {
  #  $filenumber = shift;
  #  $dirname = shift;  
  #}
  #
  #if (scalar(@_) == 1) {
  #  $headingnumber = shift;
  #}

  if (scalar(@_) == 1) {
    $filenumber = shift;
    $headingnumber = shift;
  }


  if ($t[0] =~ /(omit|elmarad)/i) {
    $t[0] =~ s/^\s*\#/\!x\!/;
  } else {
    $t[0] =~ s/^\s*\#/\!\!/;
  }

  #cycle by lines
  my $it;
  my $line_prefix;

  for ($it = 0; $it < @t; $it++) {   
    $line = $t[$it];

    # Should this line be joined to the next? Strip off the continuation
    # character as we check.
    my $line_continues = ($line =~ s/\s*~\s*$//);

    # The first batch of transformations are performed on the current
    # input line only.
    #$ and & references
    if ($line !~ /(callpopup|rubrics)/i && $line =~ /^\s*[\$\&]/)    #??? was " /[\#\$\&]/)
    {
      $line =~ s/\.//g;
      $line =~ s/\s+$//;
      $line =~ s/^\s+//;

      #prepares reading the part of common w/ antiphona
      if ($line =~ /psalm/ && $t[$it - 1] =~ /^\s*Ant\. /i) {
        $line = expand($line, $lang, $t[$it - 1]);
      } else {
        $line = expand($line, $lang);
      }

      if ((!$Tk && $line !~ /\<input/i) || ($Tk && $line !~ /\% .*? \%/)) {
        #print "\n\n\n((( calldepth = $calldepth (((((((((((((((((((((((((((((((((((\n\n$line\n\n))))))))))))))))))))))))))))))))))))\n\n\n";
        
        if ($lang =~ /Latin/i) { 
          $line = resolve_refs($line, $lang); #PLL 17-06-2020 extra params $filenumber, $dirname
        }
        else {
          $line = resolve_refs($line, $lang); #PLL 17-06-2020 extra params $filenumber, $dirname
        }
      }    #for special chars
    }

    #cross
    $line = setcross($line);

    #red prefix
    if ($line =~ /^\s*(R\.|V\.|S\.|P\.|M\.|A\.|O\.|C\.|D\.|Benedictio\.* |Absolutio\.* |Ant\. |Ps\. )(.*)/s) {
      my $h = $1;
      my $l = $2;

      if ($h =~ /(Benedictio|Absolutio)/) {
        my $str = $1;
        if ($lang !~ /Latin/i) { $str = $translate{$str}; }
        $h =~ s/(Benedictio|Absolutio)/$str/;
      }
      $line = setfont($redfont, $h) . $l;
    }

    #Quad6 Gospels
    if ($winner =~ /Quad6/) {
      $line =~ s/(\b[A-Z]\.)/setfont($redfont, $1)/eg;
    }

    #consecration words
    if ($line =~ /\s*\!\[\:(.*?)\:\]/) {
      $line = $1;
      my $cfont = $redfont;
      $cfont =~ s/red/blue/i;
      $line = setfont($cfont, $line);
    } elsif ($line =~ /^\s*\!\!\!(.*)/s) {
      $line = $1;
      my $cfont = $redfont;
      $cfont =~ s/red/black/i;
      $line = setfont($cfont, $line);
    }

    #small omitted comment
    elsif ($line =~ /^\s*\!x\!\!(.*)/s) {
      $l = $1;
      $line = setfont($smallfont, $l);
    }

    #small omitted title
    elsif ($line =~ /^\s*\!x\!(.*)/s) {
      $l = $1;
      $line = setfont($smallblack, $l);
    }

    #large chapter title
    elsif ($line =~ /^\s*\!\!(.*)/s) {
      my $l = $1; #$l appears to have the reading!
      my $suffix = '';

      #PLL 18-06-2020
      ##$endoutputtext = 1;
      #outputordo($fragment, $lang, 1); # end of the current file happens when a new heading is found
      ##outputordo("", $lang, 1); # end of the current file happens when a new heading is found
      #$fragment = "";

      if ($lang =~ /Latin/i) {
        $mass_heading = $l;
        $mass_heading =~ s/&nbsp;//;
        if ($Propers) {
          write_ordo($mass_heading, $fnn_la, $hnn_la, $lang, $dnn); #PLL 17-06-2020
          #$headingnumber += 1; #PLL 17-06-2020
        }
        $hnn_la += 1; #PLL 18-06-2020
      }
      else {
        $mass_heading_alt = $l;
        $mass_heading_alt =~ s/&nbsp;//;
        if ($Propers) {
          write_ordo($mass_heading_alt, $fnn_alt, $hnn_alt, $lang, $dnn); #PLL 17-06-2020
        }
        $hnn_alt += 1; #PLL 18-06-2020
      }
      #$endoutputtext = 1;
      #outputordo($t, $lang);  
      #PLL 18-06-2020
      
      if ($l =~ /\{.*?\}/) {
        $l =~ s/(\{.*?\})//;
        $suffix = $1;
        $suffix = setfont($smallblack, $suffix);
      }
      $line = setfont($largefont, $l) . " $suffix\n";
    }

    #red line
    elsif ($line =~ /^\s*\!(.*)/s) {
      $l = $1;
      $line = setfont($redfont, $l);
    }

    # Prepend any previous lines that tilde-connect to the current line.
    $line = "$line_prefix $line" if ($line_prefix);

    # The remaining transformations are peformed on the whole line as
    # built up from tilde-connected lines. Critically, these are
    # performed once for each line of input, so they should be
    # idempotent.
    # First letter red.
    $line =~ s/^\s*r\.\s*(.)(.*)/setfont($largefont, $1) . $2/em;

    # First letter initial.
    $line =~ s/
        (^|\{\:.*?\:\})      # Beginning of line or {::} construction.
        \s*v\.\s*(.)(.*)     # 'v.' plus a letter plus the rest.
      /
        $1 . setfont($initiale, $2) . $3
      /emx;

    # Connect lines marked by tilde.
    if ($line_continues && $it < $#t) {
      $line_prefix = $line;
    } 
    else {
      $line_prefix = '';
      $t .= "$line<BR>\n";
      #$fragment .= "$line<BR>\n";

      ##$endoutputtext = 1; # PLL
      ##outputordo("$line<BR>\n", $lang, 0); # PLL append last line to end of current file
    }


    # PLL 18-06-2020
    #if ($calldepth > 1) {
    #  outputordo("$line<BR>\n", $lang);  
    #}
    # PLL 18-06-2020

  }    #line by line cycle ends

  #removes occasional double linebreaks
  $t =~ s/\<BR\>\s*\<BR\>/\<BR\>/g;
  $t =~ s/<\/P>\s*<BR>/<\/P>/g;

  # PLL 18-06-2020
  #if ($Propers && $calldepth == 1) {
  #  outputordo($t, $lang, 0);  
  #  ##outputordo($fragment, $lang); #*********************
  #}
  # PLL 18-06-2020

  $calldepth--; # PLL 19-06-2020

  #print "\n\n\n++++ calldepth: $calldepth ++++++++++++++++++++++++++++++++++++++++++++++\n\n$t\n\n-------------------------------------------------\n\n\n";
  return $t;
}

# PLL 18-06-2020
sub outputordo {
  my $inputtext = shift;
  my $textlang = shift;
  my $incfilenumber = shift;

  if ($endoutputtext == 1) {
    $outputtext =~ s/\<BR\>\s*\<BR\>/\<BR\>/g;
    if ($textlang =~ /Latin/i) { 
      $outputtext_la .= $inputtext;
      $outputtext_la = spell_var($outputtext_la);
      if ($outputtext_la && $outputtext_la !~ /^\s+$/) { 
        #print "\n\n\n** LA calldepth = $calldepth ********************************\n\n$outputtext_la\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^";
        write_ordo($outputtext_la, $fnn_la, $hnn_la, $textlang, $dnn);  #PLL 17-06-2020
      }
      #$outputtext_la = $inputtext;
      if ($incfilenumber) {
        $fnn_la += 1;
        $hnn_la = 0;
      }
      $outputtext_la = "";
    } else {
      $outputtext_alt .= $inputtext;
      if ($outputtext_alt && $outputtext_alt !~ /^\s+$/) { 
        #print "\n\n\n** EN calldepth = $calldepth ********************************\n\n$outputtext_alt\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^";
        write_ordo($outputtext_alt, $fnn_alt, $hnn_alt, $textlang, $dnn);  #PLL 17-06-2020
      }
      #$outputtext_alt = $inputtext;
      if ($incfilenumber) {
        $fnn_alt += 1;
        $hnn_alt = 0;
      }
      $outputtext_alt = "";
    }
    $endoutputtext = 0;
  }
  else {
    if ($textlang =~ /Latin/i) { 
      $outputtext_la = $outputtext_la . $inputtext;
      #print "\n\n\n= LA calldepth = $calldepth =================================\n\n$outputtext_la\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^";
    }
    else {
      $outputtext_alt = $outputtext_alt . $inputtext;
      #print "\n\n\n= EN calldepth = $calldepth =================================\n\n$outputtext_alt\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^";
    }
  }
}
# PLL 18-06-2020




#*** Alleluia($lang)
# return the text Alleluia or Laus tibi
sub Alleluia {
  my $lang = shift;
  our %prayers;
  my $text = $prayers{$lang}->{'Alleluia'};
  my @text = split("\n", $text);
  $text = $text[0];

  #if ($dayname[0] =~ /Pasc/i) {$text = "Alleluia, alleluia, alleluia";}
  return $text;
}

#*** Benedicamus_Domino
# adds Alleluia, alleluia for Pasc0
sub Benedicamus_Domino {
  my $lang = shift;
  our %prayers;
  my $text = $prayers{$lang}->{'Benedicamus Domino'};
  if ($dayname[0] !~ /Pasc0/i) { return $text; }
  my @text = split("\n", $text);
  return "$text[0]. Alleluia, alleluia\n$text[1]. Alleluia, alleluia\n";
}

sub depunct {
  my $item = shift;
  $item =~ s/[\.\,\:\?\!\"\'\;\*]//g;
  $item =~ s/[áÁ]/a/g;
  $item =~ s/[éÉ]/e/g;
  $item =~ s/[íí]/i/g;
  $item =~ s/[óöõÓÖÔ]/o/g;
  $item =~ s/[úüûÚÜÛ]/u/g;
  $item =~ s/æ/ae/g;
  return $item;
}

#*** translate($name)
# return the translated name (called only for column2 if necessary)
sub translate {
  my $name = shift;
  my $n = $name;
  my $prefix = '';
  if ($n =~ s/(\$|\&)//) { $prefix = $1; }
  $n =~ s/^\n*//;
  $n =~ s/\n*$//;
  $n =~ s/\_/ /g;
  if (!exists($translate{$n})) { return $name; }
  $n = $translate{$n};
  if ($name !~ /(omit|elmarad)/i) { $n = $prefix . $n; }
  $n =~ s/\n*$//;
  return "$n";
}

#*** getordinarium($lang, $command)
# returns the full pathname of ordinarium for the language and hora
sub getordinarium {
  my $lang = shift;
  my @script;

  if ($Propers && (@script = do_read("$datafolder/Latin/Ordo/Propers.txt"))) {
    return @script;
  }
  my $fname = 'Ordo';
  if ($version =~ /(1967|Newcal)/i) { $fname = 'Ordo67'; }
  if ($NewMass) { $fname = ($column == 1) ? $ordos{$version1} : $ordos{$version2}; }
  $fname = checkfile($lang, "Ordo/$fname.txt");

  if (@script = do_read($fname)) {
    $_ = "$_\n" for @script;
  } else {
    $error = "$fname cannot open!";
  }
  return @script;
}

sub columnsel {
  my $lang = shift;
  if ($Ck || $NewMass) { return ($column == 1) ? 1 : 0; }
  return ($lang =~ /$lang1/i) ? 1 : 0;
}

#PLL 16-06-2020
sub write_ordo {
    my $outputtext = shift; # text
    my $n = shift; # file number
    my $m = shift; # file subpart number
    my $lang = shift; # language
    my $dirname = shift; # subdirectory
    my $overwrite = shift; # whether to overwrite file if present
    my $append = shift; # whether to clobber or append

    if ($already_output_latin && $lang eq "Latin") {
      return;
    }

    my $tldname = %tldnames{$lang};

    my $path = "./output/$op_dir/" . $tldname . "/" . $dirname;
    checkdir($path);

  if ($output_all_data) {
    my $fname = $path . "/" . $n . "_" . $m . "_" . $tldname . ".txt";

    if (!$overwrite && -e $fname) {
      return; # don't output text more than once (now each whole subheading text is output at once rather than in bits as before)
    }

    if ($append) {
      open(DATA, ">>$fname") or die "Couldn't open file $fname!";  
    }
    else {
      open(DATA, ">$fname") or die "Couldn't open file $fname!";
    }
    binmode(DATA, ':encoding(utf-8)');
    print DATA $outputtext;
    close(DATA);
  }
}

sub write_head {
  my $outputtext = shift; # text
  my $lang = shift; # language
  my $dirname = shift; # subdirectory
  my $filename = shift; # file name prefix

  my $tldname = %tldnames{$lang};

  my $path = "./output/$op_dir/" . $tldname . "/" . $dirname;
  checkdir($path);

  my $fname = $path . "/" . $filename . ".txt";
  open(DATA, "+>$fname") or die "Couldn't open file $fname!";
  binmode(DATA, ':encoding(utf-8)');
  print DATA $outputtext;
  close(DATA);
}

sub write_all_head {
  # record of all masses for Saints' days
  
  my $t = shift; # text
  my $mm = shift; # month number
  my $dd = shift; # day number
  my $lang = shift; # language

  if ($already_output_latin || $lang ne "Latin") { # only output once
    return;
  }

  my $path = "./output/$op_dir/"; #. $dirname;
  checkdir($path);

  my $fname = $path . "days.txt";

  open(DATA, ">>$fname") or die "Couldn't open file days.txt, $!";
  binmode(DATA, ':encoding(utf-8)');

  my $txt =  $mm . "," . $dd . ",\"" . $t . "\"\n";
  print DATA $txt; # csv format
  close DATA;
}

sub checkdir {
  $directories = shift;

  if ( !-d $directories ) {
      make_path $directories or die "Failed to create path: $directories";
  }
}

sub ordo_dir_exists {
    my $lang = shift; # language
    my $dirname = shift; # subdirectory

    my $tldname = %tldnames{$lang};

    my $path = "./output/$op_dir/" . $tldname . "/" . $dirname;
    if ( -e $path ) {
      return 1;
    }
    
    return 0;
}

#PLL 16-06-2020