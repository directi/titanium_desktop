import os.path as path
import sdk
import distutils.dir_util as dir_util
from kroll import BuildConfig

targets = COMMAND_LINE_TARGETS
clean = 'clean' in targets or ARGUMENTS.get('clean', 0)

build = BuildConfig(
	PRODUCT_VERSION = sdk.get_titanium_version(),
	PRODUCT_NAME = 'Titanium',
	GLOBAL_NS_VARNAME = 'Titanium',
	CONFIG_FILENAME = 'tiapp.xml',
	BUILD_DIR = path.abspath('build'),
	THIRD_PARTY_DIR = path.join(path.abspath('kroll'), 'thirdparty'),
	DISTRIBUTION_URL = 'api.appcelerator.net',
	CRASH_REPORT_URL = 'api.appcelerator.net/p/v1/app-crash-report'
)

if clean:
	print "Obliterating your build directory: %s" % build.dir
	if path.exists(build.dir):
		dir_util.remove_tree(build.dir)
	Exit(0)

build.set_kroll_source_dir(path.abspath('kroll'))
build.titanium_source_dir = path.abspath('.')
build.titanium_sdk_dir = path.join(build.titanium_source_dir, 'sdk')

# debug build flag
debug = ARGUMENTS.get('debug', 0)
# thirdparty_check build flag
thirdparty_check = ARGUMENTS.get('thirdparty_check', 0)

Export('build', 'debug', 'thirdparty_check')

if debug:
	build.env.Append(CPPDEFINES = ('DEBUG', 1))
	if build.is_win32():
		build.env.Append(CCFLAGS=['/Z7', '/MD'])  # max debug
		build.env.Append(CPPDEFINES=('WIN32_CONSOLE', 1))
		build.env.Append(CPPDEFINES=('WIN32_LEAN_AND_MEAN',1))
		build.env.Append(LINKFLAGS=['/LTCG', '/DEBUG', '/PDB:${TARGET}.pdb'])
		#build.env.Append(LINKFLAGS=['/VERBOSE:LIB'])
	else:
		build.env.Append(CPPFLAGS=['-g'])  # debug
else:
	build.env.Append(CPPDEFINES = ('NDEBUG', 1 ))
	if build.is_win32():
		build.env.Append(CCFLAGS=['/MD', '/O2', '/GL'])
		build.env.Append(LINKFLAGS=['/LTCG', '/INCREMENTAL:NO', '/OPT:REF'])
	else:
		build.env.Append(CPPFLAGS = ['-O9']) # max optimizations

if build.is_win32():
	build.env.Append(CCFLAGS=['/EHsc', '/GR', '/DUNICODE', '/D_UNICODE'])

## Kroll *must not be required* for installation
SConscript('kroll/SConscript.thirdparty')

# After Kroll builds, the environment will  link 
# against libkroll, so anything that should not be
# linked against libkroll should be above this point.
SConscript('kroll/SConscript', exports='debug')
SConscript('modules/SConscript')
SConscript('SConscript.dist')
SConscript('SConscript.test')