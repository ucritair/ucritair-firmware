from imgui_bundle import imgui;
from ee_assets import AssetManager, AssetDocument, DocumentHelper;
from ee_sprites import SpritePreview;
import ee_types;
from ee_tool_window import ToolWindow, ToolWindowRegistry;
from ee_sprite_explorer import SpriteExplorer;
from ee_file_explorer import FileExplorer;

class DocumentEditor:
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
							node[e.name] = DocumentEditor._render_node(e.name, e.T, node[e.name], id(node));
				imgui.tree_pop();
			imgui.pop_id();
			DocumentEditor._topmost_id = identifier;
			return node;
		
		elif isinstance(T, ee_types.List):
			imgui.push_id(identifier);
			if imgui.tree_node(title):
				for i in range(len(node)):
					imgui.push_id(i);
					node[i] = DocumentEditor._render_node(f"{title}[{i}]", T.T, node[i], id(node));
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
		if imgui.begin_popup_context_item(DocumentEditor._topmost_id):
			if imgui.menu_item_simple("Delete"):
				doc.delete_entry(idx);
			if imgui.menu_item_simple("Duplicate"):
				doc.duplicate_entry(idx);
			imgui.end_popup();
	
	def render(doc : AssetDocument):
		_, DocumentEditor._search_term = imgui.input_text("Search", DocumentEditor._search_term);
		subset = list(filter(lambda e: len(DocumentEditor._search_term) == 0 or DocumentEditor._search_term in e["name"], doc.instances));

		for (idx, entry) in enumerate(doc.instances):
			if entry in subset:
				root = doc.typist.root();
				name = DocumentHelper.get_name(doc, idx);
				number = DocumentHelper.get_number(doc, idx);
				DocumentEditor._render_node(f"{name} {number}####{id(entry)}", root.T, entry, id(entry));
				DocumentEditor._context_popup(doc, idx);