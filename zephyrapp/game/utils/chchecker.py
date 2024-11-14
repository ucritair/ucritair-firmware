import sys;
import os;

if(len(sys.argv) != 1):
	print("usage: python3 chchecker.py");
	exit();

src = os.fsencode("src");
for entry in os.listdir(src):
	path = os.path.join("src", os.fsdecode(entry));
	if path.endswith(".h") or path.endswith(".c"):
		with open(path, "r") as file:
			lines = file.read().split("\n");
			lines = [line.strip() for line in lines];
			lines = [line.split(" ") for line in lines];
			for tokens in lines:
				if(tokens[0] == "#include" and not ".h" in tokens[1]):
					print(path);
					print(tokens);
	
		