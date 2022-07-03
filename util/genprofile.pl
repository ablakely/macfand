#!/usr/bin/perl

use strict;
use warnings;


my $MIN_SPEED = 950;
my $MAX_SPEED = 6200;


my (@fans, @sensors, @ctrl, @deftargets, %profile);

sub read_dump {
    open(my $FH, "<./smcdump.txt") or die $!;
    my @lines = <$FH>;
    close($FH) or die $!;

    return @lines;
}

sub sensorDescElem {
    my ($key, $lowval, $highval, $desc) = @_;

    chomp $desc;

    my %elem = (
        key => $key,
        low => $lowval,
        high => $highval,
        desc => $desc
    );

    push(@sensors, \%elem);
}

sub fanDescElem {
    my ($key, $lowval, $highval, $desc) = @_;

    chomp $desc;

    my %elem = (
        key => $key,
        low => $lowval,
        high => $highval,
        desc => $desc
    );

    push(@fans, \%elem);
}

sub fanCtrlElem {
    my ($key, $lowval, $highval, $desc) = @_;

    chomp $desc;

    my %elem = (
        key => $key,
        low => $lowval,
        high => $highval,
        desc => $desc
    );

    push(@ctrl, \%elem);
}

sub fanCtrlTarget {
    my ($target) = @_;

    push(@deftargets, $target);
}

sub gen_profile {
    my ($i, $FH);

    open($FH, ">../machines/$profile{filename}") or die $!;

    my $machname = $profile{filename};
    $machname =~ s/\.conf//gs;

    print "\nfile: $profile{filename}\n";
    print "name: $profile{name}\n";
    print "memory: $profile{memory}\n";

    print $FH "# $profile{filename}\n#\n# Machine Name: $profile{name}\n#\n";
    print $FH "# macfand profile for machines with identifer: $machname\n";
    print $FH "# Automatically generated using data from https://logi.wiki/index.php/SMC_Sensor_Codes\n\n";

    print $FH "presets =\n{\n\tblacklist = ();\n\ttemp_avg_floor = 45;\n\ttemp_avg_ceiling = 55;\n";


    print $FH "\n\tfan_ctrl = (\n";

    for (my $i = 0; $i < scalar(@ctrl); $i++) {
        print $FH "\t\t{\n\t\t\t";
        print $FH "use_avgctrl = true;\n\t\t\t";
        
        if ($ctrl[$i]->{low}) {
            print $FH "min_speed = ".$ctrl[$i]->{low}."\n\t\t\t";
        }

        if ($ctrl[$i]->{high}) {
            print $FH "max_speed = ".$ctrl[$i]->{high}."\n";
        }

        print $FH "\t\t},\n";
    }

    print $FH "\t);\n};\n\n";

    print $FH "profile =\n{\n\tsensor_desc = (\n";

    for (my $i = 0; $i < scalar(@sensors); $i++) {
        print $FH "\t\t{\n\t\t\t";
        print $FH "sensor = \"".$sensors[$i]->{key}."\";\n\t\t\t";
        
        if ($sensors[$i]->{low}) {
            print $FH "floor = ".$sensors[$i]->{low}.";\n\t\t\t";
        }

        if ($sensors[$i]->{high}) {
            print $FH "ceiling = ".$sensors[$i]->{high}.";\n\t\t\t";
        }

        print $FH "value = \"".$sensors[$i]->{desc}."\";\n\t\t},\n";
    }
    
    print $FH "\t);\n\tfan_desc = (\n";
    
    my $tmp;

    for (my $i = 0; $i < scalar(@fans); $i++) {
        print $FH "\t\t{\n\t\t\t";
        
        $tmp = $i + 1;
        print $FH "num = $tmp;\n\t\t\t";
        print $FH "value = \"".$fans[$i]->{desc}."\";\n\t\t},\n";
    }

    print $FH "\t);\n};\n";
}

my @dump = read_dump();

foreach my $line (@dump) {
    if ($line =~ /Dumping (.*?) from (.*?)/igs) {
        if ($profile{filename}) {
            gen_profile(%profile);
            @sensors = ();
            @fans = ();
            @ctrl = ();
            @deftargets = ();
        }
        
        $profile{filename} = lc($1).".conf";
    }

    if ($line =~ /Name\: (.*)/) {
        $profile{name} = $1;
    }

    if ($line =~ /Memory: (.*)/) {
        $profile{memory} = $1;
    }

    if ($line =~ /Defaults\: (.*?)\, temperature\: (.*?)\, (.*?)\n/) {
        if ($1 eq $2) {
            fanCtrlTarget($1);
        } else {
            fanCtrlTarget($1);
            fanCtrlTarget($2);
        }
    }

    if ($line =~ /Key (.*?) low (.*?)\s+high (.*?)\s+(.*?)\n/gsi) {
        
        my @keysplit = split("", $1);

        if ($keysplit[0] eq "F") {
            # fan
            fanDescElem($1, $2, $3, $4);
            fanCtrlElem($1, $2, $3, $4);
        } else {
            # sensor
            sensorDescElem($1, $2, $3, $4);
        }
    }
}