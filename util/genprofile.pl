#!/usr/bin/perl

use strict;
use warnings;
use Data::Dumper;

sub read_dump {
    open(my $FH, "<./smcdump.txt") or die $!;
    my @lines = <$FH>;
    close($FH) or die $!;

    return @lines;
}

sub gen_profile {
    my (%profile) = @_;

    my $machname = $profile{filename};
    $machname =~ s/\.conf//gs;

    print "\nfile: $profile{filename}\n";
    print "name: $profile{name}\n";
    print "memory: $profile{memory}\n";

    my @sensors = (keys %{$profile{sensors}});
    for (my $i = 0; $i < scalar(@sensors); $i++) {
        print $sensors[$i]." - ".$profile{sensors}{$sensors[$i]}."\n";
    }

    open(my $FH, ">../machines/$profile{filename}") or die $!;

    print $FH "# $profile{filename}\n#\n# Machine Name: $profile{name}\n#\n";
    print $FH "# macfand profile for machines with identifer: $machname\n";
    print $FH "# Automatically generated using data from https://logi.wiki/index.php/SMC_Sensor_Codes\n\n";

    print $FH "presets =\n{\n\tblacklist = ();\n\ttemp_avg_floor = 45;\n\ttemp_avg_ceiling = 55;\n\n\tfan_ctrl = ();\n};\n\n";
    print $FH "profile =\n{\n\tsensor_desc = (\n";

    for (my $i = 0; $i < scalar(@sensors); $i++) {
        print $FH "\t\t{\n\t\t\tsensor = \"".$sensors[$i]."\";\n\t\t\tvalue = \"".$profile{sensors}{$sensors[$i]}."\";\n\t\t},\n";
    }
    
    print $FH "\t);\n\tfan_desc = ();\n};\n";
}

my @dump = read_dump();
my %profile;

foreach my $line (@dump) {
    if ($line =~ /Dumping (.*?) from (.*?)/igs) {
        if ($profile{filename}) {
            gen_profile(%profile);
            $profile{sensors} = {};
        }
        
        $profile{filename} = lc($1).".conf";
    }

    if ($line =~ /Name\: (.*)/) {
        $profile{name} = $1;
    }

    if ($line =~ /Memory: (.*)/) {
        $profile{memory} = $1;
    }

    if ($line =~ /Key (.*?) low (\d+)\s+high (\d+)\s+(.*)/gsi) {
        $profile{sensors}{$1} = $4;
        chomp $profile{sensors}{$1};
    }
}
