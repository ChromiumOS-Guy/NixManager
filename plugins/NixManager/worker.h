#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QThread>
#include "worker-logic.h"

/**
 * @brief The Worker class performs long-running, synchronous tasks
 * by calling WorkerLogic methods. It must inherit QObject to use signals/slots,
 * and it is designed to be moved into a QThread.
 *
 * This object is the active component residing in the separate thread,
 * handling the blocking calls to the underlying system layer.
 */
class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(QObject *parent = nullptr);

public slots:
    // =========================================================================
    // Slot wrappers that call WorkerLogic
    // These slots are invoked asynchronously via QMetaObject::invokeMethod
    // from the Controller in the main thread.
    // NOTE: All slots now include the 'operation' string, passed from Controller
    // to distinguish the request type upon result emission.
    // =========================================================================
    void hm_switch(bool allow_insecure, const QVariant& requestId, const QString& operation);
    void hm_version(const QVariant& requestId, const QString& operation);
    void read_packages(const QString& packageType, const QVariant& requestId, const QString& operation);
    void add_packages(const QString& packagesJsonString, bool allow_insecure, const QString& packageType, bool overwrite, const QVariant& requestId, const QString& operation);
    void delete_packages(const QString& packagesJsonString, const QString& packageType, const QVariant& requestId, const QString& operation);
    void search_packages(const QString& quarry, bool local, const QString& base_url, int timeout, const QVariant& requestId, const QString& operation);
    void update_channels(const QVariant& requestId, const QString& operation);
    void list_channels(const QVariant& requestId, const QString& operation);
    void add_channel(const QString& url, const QString& name, const QVariant& requestId, const QString& operation);
    void remove_channel(const QString& name, const QVariant& requestId, const QString& operation);
    void list_generations(const QVariant& requestId, const QString& operation);
    void switch_generation(const QString& generation_id, const QVariant& requestId, const QString& operation);
    void delete_generation(const QString& generation_id, const QVariant& requestId, const QString& operation);
    void delete_old_generations(const QVariant& requestId, const QString& operation);
    void hm_expire_generations(const QString& timestamp, const QVariant& requestId, const QString& operation);
    void hm_list_generations(const QVariant& requestId, const QString& operation);
    void install_nix_home_manager(const QString& version, const QVariant& requestId, const QString& operation);
    void uninstall_nix_home_manager(const QVariant& requestId, const QString& operation);
    void detect_nix_home_manager(const QVariant& requestId, const QString& operation);


signals:
    /**
     * @brief Signal emitted after a synchronous WorkerLogic function returns.
     * This signal travels back to the main thread's Controller.
     * @param resultJson JSON string containing the operation's result.
     * @param requestId Original request identifier to match results to requests.
     * @param operation The name of the method that completed (e.g., "hm_version").
     */
    void operation_finished(const QString& resultJson, const QVariant& requestId, const QString& operation);
};

#endif // WORKER_H