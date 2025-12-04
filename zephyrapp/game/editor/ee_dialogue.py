from imgui_bundle import imgui, imgui_node_editor as imnodes;
from ee_assets import AssetManager;
from ee_cowtools import *;
from typing import TypeVar, Generic, Type;
from ee_procs import ProcExplorer;

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
		imgui.push_id(str(id(self.asset)));
		
		imnodes.begin_pin(self.in_pin_id, imnodes.PinKind.input);
		imgui.text("[]");
		imnodes.end_pin();

		imgui.same_line();

		imgui.text(self.asset["name"]);

		max_line_chars = max(len(self.asset["name"]), max(len(s) for s in self.asset["lines"]) if len(self.asset["lines"]) > 0 else 0)+3;
		max_line_width = max_line_chars * 8;

		trash = [];
		for (idx, line) in enumerate(self.asset["lines"]):
			imgui.set_next_item_width(max_line_width);
			_, self.asset["lines"][idx] = imgui.input_text(f"##line{idx}", self.asset["lines"][idx]);
			imgui.same_line();
			if imgui.button(f"-##line{idx}"):
				trash.append(idx);
		imgui.dummy((max_line_width, 0));
		imgui.same_line();
		if imgui.button("+##line"):
			self.asset["lines"].append("");
		for idx in trash:
			del self.asset["lines"][idx];
		
		imgui.new_line();

		trash = [];
		imgui.begin_group();
		for (idx, edge) in enumerate(self.asset["edges"]):
			imgui.dummy((max_line_width-len(edge["text"])*8-36, 0));
			imgui.same_line();
			if imgui.button(f"-##edge{idx}"):
				trash.append(idx);
			imgui.same_line();
			if edge["proc"] == "":
				imgui.set_next_item_width((len(edge["text"])+1) * 8);
				_, edge["text"] = imgui.input_text(f"##edge{idx}", edge["text"]);
				imgui.same_line();
				imnodes.begin_pin(self.out_pin_ids[idx], imnodes.PinKind.output);
				imgui.text("[]");
				imnodes.end_pin();
			else:
				imgui.push_style_color(imgui.Col_.text, (0.75, 0.75, 0.75, 0.75));
				imgui.text(edge["text"]);
				imgui.pop_style_color();
		imgui.dummy((max_line_width, 0));
		imgui.same_line();
		if imgui.button("+##edge"):
			self.asset["edges"].append({"text" : "", "proc" : "", "node" : ""});
		for idx in trash:
			del self.asset["edges"][idx];
		imgui.end_group();

		imgui.pop_id();
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

def make_verdant(first_tree):
	forest = [];
	stack = [first_tree];
	visited = set();

	while len(stack) > 0:
		head = stack.pop(-1);
		forest.append(head);
		visited.add(id(head));
		for edge in head["edges"]:
			next_node = AssetManager.search("dialogue", edge["node"]);
			if next_node != None and not id(next_node) in visited:
				stack.append(next_node);
	
	return forest;

class DialogueGraph:
	def __init__(self):
		self.context = imnodes.create_editor();

		self.node_bank = AssetManager.get_assets("dialogue");
		self.node_assets = [];
		self.links = [];
	
		self.node_buffer = None;
	
	def __del__(self):
		imnodes.destroy_editor(self.context);

	def draw(self):
		GenID.reset_frame_ids();
		
		imgui.begin_group();
		imgui.set_next_item_width(160);
		if imgui.begin_combo(f"##{id(self.node_buffer)}", self.node_buffer["name"] if self.node_buffer != None else ""):
			for asset in self.node_bank:
				selected = self.node_buffer != None and asset == self.node_buffer;
				if imgui.selectable(asset["name"], selected)[0]:
					self.node_buffer = asset;
				if selected:
					imgui.set_item_default_focus();
			imgui.end_combo();
		imgui.same_line();
		if imgui.button(f"+##{id(self.node_buffer)}"):
			forest = make_verdant(self.node_buffer);
			for tree in forest:
				if not tree in self.node_assets:
					self.node_assets.append(tree);

		trash = []
		for node in self.node_assets:
			if imgui.button(f"{node["name"]} x"):
				trash.append(node);
		self.node_assets = [n for n in self.node_assets if not n in trash];

		imgui.end_group();
		imgui.same_line();

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


#########################################################
## DIALOGUE EDITOR

class DialogueEditor:
	def __init__(self):		
		self.nodes = AssetManager.get_assets("dialogue");
		self.node = None;
		self.trash = [];
	
	def node_selector(self):
		if imgui.begin_combo(f"Dialogue", self.node["name"] if not self.node is None else "None"):
			for node in self.nodes:
				selected = node == self.node;
				if imgui.selectable(node["name"], selected)[0]:
					self.node = node;
				if selected:
					imgui.set_item_default_focus();
			imgui.end_combo();

	def draw(self):
		for key, idx in self.trash:
			del self.node[key][idx];
		self.trash = [];

		self.node_selector();

		if self.node != None:
			if imgui.collapsing_header("Lines"):
				for idx in range(len(self.node["lines"])):
					_, self.node["lines"][idx] = imgui.input_text(str(idx), self.node["lines"][idx]);
					imgui.same_line();
					if imgui.button(f"Delete##line{idx}"):
						self.trash.append(("lines", idx));
				if imgui.button("New##line"):
					self.node["lines"].append("Hello, world!");

			if imgui.collapsing_header("Edges"):
				for (idx, edge) in enumerate(self.node["edges"]):
					imgui.push_id(idx);
					if imgui.tree_node(f"{edge["text"]}####{idx}"):
						_, edge["text"] = imgui.input_text("Text", edge["text"]);
						if imgui.begin_combo("Node", edge["node"]):
							for node in self.nodes:
								selected = node["name"] == edge["node"];
								if imgui.selectable(node["name"], selected)[0]:
									edge["node"] = node["name"];
								if selected:
									imgui.set_item_default_focus();
							imgui.end_combo();
						
						imgui.text(f"proc");
						imgui.same_line();
						_, edge["proc"] = imgui.input_text("##", str(edge["proc"]));
						imgui.same_line();
						if imgui.button("..."):
							ProcExplorer("src/world");
						if ProcExplorer.live():
							edge["proc"] = ProcExplorer.render(edge["proc"]);

						imgui.tree_pop();
					imgui.pop_id();
					if imgui.button(f"Delete##edge{idx}"):
						self.trash.append(("edges", idx));
				if imgui.button("New##edge"):
					new = AssetManager.get_document("dialogue").type_helper.prototype("/edges", True);
					self.node["edges"].append(new);