#include "S3BackupStorage.h"

using namespace std;
using namespace Aws;
using namespace Aws::S3;
using namespace Aws::S3::Model;

atomic_int upload_tasks_running(0);

void PutObjectResultHandler(const S3Client* client,
                            const PutObjectRequest& request,
                            const PutObjectOutcome& outcome,
                            const shared_ptr<const Aws::Client::AsyncCallerContext>& baseCtx)
{
    auto ctx = static_pointer_cast<const S3UploadContext>(baseCtx);

    if (!outcome.IsSuccess())
    {
        cout << "Error: " << outcome.GetError().GetMessage() << endl;
    }

    // Return buffer index into the correct instance of S3BackupStorage
    ctx->storage->m_upload_queue.push(ctx->bufferIndex);

    upload_tasks_running--;
}

S3BackupStorage::S3BackupStorage(string clientId,
                                 string volumeId,
                                 string region)
    : m_upload_queue(UploadBatchSize)
{
    m_connectTimeoutMs = 30000;
    m_requestTimeoutMs = 600000;

    m_clientId = clientId;
    m_volumeId = volumeId;
    m_region = region;

    InitAPI(m_options);

    Client::ClientConfiguration config;
    config.scheme = Http::Scheme::HTTPS;
    config.region = m_region.c_str();
    config.connectTimeoutMs = m_connectTimeoutMs;
    config.requestTimeoutMs = m_requestTimeoutMs;
    config.executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("ThreadPool", 20);

    m_s3Client = new S3Client(config);

    // Fill queue with free buffer indices
    for (int i = 0; i < UploadBatchSize; i++)
        upload_queue.push(i);
}

S3BackupStorage::~S3BackupStorage()
{
    delete m_s3Client;

    ShutdownAPI(m_options);
}

int S3BackupStorage::GetFreeBufferOffsetIndex()
{
    auto v = upload_queue.pop();
    return v.value();   // safe, pop blocks until value exists
}

void S3BackupStorage::UploadBackupSectorDataAsync(string backupId,
                                                  string item,
                                                  string key,
                                                  const char* bufferOffset,
                                                  int bufferOffsetIndex,
                                                  size_t bufferSize)
{
    streambuf* buf = new membuf((char*)bufferOffset,
                                (char*)bufferOffset + bufferSize);

    auto objectStream = Aws::MakeShared<Aws::IOStream>("BlockUpload", buf);

    PutObjectRequest request;
    request.WithBucket(GetVolumeBucket() + "/backups/" + backupId + "/blockdata/").WithKey(item);
    request.SetBody(objectStream);
    request.SetContentLength(bufferSize);

    // Create our custom context with pointer to *this* and buffer index
    auto context = Aws::MakeShared<S3UploadContext>("PutCtx", this, bufferOffsetIndex);

    upload_tasks_running++;

    m_s3Client->PutObjectAsync(request, PutObjectResultHandler, context);
}
