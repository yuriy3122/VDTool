#ifndef BACKUPSTORAGE_H
#define BACKUPSTORAGE_H

#include "CommonTypes.h"

//abstract class, inherit from this class to implement custom backup storage
class BackupStorage
{
public:
	BackupStorage() {};

	BackupStorage(std::string clientId, std::string volumeId, std::string region) :
		m_clientId(clientId),
		m_volumeId(volumeId),
		m_region(region)
	{
	}

	virtual int GetFreeBufferOffsetIndex() = 0;

	virtual void UploadBackupSectorDataAsync(std::string backupId, std::string item, std::string key, const char* bufferOffset, int bufferOffsetIndex, size_t bufferSize) = 0;

	virtual void WaitForAllUploadTasksToComplete() = 0;

	virtual void UploadBackupMetaData(std::string backupId, BackupMetaData &metadata) = 0;

	virtual VolumeMetaData GetVolumeMetaData(std::string volumeId) = 0;

	virtual BackupMetaData GetBackupMetaData(std::string backupId) = 0;

	virtual void UploadRestoreTaskMetaData(const RestoreTaskMetaData &metadata) = 0;

	virtual RestoreTaskMetaData GetRestoreTaskMetaData(std::string restoreId) = 0;

	virtual int GetBackupBlockData(std::string backupId, int partId, std::string key, const std::vector<int>& indices, char* buffer) = 0;

	virtual int ListObjects(std::string backupId, int partId, std::vector<int>& objects) = 0;

protected:
	std::string m_clientId;
	std::string m_volumeId;
	std::string m_region;
};

#endif
