import sys
import os

ENVIRON = "lib/ansi/env.o"
ANSI = "lib/ansi/"
STDLIB = "lib/stdlib/"
GEN = "lib/gen/"
POSIX = "lib/posix/"
SIGNAL = "lib/posix/_sigset.o"
UTIL ="lib/util/"
STDIO ="lib/stdio/"
SYSCALL = "lib/syscall/"
STRING = "lib/ansi/string.o"
DEBUG = "lib/util/debug.o"

built_in = {
	"stdlib.h": {STDLIB, ANSI},
	"string.h": {STRING},
	"signal.h": {SIGNAL},
	"ucontext.h": {GEN},
	"unistd.h": {POSIX, ANSI, STDLIB},
	"util.h":	{UTIL},
	"stdio.h":	{STDIO},
	"debug.h":	{DEBUG},
	"dirent.h":	{POSIX, STDLIB, ANSI}
}

dir_path = ""

def get_all_files(libs):
	ret = ''
	for lib in libs:
		if lib.endswith("/"):
			for file in os.listdir(lib):
				if file.endswith(".o") and 'env.o' != file:
					ret += lib + file + ' '
		else:
			ret += lib + ' '
	return ret

def include_iterate(start, end,line):
	i = 0
	j = 0
	# get header name
	while(line[i] != start):
		i += 1
	i += 1
	j = i
	while(line[j] != end):
		j += 1
	# print(line, i, j, line[i:j])
	return i,j

def get_sys_include(line):
	i,j = include_iterate('<', '>',line)
	if(j > i + 1):
		filename = line[i:j]
		if(filename in built_in):
			# print(built_in[filename])
			return built_in[filename]
	return set()

def get_local_include(line):
	global dir_path
	i,j = include_iterate('"', '"',line)
	if(j > i + 1):
		filename = line[i:j]
		return do_include_search(dir_path + filename)
	return set()

def do_include_search(filename):
	libs = set()
	global dir_path
	dir_path = os.path.dirname(os.path.realpath(filename)) + '/'
	# print(dir_path)
	with open(filename) as f:
		tocontinue = True
		for line in f:
			if "#include" in line:
				tlib = set()
				if "<" in line:
					tlib = get_sys_include(line)
				else:
					tlib = get_local_include(line)
				if(len(tlib) > 0):
					libs.update(tlib)
	return libs	

def main():
	if(len(sys.argv) < 2):
		return -1
	libs = {ENVIRON, SYSCALL }
	for i in range(1,len(sys.argv)):
		tlib = do_include_search(sys.argv[i])
		libs.update(tlib)

	if STRING in libs and ANSI in libs:
		libs.remove(STRING)
	if SIGNAL in libs and POSIX in libs:
		libs.remove(SIGNAL)
		
	print(get_all_files(libs), end='')

if __name__ == '__main__':
	main()

