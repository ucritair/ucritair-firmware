from PIL import Image;
from pathlib import Path;

from ee_assets import *;
from ee_cowtools import *;

def _is_image_black(img):
	pixels = img.load();
	for y in range(img.height):
		for x in range(img.width):
			p = pixels[x, y];
			if p[3] >= 128 and p[0] > 16 and p[1] > 16 and p[2] > 16:
				return False;
	return True;

def _invert_black(img):
	pixels = img.load();
	inversion = [];
	result = Image.new(img.mode, img.size);
	for y in range(img.height):
		for x in range(img.width):
			p = pixels[x, y];
			r = 255 - p[0];
			g = 255 - p[1];
			b = 255 - p[2];
			a = p[3];
			inversion.append((r, g, b, a));
	result.putdata(inversion);
	return result;

class EditorSprite:
	def _cut_frame(self, i):
		box = (0, i * self.frame_height, self.raw_width, (i+1) * self.frame_height);
		return self.raw_image.crop(box);

	def __init__(self, path, frames):
		self.path = Path(path);
		self.raw_image = Image.open(self.path);
		self.raw_width = self.raw_image.width;
		self.raw_height = self.raw_image.height;

		self.frame_count = max(frames, 1);
		self.frame_width = self.raw_width;
		self.frame_height = self.raw_height // self.frame_count;
		self.frame_images = [self._cut_frame(i) for i in range(self.frame_count)];
		self.frame_textures = [make_texture(frame.tobytes(), frame.width, frame.height) for frame in self.frame_images];

		self.preview_width = self.raw_width * self.frame_count;
		self.preview_height = self.frame_height;
		self.preview_image = Image.new("RGBA", (self.preview_width, self.preview_height));
		for (idx, frame) in enumerate(self.frame_images):
			xy = (idx * self.frame_width, 0);
			self.preview_image.paste(frame, xy);
		if _is_image_black(self.preview_image):
			self.preview_image = _invert_black(self.preview_image);
		self.preview_texture = make_texture(self.preview_image.tobytes(), self.preview_width, self.preview_height);

class SpriteBank:
	mapping = {};

	def update():
		sprites = AssetManager.get_assets("sprite") + AssetManager.get_assets("tinysprite");
		for sprite in sprites:
			name = sprite["name"];
			asset_path = sprite["path"];
			real_path = AssetManager.get_document("sprite").directory / asset_path;
			frames = sprite["frames"] if "frames" in sprite else 1;
			if name not in SpriteBank.mapping or frames != SpriteBank.mapping[name].frame_count or real_path != SpriteBank.mapping[name].path:
				editor_sprite = EditorSprite(real_path, frames);
				SpriteBank.mapping[name] = editor_sprite;
				SpriteBank.mapping[asset_path] = editor_sprite;
	
	def get(name):
		if not name in SpriteBank.mapping:
			SpriteBank.update();
		return SpriteBank.mapping[name];

class SpritePreview:
	def draw(name):
		sprite = SpriteBank.get(name);
		imgui.image(sprite.preview_texture, (sprite.preview_width * 2, sprite.preview_height * 2));

	def draw_thumbnail(name, size):
		sprite = SpriteBank.get(name);
		aspect = sprite.frame_width / sprite.frame_height;
		imgui.image(sprite.frame_textures[0], (aspect * size, size));