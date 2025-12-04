#!/usr/bin/env python3

import os;
from pathlib import Path;
from PIL import Image;
from stat import *;
from collections import OrderedDict;

import glfw;
from imgui_bundle import imgui;

import ee_types;
from ee_cowtools import *;
from ee_canvas import Canvas;
from ee_assets import *;
import ee_context;
from ee_tool_window import ToolWindow, ToolWindowRegistry;

from ee_scene_editor import SceneEditor;
from ee_sprites import SpriteBank, SpritePreview;
from ee_input import InputManager;
from ee_prop_editor import PropEditor;
from ee_procs import ProcExplorer;
from ee_dialogue import DialogueGraph, DialogueEditor;
from ee_recipes import RecipeEditor;
from ee_qr_encoder import QREncoder;
from ee_file_explorer import FileExplorer;
from ee_sprite_explorer import SpriteExplorer;
from ee_mesh2d_editor import Mesh2DEditor;
from ee_theme_editor import ThemeEditor;
from ee_anim_viewer import AnimationViewer;


#########################################################
## DOCUMENT GUI

class DocumentRenderer:
	_search_term = "";
	_topmost_id = None;
	
	def _render_node(title, T, node, identifier):
		identifier = str(identifier)+title;

		if isinstance(T, ee_types.Object):
			imgui.push_id(identifier);
			if imgui.tree_node(title):
				if AssetManager.active_document.type == "sprite":
					SpritePreview.draw(node["name"]);
				for e in T.elements:
					if e.name in node:
						if not e.get_attribute("read-only"):
							node[e.name] = DocumentRenderer._render_node(e.name, e.T, node[e.name], id(node));
				imgui.tree_pop();
			imgui.pop_id();
			DocumentRenderer._topmost_id = identifier;
			return node;
		
		elif isinstance(T, ee_types.List):
			imgui.push_id(identifier);
			if imgui.tree_node(title):
				for i in range(len(node)):
					imgui.push_id(i);
					node[i] = DocumentRenderer._render_node(f"{title}[{i}]", T.T, node[i], id(node));
					imgui.pop_id();
				imgui.tree_pop();
				if imgui.button("+"):
					node.append(T.T.prototype());
			imgui.pop_id();
			return node;
		
		elif isinstance(T, ee_types.Asset):
			match T.name:
				case "sprite":
					SpritePreview.draw(node);

			imgui.text(title);
			imgui.same_line();
			
			match T.name:
				case "sprite":
					_, result = imgui.input_text(f"##{identifier}", node);
					imgui.same_line();
					spexp = ToolWindowRegistry.lookup(SpriteExplorer);
					if imgui.button(f"...##{identifier}") and not spexp.is_open():
						spexp.open();
						spexp.get().configure(node);
					if spexp.is_open() and spexp.get().is_targeting(node):
						harvest = spexp.get_result();
						result = harvest if harvest != None else result;
					return result;
				
				case "proc":
					_, result = imgui.input_text(f"##{identifier}", str(node));
					imgui.same_line();
					if imgui.button(f"...##{identifier}"):
						ProcExplorer("src/procs");
					result = ProcExplorer.render(node) if ProcExplorer.live() else result;
					return result;
				
				case _:
					if T.name in AssetManager.types():
						result = node;
						if imgui.begin_combo(f"##{identifier}", node):
							assets = AssetManager.get_assets(T.name).copy();
							assets.append(None);
							for asset in assets:
								name = asset["name"] if asset != None else "##";
								selected = result == name;
								if imgui.selectable(name, selected)[0]:
									result = name
								if selected:
									imgui.set_item_default_focus();
							imgui.end_combo();
						return result;
					else:
						_, result = imgui.input_text(f"##{identifier}", str(node));
						return result;
		
		elif isinstance(T, ee_types.File):
			imgui.text(title);
			imgui.same_line();

			_, result = imgui.input_text(f"##{identifier}", str(node));
			imgui.same_line();
			fexp = ToolWindowRegistry.lookup(FileExplorer);
			if imgui.button(f"...##{identifier}") and not fexp.is_open():
				fexp.open();
				fexp.get().configure(node, AssetManager.active_document.directory, T.pattern);
			if fexp.is_open() and fexp.get().is_targeting(node):
				harvest = fexp.get_result();
				result = harvest if harvest != None else result;
			return result;

		elif isinstance(T, ee_types.Enum):
			imgui.text(title);
			imgui.same_line();

			result = node;
			if imgui.begin_combo(f"##{identifier}", result):
				for item in T.values:
					selected = result == item;
					if imgui.selectable(item, selected)[0]:
						result = item;
					if selected:
						imgui.set_item_default_focus();	
				imgui.end_combo();
			return result;

		elif isinstance(T, ee_types.Primitive):
			imgui.text(title);
			imgui.same_line();

			match type(T):
				case ee_types.Int:
					_, result = imgui.input_int(f"##{identifier}", int(node));
					return result;
				case ee_types.Float:
					_, result = imgui.input_float(f"##{identifier}", float(node));
					return result;
				case ee_types.Bool:
					return imgui.checkbox(f"##{identifier}", bool(node))[1];
				case ee_types.String:
					_, result = imgui.input_text(f"##{identifier}", str(node));
					return result;

	def _context_popup(doc : AssetDocument, idx):
		if imgui.begin_popup_context_item(DocumentRenderer._topmost_id):
			if imgui.menu_item_simple("Delete"):
				doc.delete_entry(idx);
			if imgui.menu_item_simple("Duplicate"):
				doc.duplicate_entry(idx);
			imgui.end_popup();
	
	def render(doc : AssetDocument):
		_, DocumentRenderer._search_term = imgui.input_text("Search", DocumentRenderer._search_term);
		subset = list(filter(lambda e: len(DocumentRenderer._search_term) == 0 or DocumentRenderer._search_term in e["name"], doc.instances));

		for (idx, entry) in enumerate(doc.instances):
			if entry in subset:
				root = doc.typist.root();
				name = DocumentHelper.get_name(doc, idx);
				number = DocumentHelper.get_number(doc, idx);
				DocumentRenderer._render_node(f"{name} {number}####{id(entry)}", root.T, entry, id(entry));
				DocumentRenderer._context_popup(doc, idx);


#########################################################
## EDITOR GUI

ee_context.create_context(1920, 1080);
AssetManager.initialize(["sprites", "sounds", "meshes", "data"]);
InputManager.initialize(ee_context.glfw_handle, ee_context.imgui_impl);

window_flag_list = [
	imgui.WindowFlags_.no_saved_settings,
	imgui.WindowFlags_.no_move,
	imgui.WindowFlags_.no_resize,
	imgui.WindowFlags_.no_nav_inputs,
	imgui.WindowFlags_.no_nav_focus,
	imgui.WindowFlags_.no_collapse,
	imgui.WindowFlags_.no_background,
	imgui.WindowFlags_.no_bring_to_front_on_focus,
];
window_flags = foldl(lambda a, b : a | b, 0, window_flag_list);

splash_img = Image.open("editor/splash.png");
splash_tex = make_texture(splash_img.tobytes(), splash_img.width, splash_img.height);
splash_flag_list = [
	imgui.WindowFlags_.no_scrollbar,
	imgui.WindowFlags_.no_scroll_with_mouse
];
splash_flags = foldl(lambda a, b : a | b, 0, splash_flag_list);

tool_flags = [
			imgui.WindowFlags_.no_saved_settings,
			imgui.WindowFlags_.no_collapse,
];

ToolWindowRegistry.register(ToolWindow(QREncoder, "QR Encoder", flags=tool_flags));
ToolWindowRegistry.register(ToolWindow(FileExplorer, "File Explorer", flags=tool_flags, hidden=True));
ToolWindowRegistry.register(ToolWindow(SpriteExplorer, "Sprite Explorer", flags=tool_flags, hidden=True));
ToolWindowRegistry.register(ToolWindow(DialogueEditor, "Dialogue Editor", size=(1000, 600), flags=tool_flags));
ToolWindowRegistry.register(ToolWindow(Mesh2DEditor, "Mesh2DEditor", flags=tool_flags));
ToolWindowRegistry.register(ToolWindow(ThemeEditor, "Theme Editor", flags=tool_flags));
ToolWindowRegistry.register(ToolWindow(AnimationViewer, "Animation Viewer", flags=tool_flags));
ToolWindowRegistry.register(ToolWindow(SceneEditor, "Scene Editor", size=(1280, 720), flags=tool_flags));
ToolWindowRegistry.register(ToolWindow(PropEditor, "Prop Editor", flags=tool_flags));
ToolWindowRegistry.register(ToolWindow(DialogueGraph, "Dialogue Graph", size=(1280, 720), flags=tool_flags));
ToolWindowRegistry.register(ToolWindow(RecipeEditor, "Recipe Editor", flags=tool_flags));

while ee_context.alive():
	ee_context.begin_frame();
	
	SpriteBank.update();
	InputManager.update();
	if AssetManager.active_document != None:
		AssetManager.active_document.refresh();

	if InputManager.is_held(glfw.KEY_LEFT_SUPER) and InputManager.is_pressed(glfw.KEY_S):
		for document in AssetManager.documents:
			print(f"Saving {document.path}!");
			document.save();

	imgui.set_next_window_pos((0, 0));
	imgui.begin("Editor", flags=window_flags | (splash_flags if AssetManager.active_document == None else 0));

	if imgui.begin_main_menu_bar():
		if imgui.begin_menu("File"):
			if imgui.begin_menu("Open"):
				for document in AssetManager.documents:
					selected = document == AssetManager.active_document;
					if imgui.menu_item_simple(str(document.path), selected = selected):
						AssetManager.active_document = document;
				imgui.end_menu();
			if imgui.menu_item_simple("Save"):
				for document in AssetManager.documents:
					print(f"Saving {document.path}!");
					document.save();
			if imgui.menu_item_simple("Close", enabled=AssetManager.active_document != None):
				AssetManager.active_document = None;
			imgui.end_menu();

		if imgui.begin_menu("Assets", enabled=AssetManager.active_document != None):
			if imgui.begin_menu("Sort", enabled=AssetManager.active_document != None):
				if imgui.menu_item_simple("Name"):
					DocumentHelper.sort_by_name(AssetManager.active_document);
				if imgui.menu_item_simple("Number"):
					DocumentHelper.sort_by_number(AssetManager.active_document);
				if imgui.menu_item_simple("Rank"):
					DocumentHelper.sort_by_rank(AssetManager.active_document);
				imgui.end_menu();
			if imgui.menu_item_simple("New"):
				AssetManager.active_document.spawn_entry();
			imgui.end_menu();
		
		if imgui.begin_menu("Tools"):
			for tool in ToolWindowRegistry.all():
				if not tool.hidden and imgui.menu_item_simple(tool.title):
					tool.open();
			imgui.end_menu();
		imgui.end_main_menu_bar();
	
	if AssetManager.active_document == None:
		imgui.set_scroll_x(0);
		imgui.set_scroll_y(0);
		imgui.image(splash_tex, (splash_img.width, splash_img.height));
	else:
		DocumentRenderer.render(AssetManager.active_document);
	
	for tool in ToolWindowRegistry.all():
		tool.draw();
	
	imgui.end();

	ee_context.end_frame();

ee_context.destroy_context();

