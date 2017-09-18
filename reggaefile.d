import reggae;
import std.algorithm;
import std.array;
import std.conv;

enum string[] SubProjects = ["hydra", "hydra_graphics", "hydra_network", "hydra_physics", "hydra_sound"];
enum string SubProjectsInclude = SubProjects.map!((string x) => "-I" ~ x ~ "/include").joiner(" ").array.to!string;
enum string SubProjectsLink = SubProjects.map!((string x) => "-l" ~ x).joiner(" ").array.to!string;

enum string CFlagsLib = "-O0 -std=c++14 -ffat-lto-objects -ggdb -Wall -Wextra -Werror -fdiagnostics-color=always -fopenmp -fPIC " ~ SubProjectsInclude;
enum string CFlagsExec = "-O0 -std=c++14 -ffat-lto-objects -ggdb -Wall -Wextra -Werror -fdiagnostics-color=always -fopenmp -fPIC -Ibarcode/include " ~ SubProjectsInclude;

enum LFlagsHydraLib = "-O0 -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodegame.objs -Lobjs/barcodegame.objs -ggdb -fdiagnostics-color=always -fopenmp -lm -ldl -lSDL2";
enum LFlagsGraphicsLib = "-O0 -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodegame.objs -Lobjs/barcodegame.objs -ggdb -fdiagnostics-color=always -fopenmp -ldl -lhydra -lGL -lSDL2 -lSDL2_image -lSDL2_ttf -lassimp";
enum LFlagsNetworkLib = "-O0 -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodegame.objs -Lobjs/barcodegame.objs -ggdb -fdiagnostics-color=always -fopenmp -lhydra -lhydra_graphics -lSDL2_net";
enum LFlagsPhysicsLib = "-O0 -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodegame.objs -Lobjs/barcodegame.objs -ggdb -fdiagnostics-color=always -fopenmp -lhydra -lhydra_graphics -lSDL2";
enum LFlagsSoundLib = "-O0 -shared -Wl,--no-undefined -Wl,-rpath,objs/barcodegame.objs -Lobjs/barcodegame.objs -ggdb -fdiagnostics-color=always -fopenmp -lhydra -lhydra_graphics -lSDL2_mixer";
enum LFlagsExec = "-O0 -Wl,--no-undefined -Wl,-rpath,objs/barcodegame.objs -Lobjs/barcodegame.objs -ggdb -fdiagnostics-color=always -fopenmp " ~ SubProjectsLink;

enum CompileCommand : string {
	CompileLib = "g++ -c " ~ CFlagsLib ~ " $in -o $out",
	CompileExec = "g++ -c " ~ CFlagsExec ~ " $in -o $out",
	LinkExec = "g++ " ~ LFlagsExec ~ " $in -o $out"
}
enum string Link(string lib) = "g++ " ~ lib ~ " $in -o $out";

Target[] MakeObjects(string src, CompileCommand cmd)() {
	import std.file : dirEntries, SpanMode;
	import std.process : executeShell;
	import std.algorithm : map;
	import std.array : array, replace, split;
	import std.range : chain;
	import std.stdio : writeln;

	Target[] objs;

	foreach (f; chain(dirEntries(src, "*.cpp", SpanMode.breadth), dirEntries(src, "*.c", SpanMode.breadth)).filter!(x => !x.isDir)) {
		auto exec = executeShell("g++ -MM " ~ CFlagsLib ~ " " ~ f);
		if (exec.status) {
			import std.stdio : stderr;

			stderr.writeln("Returned: ", exec.status, "\n", exec.output);
			assert(0);
		}

		auto head = exec.output.split(":")[1].replace("\n", " ").split(" ").filter!(s => !s.empty && s != "\\").map!(x => Target(x)).array[1 .. $];
		//writeln(f, " needs: ", head);
		objs ~= Target(f ~ ".o", cmd, [Target(f)], head);
	}

	return objs;
}

Build myBuild() {
	auto libhydra = Target("libhydra.so", Link!(LFlagsHydraLib), MakeObjects!("hydra/src/", CompileCommand.CompileLib));
	auto libhydra_graphics = Target("libhydra_graphics.so", Link!(LFlagsGraphicsLib), MakeObjects!("hydra_graphics/src/", CompileCommand.CompileLib), [libhydra]);
	auto libhydra_network = Target("libhydra_network.so", Link!(LFlagsNetworkLib), MakeObjects!("hydra_network/src/", CompileCommand.CompileLib), [libhydra, libhydra_graphics]);
	auto libhydra_physics = Target("libhydra_physics.so", Link!(LFlagsPhysicsLib), MakeObjects!("hydra_physics/src/", CompileCommand.CompileLib), [libhydra, libhydra_graphics]);
	auto libhydra_sound = Target("libhydra_sound.so", Link!(LFlagsSoundLib), MakeObjects!("hydra_sound/src/", CompileCommand.CompileLib), [libhydra, libhydra_graphics]);

	auto barcode = Target("barcodegame", CompileCommand.LinkExec, MakeObjects!("barcode/src/", CompileCommand.CompileExec), [libhydra, libhydra_graphics, libhydra_network, libhydra_physics, libhydra_sound]);

	return Build(barcode);
}
