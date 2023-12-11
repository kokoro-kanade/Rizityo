#include "FileIO.h"

namespace Rizityo::IO
{
	bool ReadFile(std::filesystem::path path, OUT std::unique_ptr<uint8[]>& data, OUT uint64& size)
	{
		if (!std::filesystem::exists(path))
			return false;

		size = std::filesystem::file_size(path);
		assert(size);
		if (!size)
			return false;

		data = std::make_unique<uint8[]>(size);
		std::ifstream file{ path, std::ios::in | std::ios::binary };
		if (!file || !file.read((char*)data.get(), size))
		{
			file.close();
			return false;
		}

		file.close();
		return true;
	}
}