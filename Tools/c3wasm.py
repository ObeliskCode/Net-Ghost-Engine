#!/usr/bin/env python3
import subprocess, sys, os

_thisdir = os.path.split(os.path.abspath(__file__))[0]
sys.path.append(_thisdir)

C3 = '/usr/local/bin/c3c'

islinux=iswindows=c3gz=c3zip=None
if sys.platform == 'win32':
	BLENDER = 'C:/Program Files/Blender Foundation/Blender 4.2/blender.exe'
	c3zip = 'https://github.com/c3lang/c3c/releases/download/latest/c3-windows.zip'
	C3 = os.path.join(_thisdir,'c3/c3c.exe')
	iswindows=True
elif sys.platform == 'darwin':
	BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'
	c3zip = 'https://github.com/c3lang/c3c/releases/download/latest/c3-macos.zip'
else:
	BLENDER = 'blender'
	c3gz = 'https://github.com/c3lang/c3c/releases/download/latest/c3-ubuntu-20.tar.gz'
	islinux=True

if not os.path.isfile(C3):
	C3 = '/opt/c3/c3c'
	if not os.path.isfile(C3):
		if not os.path.isdir('./c3'):
			if c3gz:
				if not os.path.isfile('c3-ubuntu-20.tar.gz'):
					cmd = 'wget -c %s' % c3gz
					print(cmd)
					subprocess.check_call(cmd.split())
				cmd = 'tar -xvf c3-ubuntu-20.tar.gz'
				print(cmd)
				subprocess.check_call(cmd.split())
			elif c3zip and iswindows:
				if not os.path.isfile('c3-windows.zip'):
					cmd = ['curl', '-o', 'c3-windows.zip', c3zip]
					print(cmd)
					subprocess.check_call(cmd)
				cmd = ['tar', '-xf', './c3-windows.zip']
				print(cmd)
				subprocess.check_call(cmd)
			elif c3zip:
				if not os.path.isfile('c3-macos.zip'):
					cmd = ['curl', '-o', 'c3-macos.zip', c3zip]
					print(cmd)
					subprocess.check_call(cmd)

		if islinux:
			C3 = os.path.abspath('./c3/c3c')
		elif iswindows:
			C3 = os.path.abspath('./c3/c3c.exe')

print('c3c:', C3)
# assert c3 installed
assert os.path.isfile(C3)

## @Environment
#
##
def emsdk_update():
	subprocess.check_call(["git", "pull"], cwd=EMSDK)
	subprocess.check_call(["./emsdk", "install", "latest"], cwd=EMSDK)
	subprocess.check_call(["./emsdk", "activate", "latest"], cwd=EMSDK)


EMSDK = os.path.join(_thisdir, "emsdk")
if "--install-wasm" in sys.argv and not os.path.isdir(EMSDK):
	cmd = [
		"git","clone","--depth","1",
		"https://github.com/emscripten-core/emsdk.git",
	]
	print(cmd)
	subprocess.check_call(cmd)
	emsdk_update()

if iswindows:
	EMCC = os.path.join(EMSDK, "upstream/emscripten/emcc.exe")
	WASM_OBJDUMP = os.path.join(EMSDK, "upstream/bin/llvm-objdump.exe")
else:
	EMCC = os.path.join(EMSDK, "upstream/emscripten/emcc")
	WASM_OBJDUMP = os.path.join(EMSDK, "upstream/bin/llvm-objdump")
if not EMCC and "--install-wasm" in sys.argv:
	emsdk_update()

# assert emscripten installed
assert os.path.isdir(EMSDK)

BLENDER_PATH = os.path.join(_thisdir, "blender")

if "--blender-install" in sys.argv:
	if "--blender-git" in sys.argv:
		if not os.path.isdir(BLENDER_PATH):
			cmd = "git clone --depth=1 https://github.com/blender/blender.git"
			print(cmd)
			subprocess.check_call(cmd.split(), cwd=_thisdir)
		cmd = "python ./blender/build_files/utils/make_update.py --no-libraries"
		print(cmd)
		subprocess.check_call(cmd.split(), cwd=_thisdir)
		subprocess.check_call(["make"], cwd=BLENDER_PATH)
	else:
		os.system("sudo apt install blender", cwd=_thisdir)

try:
	import bpy
except:
	bpy = None

if not bpy:
	blender = "blender"  # Linux
	if sys.platform == "win32":  # Windows
		blender = "C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"
	elif sys.platform == "darwin":  # Apple
		blender = "/Applications/Blender.app/Contents/MacOS/Blender"

	command = []
	user_opts = []
	test_bevy = False
	for i, arg in enumerate(sys.argv):
		if arg.endswith(".blend"):
			command.append(os.path.expanduser(arg))
		elif arg.endswith(".exe"):  # Windows
			blender = arg
		elif arg.endswith(".app"):  # Apple
			blender = arg + "/Contents/MacOS/Blender"
		elif arg.endswith(("blender", "Blender")):
			blender = arg
		elif arg.startswith("--"):
			user_opts.append(arg)

	command = [blender] + command + ["--python", __file__]

	if user_opts:
		command.append("--")
		command += user_opts
	print(command)

	subprocess.check_call(command)
	sys.exit()

# assert bpy
assert bpy