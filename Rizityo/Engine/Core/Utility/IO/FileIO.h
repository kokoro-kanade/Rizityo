#pragma once
#include "CommonHeaders.h"
#include <filesystem>
#include <fstream>

namespace Rizityo::IO
{
	bool ReadFile(std::filesystem::path path, OUT std::unique_ptr<uint8[]>& data, OUT uint64& size);
}