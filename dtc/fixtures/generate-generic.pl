#!/usr/bin/perl
#
# DMX Total Control - generic fixture library generator
#
# Writes the QLC+ fixture definitions (.qxf) of the basic generic fixtures
# shipped with DMX Total Control into ./Generic. Run it after editing the
# tables below:   perl generate-generic.pl
#
# Licensed under the Apache License, Version 2.0

use strict;
use warnings;
use utf8;
use File::Basename;
use File::Path qw(make_path);

my $outdir = dirname(__FILE__) . "/Generic";
make_path($outdir);

# ---------------------------------------------------------------------------
# Channel building blocks
#   preset channels: [ name, preset ]
#   custom channels: { name, group, colour?, byte?, caps => [[min, max, label], ...] }
# ---------------------------------------------------------------------------
my %ch = (
    Dimmer     => ['Dimmer', 'IntensityMasterDimmer'],
    Red        => ['Red', 'IntensityRed'],
    Green      => ['Green', 'IntensityGreen'],
    Blue       => ['Blue', 'IntensityBlue'],
    White      => ['White', 'IntensityWhite'],
    Amber      => ['Amber', 'IntensityAmber'],
    UV         => ['UV', 'IntensityUV'],
    Pan        => ['Pan', 'PositionPan'],
    PanFine    => ['Pan fine', 'PositionPanFine'],
    Tilt       => ['Tilt', 'PositionTilt'],
    TiltFine   => ['Tilt fine', 'PositionTiltFine'],
    PTSpeed    => ['Pan/Tilt speed', 'SpeedPanTiltFastSlow'],
    Strobe     => ['Strobe', 'ShutterStrobeSlowFast'],
    Focus      => ['Focus', 'BeamFocusNearFar'],
    Zoom       => ['Zoom', 'BeamZoomSmallBig'],
    PrismRot   => ['Prism rotation', 'PrismRotationSlowFast'],
    Shutter    => { name => 'Shutter', group => 'Shutter', caps => [
                      [0, 7, 'Closed'], [8, 15, 'Open'], [16, 131, 'Strobe slow to fast'],
                      [132, 139, 'Open'], [140, 255, 'Random strobe'] ] },
    Program    => { name => 'Program', group => 'Effect', caps => [
                      [0, 9, 'No function'], [10, 59, 'Colour jump'], [60, 109, 'Colour fade'],
                      [110, 159, 'Colour pulse'], [160, 209, 'Sound active'], [210, 255, 'Auto'] ] },
    ProgSpeed  => { name => 'Program speed', group => 'Speed', caps => [ [0, 255, 'Slow to fast'] ] },
    ColorWheel => { name => 'Colour wheel', group => 'Colour', caps => [
                      [0, 9, 'White'], [10, 19, 'Red'], [20, 29, 'Orange'], [30, 39, 'Yellow'],
                      [40, 49, 'Green'], [50, 59, 'Blue'], [60, 69, 'Cyan'], [70, 79, 'Magenta'],
                      [80, 127, 'Split colours'], [128, 191, 'Rotation slow to fast'],
                      [192, 255, 'Reverse rotation slow to fast'] ] },
    Gobo       => { name => 'Gobo wheel', group => 'Gobo', caps => [
                      [0, 7, 'Open'], [8, 15, 'Gobo 1'], [16, 23, 'Gobo 2'], [24, 31, 'Gobo 3'],
                      [32, 39, 'Gobo 4'], [40, 47, 'Gobo 5'], [48, 55, 'Gobo 6'], [56, 63, 'Gobo 7'],
                      [64, 127, 'Gobo shake slow to fast'], [128, 255, 'Wheel rotation slow to fast'] ] },
    GoboRot    => { name => 'Gobo rotation', group => 'Gobo', caps => [
                      [0, 4, 'Stop'], [5, 129, 'Rotation slow to fast'], [130, 134, 'Stop'],
                      [135, 255, 'Reverse rotation slow to fast'] ] },
    Prism      => { name => 'Prism', group => 'Prism', caps => [ [0, 127, 'Off'], [128, 255, 'On'] ] },
    Reset      => { name => 'Control', group => 'Maintenance', caps => [
                      [0, 199, 'No function'], [200, 209, 'Lamp on'], [210, 249, 'No function'],
                      [250, 255, 'Reset'] ] },
    Fog        => { name => 'Fog', group => 'Effect', caps => [ [0, 9, 'Off'], [10, 255, 'Fog output'] ] },
    Haze       => { name => 'Haze', group => 'Effect', caps => [ [0, 9, 'Off'], [10, 255, 'Haze output'] ] },
    Fan        => { name => 'Fan', group => 'Speed', caps => [ [0, 9, 'Off'], [10, 255, 'Fan speed'] ] },
    LaserMode  => { name => 'Mode', group => 'Effect', caps => [
                      [0, 49, 'Off'], [50, 99, 'Auto'], [100, 149, 'Sound active'], [150, 255, 'DMX'] ] },
    LaserPat   => { name => 'Pattern', group => 'Effect', caps => [ [0, 255, 'Pattern selection'] ] },
    LaserRot   => { name => 'Rotation', group => 'Effect', caps => [
                      [0, 127, 'Fixed angle'], [128, 255, 'Rotation slow to fast'] ] },
    LaserSpeed => { name => 'Speed', group => 'Speed', caps => [ [0, 255, 'Slow to fast'] ] },
    LaserColor => { name => 'Colour', group => 'Colour', caps => [
                      [0, 63, 'Red'], [64, 127, 'Green'], [128, 191, 'Yellow'], [192, 255, 'Colour change'] ] },
);

# ---------------------------------------------------------------------------
# Fixtures: model, type, channels used, modes ([name, [channel keys], heads])
# ---------------------------------------------------------------------------
sub par_modes
{
    my @colours = @_;
    my $n = scalar @colours;
    my $tag = join('', map { $_ eq 'UV' ? '+UV' : substr($_, 0, 1) } @colours);
    return (
        ["${n}ch $tag", [@colours]],
        [($n + 1) . "ch D$tag", ['Dimmer', @colours]],
        [($n + 2) . "ch D$tag+Strobe", ['Dimmer', @colours, 'Strobe']],
        [($n + 4) . "ch Full", ['Dimmer', @colours, 'Strobe', 'Program', 'ProgSpeed']],
    );
}

my @fixtures = (
    { model => 'LED PAR RGB', type => 'Color Changer', modes => [ par_modes(qw(Red Green Blue)) ] },
    { model => 'LED PAR RGBW', type => 'Color Changer', modes => [ par_modes(qw(Red Green Blue White)) ] },
    { model => 'LED PAR RGBA', type => 'Color Changer', modes => [ par_modes(qw(Red Green Blue Amber)) ] },
    { model => 'LED PAR RGBWA', type => 'Color Changer', modes => [ par_modes(qw(Red Green Blue White Amber)) ] },
    { model => 'LED PAR RGBWA+UV', type => 'Color Changer', modes => [ par_modes(qw(Red Green Blue White Amber UV)) ] },
    { model => 'LED PAR White', type => 'Dimmer', modes => [
        ['1ch Dimmer', ['Dimmer']], ['2ch Dimmer+Strobe', ['Dimmer', 'Strobe']] ] },
    { model => 'Strobe', type => 'Strobe', modes => [
        ['1ch Strobe', ['Strobe']], ['2ch Dimmer+Strobe', ['Dimmer', 'Strobe']] ] },
    { model => 'Blinder 2 Lamps', type => 'Dimmer',
      extra => { L1 => ['Lamp 1', 'IntensityDimmer'], L2 => ['Lamp 2', 'IntensityDimmer'] },
      modes => [ ['1ch Master', ['Dimmer']], ['2ch Lamps', ['L1', 'L2'], [[0], [1]]] ] },
    { model => 'LED Bar RGB 8 Segments', type => 'LED Bar (Pixels)', segments => 8,
      modes => [ ['3ch RGB', ['Red', 'Green', 'Blue']],
                 ['5ch DRGB+Strobe', ['Dimmer', 'Red', 'Green', 'Blue', 'Strobe']],
                 ['24ch Pixels', 'PIXELS'],
                 ['26ch Pixels+Dimmer', 'DIMMER_PIXELS'] ] },
    { model => 'Moving Head Spot', type => 'Moving Head', modes => [
        ['9ch Basic', [qw(Pan Tilt PTSpeed Dimmer Shutter ColorWheel Gobo Prism Reset)]],
        ['11ch Standard', [qw(Pan Tilt PanFine TiltFine PTSpeed Dimmer Shutter ColorWheel Gobo Prism Reset)]],
        ['14ch Extended', [qw(Pan PanFine Tilt TiltFine PTSpeed Dimmer Shutter ColorWheel Gobo GoboRot Prism PrismRot Focus Reset)]] ] },
    { model => 'Moving Head Wash RGBW', type => 'Moving Head', modes => [
        ['10ch Basic', [qw(Pan Tilt PTSpeed Dimmer Shutter Red Green Blue White Reset)]],
        ['14ch Extended', [qw(Pan PanFine Tilt TiltFine PTSpeed Dimmer Shutter Red Green Blue White Zoom Program Reset)]] ] },
    { model => 'Moving Head Beam', type => 'Moving Head', modes => [
        ['13ch Standard', [qw(Pan PanFine Tilt TiltFine PTSpeed Dimmer Shutter ColorWheel Gobo Prism PrismRot Focus Reset)]] ] },
    { model => 'Scanner', type => 'Scanner', modes => [
        ['6ch Basic', [qw(Pan Tilt ColorWheel Gobo Shutter Dimmer)]],
        ['8ch Standard', [qw(Pan Tilt PTSpeed ColorWheel Gobo GoboRot Shutter Dimmer)]] ] },
    { model => 'Fog Machine', type => 'Smoke', modes => [ ['1ch', ['Fog']] ] },
    { model => 'Hazer', type => 'Hazer', modes => [ ['1ch', ['Haze']], ['2ch Haze+Fan', ['Haze', 'Fan']] ] },
    { model => 'Laser', type => 'Laser', modes => [
        ['6ch', [qw(LaserMode LaserPat LaserColor Zoom LaserRot LaserSpeed)]] ] },
);

sub xml_escape
{
    my $s = shift;
    $s =~ s/&/&amp;/g; $s =~ s/</&lt;/g; $s =~ s/>/&gt;/g; $s =~ s/"/&quot;/g;
    return $s;
}

sub channel_xml
{
    my ($def) = @_;
    if (ref $def eq 'ARRAY')
    {
        return sprintf(" <Channel Name=\"%s\" Preset=\"%s\"/>\n", xml_escape($def->[0]), $def->[1]);
    }
    my $x = sprintf(" <Channel Name=\"%s\">\n", xml_escape($def->{name}));
    $x .= sprintf("  <Group Byte=\"%d\">%s</Group>\n", $def->{byte} // 0, $def->{group});
    $x .= sprintf("  <Colour>%s</Colour>\n", $def->{colour}) if $def->{colour};
    foreach my $c (@{$def->{caps}})
    {
        $x .= sprintf("  <Capability Min=\"%d\" Max=\"%d\">%s</Capability>\n", $c->[0], $c->[1], xml_escape($c->[2]));
    }
    return $x . " </Channel>\n";
}

sub channel_name
{
    my ($def) = @_;
    return ref $def eq 'ARRAY' ? $def->[0] : $def->{name};
}

foreach my $f (@fixtures)
{
    my %pool = (%ch, %{$f->{extra} // {}});
    my @modes;

    # pixel bars: one RGB triplet per segment
    if ($f->{segments})
    {
        my @pixels;
        for my $s (1 .. $f->{segments})
        {
            for my $c (qw(Red Green Blue))
            {
                my $key = "$c$s";
                $pool{$key} = ["$c $s", "Intensity$c"];
                push @pixels, $key;
            }
        }
        foreach my $m (@{$f->{modes}})
        {
            my ($name, $chs) = @$m;
            if ($chs eq 'PIXELS' || $chs eq 'DIMMER_PIXELS')
            {
                my @list = $chs eq 'PIXELS' ? @pixels : ('Dimmer', 'Strobe', @pixels);
                my $offset = $chs eq 'PIXELS' ? 0 : 2;
                my @heads = map { my $b = $offset + $_ * 3; [$b, $b + 1, $b + 2] } (0 .. $f->{segments} - 1);
                push @modes, [$name, \@list, \@heads];
            }
            else
            {
                push @modes, $m;
            }
        }
    }
    else
    {
        @modes = @{$f->{modes}};
    }

    # channels in order of first use
    my @order;
    my %seen;
    foreach my $m (@modes)
    {
        foreach my $k (@{$m->[1]})
        {
            die "Unknown channel $k in $f->{model}\n" unless exists $pool{$k};
            push @order, $k unless $seen{$k}++;
        }
    }

    my $xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE FixtureDefinition>\n";
    $xml .= "<FixtureDefinition xmlns=\"http://www.qlcplus.org/FixtureDefinition\">\n";
    $xml .= " <Creator>\n  <Name>DMX Total Control</Name>\n  <Version>0.1.0</Version>\n  <Author>DMX Total Control</Author>\n </Creator>\n";
    $xml .= " <Manufacturer>Generic</Manufacturer>\n";
    $xml .= sprintf(" <Model>%s</Model>\n", xml_escape($f->{model}));
    $xml .= sprintf(" <Type>%s</Type>\n", xml_escape($f->{type}));
    $xml .= channel_xml($pool{$_}) foreach @order;

    foreach my $m (@modes)
    {
        my ($name, $chs, $heads) = @$m;
        $xml .= sprintf(" <Mode Name=\"%s\">\n", xml_escape($name));
        for my $i (0 .. $#$chs)
        {
            $xml .= sprintf("  <Channel Number=\"%d\">%s</Channel>\n", $i, xml_escape(channel_name($pool{$chs->[$i]})));
        }
        foreach my $h (@{$heads // []})
        {
            $xml .= "  <Head>\n" . join('', map { "   <Channel>$_</Channel>\n" } @$h) . "  </Head>\n";
        }
        $xml .= " </Mode>\n";
    }
    $xml .= "</FixtureDefinition>\n";

    (my $file = "Generic-$f->{model}.qxf") =~ s/[^A-Za-z0-9.+\-]+/-/g;
    open(my $fh, '>:encoding(UTF-8)', "$outdir/$file") or die "$outdir/$file: $!";
    print $fh $xml;
    close($fh);
    print "$file\n";
}
