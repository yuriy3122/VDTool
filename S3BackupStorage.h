#include <fstream>
#include <aws/core/Aws.h>
#include <aws/core/utils/threading/Executor.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/client/DefaultRetryStrategy.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include "core/membuf.h"
#include "core/thread_safe_queue.h"
#include "BackupStorage.h"

class S3BackupStorage final: public BackupStorage
{
public:
	S3BackupStorage(std::string clientId, std::string volumeId, std::string region);

	S3BackupStorage() = delete;
	S3BackupStorage(const S3BackupStorage&) = delete;
	S3BackupStorage& operator =(const S3BackupStorage&) = delete;
	S3BackupStorage(S3BackupStorage&&) = delete;
	S3BackupStorage& operator =(S3BackupStorage&&) = delete;

	~S3BackupStorage();

	int GetFreeBufferOffsetIndex() override;

	void UploadBackupSectorDataAsync(std::string backupId, std::string item, std::string key, const char* bufferOffset, int bufferOffsetIndex, size_t bufferSize) override;

	void WaitForAllUploadTasksToComplete() override;

	void UploadBackupMetaData(std::string backupId, BackupMetaData &metadata) override;

	VolumeMetaData GetVolumeMetaData(std::string volumeId) override;

	BackupMetaData GetBackupMetaData(std::string backupId) override;

	void UploadRestoreTaskMetaData(const RestoreTaskMetaData &metadata) override;

	RestoreTaskMetaData GetRestoreTaskMetaData(std::string restoreId) override;

	int GetBackupBlockData(std::string backupId, int partId, std::string key, const std::vector<int>& indices, char* buffer) override;

	int ListObjects(std::string backupId, int partId, std::vector<int>& objects) override;

private:
	std::string GetVolumeBucket() const;

	long m_connectTimeoutMs;
	long m_requestTimeoutMs;

	Aws::SDKOptions m_options;

	Aws::S3::S3Client *m_s3Client;
};
