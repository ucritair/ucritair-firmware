class HTML_writer:
	def __init__(self, path):
		self.path = path;
		self.file = open(self.path, "w");
		self.stack = [];
	
	def __make_tabs(self):
		return '\t'*len(self.stack);

	def __make_args(self, kwargs):
		arg_string = "";
		for key, value in kwargs.items():
			if key.startswith("_"):
				key = key[1:];
			key = key.replace("data_", "data-");
			arg_string += f" {key}=\"{value}\"";
		return arg_string;

	def __plain(self, s):
		self.file.write(f"{s}\n");

	def __open_tag(self, tag, **kwargs):
		self.file.write(f"{self.__make_tabs()}<{tag}{self.__make_args(kwargs)}>\n");
		self.stack.append((tag, kwargs));
	
	def __close_tag(self):
		tag, kwargs = self.stack.pop();
		self.file.write(f"{self.__make_tabs()}</{tag}>\n");
	
	def __one_line(self, key, value, **kwargs):
		self.file.write(f"{self.__make_tabs()}<{key}{self.__make_args(kwargs)}>{value}</{key}>\n");
	
	def __one_tag(self, tag, **kwargs):
		self.file.write(f"{self.__make_tabs()}<{tag}{self.__make_args(kwargs)} />\n");
	
	def __one_token(self, tag, **kwargs):
		self.file.write(f"{self.__make_tabs()}<{tag}{self.__make_args(kwargs)}>\n");
	
	def start(self, **kwargs):
		self.__one_token("!DOCTYPE HTML");
		self.__open_tag("html");
		self.__open_tag("head");

		if "title" in kwargs.keys():
			self.title = kwargs["title"];
			self.__one_line("title", self.title);
		else:
			self.title = self.path;
		
		if "stylesheet" in kwargs.keys():
			self.__one_tag("link", rel="stylesheet", href="sakura.css");
		
		self.__close_tag();
		self.__open_tag("body");
	
	def end(self):
		self.__close_tag();
		self.__close_tag();
		self.file.close();
	
	def heading(self, tier, s):
		self.__one_line(f"h{tier}", s, id=s.lower());

	def newline(self):
		self.__one_token("br");
	
	def start_text_block(self,):
		self.__open_tag("div", data_text_block=True);

	def text(self, s):
		head = self.stack[-1] if len(self.stack) > 0 else None;
		if head != None and "data_text_block" in head[1]:
			self.__plain(f"{self.__make_tabs()}{s}");
		else:
			self.__one_line("p", s);
	
	def end_text_block(self):
		self.__close_tag();
		self.in_text_block = False;
	
	def start_table(self, columns):
		self.__open_tag("table");

		num_cols = len(columns);
		self.__open_tag("colgroup");
		for i in range(num_cols):
			self.__one_tag("col", style=f"width: {100/num_cols}%");
		self.__close_tag();

		self.__open_tag("thead");
		self.__open_tag("tr");
		for column in columns:
			self.__one_line("th", column);
		self.__close_tag();
		self.__close_tag();
	
		self.__open_tag("tbody");

	def table_row(self, columns):
		self.__open_tag("tr");
		for column in columns:
			self.__one_line("td", column);
		self.__close_tag();
	
	def end_table(self):
		self.__close_tag();
		self.__close_tag();

	def horizontal_selector(self, entries):
		self.__open_tag("div");
		for entry in entries:
			title, link = entry;
			if title == self.title:
				self.__one_line("a", title, _class="active", href=link);
			else:
				self.__one_line("a", title, href=link);
		self.__close_tag();

	def vertical_selector(self, entries):
		self.__open_tag("div");
		for entry in entries:
			title, link = entry;
			if title == self.title:
				self.__one_line("a", title, _class="active", href=link);
			else:
				self.__one_line("a", title, href=link);
			self.__one_token("br");
		self.__close_tag();

	def image(self, path, **kwargs):
		self.__one_token("img", src=path, **kwargs);