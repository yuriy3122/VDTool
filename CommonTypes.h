#ifndef COMMONTYPES_H
#define COMMONTYPES_H

#include <string>
#include <vector>
#include <map>
#include <basetsd.h>
#include <initguid.h>
#include <guiddef.h>
#include "vixDiskLib.h"

constexpr auto BACKUP_UUID_SIZE = 32;
constexpr auto DATA_BUFFER_SIZE = 1024 * 1024 * 1024;
constexpr auto MB_BLOCK_SIZE = 1024 * 1024;
constexpr auto CMP_SIZE = 1024;
constexpr auto SECTOR_NUM = 2048;

enum BackupStatus
{
	Running = 1,
	Complete = 2,
	Error = 3
};

const int UploadBatchSize = 10;

struct VolumeMetaData
{
	std::vector<std::string> backupIds;
};

struct BackupMetaData
{
	BackupStatus status;
	std::string encryptionKey;
	std::map<uint32_t, uint64_t> blockHashTable;
	std::vector<uint32_t> emptyBlocks;
};

enum RestoreStatus
{
	RestoreRunning = 1,
	RestoreComplete = 2,
	RestoreError = 3
};

struct RestoreTaskMetaData
{
	std::string restoreId;
	RestoreStatus status;
	std::string encryptionKey;
};

struct InputParams
{
	VixDiskLibConnectParams cnxParams;

	std::string libDir;
	std::string cfgFile;
	std::string transportModes;

	std::string snapshotRef;
	bool fullBackup;
	bool restore;
	int volumeSize;
	std::string changedDiskAreasFilePath;

	std::string vmdk;
};

#endif
