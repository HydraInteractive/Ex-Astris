import reggae;

enum CFlagsLib = "-O0 -ffat-lto-objects -std=c++1z -ggdb -Wall -Werror -fdiagnostics-color=always -fopenmp -fPIC -Ihydra/include";
enum CFlagsExec = "-O0 -ffat-lto-objects -std=c++1z -ggdb -Wall -Werror -fdiagnostics-color=always -fopenmp -fPIC -Ihydra/include -Iexample/include";
enum LFlagsLib = "-O3 -shared -ggdb -fdiagnostics-color=always -fopenmp -lm -ldl -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lGL -lassimp";
enum LFlagsExec = "-O3 -Wl,--no-undefined -Wl,-rpath,. -Lobjs/examplegame.objs -L. -lhydra -ggdb -fdiagnostics-color=always -fopenmp";

enum CompileCommand {
	CompileLib = "g++ -c " ~ CFlagsLib ~ " $in -o $out",
	CompileExec = "g++ -c " ~ CFlagsExec ~ " $in -o $out",
	LinkLib = "g++ " ~ LFlagsLib ~ " $in -o $out",
	LinkExec = "g++ " ~ LFlagsExec ~ " $in -o $out"
}

Target[] MakeObjects(string src, CompileCommand cmd)() {
	import std.file : dirEntries, SpanMode;
	import std.process : executeShell;
	import std.algorithm : map;
	import std.array : array, replace, split;
	import std.range : chain;

	Target[] objs;

	foreach (f; chain(dirEntries(src, "*.cpp", SpanMode.breadth), dirEntries(src, "*.c", SpanMode.breadth)).filter!(x => !x.isDir)) {
		auto exec = executeShell("g++ -MM " ~ f);
		if (exec.status) {
			import std.stdio : stderr;

			stderr.writeln("Returned: ", exec.status, "\n", exec.output);
			assert(0);
		}

		auto head = exec.output.split(":")[1].replace("\n", " ").split(" ").filter!(s => !s.empty && s != "\\").map!(x => Target(x)).array;
		objs ~= Target(f ~ ".o", cmd, [Target(f)], head);
	}

	return objs;
}

Build myBuild() {
	auto libhydra = Target("libhydra.so", CompileCommand.LinkLib, MakeObjects!("hydra/src/", CompileCommand.CompileLib));
	auto example = Target("examplegame", CompileCommand.LinkExec, MakeObjects!("example/src/", CompileCommand.CompileExec), [libhydra]);

	return Build(example);
}
