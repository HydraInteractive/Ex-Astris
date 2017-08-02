#include <hydra/world/blueprintloader.hpp>

#include <cstdio>
#include <cstdlib>

using namespace Hydra::World;

std::unique_ptr<Blueprint> BlueprintLoader::load(const std::string& file) {
	std::unique_ptr<Blueprint> bp;

	char * data = nullptr;
	size_t len = 0;

	{
		FILE* fp = fopen(file.c_str(), "rb");
		if (!fp)
			return bp;

		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		data = static_cast<char*>(malloc(len));
		fread(data, len, 1, fp);

		fclose(fp);
	}

	bp = std::make_unique<Blueprint>();

	unpack(bp->objectHandle, data, len);

	free(data);

	msgpack::object object = bp->objectHandle.get();

	if (object.via.array.size != 2)
		return std::unique_ptr<Blueprint>();

	bp->name = object.via.array.ptr[0].as<std::string>();
	bp->data = &object.via.array.ptr[1].via.map;

	return bp;
}

void BlueprintLoader::save(const std::string& file, const std::string& name, std::shared_ptr<IEntity> entity) {
	msgpack::sbuffer buffer;
	msgpack::packer<msgpack::sbuffer> o{buffer};

	o.pack_array(2);
	o.pack(name);
	entity->pack(o);

	FILE* fp = fopen(file.c_str(), "wb");
	if (!fp)
		return;

	fwrite(buffer.data(), buffer.size(), 1, fp);

	fclose(fp);
}
