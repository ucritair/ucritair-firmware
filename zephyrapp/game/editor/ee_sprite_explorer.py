from imgui_bundle import imgui;
from ee_assets import AssetManager;
from ee_sprites import SpritePreview;

class SpriteExplorer:
	def __init__(self):
		self.target = None;
		self.result = None;
	
	def configure(self, target):
		self.target = target;

	def draw(self):
		listings = [s["name"] for s in AssetManager.get_assets("sprite")];
		listings.sort();
		
		for item in listings:
			imgui.begin_group();
			SpritePreview.draw_thumbnail(item, 48);
			imgui.same_line();
			if imgui.menu_item_simple(item):
				self.result = item;
			imgui.end_group();
		
	def is_targeting(self, target):
		return target == self.target;

	def should_close(self):
		return self.result != None;
		
	def get_result(self):
		return self.result;