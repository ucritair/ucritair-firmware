from imgui_bundle import imgui, imgui_node_editor as imgui_noded;
from ee_assets import AssetManager;
from ee_cowtools import *;
from typing import TypeVar, Generic, Type;

#########################################################
## DIALOGUE GRAPH

class GenID:
	eeid = iter(EEID());

	def init():
		GenID.eeid = iter(EEID());

	def __init__(self, cast_type):
		self.cast_type = cast_type;
	
	def __iter__(self):
		return self;

	def __next__(self):
		return self.cast_type((next(GenID.eeid)));

nid = iter(GenID(imgui_noded.NodeId));
pid = iter(GenID(imgui_noded.PinId));
lid = iter(GenID(imgui_noded.LinkId));

class GraphNode:
	def __init__(self, node):
		self.node = node;
		self.node_id = next(nid);
		self.in_pin_id = next(pid);
		self.out_pin_ids = [];
		for edge in node["edges"]:
			self.out_pin_ids.append(next(pid));

	def draw(self):
		imgui_noded.begin_node(self.node_id);
		
		imgui.text(self.node["name"]);
		max_line_width = max(len(self.node["name"]), max(len(s) for s in self.node["lines"]))+3;
		imgui.text("-"*max_line_width);
		for (idx, line) in enumerate(self.node["lines"]):
			imgui.text(f"{idx}: {line}");
		imgui.text("-"*max_line_width);

		imgui_noded.begin_pin(self.in_pin_id, imgui_noded.PinKind.input);
		imgui.text("[]");
		imgui_noded.end_pin();

		imgui.same_line();

		imgui.begin_group();
		for (idx, edge) in enumerate(self.node["edges"]):
			imgui.text(" " * (max_line_width - 5 - len(edge["text"])));
			imgui.same_line();
			imgui_noded.begin_pin(self.out_pin_ids[idx], imgui_noded.PinKind.output);
			imgui.text(edge["text"]);
			imgui_noded.end_pin();
		imgui.end_group();

		imgui_noded.end_node();

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

		self.context = imgui_noded.create_editor();

		self.dialogue_nodes = AssetManager.get_assets("dialogue");
		self.links = [];
	
	def __del__(self):
		imgui_noded.destroy_editor(self.context);

	def live():
		return DialogueGraph._ != None;

	def render():
		self = DialogueGraph._;
		if self == None:
			return;
	
		GenID.init();
		
		imgui.set_next_window_size(self.size);
		_, open = imgui.begin("Dialogue Graph", self != None, flags=self.window_flags);

		imgui_noded.set_current_editor(self.context);
		imgui_noded.begin(str(next(nid)), imgui.ImVec2(0, 0));

		graph_nodes = { n["name"] : GraphNode(n) for n in self.dialogue_nodes };

		for graph_node in graph_nodes.values():
			graph_node.draw();
			for (idx, edge) in enumerate(graph_node.node["edges"]):
				if edge["node"] in graph_nodes:
					that_node = graph_nodes[edge["node"]];
					link_id = next(lid);
					imgui_noded.link(link_id, that_node.in_pin_id, graph_node.out_pin_ids[idx]);
		for link in self.links:
			link_id, in_pin_id, out_pin_id = link;
			print(link);
			imgui_noded.link(link_id, in_pin_id, out_pin_id);
		
		if imgui_noded.begin_create():
			in_pin_id = imgui_noded.PinId();
			out_pin_id = imgui_noded.PinId();
	
			if imgui_noded.query_new_link(in_pin_id, out_pin_id):
				if in_pin_id and out_pin_id:
					if imgui_noded.accept_new_item():
						link = (next(lid), in_pin_id, out_pin_id);
						self.links.append(link);
						imgui_noded.link(link[0], link[1], link[2]);
			
			imgui_noded.end_create();

		imgui_noded.end();
		self.size = imgui.get_window_size();
		imgui.end();

		if not open:
			DialogueGraph._ = None;
