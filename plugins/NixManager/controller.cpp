#include "controller.h"
#include <QDebug>
#include <QThread>
#include <QVariant> // Needed for Q_ARG(QVariant, ...)

Controller::Controller(QObject *parent)
    : QObject(parent), m_worker(new Worker)
{
    // 1. Move the Worker object to the newly created thread
    m_worker->moveToThread(&m_workerThread);

    // 2. Connect the Worker's signal (result) to the Controller's signal (result)
    // This pipes the result back to the main thread listener
    connect(m_worker, &Worker::operation_finished,
            this, &Controller::operation_result);

    // 3. Ensure proper cleanup: when the thread finishes, delete the worker object.
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    // 4. Start the thread
    m_workerThread.start();

    qDebug() << "Controller initialized. Controller in thread:"
             << QThread::currentThread()
             << "Worker is in thread:"
             << m_workerThread.currentThread();
}

Controller::~Controller()
{
    qDebug() << "Stopping Worker thread...";
    // Request the thread to quit and wait for it to stop gracefully
    m_workerThread.quit();
    m_workerThread.wait();
    // The m_worker object will be automatically deleted via QObject::deleteLater
    // once the thread finishes (due to the connection in the constructor).
    qDebug() << "Worker thread stopped.";
}

// NOTE: The incorrect CONTROLLER_REQUEST macro has been removed.
// The functions below now use explicit QMetaObject::invokeMethod with Q_ARG 
// for each parameter, which is the correct and safest way for Qt concurrent calls.

void Controller::request_hm_switch(const QVariant& requestId, bool allow_insecure)
{
    QMetaObject::invokeMethod(m_worker, "hm_switch", Qt::QueuedConnection,
        Q_ARG(bool, allow_insecure),
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "hm_switch"));
}

void Controller::request_hm_version(const QVariant& requestId)
{
    QMetaObject::invokeMethod(m_worker, "hm_version", Qt::QueuedConnection,
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "hm_version"));
}

void Controller::request_read_packages(const QVariant& requestId, const QString& packageType)
{
    QMetaObject::invokeMethod(m_worker, "read_packages", Qt::QueuedConnection,
        Q_ARG(QString, packageType),
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "read_packages"));
}

void Controller::request_add_packages(const QVariant& requestId, const QString& packagesJsonString, bool allow_insecure, const QString& packageType, bool overwrite)
{
    QMetaObject::invokeMethod(m_worker, "add_packages", Qt::QueuedConnection,
        Q_ARG(QString, packagesJsonString),
        Q_ARG(bool, allow_insecure),
        Q_ARG(QString, packageType),
        Q_ARG(bool, overwrite),
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "add_packages"));
}

void Controller::request_delete_packages(const QVariant& requestId, const QString& packagesJsonString, const QString& packageType)
{
    QMetaObject::invokeMethod(m_worker, "delete_packages", Qt::QueuedConnection,
        Q_ARG(QString, packagesJsonString),
        Q_ARG(QString, packageType),
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "delete_packages"));
}

void Controller::request_search_packages(const QVariant& requestId, const QString& quarry, bool local, const QString& base_url, int timeout)
{
    QMetaObject::invokeMethod(m_worker, "search_packages", Qt::QueuedConnection,
        Q_ARG(QString, quarry),
        Q_ARG(bool, local),
        Q_ARG(QString, base_url),
        Q_ARG(int, timeout),
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "search_packages"));
}

void Controller::request_update_channels(const QVariant& requestId)
{
    QMetaObject::invokeMethod(m_worker, "update_channels", Qt::QueuedConnection,
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "update_channels"));
}

void Controller::request_list_channels(const QVariant& requestId)
{
    QMetaObject::invokeMethod(m_worker, "list_channels", Qt::QueuedConnection,
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "list_channels"));
}

void Controller::request_add_channel(const QVariant& requestId, const QString& url, const QString& name)
{
    QMetaObject::invokeMethod(m_worker, "add_channel", Qt::QueuedConnection,
        Q_ARG(QString, url),
        Q_ARG(QString, name),
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "add_channel"));
}

void Controller::request_remove_channel(const QVariant& requestId, const QString& name)
{
    QMetaObject::invokeMethod(m_worker, "remove_channel", Qt::QueuedConnection,
        Q_ARG(QString, name),
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "remove_channel"));
}

void Controller::request_list_generations(const QVariant& requestId)
{
    QMetaObject::invokeMethod(m_worker, "list_generations", Qt::QueuedConnection,
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "list_generations"));
}

void Controller::request_switch_generation(const QVariant& requestId, const QString& generation_id)
{
    QMetaObject::invokeMethod(m_worker, "switch_generation", Qt::QueuedConnection,
        Q_ARG(QString, generation_id),
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "switch_generation"));
}

void Controller::request_delete_generation(const QVariant& requestId, const QString& generation_id)
{
    QMetaObject::invokeMethod(m_worker, "delete_generation", Qt::QueuedConnection,
        Q_ARG(QString, generation_id),
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "delete_generation"));
}

void Controller::request_delete_old_generations(const QVariant& requestId)
{
    QMetaObject::invokeMethod(m_worker, "delete_old_generations", Qt::QueuedConnection,
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "delete_old_generations"));
}

void Controller::request_hm_expire_generations(const QVariant& requestId, const QString& timestamp)
{
    QMetaObject::invokeMethod(m_worker, "hm_expire_generations", Qt::QueuedConnection,
        Q_ARG(QString, timestamp),
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "hm_expire_generations"));
}

void Controller::request_hm_list_generations(const QVariant& requestId)
{
    QMetaObject::invokeMethod(m_worker, "hm_list_generations", Qt::QueuedConnection,
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "hm_list_generations"));
}

void Controller::request_install_nix_home_manager(const QVariant& requestId, const QString& version)
{
    QMetaObject::invokeMethod(m_worker, "install_nix_home_manager", Qt::QueuedConnection,
        Q_ARG(QString, version),
        Q_ARG(QVariant, requestId),
        Q_ARG(QString, "install_nix_home_manager"));
}

void Controller::request_uninstall_nix_home_manager(const QVariant& requestId)
{
    QMetaObject::invokeMethod(m_worker, "uninstall_nix_home_manager", Qt::QueuedConnection,
        Q_ARG(QVariant, requestId), Q_ARG(QString, "uninstall_nix_home_manager"));
}

void Controller::request_detect_nix_home_manager(const QVariant& requestId)
{
    QMetaObject::invokeMethod(m_worker, "detect_nix_home_manager", Qt::QueuedConnection,
        Q_ARG(QVariant, requestId), Q_ARG(QString, "detect_nix_home_manager"));
}