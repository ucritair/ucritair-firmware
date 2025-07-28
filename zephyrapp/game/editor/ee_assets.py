from pathlib import Path;
import json;
import ee_types;
import copy;

#########################################################
## DOCUMENT MODEL

class AssetDocument:
	def __init__(self, path):
		self.path = Path(path);
		self.directory = self.path.parent;
		self.file = open(path, "r+");

		self.data = json.load(self.file);
		keys = self.data.keys();
		self.type = next(k for k in keys if k != "instances");
		self.typist = ee_types.Typist(self.type, self.data[self.type]);
		self.type_helper = ee_types.TypeHelper(self.typist);
		self.entries = self.data["instances"];
	
		self.id_set = set();
		if "id" in self.typist.root().T.keys():
			for entry in self.entries:
				self.id_set.add(entry["id"]);
	
	def take_free_id(self):
		M = max(self.id_set);
		i = 0;
		while i <= M:
			if not i in self.id_set:
				self.id_set.add(i);
				return i;
			i += 1;
		self.id_set.add(M+1);
		return M+1;
	
	def spawn_entry(self):
		new = self.type_helper.prototype();
		new["name"] = f"new_{self.type}";
		if "id" in new:
			new["id"] = self.take_free_id();
		self.entries.append(new);
	
	def duplicate_entry(self, idx):
		new = copy.deepcopy(self.entries[idx]);
		if "id" in new:
			new["id"] = self.take_free_id();
		self.entries.append(new);
	
	def delete_entry(self, idx):
		entry = self.entries[idx];
		if "id" in entry:
			self.id_set.remove(entry["id"]);
		del self.entries[idx];
	
	def refresh(self):
		for i in range(len(self.entries)):
			self.type_helper.rectify(self.entries[i]);
	
	def save(self):
		self.file.seek(0);
		self.file.truncate();
		self.file.write(json.dumps(self.data, indent=4));

class DocumentHelper:
	def get_name(doc, idx):
		node = doc.entries[idx];
		if "name" in node:
			return node["name"];
		elif "path" in node:
			return node["path"];
		elif "id" in node:
			return node["id"];
		return str(id(node));

	def get_number(doc, idx):
		node = doc.entries[idx];
		if "id" in node:
			return node["id"];
		return id(node);

	def get_rank(doc, idx):
		node = doc.entries[idx];
		data = doc.type_helper.collect(node);
		for (path, value) in data:
			element = doc.typist.search(path);
			if element.has_attribute("rank"):
				return value;
		return 0;

	def _sort_by(doc, f):
		sorted_entries = [node for (idx, node) in sorted(enumerate(doc.entries), key = lambda x: f(doc, x[0]))];
		doc.entries = sorted_entries;
	def sort_by_name(doc):
		DocumentHelper._sort_by(doc, DocumentHelper.get_name);
	def sort_by_number(doc):
		DocumentHelper._sort_by(doc, DocumentHelper.get_number);
	def sort_by_rank(doc):
		DocumentHelper._sort_by(doc, DocumentHelper.get_rank);

class AssetManager:
	directories = [];
	documents = [];
	active_document = None;

	def initialize(directories):
		AssetManager.directories = [Path(d) for d in directories];
		AssetManager.documents = [];

		for directory in AssetManager.directories:
			for filepath in directory.iterdir():
				ext = filepath.suffix;
				if ext == ".json":
					try:
						document = AssetDocument(filepath);
						AssetManager.documents.append(document);
						print(f"[AssetManager] Loaded {filepath}");
					except Exception as e:
						print(f"[AssetManager] Failed to load {filepath}!\n\t({e})");
	
	def types():
		return [document.type for document in AssetManager.documents];
	def has_type(name):
		return name in [document.type for document in AssetManager.documents];

	def get_document(name):
		return next(document for document in AssetManager.documents if document.type == name);

	def get_assets(asset_type):
		return next(document.entries for document in AssetManager.documents if document.type == asset_type);
	def search(asset_type, asset_name):
		return next(asset for asset in AssetManager.get_assets(asset_type) if asset["name"] == asset_name);

	