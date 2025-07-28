from PIL import Image;

from ee_assets import *;
from ee_cowtools import *;

class EditorSprite:
	def _cut_frame(self, i):
		box = (0, i * self.frame_height, self.raw_width, (i+1) * self.frame_height);
		return self.raw_image.crop(box);

	def __init__(self, sprite_name):
		sprite = AssetManager.search("sprite", sprite_name);

		self.path = Path("sprites")/Path(sprite["path"]);
		self.raw_image = Image.open(self.path);
		self.raw_width = self.raw_image.width;
		self.raw_height = self.raw_image.height;

		self.frame_count = max(sprite["frames"], 1);
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
		self.preview_texture = make_texture(self.preview_image.tobytes(), self.preview_width, self.preview_height);

class SpriteBank:
	mapping = {};

	def initialize():
		SpriteBank.mapping = {};
		sprites = AssetManager.get_assets("sprite");
		for sprite in sprites:
			esprite = EditorSprite(sprite["name"]);
			SpriteBank.mapping[sprite["name"]] = esprite;
			SpriteBank.mapping[sprite["path"]] = esprite;

	def get(name):
		return SpriteBank.mapping[name];