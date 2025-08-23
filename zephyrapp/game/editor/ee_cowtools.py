import OpenGL;
OpenGL.FULL_LOGGING = True;
from OpenGL.GL import *;
from imgui_bundle import imgui;
from ee_assets import AssetManager;

def foldl(f, acc, xs):
	if len(xs) == 0:
		return acc;
	else:
		h, t = xs[0], xs[1:];
		return foldl(f, f(acc, h), t);

def foldr(f, acc, xs):
	if len(xs) == 0:
		return acc;
	else:
		h, t = xs[0], xs[1:];
		return f(h, foldr(f, acc, t));

def make_texture(buffer, width, height):
	texture = glGenTextures(1);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, 	GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	return texture;

def aabb_point_intersect(aabb, point):
	x0, y0, x1, y1 = aabb;
	x, y = point;
	if x < x0 or x > x1:
		return False;
	if y < y0 or y > y1:
		return False;
	return True;

def move_aabb(aabb, delta):
	x0, y0, x1, y1 = aabb;
	dx, dy = delta;
	return x0+dx, y0+dy, x1+dx, y1+dy;

def enforce_key(dict, key, default_value):
	if not key in dict:
		dict[key] = default_value;

def clamp(v, a, b):
	return min(max(v, a), b);

def aabb_contains(aabb_0, aabb_1):
	x0, y0, x1, y1 = aabb_1;
	a = x0, y0;
	b = x1, y0;
	c = x1, y1;
	d = x0, y1;
	return (
		aabb_point_intersect(aabb_0, a) and
		aabb_point_intersect(aabb_0, b) and
		aabb_point_intersect(aabb_0, c) and
		aabb_point_intersect(aabb_0, d)
	);

def aabb_equals(a, b):
	for i in range(4):
		if a[i] != b[i]:
			return False;
	return True;

def value_with_effect(value, effect_expr):
	return value;

class EEID:
	def __iter__(self):
		self.eeid = 0;
		return self;

	def __next__(self):
		result = self.eeid;
		self.eeid += 1;
		return result;

def imgui_selector(ident, candidates, value, name_f = lambda x: x):
	if imgui.begin_combo(f"##{ident}", name_f(value)):
		for candidate in candidates:
			selected = candidate == value;
			if imgui.selectable(name_f(candidate), selected)[0]:
				value = candidate;
			if selected:
				imgui.set_item_default_focus();
		imgui.end_combo();
	return value;

def imgui_asset_selector(ident, asset_type, asset):
	return imgui_selector(
		ident,
		AssetManager.get_assets(asset_type), asset,
		lambda x: x["name"] if x != None else "None"
	);

def imgui_enum_selector(ident, enum_type, value):
	return imgui_selector(
		ident,
		enum_type, value,
		lambda x: x.name
	);

def imgui_aabb_xywh(ident, aabb):
	x0, y0, x1, y1 = aabb;
	w, h = x1-x0, y1-y0;
	x0, y0 = imgui.input_int2(f"X Y##{ident}", [int(x0), int(y0)])[1];
	w, h = imgui.input_int2(f"W H##{ident}", [int(w), int(h)])[1];
	return x0, y0, x0+w, y0+h;

def imgui_aabb_xyxy(ident, aabb):
	x0, y0, x1, y1 = aabb;
	x0, y0 = imgui.input_int2(f"X0 Y0##{ident}", [int(x0), int(y0)])[1];
	x1, y1 = imgui.input_int2(f"X1 Y1##{ident}", [int(x1), int(y1)])[1];
	return x0, y0, x1, y1;