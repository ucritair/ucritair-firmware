import enum;

class HTMLMode(enum.Enum):
	DEFAULT = 0
	INLINE = 1

class HTMLWriter:
	def __init__(self, path):
		self.path = path;
		self.file = open(self.path, "w");
		self.stack = [];
	
# PRIVATE
	
	def _make_tabs(self):
		return '\t'*len(self.stack);

	def _make_args(self, kwargs):
		arg_string = "";
		for key, value in kwargs.items():
			if key.startswith("_"):
				key = key[1:];
			key = key.replace("data_", "data-");
			arg_string += f" {key}=\"{value}\"";
		return arg_string;

# SEMI-PRIVATE

	def plain(self, s):
		self.file.write(f"{s}\n");

	def open_tag(self, tag, **kwargs):
		self.file.write(f"{self._make_tabs()}<{tag}{self._make_args(kwargs)}>\n");
		self.stack.append((tag, kwargs));
	
	def close_tag(self):
		tag, kwargs = self.stack.pop();
		self.file.write(f"{self._make_tabs()}</{tag}>\n");
	
	def one_line(self, key, value, **kwargs):
		self.file.write(f"{self._make_tabs()}<{key}{self._make_args(kwargs)}>{value}</{key}>\n");
	
	def one_tag(self, tag, **kwargs):
		self.file.write(f"{self._make_tabs()}<{tag}{self._make_args(kwargs)} />\n");
	
	def one_token(self, tag, **kwargs):
		self.file.write(f"{self._make_tabs()}<{tag}{self._make_args(kwargs)}>\n");
	
# PUBLIC
	
	def start(self, title, **kwargs):
		self.title = title;

		self.one_token("!DOCTYPE HTML");
		self.open_tag("html");
		self.open_tag("head");
		
		self.one_tag("link", rel="stylesheet", href="/sakura.css");
		
		self.close_tag();
		self.open_tag("body");
	
	def end(self):
		self.close_tag();
		self.close_tag();
		self.file.close();
	
	def heading(self, tier, s, **kwargs):
		self.one_line(f"h{tier}", s, id=s.lower(), **kwargs);

	def newline(self):
		self.one_token("br");
	
	def begin_text_block(self,):
		self.open_tag("div", data_text_block=True);

	def text(self, s, mode=HTMLMode.DEFAULT, **kwargs):
		s = s.replace("\\n", "<br>");
		match mode:
			case HTMLMode.DEFAULT:
				head = self.stack[-1] if len(self.stack) > 0 else None;
				if head != None and "data_text_block" in head[1]:
					self.plain(f"{self._make_tabs()}{s}");
				else:
					self.one_line("p", s);
			case HTMLMode.INLINE:
				return f"<p {self._make_args(kwargs)}>{s}</p>";
	
	def end_text_block(self):
		self.close_tag();
		self.in_text_block = False;
	
	def begin_table(self, columns):
		self.open_tag("table");

		num_cols = len(columns);
		self.open_tag("colgroup");
		for i in range(num_cols):
			self.one_tag("col", style=f"width: {100/num_cols}%");
		self.close_tag();

		self.open_tag("thead");
		self.open_tag("tr");
		for column in columns:
			self.one_line("th", column);
		self.close_tag();
		self.close_tag();
	
		self.open_tag("tbody");

	def table_row(self, columns, row_id=None):
		if row_id != None:
			self.open_tag("tr", _id=row_id);
		else:
			self.open_tag("tr");
		for column in columns:
			self.one_line("td", column);
		self.close_tag();
	
	def end_table(self):
		self.close_tag();
		self.close_tag();

	def image(self, path, mode=HTMLMode.DEFAULT, **kwargs):
		match mode:
			case HTMLMode.DEFAULT:
				self.one_token("img", src=path, **kwargs);
			case HTMLMode.INLINE:
				return f"<img src={path} {self._make_args(kwargs)}>";

	def begin_div(self):
		self.open_tag("div");

	def end_div(self):
		self.close_tag();

	def begin_list(self):
		self.open_tag("ul");
	
	def list_item(self, item):
		self.one_line("li", item);

	def end_list(self):
		self.close_tag();

	def begin_same_line(self):
		self.open_tag("div", id="container", style="white-space:nowrap");
	
	def end_same_line(self):
		self.close_tag();

	def link(self, text, link, mode=HTMLMode.DEFAULT, **kwargs):
		match mode:
			case HTMLMode.DEFAULT:
				self.one_line("a", text, href=link, **kwargs);
			case HTMLMode.INLINE:
				return f"<a href={link} {self._make_args(kwargs)}>{text}</a>";

	def line(self, s, **kwargs):
		self.text(s, **kwargs);
		self.one_token("br");