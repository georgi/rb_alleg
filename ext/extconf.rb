require "mkmf"

unless have_library('png', 'png_read_info')
  puts "libpng not found"
  exit
end

unless have_library('z', 'deflateEnd')
  puts "libz not found"
  exit
end

unless have_library('alleg', 'install_allegro')
  puts "lib alleg not found, probably you should symlink liballeg-x.x.x.so to liballeg.so"
  exit
end

unless find_library("jpgal", "load_jpg", "../lib/")
  puts "lib jpgalleg not found"
  exit
end

unless find_library("ldpng", "load_png", "../lib/")
  puts "lib ldpng not found"
  exit
end

create_makefile "Allegro"
