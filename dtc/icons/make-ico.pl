#!/usr/bin/perl
#
# DMX Total Control - make-ico.pl <out.ico> <png>...
#
# A Windows .ico with the given PNG files inside (PNG icons: Windows Vista
# and later). Each PNG must be square, 256 px at most.
#
# Licensed under the Apache License, Version 2.0

use strict;
use warnings;

my ($out, @pngs) = @ARGV;
die "usage: make-ico.pl out.ico image.png...\n" unless $out && @pngs;

my @images;
for my $file (@pngs) {
    open(my $fh, '<:raw', $file) or die "$file: $!\n";
    local $/;
    my $data = <$fh>;
    close($fh);
    die "$file: not a PNG\n" unless substr($data, 0, 8) eq "\x89PNG\r\n\x1a\n";
    # the size is in the IHDR chunk, right after the signature
    my ($width, $height) = unpack('NN', substr($data, 16, 8));
    die "$file: ${width}x${height}, an icon is square and 256 px at most\n"
        if $width != $height || $width > 256;
    push @images, { size => $width, data => $data };
}

# the directory: 6 bytes, then 16 per image, then the images
my $offset = 6 + 16 * @images;
my $header = pack('vvv', 0, 1, scalar @images);
my $body = '';
for my $image (@images) {
    my $side = $image->{size} == 256 ? 0 : $image->{size};
    $header .= pack('CCCCvvVV', $side, $side, 0, 0, 1, 32, length($image->{data}), $offset);
    $offset += length($image->{data});
    $body .= $image->{data};
}

open(my $fh, '>:raw', $out) or die "$out: $!\n";
print $fh $header, $body;
close($fh);
