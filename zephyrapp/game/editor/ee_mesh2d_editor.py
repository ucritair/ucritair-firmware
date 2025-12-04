from imgui_bundle import imgui;
from ee_canvas import Canvas;
from ee_assets import AssetManager;
from collections import OrderedDict;
import ee_context;

class Mesh2DEditor:
	def __init__(self):
		self.polyline = [];
		self.edge_buffer = [];
		self.trash = None;

		self.canvas = Canvas(240, 120, 2);
		self.canvas_scale = 2;

		self.grid_dist = 4;
		self.show_grid = True;
		self.show_verts = False;
		
		self.edit_mode = False;
		self.was_left_click = False;
		self.was_right_click = False;
		self.last_edge_closed = True;
		self.last_click = imgui.ImVec2(-1, -1);
	
		self.mesh_pool = AssetManager.get_assets("mesh2d");
		self.mesh = self.mesh_pool[0] if len(self.mesh_pool) > 0 else None;
		if(not self.mesh is None):
			self.open_mesh(self.mesh);
	
	def in_bounds(self, p):
		if p.x < 0:
			return False;
		if p.x >= self.canvas.width:
			return False;
		if p.y < 0:
			return False;
		if p.y >= self.canvas.height:
			return False;
		return True;

	def open_mesh(self, mesh):
		self.mesh = mesh;
		self.polyline = [];
		self.edge_buffer = [];
		if not mesh is None:
			V = mesh['verts'];
			E = mesh['edges'];
			n_E = mesh['edge_count'];
			e_idx = 0;
			while e_idx < n_E:
				v0_idx = E[e_idx*2+0];
				v1_idx = E[e_idx*2+1];
				v0_x = V[v0_idx*2+0];
				v0_y = V[v0_idx*2+1];
				v1_x = V[v1_idx*2+0];
				v1_y = V[v1_idx*2+1];
				self.polyline.append([
					imgui.ImVec2(v0_x, v0_y),
					imgui.ImVec2(v1_x, v1_y)
				]);
				e_idx += 1;
	
	def shunt(self):
		corner = imgui.ImVec2(256, 256);
		for [v0, v1] in self.polyline:
			corner.x = min(v0.x, v1.x, corner.x);
			corner.y = min(v0.y, v1.y, corner.y);
		for [v0, v1] in self.polyline:
			v0 -= corner;
			v1 -= corner;
	def center(self):
		tl = imgui.ImVec2(256, 256);
		br = imgui.ImVec2(-1, -1);
		for [v0, v1] in self.polyline:
			tl.x = min(v0.x, v1.x, tl.x);
			tl.y = min(v0.y, v1.y, tl.y);
			br.x = max(v0.x, v1.x, br.x);
			br.y = max(v0.y, v1.y, br.y);
		middle = (tl+br)/2;
		shift = imgui.ImVec2(120, 60) - middle;
		for [v0, v1] in self.polyline:
			v0 += shift;
			v1 += shift;
	def translate(self, delta):
		for [v0, v1] in self.polyline:
			v0 += delta;
			v1 += delta;

	def write_mesh(self):
		polyline = [((int(v0.x), int(v0.y)), (int(v1.x), int(v1.y))) for [v0, v1] in self.polyline];
		polyline = list(set(polyline));
		flat_polyline = [];
		for p in polyline:
			flat_polyline += p;
		
		vert_map = OrderedDict();
		for v in flat_polyline:
			if not v in vert_map:
				vert_map[v] = len(vert_map);
		
		flat_edges = [];
		for v in flat_polyline:
			flat_edges.append(vert_map[v]);
		flat_verts = [];
		for v in vert_map.keys():
			flat_verts += [v[0], v[1]];
		
		self.mesh['verts'] = flat_verts.copy();
		self.mesh['vert_count'] = len(flat_verts) // 2;
		self.mesh['edges'] = flat_edges.copy();
		self.mesh['edge_count'] = len(flat_edges) // 2;
	
	def close_mesh(self):
		if not self.mesh is None:
			self.write_mesh();
			self.mesh = None;
	
	def buffer_vertex(self, v):
		self.edge_buffer.append(imgui.ImVec2(v));
		if len(self.edge_buffer) == 2:
			l = self.edge_buffer[0];
			if v != l:
				self.polyline.append(self.edge_buffer);
				self.edge_buffer = [imgui.ImVec2(v)];
			else:
				self.edge_buffer = [];
	
	def delete_vertex(self, v):
		if v in self.edge_buffer:
			self.edge_buffer = [];
		i = 0;
		while i < len(self.polyline):
			if v in self.polyline[i]:
				del self.polyline[i];
				i -= 1;
			i += 1;
	
	def snap(self, p):
		p.x /= self.grid_dist;
		p.y /= self.grid_dist;
		p.x = round(p.x);
		p.y = round(p.y);
		p.x *= self.grid_dist;
		p.y *= self.grid_dist;
		p.x = min(max(p.x, 0), self.canvas.width-1);
		p.y = min(max(p.y, 0), self.canvas.height-1);
		return p;

	def draw(self):
		if imgui.begin_combo(f"Meshes", self.mesh["name"] if not self.mesh is None else "None"):
			for mesh in self.mesh_pool:
				selected = mesh == self.mesh;
				if imgui.selectable(mesh["name"], selected)[0]:
					self.close_mesh();
					self.open_mesh(mesh);
				if selected:
					imgui.set_item_default_focus();
			imgui.end_combo();

		canvas_pos = imgui.get_cursor_screen_pos();
		mouse_pos = ee_context.imgui_io.mouse_pos;
		brush_pos = mouse_pos - canvas_pos;
		brush_pos /= self.canvas_scale;
		in_bounds = self.in_bounds(brush_pos);
		vertex = self.snap(brush_pos);

		if self.edit_mode:
			if self.last_click == imgui.ImVec2(-1, -1):
				self.last_click = vertex;
			if in_bounds:
				if imgui.is_mouse_down(0) and not self.was_left_click:
					if ee_context.imgui_io.key_shift:
						tl = imgui.ImVec2(256, 256);
						br = imgui.ImVec2(-1, -1);
						for [v0, v1] in self.polyline:
							tl.x = min(v0.x, v1.x, tl.x);
							tl.y = min(v0.y, v1.y, tl.y);
							br.x = max(v0.x, v1.x, br.x);
							br.y = max(v0.y, v1.y, br.y);
						middle = (tl+br)/2;
						delta = vertex - middle;
						self.translate(delta);
						self.last_click = vertex;
					else:
						self.buffer_vertex(vertex);
						self.was_left_click = True;
				if not imgui.is_mouse_down(0):
					self.was_left_click = False;
				
				if imgui.is_mouse_down(1) and not self.was_right_click:
					self.delete_vertex(vertex);
					self.was_right_click = True;
				if not imgui.is_mouse_down(1):
					self.was_right_click = False;
		
		self.canvas.clear((0, 0, 0));
		if self.show_grid:
			for y in range(0, self.canvas.height, self.grid_dist):
				for x in range(0, self.canvas.width, self.grid_dist):
					self.canvas.draw_pixel(x, y, (128, 128, 128));
			self.canvas.draw_rect_old(0, 0, self.canvas.width, self.canvas.height, (128, 128, 128));
		
		for [v0, v1] in self.polyline:
			self.canvas.draw_line(v0.x, v0.y, v1.x, v1.y, (255, 255, 255));
			if self.show_verts:
				self.canvas.draw_circle(v0.x, v0.y, 1, (255, 255, 255));
				self.canvas.draw_circle(v1.x, v1.y, 1, (255, 255, 255));
		
		if self.edit_mode:
			if len(self.edge_buffer) > 0:
				last = self.edge_buffer[-1];
				self.canvas.draw_line(last.x, last.y, vertex.x, vertex.y, (255, 255, 255));
			self.canvas.draw_circle(vertex.x, vertex.y, 2, (255, 255, 255));
			
		self.canvas.render();
		_, self.show_grid = imgui.checkbox("Show grid", self.show_grid);
		imgui.same_line();
		_, self.show_verts = imgui.checkbox("Show verts", self.show_verts);
		imgui.same_line();
		imgui.push_item_width(72);
		_, self.grid_dist = imgui.input_int("Cell size", self.grid_dist);
		imgui.pop_item_width();

		_, self.edit_mode = imgui.checkbox("Edit Mode", self.edit_mode);
		if self.edit_mode:
			imgui.same_line();
			if imgui.button("Shunt"):
				self.shunt();
			imgui.same_line();
			if imgui.button("Center"):
				self.center();
			if imgui.button("Clear"):
				self.trash = self.polyline, self.edge_buffer;
				self.polyline = [];
				self.edge_buffer = [];
			imgui.same_line();
			if imgui.button("Restore") and self.trash != None:
				self.polyline, self.edge_buffer = self.trash;
				self.trash = None;

	def __del__(self):
		self.close_mesh();