// worker.cpp
#include "worker.h"
#include "worker-logic.h"
#include <QDebug>
#include <QThread>

Worker::Worker(QObject *parent) : QObject(parent)
{
    // Log the thread ID to confirm this object lives in a separate thread
    qDebug() << "Worker created in thread:" << QThread::currentThread();
}

// Macro to simplify implementation of slots calling WorkerLogic and emitting the result
// The WorkerLogic::func is the *sync function name*
// Arguments: (WorkerLogic sync function, requestId, operation name string, ...WorkerLogic args)
#define WORKER_LOGIC_SLOT(logic_func, req_id, op_name, logic_args) \
{ \
    qDebug() << "Worker: Starting " << op_name << " in thread:" << QThread::currentThread(); \
    QString result = WorkerLogic::logic_func logic_args; \
    emit operation_finished(result, req_id, op_name); \
    qDebug() << "Worker: Finished " << op_name; \
}

// Note: All slot signatures now include 'const QString& operation' as the final parameter.

void Worker::hm_switch(bool allow_insecure, const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(hm_switch_sync, requestId, operation, (allow_insecure));
}

void Worker::hm_version(const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(hm_version_sync, requestId, operation, ());
}

void Worker::read_packages(const QString& packageType, const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(read_packages_sync, requestId, operation, (packageType));
}

void Worker::add_packages(const QString& packagesJsonString, bool allow_insecure, const QString& packageType, bool overwrite, const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(add_packages_sync, requestId, operation, (packagesJsonString, allow_insecure, packageType, overwrite));
}

void Worker::delete_packages(const QString& packagesJsonString, const QString& packageType, const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(delete_packages_sync, requestId, operation, (packagesJsonString, packageType));
}

void Worker::search_packages(const QString& quarry, bool local, const QString& base_url, int timeout, const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(search_packages_sync, requestId, operation, (quarry, local, base_url, timeout));
}

void Worker::update_channels(const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(update_channels_sync, requestId, operation, ());
}

void Worker::list_channels(const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(list_channels_sync, requestId, operation, ());
}

void Worker::add_channel(const QString& url, const QString& name, const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(add_channel_sync, requestId, operation, (url, name));
}

void Worker::remove_channel(const QString& name, const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(remove_channel_sync, requestId, operation, (name));
}

void Worker::list_generations(const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(list_generations_sync, requestId, operation, ());
}

void Worker::switch_generation(const QString& generation_id, const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(switch_generation_sync, requestId, operation, (generation_id));
}

void Worker::delete_generation(const QString& generation_id, const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(delete_generation_sync, requestId, operation, (generation_id));
}

void Worker::delete_old_generations(const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(delete_old_generations_sync, requestId, operation, ());
}

void Worker::hm_expire_generations(const QString& timestamp, const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(hm_expire_generations_sync, requestId, operation, (timestamp));
}

void Worker::hm_list_generations(const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(hm_list_generations_sync, requestId, operation, ());
}

void Worker::install_nix_home_manager(const QString& version, const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(install_nix_home_manager_sync, requestId, operation, (version));
}

void Worker::uninstall_nix_home_manager(const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(uninstall_nix_home_manager_sync, requestId, operation, ());
}

void Worker::detect_nix_home_manager(const QVariant& requestId, const QString& operation)
{
    WORKER_LOGIC_SLOT(detect_nix_home_manager_sync, requestId, operation, ());
}

#undef WORKER_LOGIC_SLOT