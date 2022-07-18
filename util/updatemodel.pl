#!/usr/bin/perl -w
# updatemodel.pl - Fan control daemon for Apple Computers
#
# macfand - Mac Fan Control Daemon
# Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>

use strict;
use warnings;

sub findModel {
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

        my $machtest = "/usr/local/macfand/machines/$retval.conf";

        if (-e $machtest) {
            print "Found supported machine: $retval, updating configuration.\n";
            update_config($retval);
        } else {
            print "\nNOTICE: Please manually edit your /etc/macfand.conf\nThere is not currently a profile configuration available for your Mac ($retval).\n\n";
            print "Visit: https://github.com/ablakely/macfand for more information.\n\n";
        }
    }
}


sub update_config {
    my ($model) = @_;
    my @newconfig;

    open(my $fh, "</etc/macfand.conf") or die $!;

    while (my $line = <$fh>) {
        if ($line =~ /\s+modelID = \"(.*?)"\;/) {
            push(@newconfig, "    modelID = \"$model\";\n");
        } else {
            push(@newconfig, $line);
        }
    }

    close($fh);

    open($fh, ">/etc/macfand.conf") or die $!;

    foreach my $line (@newconfig) {
        print $fh $line;
    }

    close($fh);
}

if (`whoami` !~ /root/) {
    print "Error: Script requires root.\n";
} else {
    findModel();
}