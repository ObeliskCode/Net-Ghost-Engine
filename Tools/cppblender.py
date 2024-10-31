#!/usr/bin/env python3
import subprocess, sys, os
_thisdir = os.path.split(os.path.abspath(__file__))[0]

if "--blender-install" in sys.argv:
	if "--blender-git" in sys.argv:
		if not os.path.isdir("./blender"):
			cmd = "git clone --depth 1 https://github.com/blender/blender.git"
			print(cmd)
			subprocess.check_call(cmd.split())
		cmd = "python3 ./blender/build_files/utils/make_update.py --no-libraries"
		print(cmd)
		subprocess.check_call(cmd.split(), cwd="./blender")
		subprocess.check_call(["make"], cwd="./blender")
	else:
		os.system("sudo apt install blender")

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


## Imports ##
assert bpy
import math, mathutils, json
from http.server import HTTPServer
from http.server import BaseHTTPRequestHandler


## Libs ##
BUDIR = os.path.join(_thisdir,'basis_universal')

if not os.path.isdir(BUDIR):
	cmd = 'git clone --depth 1 https://github.com/BinomialLLC/basis_universal.git'
	print(cmd)
	subprocess.check_call(cmd.split())
	# [TODO] fix basis_universal install
	subprocess.check_call(['cmake', '.'], cwd=BUDIR)
	subprocess.check_call(['make'], cwd=BUDIR)

BASISU = os.path.join(BUDIR, 'bin/basisu')
assert os.path.isfile(BASISU)


def netghost2json():
	dump = {}
	camdump = {}
	lightdump = {}
	shaders = {}
	vshaders = {}
	fshaders = {}
	for ob in bpy.data.objects:
		if ob.type == "CAMERA":
			print("dumping camera:", ob)
			camdump[ob.name] = {
				"pos": list(ob.location),
				"rot": list(ob.rotation_euler),
				"scripts": [],
			}
		if ob.type == "LIGHT":
			print("dumping light:", ob)
			lightdump[ob.name] = {
				"pos": list(ob.location),
				"scripts": [],
			}
		if ob.type == "MESH":
			print("dumping mesh:", ob)
			dump[ob.name] = {
				"pos": list(ob.location),
				"rot": list(ob.rotation_euler),
				"scl": list(ob.scale),
				"verts": [(v.co.x, v.co.y, v.co.z) for v in ob.data.vertices],
				"normals": [
					(v.normal.x, v.normal.y, v.normal.z) for v in ob.data.vertices
				],
				"indices": [],
				"scripts": [],
			}
			if ob.netghost_glsl_vertex:
				txt = ob.netghost_glsl_vertex
				if txt.name not in vshaders:
					vshaders[txt.name] = txt.as_string()
				dump[ob.name]["vshader"] = txt.name
			if ob.netghost_glsl_fragment:
				txt = ob.netghost_glsl_fragment
				if txt.name not in fshaders:
					fshaders[txt.name] = txt.as_string()
				dump[ob.name]["fshader"] = txt.name

			if ob.netghost_glsl_vertex and ob.netghost_glsl_fragment:
				sname = ob.netghost_glsl_vertex.name + ob.netghost_glsl_fragment.name
				sname = sname.replace(".", "_").replace("-", "_").replace("+", "_")
				if sname not in shaders:
					shaders[sname] = {
						"vert": ob.netghost_glsl_vertex.as_string(),
						"frag": ob.netghost_glsl_fragment.as_string(),
					}
				dump[ob.name]["shader"] = sname

			if ob.parent:
				dump[ob.name]["parent"] = ob.parent.name
			for face in ob.data.polygons:
				for i in range(3):
					dump[ob.name]["indices"].append(face.vertices[i])
			for i in range(MAX_SCRIPTS_PER_OBJECT):
				txt = getattr(ob, "netghost_script" + str(i))
				if txt:
					dump[ob.name]["scripts"].append(txt.as_string())
			if ob.keys():
				dump[ob.name]["props"] = {}
				props = {}
				for k in ob.keys():
					if (
						type(ob[k]) is float
					):  ## GOTCHA, there is other blender DNA/RNA hacks here
						props[k] = ob[k]
				if props:
					dump[ob.name]["props"] = props

	print(dump)
	info = 	{
			"objects": dump,
			"cameras": camdump,
			"lights": lightdump,
			"vshaders": vshaders,
			"fshaders": fshaders,
			"shaders": shaders,
		}
	if bpy.data.worlds[0].netghost_javascript:
		js = bpy.data.worlds[0].netghost_javascript.as_string()
		print(js)
		info['javascript'] = js
	return json.dumps(info)


def test():
	txt = bpy.data.texts.new(name="my.c++.py")
	txt.from_string(TEST2)
	ob = bpy.data.objects["Cube"]
	ob.netghost_script0 = txt
	ob["myprop"] = 1.0

	txt = bpy.data.texts.new(name="my-vshader.glsl.py")
	txt.from_string(TEST_GLSL_VERT)
	ob.netghost_glsl_vertex = txt

	txt = bpy.data.texts.new(name="my-fshader.glsl.py")
	txt.from_string(TEST_GLSL_FRAG)
	ob.netghost_glsl_fragment = txt


def flagloop():
	if __name__ == "__main__":
		if "--dump" in sys.argv:
			tmpj = "/tmp/dump.json"
			open(tmpj, "w").write(netghost2json())

		elif "--test" in sys.argv:
			test()


if "--debug" in sys.argv:
	builder_script = os.path.join(_thisdir, "build.py")
	print("builder_script:", builder_script)
	assert os.path.isfile(builder_script)

## NetGhost Blender DNA/RNA
MAX_SCRIPTS_PER_OBJECT = 8
for i in range(MAX_SCRIPTS_PER_OBJECT):
	setattr(
		bpy.types.Object,
		"netghost_script" + str(i),
		bpy.props.PointerProperty(name="script%s" % i, type=bpy.types.Text),
	)

bpy.types.World.netghost_server = bpy.props.PointerProperty(
	name="NetGhost Server", type=bpy.types.Text
)
bpy.types.World.netghost_javascript = bpy.props.PointerProperty(
	name="NetGhost JavaScript", type=bpy.types.Text
)


bpy.types.Object.netghost_script_init = bpy.props.PointerProperty(
	name="script init", type=bpy.types.Text
)

bpy.types.Object.netghost_glsl_vertex = bpy.props.PointerProperty(
	name="vertex shader", type=bpy.types.Text
)
bpy.types.Object.netghost_glsl_fragment = bpy.props.PointerProperty(
	name="fragment shader", type=bpy.types.Text
)

bpy.types.Object.netghost_spawnable = bpy.props.BoolProperty(name="spawnable")
bpy.types.Object.netghost_owner = bpy.props.PointerProperty(
	name="owner", type=bpy.types.Text
)
bpy.types.Object.netghost_prefab = bpy.props.PointerProperty(
	name="prefab", type=bpy.types.Collection
)

bpy.types.Object.netghost_public = bpy.props.BoolProperty(name="PUBLIC")
bpy.types.Object.netghost_public_r = bpy.props.BoolProperty(name="readable")
bpy.types.Object.netghost_public_w = bpy.props.BoolProperty(name="writeable")
bpy.types.Object.netghost_public_x = bpy.props.BoolProperty(name="executable")

bpy.types.Object.netghost_fri = bpy.props.BoolProperty(name="FRIENDS")
bpy.types.Object.netghost_fri_r = bpy.props.BoolProperty(name="readable")
bpy.types.Object.netghost_fri_w = bpy.props.BoolProperty(name="writeable")
bpy.types.Object.netghost_fri_x = bpy.props.BoolProperty(name="executable")

class netghost:
	servers = []
	@staticmethod
	def http( klass ):
		print('binding new http server:', klass)
		port = 8000
		if hasattr(klass, 'netghost_port'):
			port = klass.nethost_port
		d = HTTPServer(('localhost', port), klass)
		if hasattr(klass, 'netghost_timeout'):
			d.timeout = klass.netghost_timeout
		else:
			d.timeout = 0.05
		##TODO if netghost.servers and netghost.servers[-1].netghost_port == port: kill the previous server
		d.netghost_port = port
		netghost.servers.append(d)
		return klass

	@staticmethod
	def render(name, width=128, height=128, zoom=3):
		ob = bpy.data.objects[name]
		if ob.type=='EMPTY':
			if ob.empty_display_type=='IMAGE':
				return open(ob.data.filepath,'rb').read()
			## TODO render collections
			return ''
		for o in bpy.data.objects:
			if o.type=='LIGHT': continue
			o.hide_render=True
		ob.hide_render=False
		bounds = get_object_bounds(ob)
		print('rendering:', ob, bounds)
		bpy.context.scene.render.film_transparent = True
		bpy.context.scene.render.image_settings.color_mode = 'RGBA'
		bpy.context.scene.render.filepath='/tmp/__netghost__.png'
		bpy.context.scene.render.resolution_x = width * zoom
		bpy.context.scene.render.resolution_y = height * zoom
		cam = bpy.data.objects['Camera']
		cam.location = bounds[0] - mathutils.Vector((0, bounds[1].y, 0))
		cam.rotation_euler = [math.pi/2, 0,0]
		cam.data.ortho_scale = max(bounds[1].x, bounds[1].z) * zoom
		cam.data.type = 'ORTHO'
		bpy.ops.render.render(animation=False, write_still=True)
		for o in bpy.data.objects: o.hide_render=False
		tmp = '/tmp/tmp.png'
		cmd = ['convert', bpy.context.scene.render.filepath, '-fuzz', '80%', '-trim', '+repage', tmp]
		print(cmd)
		subprocess.check_call(cmd)
		return open(tmp, 'rb').read()

	@staticmethod
	def basisu( input, mode="KTX2", compression=3 ):
		cmd = [BASISU]
		tmp = input
		if mode=="KTX2":
			cmd.append('-ktx2')
			tmp = input[:-4] + '.ktx2'
		else:
			assert output.endswith('.basis')
			cmd.append('-uastc')
			tmp = input[:-4] + '.basis'
		assert compression <= 5
		cmd.append('-comp_level')
		cmd.append(str(compression))
		cmd.append(input)
		print(cmd)
		subprocess.check_call(cmd, cwd='/tmp')
		return open(tmp,'rb').read()


_timer = None
@bpy.utils.register_class
class HttpServerOperator(bpy.types.Operator):
	"NetGhost HTTP Server"
	bl_idname = "netghost.run"
	bl_label = "netghost_run"
	bl_options = {'REGISTER'}
	def modal(self, context, event):
		if event.type == "TIMER":
			if netghost.servers:
				for d in netghost.servers:
					## this works if each server is on a different port
					d.handle_request() # this blocks for a short time
		return {'PASS_THROUGH'} # will not supress event bubbles

	def invoke (self, context, event):
		global _timer
		if _timer is None:
			_timer = self._timer = context.window_manager.event_timer_add(
				time_step=0.05,
				window=context.window
			)
			context.window_manager.modal_handler_add(self)
			return {'RUNNING_MODAL'}
		return {'FINISHED'}

	def execute (self, context):
		return self.invoke(context, None)
bpy.ops.netghost.run()




@bpy.utils.register_class
class NetGhostNetPanel(bpy.types.Panel):
	bl_idname = "OBJECT_PT_NetGhost_Net_Panel"
	bl_label = "NetGhost Network"
	bl_space_type = "PROPERTIES"
	bl_region_type = "WINDOW"
	bl_context = "object"

	def draw(self, context):
		if not context.active_object:
			return
		self.layout.prop(context.active_object, "netghost_spawnable")
		self.layout.prop(context.active_object, "netghost_owner")
		self.layout.prop(context.active_object, "netghost_prefab")
		self.layout.prop(context.active_object, "netghost_public")
		if context.active_object.netghost_public:
			self.layout.prop(context.active_object, "netghost_public_r")
			self.layout.prop(context.active_object, "netghost_public_w")
			self.layout.prop(context.active_object, "netghost_public_x")

		self.layout.prop(context.active_object, "netghost_fri")
		if context.active_object.netghost_fri:
			self.layout.prop(context.active_object, "netghost_fri_r")
			self.layout.prop(context.active_object, "netghost_fri_w")
			self.layout.prop(context.active_object, "netghost_fri_x")


@bpy.utils.register_class
class NetGhostGLSLPanel(bpy.types.Panel):
	bl_idname = "OBJECT_PT_NetGhost_GLSL_Panel"
	bl_label = "NetGhost GLSL"
	bl_space_type = "PROPERTIES"
	bl_region_type = "WINDOW"
	bl_context = "object"

	def draw(self, context):
		if not context.active_object:
			return
		self.layout.label(text="Attach GLSL Shaders")
		self.layout.prop(context.active_object, "netghost_glsl_vertex")
		self.layout.prop(context.active_object, "netghost_glsl_fragment")


@bpy.utils.register_class
class NetGhostScriptsPanel(bpy.types.Panel):
	bl_idname = "OBJECT_PT_NetGhost_Scripts_Panel"
	bl_label = "NetGhost Scripts"
	bl_space_type = "PROPERTIES"
	bl_region_type = "WINDOW"
	bl_context = "object"

	def draw(self, context):
		if not context.active_object:
			return
		self.layout.label(text="Attach C++ Scripts")
		self.layout.prop(context.active_object, "netghost_script_init")

		foundUnassignedScript = False
		for i in range(MAX_SCRIPTS_PER_OBJECT):
			hasProperty = (
				getattr(context.active_object, "netghost_script" + str(i)) != None
			)
			if hasProperty or not foundUnassignedScript:
				self.layout.prop(context.active_object, "netghost_script" + str(i))
			if not foundUnassignedScript:
				foundUnassignedScript = not hasProperty


@bpy.utils.register_class
class NetGhostExport(bpy.types.Operator):
	bl_idname = "netghost.export"
	bl_label = "Export EXE"

	@classmethod
	def poll(cls, context):
		return True

	def execute(self, context):
		tmpj = "/tmp/b2ghost.json"
		open(tmpj, "w").write(netghost2json())
		cmd = ["python3", "./build.py", tmpj]
		print(cmd, _thisdir)
		subprocess.check_call(cmd, cwd=_thisdir)
		return {"FINISHED"}


@bpy.utils.register_class
class NetGhostExportWasm(bpy.types.Operator):
	bl_idname = "netghost.export_wasm"
	bl_label = "Export WASM"

	@classmethod
	def poll(cls, context):
		return True

	def execute(self, context):
		tmpj = "/tmp/b2ghost.json"
		open(tmpj, "w").write(netghost2json())
		cmd = ["python3", "./build.py", "--wasm", tmpj, "--output=/tmp/test.html"]
		print(cmd, _thisdir)
		subprocess.check_call(cmd, cwd=_thisdir)
		html = open("/tmp/test.html").read()
		print("emscripten flat html bytes:", len(html))
		if "__index__.html" not in bpy.data.texts:
			bpy.data.texts.new(name="__index__.html")
		bpy.data.texts["__index__.html"].from_string(html)
		if context.world.netghost_server:
			#try_run_server
			scope = globals()
			exec(context.world.netghost_server.as_string(), scope, scope)
			assert len(netghost.servers)

		if not netghost.servers:
			## load simple default server
			scope = globals()
			simple = open(os.path.join(_thisdir,'Resources/simple_server.py')).read()
			exec(simple, scope, scope)
			assert len(netghost.servers)

		import webbrowser
		webbrowser.open("http://localhost:8000/")
		return {"FINISHED"}


@bpy.utils.register_class
class NetGhostExportWasm(bpy.types.Operator):
	bl_idname = "netghost.simple_server"
	bl_label = "Simple HTTP Server"

	@classmethod
	def poll(cls, context):
		return True

	def execute(self, context):
		if not netghost.servers:
			## load simple default server
			scope = globals()
			simple = open(os.path.join(_thisdir,'Resources/simple_server.py')).read()
			print(simple)
			exec(simple, scope, scope)
			assert len(netghost.servers)

		import webbrowser
		webbrowser.open("http://localhost:8000/")
		return {"FINISHED"}



@bpy.utils.register_class
class NetGhostWorldPanel(bpy.types.Panel):
	bl_idname = "WORLD_PT_NetGhostWorld_Panel"
	bl_label = "NetGhost Export"
	bl_space_type = "PROPERTIES"
	bl_region_type = "WINDOW"
	bl_context = "world"

	def draw(self, context):
		self.layout.prop(context.world, "netghost_server")
		self.layout.prop(context.world, "netghost_javascript")
		self.layout.operator("netghost.export_wasm", icon="CONSOLE")
		self.layout.operator("netghost.export", icon="CONSOLE")
		self.layout.operator("netghost.simple_server", icon="CONSOLE")


## from HolyBlender
def get_object_bounds(obj) -> (mathutils.Vector, mathutils.Vector):
	_min = mathutils.Vector((float('inf'), float('inf'), float('inf')))
	_max = mathutils.Vector((float('-inf'), float('-inf'), float('-inf')))
	## note: blender already has object bounds that is more acurate than below,
	## because the blender cached bbox should take into account modifiers, see obj.dimensions
	## https://blender.stackexchange.com/questions/8459/get-blender-x-y-z-and-bounding-box-with-script
	## TODO, below is very slow on big meshes, this needs rewrite
	if obj.type == 'MESH':
		for vertex in obj.data.vertices:
			_min.x = min((obj.matrix_world @ vertex.co).x, _min.x)
			_min.y = min((obj.matrix_world @ vertex.co).y, _min.y)
			_min.z = min((obj.matrix_world @ vertex.co).z, _min.z)
			_max.x = max((obj.matrix_world @ vertex.co).x, _max.x)
			_max.y = max((obj.matrix_world @ vertex.co).y, _max.y)
			_max.z = max((obj.matrix_world @ vertex.co).z, _max.z)
	else:
		raise RuntimeError('not implemented type: %s' % obj.type)
	return ((_min + _max) / 2, _max - _min)



TEST1 = """

std::cout << "hello GHOST" << std::endl;
std::cout << "object ID=" << self.m_id << std::endl;
std::cout << "object transform-flag=" << self.transform_flag << std::endl;

"""

TEST2 = (
	TEST1
	+ """

std::cout << "object blender-prop myprop=" << myprop << std::endl;
myprop += 0.1;

"""
)

TEST_GLSL_VERT = """
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 camMatrix;
uniform mat4 transform;
void main(){
	gl_Position = camMatrix * transform * vec4(aPos, 1.0);
}
"""

TEST_GLSL_FRAG = """
#version 330 core
out vec4 FragmentColor;
void main(){
	FragmentColor = vec4(1.0f,1.0f,1.0f,1.0f);
}
"""

flagloop()
