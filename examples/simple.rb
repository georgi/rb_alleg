require "../ext/Allegro.so"
include Allegro

class Sprite

  attr_accessor :bitmap, :sequence, :x, :y, :step, :step_count, :width, :height, :velocity

  def initialize(seq)
    self.sequence = seq
    self.bitmap = seq[0]
    self.step = 0
    self.step_count = 8
    self.velocity = 10
  end

  def do_step
    self.step = (step + 1) % step_count
  end

  def move_left
    self.x -= velocity
    self.bitmap = sequence[4 + step / 4]
    do_step
  end

  def move_up
    self.y -= velocity
    self.bitmap = sequence[0 + step / 4]
    do_step
  end

  def move_right
    self.x += velocity
    self.bitmap = sequence[6 + step / 4]
    do_step
  end

  def move_down
    self.y += velocity
    self.bitmap = sequence[2 + step / 4]
    do_step
  end

  def render(bmp)
    bitmap.masked_stretch_blit(bmp, 0, 0, bitmap.width, bitmap.height, x, y, width, height)
  end

end

# Gfx.set_close_button_callback(method(:exit).to_proc)
Gfx.set_mode(:autodetect_windowed, 600, 400)

buf  = Bitmap.new(Screen.width, Screen.height)

seq = [:bk1, :bk2, :fr1, :fr2, :lf1, :lf2, :rt1, :rt2].map do |name|
  bmp = Bitmap.load("sprites/amg1_#{name}.png")
end

seq.each do |bmp|
  bmp.set_mask(Color.new(255, 255, 255, 255))
end

player = Sprite.new(seq)
player.x = 100
player.y = 100
player.width = 64
player.height = 64

loop do
  case
  when Key[:esc]   then exit
  when Key[:left]  then player.move_left
  when Key[:right] then player.move_right
  when Key[:up]    then player.move_up
  when Key[:down]  then player.move_down
  end

  buf.rectfill(0, 0, buf.width, buf.height, 0x000000)
  player.render(buf)

  Gfx.vsync
  buf.blit(Screen)
end
