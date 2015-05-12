#!/usr/bin/perl

use warnings;
use strict;

my ($init) = $ARGV[0];

die "init not found!" unless $init;

open my $file, '+<', $init
    or die "Can not open file `$init': $!\n";

my @data=
    map { chomp; $_ }
    grep !/\.strftimeFormat:/,
    grep !/\.toolbar\.tools:/,
    grep !/toolbar\.widthPercent:/, <$file>;


push @data,
(
    'session.screen0.toolbar.widthPercent: 100',
    'session.screen0.strftimeFormat: %d %b, %a %02k:%M:%S',
    'session.screen0.toolbar.tools: ' .
        'prevworkspace, workspacename, nextworkspace, ' .
        'clock, prevwindow, nextwindow, iconbar, systemtray',
);

truncate $file, 0;
seek $file, 0, 0;
print $file join "\n", @data;
