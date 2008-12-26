require "mkmf"

unless have_library('png', 'png_read_info')
  puts "libpng not found"
  exit
end

unless have_library('z', 'deflateEnd')
  puts "libz not found"
  exit
end

dir_config "alleg"

unless have_library('alleg', 'install_allegro')
  puts "lib alleg not found, probably you should symlink liballeg-x.x.x.so to liballeg.so"
  exit
end

create_makefile "Allegro"
