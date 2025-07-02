class MD_writer:
	def __init__(self, path):
		self.md = open(path, "w");

	def text(self, s):
		self.md.write(f"{s}\n");

	def header(self, tier, s):
		if tier <= 0:
			self.text(s);
		else:
			tier = min(max(tier, 1), 3);
			header = "#" * (4-tier); 
			self.md.write(f"{header} {s}\n");

	def list(self, title, l):
		self.header(1, title);
		for item in list:
			self.md.write(f"- {item}\n");

	def newline(self):
		self.md.write("\n");
	
	def table_header(self, fields):
		for field in fields:
			self.md.write(f"|{field.title()}");
		self.md.write("|\n");
		for field in fields:
			self.md.write(f"|-");
		self.md.write("|\n");

	def table_row(self, values):
		for value in values:
			self.md.write(f"|{value}");
		self.md.write("|\n");

	def close(self):
		self.md.close();