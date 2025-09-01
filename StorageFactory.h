#include "S3BackupStorage.h"

// Factory pattern with RAII-friendly return type (unique_ptr)
class BackupStorageFactory {
public:
	static std::unique_ptr<BackupStorage> Create(
		const std::string& type,
		const std::string& clientId,
		const std::string& volumeId,
		const std::string& region)
	{
		if (type == "s3" || type == "glacier") {
			return std::unique_ptr<BackupStorage>(new S3BackupStorage(clientId, volumeId, region));
		}
		// Could add: "filesystem", "azure", etc.
		return nullptr;
	}
};