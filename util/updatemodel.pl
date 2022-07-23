#!/usr/bin/perl -w
# updatemodel.pl - Fan control daemon for Apple Computers
#
# macfand - Mac Fan Control Daemon
# Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>

use strict;
use warnings;

sub findModel {
    my ($configPath, $profilePath) = @_;

    if (!$configPath) { $configPath = "/etc/macfand.conf"; }
    if (!$profilePath) { $profilePath = "/usr/local/macfand/machines/"; }

    my $retval;

    my @mIDPaths = (
        "/sys/devices/virtual/dmi/id/product_name"
    );

    foreach my $path (@mIDPaths) {
        next if (!-e $path);

        open(my $fh, "<$path") or next;
        $retval = <$fh>;
        close($fh);

        chomp $retval;
        $retval = lc($retval);

        my $machtest = "$profilePath$retval.conf";

        if (-e $machtest) {
            print "Found supported machine: $retval, updating configuration.\n";
            update_config($configPath, $profilePath, $retval);
        } else {
            print "\nNOTICE: Please manually edit your /etc/macfand.conf\nThere is not currently a profile configuration available for your Mac ($retval).\n\n";
            print "Visit: https://github.com/ablakely/macfand for more information.\n\n";
        }
    }
}


sub update_config {
    my ($configPath, $profilePath, $model) = @_;
    my @newconfig;

    open(my $fh, "<$configPath") or die $!;

    while (my $line = <$fh>) {
        if ($line =~ /\s+modelID = \"(.*?)"\;/) {
            push(@newconfig, "    modelID = \"$model\";\n");
        } elsif ($line =~ /\s+profileDir = \"(.*?)"\;/) {
            push(@newconfig, "    profileDir = \"$profilePath\";\n");
        } else {
            push(@newconfig, $line);
        }
    }

    close($fh);

    open($fh, ">$configPath") or die $!;

    foreach my $line (@newconfig) {
        print $fh $line;
    }

    close($fh);
}

if (`whoami` !~ /root/) {
    print "Error: Script requires root.\n";
} else {
    if ($ARGV[0]) {
        findModel($ARGV[0]);
    }
    elsif ($ARGV[1] && $ARGV[2]) {
        findModel($ARGV[0], $ARGV[1]);
    } else {
        findModel();
    }
}