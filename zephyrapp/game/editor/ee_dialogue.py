from imgui_bundle import imgui, imgui_node_editor as imnodes;
from ee_assets import AssetManager;
from ee_cowtools import *;
from typing import TypeVar, Generic, Type;

#########################################################
## DIALOGUE GRAPH

class GenID:
	persist_eeid = iter(EEID());
	frame_eeid = iter(EEID());

	def __init__(self, cast_type, persist=False):
		self.cast_type = cast_type;
		self.persist = persist;
	
	def __iter__(self):
		return self;

	def __next__(self):
		if self.persist:
			return self.cast_type((next(GenID.persist_eeid)));
		return self.cast_type((next(GenID.frame_eeid)));

	def reset_frame_ids():
		GenID.frame_eeid = iter(EEID());

nid = iter(GenID(imnodes.NodeId));
pid = iter(GenID(imnodes.PinId));
lid = iter(GenID(imnodes.LinkId));

class GraphNode:
	def __init__(self, node):
		self.asset = node;
		self.node_id = next(nid);
		self.in_pin_id = next(pid);
		self.out_pin_ids = [];
		for edge in node["edges"]:
			self.out_pin_ids.append(next(pid));

	def draw(self):
		imnodes.begin_node(self.node_id);
		
		imnodes.begin_pin(self.in_pin_id, imnodes.PinKind.input);
		imgui.text(">");
		imnodes.end_pin();

		imgui.same_line();

		imgui.text(self.asset["name"]);
		max_line_width = max(len(self.asset["name"]), max(len(s) for s in self.asset["lines"]))+3;
		imgui.text("-"*max_line_width);
		for (idx, line) in enumerate(self.asset["lines"]):
			imgui.text(f"{idx}: {line}");
		imgui.text("-"*max_line_width);

		imgui.begin_group();
		for (idx, edge) in enumerate(self.asset["edges"]):
			imgui.text(" " * (max_line_width - 2 - len(edge["text"])));
			imgui.same_line();
			if edge["proc"] == "":
				imnodes.begin_pin(self.out_pin_ids[idx], imnodes.PinKind.output);
				imgui.text(edge["text"]);
				imnodes.end_pin();
			else:
				imgui.push_style_color(imgui.Col_.text, (0.75, 0.75, 0.75, 0.75));
				imgui.text(edge["text"]);
				imgui.pop_style_color();
		imgui.end_group();

		imnodes.end_node();

class GraphEdge:
	def __init__(self, from_pin_id, to_pin_id):
		self.link_id = next(lid);
		self.from_id = from_pin_id;
		self.to_id = to_pin_id;

class GraphRegistry:
	def __init__(self):
		self.nodes = [];
		self.links = [];

		self.by_name = {};
		self.by_node_id = {};
		self.by_pin_id = {};
		self.by_link_id = {};
	
	def register_node(self, node):
		self.nodes.append(node);
		self.by_name[node.asset["name"]] = node;
		self.by_node_id[node.node_id.id()] = node;
		self.by_pin_id[node.in_pin_id.id()] = node;
		for pin_id in node.out_pin_ids:
			self.by_pin_id[pin_id.id()] = node;
	
	def search_by_name(self, name):
		return self.by_name[name];

	def search_by_node_id(self, node_id):
		return self.by_node_id[node_id.id()];

	def search_by_pin_id(self, pin_id):
		return self.by_pin_id[pin_id.id()];

	def register_link(self, link_id, out_id, in_id):
		self.links.append((link_id, out_id, in_id));
		self.by_link_id[link_id.id()] = (link_id, out_id, in_id);

	def search_by_link_id(self, link_id):
		return self.by_link_id[link_id.id()];


class DialogueGraph:
	_ = None;

	def __init__(self):
		if DialogueGraph._ != None:
			return None;
		DialogueGraph._ = self;

		self.size = (640, 480);
		window_flag_list = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
		];
		self.window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);

		self.context = imnodes.create_editor();

		self.node_assets = AssetManager.get_assets("dialogue");
		self.links = [];
	
	def __del__(self):
		imnodes.destroy_editor(self.context);

	def live():
		return DialogueGraph._ != None;

	def render():
		self = DialogueGraph._;
		if self == None:
			return;
	
		GenID.reset_frame_ids();
		
		imgui.set_next_window_size(self.size);
		_, open = imgui.begin("Dialogue Graph", self != None, flags=self.window_flags);

		imnodes.set_current_editor(self.context);
		imnodes.begin(str(next(nid)), imgui.ImVec2(0, 0));

		registry = GraphRegistry();
		for asset in self.node_assets:
			node = GraphNode(asset);
			registry.register_node(node);

		for node in registry.nodes:
			node.draw();
			for (idx, edge) in enumerate(node.asset["edges"]):
				if edge["node"] != "" and edge["proc"] == "":
					next_node = registry.search_by_name(edge["node"]);
					if next_node != None:
						registry.register_link(next(lid), node.out_pin_ids[idx], next_node.in_pin_id);
		
		for link in registry.links:
			link_id, out_pin_id, in_pin_id = link;
			imnodes.link(link_id, out_pin_id, in_pin_id);
		
		if imnodes.begin_create():
			in_pin_id = imnodes.PinId();
			out_pin_id = imnodes.PinId();
	
			if imnodes.query_new_link(out_pin_id, in_pin_id):
				if out_pin_id and in_pin_id:
					if imnodes.accept_new_item():
						out_node = registry.search_by_pin_id(out_pin_id);
						out_edge_idx = out_node.out_pin_ids.index(out_pin_id);
						in_node = registry.search_by_pin_id(in_pin_id);
						out_node.asset["edges"][out_edge_idx]["node"] = in_node.asset["name"];			
			imnodes.end_create();
		
		if imnodes.begin_delete():
			del_lid = imnodes.LinkId();
			while imnodes.query_deleted_link(del_lid):
				if imnodes.accept_deleted_item():
					link_id, out_id, in_id = registry.search_by_link_id(del_lid);
					out_node = registry.search_by_pin_id(out_id);
					out_idx = out_node.out_pin_ids.index(out_id);
					out_node.asset["edges"][out_idx]["node"] = "";
			imnodes.end_delete();

		imnodes.end();
		self.size = imgui.get_window_size();
		imgui.end();

		if not open:
			DialogueGraph._ = None;
