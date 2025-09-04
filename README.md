# VDTool — VMware VDDK → Amazon S3 Backup

VDTool is a modern C++ utility that performs **full and incremental** backups of VMware virtual disks via the **VMware Virtual Disk Development Kit (VDDK)** and stores the data in **Amazon S3**. It is designed to be robust, and production‑ready, with attention to clean architecture, memory safety, and high throughput uploads.

---

## ✨ Highlights

- **Full & Incremental** backups (leveraging Changed Block Tracking feature).
- **Amazon S3 storage backend** with concurrent multipart-style uploads.
- **Compression + Integrity**: zlib compression and CRC verification.
- **Clear, testable boundaries** via OOP (pure virtual BackupStorage), pluggable storage backends via **Factory pattern**.
- **Safe concurrency** with a **thread-safe queue** abstraction.

---

## 📁 Project layout

```
vdtool (entry)
  └── BackupProcessor              // Orchestrates VDDK connect/open/read & writes
      ├── VDDK (vixDiskLib)        // VMware disk API
      ├── core/                    // Utilities
      │   ├── file_handler.h       // File I/O
      │   ├── compression.h        // Zlib wrapper
      │   ├── crc32.h              // CRC helper (see notes below)
      │   ├── membuf.h             // In‑memory stream
      │   └── thread_safe_queue.h  // Safe Queue
      ├── BackupStorage (abstract) // Pure virtual storage interface
      │   └── S3BackupStorage      // Concrete implementation using AWS C++ SDK
      └── StorageFactory           // Factory for BackupStorages
```

**Block format** (1 MiB logical blocks):  
Each “block” includes a tiny header with sector metadata (selected sector IDs) followed by compressed data. Blocks are grouped into **1 GiB partitions** when uploaded to S3:  
`{volumeId}/backups/{backupId}/blockdata/{partId}/{blockId}`

---

## ✅ Project Requirements

- **Compiler/IDE**: **Visual Studio 2022**
- **Windows SDK**: **10.0.26100**
- **Platform Toolset**: **v141** (VS 2017 toolset; set in project properties → General → Platform Toolset)
- **Language standard**: **C++11 or later** (project is written in C++11 style; most code is C++14/17 clean as well)
- **Third‑party SDKs/libraries**
  - **VMware VDDK 7.0.x** (headers & libs present under `include/` and `lib/` in this repo)
  - **AWS SDK for C++** (Core + S3) – via NuGet packages (example versions in `packages.config`)
  - **zlib** (bundled in `core/` and `core/zlib/`)

> ℹ️ If you open the project and NuGet reports missing packages, run **Restore NuGet Packages** from Visual Studio. Ensure the Windows SDK version above is installed via the VS Installer.

---

## 🏗️ Build (Visual Studio)

1. Open **`vdtool.vcxproj`** in **Visual Studio 2022**.
2. Right‑click the solution → **Restore NuGet Packages**.
3. In **Project Properties**:
   - **Configuration**: `Release` (recommended) or `Debug`
   - **Platform**: `x64`
   - **Platform Toolset**: `v141`
   - **Windows SDK**: `10.0.26100.0`
4. Ensure include/lib paths for **VDDK** and **zlib** are correct (already set in the project).
5. Build → **Build Solution**.

---

## ▶️ Run

VDTool reads parameters from **`input.json`** in the working directory. A minimal example:

```jsonc
{
  "clientId": "my-backup-bucket",           // S3 bucket name
  "volumeId": "vm-42",                       // Logical volume / VM identifier
  "region": "eu-west-1",                     // AWS region for the bucket
  "device": "\\\\.\\PhysicalDrive2",         // VDDK open target or a VMDK path
  "vmdk": "C:\\data\\disk1.vmdk",            // optional: local VMDK path
  "snapshotRef": "",                         // optional: VDDK snapshot reference
  "fullBackup": true,                        // full or incremental
  "restore": false,                          // set true to perform restore
  "volumeSize": 128,                         // number of 1 GiB parts (for block grouping)
  "changedDiskAreasFilePath": ""             // optional pre-computed changed areas file
}
```

**AWS credentials** are resolved via the standard AWS SDK chain (env vars, `~/.aws/credentials`, EC2/ECS metadata, etc.).

Run the tool from the build output directory:
```powershell
vdtool.exe
```
A simple log is written to `log.txt` in the current directory.

---

## 🔐 Security & Reliability

- **Transport security**: S3 client uses HTTPS.
- **S3 features**: Server‑side encryption (SSE‑S3/KMS), storage class, lifecycle policies (Glacier), and custom retry/backoff are easy to enable via the AWS SDK options.
- **Integrity**: CRC validation over block payloads.
- **Resilience**: Concurrent uploads with bounded parallelism to maintain backpressure and avoid memory spikes.
