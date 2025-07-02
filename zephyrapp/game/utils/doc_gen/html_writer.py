class HTML_writer:
	def __init__(self, path):
		self.file = open(path, "w");
		self.stack = [];
	
	def __make_tabs(self):
		return '\t'*len(self.stack);

	def __make_args(self, kwargs):
		arg_string = "";
		for key, value in kwargs.items():
			arg_string += f" {key}=\"{value}\"";
		return arg_string;

	def __plain(self, s):
		self.file.write(f"{s}\n");

	def __open_tag(self, tag, **kwargs):
		self.file.write(f"{self.__make_tabs()}<{tag}{self.__make_args(kwargs)}>\n");
		self.stack.append(tag);
	
	def __close_tag(self):
		tag = self.stack.pop();
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
			self.__one_line("title", kwargs["title"]);
		if "stylesheet" in kwargs.keys():
			self.__one_tag("link", rel="stylesheet", href="sakura.css");
		self.__close_tag();
		self.__open_tag("body");
	
	def end(self):
		self.__close_tag();
		self.__close_tag();
		self.file.close();
	
	def heading(self, tier, s):
		tier = min(max(tier, 1), 3);
		self.__one_line(f"h{tier}", s, id=s);

	def newline(self):
		self.__one_token("br");
	
	def start_text_block(self,):
		self.__open_tag("div");
		self.in_text_block = True;

	def text(self, s):
		if self.in_text_block:
			self.__plain(s);
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