#pragma once

namespace Pistachio
{
	/*
	* The filesystem class offers a way to access resources independent of actual path on the system
	* it consists of 2 different filesystems:
	* - builtin:// :These are resources that come with the engine, like default shaders and the like
	* - asset:// :Project assets. they have to be accesses like this because of release build compression
	*/
	class File
	{
	public:
		void ReadAllToBuf(std::vector<char>& buf);
		void ReadAllToBuf(char* buf);
		void ReadToBuf(std::vector<char>& buf, uint32_t numBytes);
		void ReadToBuf(char* buf, uint32_t numBytes);
	private:
		friend class FileSystem;
		std::fstream file;
		uint64_t offset = 0;
		uint64_t size = 0;
		uint64_t rel_off = 0;
	};
	class FileSystem
	{
	public:
		static File OpenFile(const std::string& in);
	};
}