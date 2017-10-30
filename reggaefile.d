import reggae;
import std.algorithm;
import std.array;
import std.conv;
import std.string;

//-fopenmp
enum optimization = "-Og -pg -ggdb -Wno-error=unknown-pragmas -ffat-lto-objects -Wno-error=maybe-uninitialized -Wno-error=null-dereference";
//enum optimization = "-O3 -ggdb -fopenmp -ffat-lto-objects -Wno-error=maybe-uninitialized -Wno-error=null-dereference";

enum string[] SubProjects = ["hydra", "hydra_graphics", "hydra_network", "hydra_physics", "hydra_sound"];
enum string SubProjectsInclude = SubProjects.map!((string x) => "-I" ~ x ~ "/include -isystem" ~ x ~ "/lib-include").joiner(" ").array.to!string;
enum string SubProjectsLink = SubProjects.map!((string x) => "-l" ~ x).joiner(" ").array.to!string;

enum warnings = "-Wall -Wextra -Werror -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wformat=2 -Wno-error=unused-parameter -Wno-error=format-nonliteral -Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-error=reorder";

enum string CFlagsLib = optimization ~ " -std=c++1z " ~ warnings ~ " -fdiagnostics-color=always -fPIC " ~ SubProjectsInclude;
enum string CFlagsExecBase = optimization ~ " -std=c++1z " ~ warnings ~ " -fdiagnostics-color=always -fPIC ";

enum string CFlagsHydraBaseLib = "-DHYDRA_BASE_EXPORTS " ~ CFlagsLib;
enum string CFlagsHydraGraphicsLib = "-DHYDRA_GRAPHICS_EXPORTS " ~ CFlagsLib;
enum string CFlagsHydraNetworkLib = "-DHYDRA_NETWORK_EXPORTS " ~ CFlagsLib;
enum string CFlagsHydraPhysicsLib = "-DHYDRA_PHYSICS_EXPORTS -DBT_THREADSAFE -isystem/usr/include/bullet " ~ CFlagsLib;
enum string CFlagsHydraSoundLib = "-DHYDRA_SOUND_EXPORTS " ~ CFlagsLib;
enum string CFlagsExec = "-DBARCODE_EXPORTS " ~ CFlagsExecBase ~ warnings ~ " -Ibarcode/include " ~ SubProjectsInclude;

enum LFlagsHydraBaseLib = optimization ~ " -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodegame.objs -Lobjs/barcodegame.objs -fdiagnostics-color=always -lm -ldl -lSDL2";
enum LFlagsHydraGraphicsLib = optimization ~ " -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodegame.objs -Lobjs/barcodegame.objs -fdiagnostics-color=always -ldl -lhydra -lGL -lSDL2 -lSDL2_image -lSDL2_ttf";
enum LFlagsHydraNetworkLib = optimization ~ " -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodegame.objs -Lobjs/barcodegame.objs -fdiagnostics-color=always -lhydra -lhydra_graphics -lSDL2_net";
enum LFlagsHydraPhysicsLib = optimization ~ " -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodegame.objs -Lobjs/barcodegame.objs -fdiagnostics-color=always -lhydra -lhydra_graphics -lSDL2 -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lSDL2_mixer";
enum LFlagsHydraSoundLib = optimization ~ " -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodegame.objs -Lobjs/barcodegame.objs -fdiagnostics-color=always -lhydra -lhydra_graphics -lSDL2_mixer";
enum LFlagsExec = optimization ~ " -rdynamic -Wl,--no-undefined -Wl,-rpath,. -Wl,-rpath,objs/barcodegame.objs -Lobjs/barcodegame.objs -fdiagnostics-color=always -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lSDL2_mixer " ~ SubProjectsLink;

enum CC = "distcc g++";

enum CompileCommand : string {
	CompileExec = CC ~ " -c " ~ CFlagsExec ~ " $in -o $out",
	LinkExec = CC ~ " " ~ LFlagsExec ~ " $in -o $out"
}
enum string Compile(string lib) = CC ~ " -c " ~ lib ~ " $in -o $out";
enum string Link(string lib) = CC ~ " " ~ lib ~ " $in -o $out";

Target[] MakeObjects(string src, CompileCommand cmd)() {
	import std.file : dirEntries, SpanMode;
	import std.process : executeShell;
	import std.algorithm : map;
	import std.array : array, replace, split;
	import std.range : chain;
	import std.stdio : writeln;

	Target[] objs;

	foreach (f; chain(dirEntries(src, "*.cpp", SpanMode.breadth), dirEntries(src, "*.c", SpanMode.breadth)).filter!(x => !x.isDir)) {
		auto flags = cmd ~ (f.indexOf("src/lib") != -1 ? " -w " : "");

		auto exec = executeShell("g++ -MM " ~ SubProjectsInclude ~ " " ~ f);
		if (exec.status) {
			import std.stdio : stderr;

			stderr.writeln("Returned: ", exec.status, "\n", exec.output);
			assert(0);
		}

		auto head = exec.output.split(":")[1].replace("\n", " ").split(" ").filter!(s => !s.empty && s != "\\").map!(x => Target(x)).array[1 .. $];
		//writeln(f, " needs: ", head);
		objs ~= Target(f ~ ".o", flags, [Target(f)], head);
	}

	return objs;
}

Build myBuild() {
	auto libhydra = Target("libhydra.so", Link!(LFlagsHydraBaseLib), MakeObjects!("hydra/src/", Compile!(CFlagsHydraBaseLib)));
	auto libhydra_graphics = Target("libhydra_graphics.so", Link!(LFlagsHydraGraphicsLib), MakeObjects!("hydra_graphics/src/", Compile!(CFlagsHydraGraphicsLib)), [libhydra]);
	auto libhydra_network = Target("libhydra_network.so", Link!(LFlagsHydraNetworkLib), MakeObjects!("hydra_network/src/", Compile!(CFlagsHydraNetworkLib)), [libhydra, libhydra_graphics]);
	auto libhydra_physics = Target("libhydra_physics.so", Link!(LFlagsHydraPhysicsLib), MakeObjects!("hydra_physics/src/", Compile!(CFlagsHydraPhysicsLib)), [libhydra, libhydra_graphics]);
	auto libhydra_sound = Target("libhydra_sound.so", Link!(LFlagsHydraSoundLib), MakeObjects!("hydra_sound/src/", Compile!(CFlagsHydraSoundLib)), [libhydra, libhydra_graphics]);
	auto barcode = Target("barcodegame", CompileCommand.LinkExec, MakeObjects!("barcode/src/", CompileCommand.CompileExec), [libhydra, libhydra_graphics, libhydra_network, libhydra_physics, libhydra_sound]);

	auto dist = optional(Target.phony("dist", `tar cfz dist-$$(git describe --long --tags | sed 's/\([^-]*-\)g/r\1/').tar.xz barcodegame assets -C objs/barcodegame.objs libhydra{,_{graphics,network,physics,sound}}.so`));

	return Build(barcode, dist);
}
