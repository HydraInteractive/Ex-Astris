import reggae;
import std.algorithm;
import std.array;
import std.conv;
import std.string;

//-fopenmp
enum optimization = "-O0 -pg -ggdb -Wno-error=unknown-pragmas -ffat-lto-objects -Wno-error=maybe-uninitialized -Wno-error=null-dereference";
//enum optimization = "-O3 -Wl,-O1 -march=native -ggdb -fopenmp -flto -ffat-lto-objects -Wno-error=maybe-uninitialized -Wno-error=null-dereference";

enum string[] SubProjects = ["hydra", "hydra_graphics", "hydra_network", "hydra_physics", "hydra_sound"];
enum string[] SubProjectsServer = ["hydra", "hydra_graphics", "hydra_network", "hydra_physics"];
enum string SubProjectsInclude = SubProjects.map!((string x) => "-I" ~ x ~ "/include -isystem" ~ x ~ "/lib-include").joiner(" ").array.to!string;
enum string SubProjectsServerInclude = SubProjectsServer.map!((string x) => "-I" ~ x ~ "/include -isystem" ~ x ~ "/lib-include").joiner(" ").array.to!string;
enum string SubProjectsLink = SubProjects.map!((string x) => "-l" ~ x).joiner(" ").array.to!string;
enum string SubProjectsServerLink = SubProjectsServer.map!((string x) => "-l" ~ x).joiner(" ").array.to!string;

enum warnings = "-Wall -Wextra -Werror -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wformat=2 -Wno-error=unused-parameter -Wno-error=format-nonliteral -Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-error=reorder -Wno-error=empty-body";

enum string CFlagsLib = optimization ~ " -std=c++1z " ~ warnings ~ " -fdiagnostics-color=always -fPIC  -DBT_THREADSAFE -isystem/usr/include/bullet " ~ SubProjectsInclude;
enum string CFlagsExecBase = optimization ~ " -std=c++1z " ~ warnings ~ " -fdiagnostics-color=always -fPIC  -DBT_THREADSAFE -isystem/usr/include/bullet ";

enum string CFlagsHydraBaseLib = "-DHYDRA_BASE_EXPORTS " ~ CFlagsLib;
enum string CFlagsHydraGraphicsLib = "-DHYDRA_GRAPHICS_EXPORTS " ~ CFlagsLib;
enum string CFlagsHydraNetworkLib = "-DHYDRA_NETWORK_EXPORTS " ~ CFlagsLib;
enum string CFlagsHydraPhysicsLib = "-DHYDRA_PHYSICS_EXPORTS " ~ CFlagsLib;
enum string CFlagsHydraSoundLib = "-DHYDRA_SOUND_EXPORTS " ~ CFlagsLib;
enum string CFlagsBarcodeExec = "-DBARCODE_EXPORTS -fuse-ld=gold " ~ CFlagsExecBase ~ warnings ~ " -Ibarcode/include " ~ SubProjectsInclude;
enum string CFlagsServerExec = "-DSERVER_EXPORTS -fuse-ld=gold " ~ CFlagsExecBase ~ warnings ~ " -Iserver/include " ~ SubProjectsServerInclude;

enum LFlagsHydraBaseLib = optimization ~ " -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodeproject.objs -Lobjs/barcodeproject.objs -fdiagnostics-color=always -lm -ldl -lSDL2";
enum LFlagsHydraGraphicsLib = optimization ~ " -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodeproject.objs -Lobjs/barcodeproject.objs -fdiagnostics-color=always -ldl -lhydra -lGL -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer";
enum LFlagsHydraNetworkLib = optimization ~ " -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodeproject.objs -Lobjs/barcodeproject.objs -fdiagnostics-color=always -lhydra -lhydra_graphics -lhydra_physics -lSDL2_net";
enum LFlagsHydraPhysicsLib = optimization ~ " -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodeproject.objs -Lobjs/barcodeproject.objs -fdiagnostics-color=always -lhydra -lhydra_graphics -lSDL2 -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lSDL2_mixer";
enum LFlagsHydraSoundLib = optimization ~ " -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodeproject.objs -Lobjs/barcodeproject.objs -fdiagnostics-color=always -lhydra -lhydra_graphics -lSDL2 -lSDL2_mixer";
enum LFlagsBarcodeExec = optimization ~ " -rdynamic -Wl,--no-undefined -Wl,-rpath,. -Wl,-rpath,objs/barcodeproject.objs -Lobjs/barcodeproject.objs -fdiagnostics-color=always -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lSDL2_mixer " ~ SubProjectsLink;
enum LFlagsServerExec = optimization ~ " -rdynamic -Wl,--no-undefined -Wl,-rpath,. -Wl,-rpath,objs/barcodeproject.objs -Lobjs/barcodeproject.objs -fdiagnostics-color=always -lSDL2_net -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath " ~ SubProjectsServerLink;

enum CC = "g++";
//enum CC = "distcc g++";

enum CompileBarcodeExec = CC ~ " -c " ~ CFlagsBarcodeExec ~ " $in -o $out";
enum CompileServerExec = CC ~ " -c " ~ CFlagsServerExec ~ " $in -o $out";
enum LinkBarcodeExec = CC ~ " " ~ LFlagsBarcodeExec ~ " $in -lstdc++fs -o $out";
enum LinkServerExec = CC ~ " " ~ LFlagsServerExec ~ " $in -lstdc++fs -o $out";
enum string Compile(string lib) = CC ~ " -c " ~ lib ~ " $in -o $out";
enum string Link(string lib) = CC ~ " " ~ lib ~ " $in -o $out";

Target[] MakeObjects(string src, string cmd)() {
	import std.file : dirEntries, SpanMode;
	import std.process : executeShell;
	import std.algorithm : map;
	import std.array : array, replace, split;
	import std.range : chain;
	import std.stdio : writeln;

	Target[] objs;

	foreach (f; chain(dirEntries(src, "*.cpp", SpanMode.breadth), dirEntries(src, "*.c", SpanMode.breadth)).filter!(x => !x.isDir && x.name[x.lastIndexOf('/') + 1] != '.')) {
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
	auto libhydra_physics = Target("libhydra_physics.so", Link!(LFlagsHydraPhysicsLib), MakeObjects!("hydra_physics/src/", Compile!(CFlagsHydraPhysicsLib)), [libhydra, libhydra_graphics]);
	auto libhydra_network = Target("libhydra_network.so", Link!(LFlagsHydraNetworkLib), MakeObjects!("hydra_network/src/", Compile!(CFlagsHydraNetworkLib)), [libhydra, libhydra_graphics, libhydra_physics]);
	auto libhydra_sound = Target("libhydra_sound.so", Link!(LFlagsHydraSoundLib), MakeObjects!("hydra_sound/src/", Compile!(CFlagsHydraSoundLib)), [libhydra, libhydra_graphics]);
	auto barcode = Target("barcodegame", LinkBarcodeExec, MakeObjects!("barcode/src/", CompileBarcodeExec), [libhydra, libhydra_graphics, libhydra_network, libhydra_physics, libhydra_sound]);
	auto server = Target("barcodeserver", LinkServerExec, MakeObjects!("server/src/", CompileServerExec), [libhydra, libhydra_graphics, libhydra_network, libhydra_physics]);

	auto project = Target.phony("barcodeproject", "(cp objs/barcodeproject.objs/barcodegame . || true); (cp objs/barcodeproject.objs/barcodeserver . || true)", [barcode, server]);

	auto dist = optional(Target.phony("dist", `tar cfz dist-$$(git describe --long --tags | sed 's/\([^-]*-\)g/r\1/').tar.xz barcodegame barcodeserver assets -C objs/barcodeproject.objs libhydra{,_{graphics,network,physics,sound}}.so`, [project]));

	return Build(project, dist);
}
