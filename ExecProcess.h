#pragma once

#include <vector>

bool ExecProcess(
	LPCTSTR cmdLine,
	const std::vector<uint8_t>& inData,
	std::vector<uint8_t>& outData
	);

