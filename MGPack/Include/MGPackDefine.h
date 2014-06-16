/******************************************************************************/
#ifndef _MGPACKDEFINE_H_
#define _MGPACKDEFINE_H_
/******************************************************************************/

class CZipMemFile;
class CZipArchive;

namespace MG
{

	/******************************************************************************/
	struct MGExtractedPackFile
	{
		CZipMemFile* zmem_file;
		int	 		 st_size;
		std::string	 whole_path;

		MGExtractedPackFile()
			:zmem_file(NULL),
			st_size(0),
			whole_path("")
		{

		}
	};
	/******************************************************************************/
	struct MGFileInfo {
		/// The archive in which the file has been found (for info when performing
		/// multi-Archive searches, note you should still open through ResourceGroupManager)
		//Archive* archive;
		/// The file's fully qualified name
		std::string filename;
		/// Path name; separated by '/' and ending with '/'
		std::string path;
		/// Base filename
		std::string basename;
		/// type
		std::string filetype;
		/// Compressed size
		unsigned int compressedSize;
		/// Uncompressed size
		unsigned int uncompressedSize;
	};

	/******************************************************************************/
	typedef std::vector<MGFileInfo> FileInfoVec;
	typedef std::map<std::string, std::string> StringMap;

	// 做同一路径下文件列表的查找
	typedef std::map<std::string, FileInfoVec>	ClassifyPathMap;
	typedef std::map<std::string, ClassifyPathMap*>	ResClassifyPathMap;

	// 做同一路径下同一类型文件列表的查找
	typedef std::multimap<std::string, MGFileInfo>	ClassifyPathMulMap;
	typedef std::map<std::string, ClassifyPathMulMap>	ClassifyPathTypeMap;
	typedef std::map<std::string, ClassifyPathTypeMap*>	ResClassifyPathTypeMap;

	/******************************************************************************/

}

#endif