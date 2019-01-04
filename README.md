# S3DatPacker
Tool to unpack/repack Settlers 3 dat archives

## Usage
./S3DatPacker <option> <filename>

options:
-u Unpack dat archive
-r Repack dat archive

When repacking, the result is saved as "test.dat".

It can only deal correctly with specific types of bitmaps right now, so when changing existing/adding new ones, 
they should adhere to the following scheme to prevent unintended results:

- Sprites, textures and menu textures are 16-bit RGB565, where black means transparent
- Color-indexed sprites are 24-bit, where pink (->RGB(255,0,255)) means transparent
- Shadows are 16-bit RGB565, where pink means shadow and black means transparent

tl;dr keep the bitmaps in the same format when you change them

## Notes
Only archives from the GFX folder are supported for now

## Changelog
-v0.5 - original release
