#!/usr/bin/env python3

import os;
from pathlib import Path;
from PIL import Image;
from stat import *;

import glfw;
from imgui_bundle import imgui;

from ee_cowtools import *;
from ee_assets import *;
import ee_context;
from ee_tool_window import ToolWindow, ToolWindowRegistry;

from ee_scene_editor import SceneEditor;
from ee_sprites import SpriteBank;
from ee_input import InputManager;
from ee_prop_editor import PropEditor;
from ee_dialogue import DialogueGraph, DialogueEditor;
from ee_recipes import RecipeEditor;
from ee_qr_encoder import QREncoder;
from ee_file_explorer import FileExplorer;
from ee_sprite_explorer import SpriteExplorer;
from ee_mesh2d_editor import Mesh2DEditor;
from ee_theme_editor import ThemeEditor;
from ee_anim_viewer import AnimationViewer;
from ee_document_editor import DocumentEditor;

if __name__ == "__main__":
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
			DocumentEditor.render(AssetManager.active_document);
		
		for tool in ToolWindowRegistry.all():
			tool.draw();
		
		imgui.end();

		ee_context.end_frame();

	ee_context.destroy_context();

