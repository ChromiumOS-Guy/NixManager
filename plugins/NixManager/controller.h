// controller.h

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QThread>
#include <QVariant>
#include "worker.h"

/**
 * @brief The Controller class manages the QThread and Worker lifecycle.
 * It provides the public, asynchronous API to access Nix/Home Manager functionality
 * by forwarding requests (via signals/slots) to the Worker object in the separate thread.
 */
class Controller : public QObject
{
    Q_OBJECT

public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller();

    // =========================================================================
    // Public Asynchronous API (Slots that trigger the Worker)
    // These slots are called by the main application logic.
    // =========================================================================

public slots:
    void request_hm_switch(const QVariant& requestId, const bool allow_insecure = false);
    void request_hm_version(const QVariant& requestId);
    void request_read_packages(const QVariant& requestId, const QString& packageType = QString::fromStdString("home"));
    void request_add_packages(const QVariant& requestId, const QString& packagesJsonString, bool allow_insecure = false, const QString& packageType = QString::fromStdString("home"), bool overwrite = false);
    void request_delete_packages(const QVariant& requestId, const QString& packagesJsonString, const QString& packageType = QString::fromStdString("home"));
    void request_search_packages(const QVariant& requestId, const QString& quarry, const bool local = false, const QString& base_url = QString::fromStdString("https://search.devbox.sh"), const int timeout = 10);
    void request_update_channels(const QVariant& requestId);
    void request_list_channels(const QVariant& requestId);
    void request_add_channel(const QVariant& requestId, const QString& url, const QString& name);
    void request_remove_channel(const QVariant& requestId, const QString& name);
    void request_list_generations(const QVariant& requestId);
    void request_switch_generation(const QVariant& requestId, const QString& generation_id);
    void request_delete_generation(const QVariant& requestId, const QString& generation_id);
    void request_delete_old_generations(const QVariant& requestId);
    void request_hm_expire_generations(const QVariant& requestId, const QString& timestamp = "-30 days");
    void request_hm_list_generations(const QVariant& requestId);
    void request_install_nix_home_manager(const QVariant& requestId, const QString& version);
    void request_uninstall_nix_home_manager(const QVariant& requestId);
    void request_detect_nix_home_manager(const QVariant& requestId);

signals:
    /**
     * @brief Signal emitted when a worker operation completes.
     * This signal carries the result back to the main application thread.
     * @param resultJson JSON string containing the operation's result.
     * @param requestId Original request identifier.
     * @param operation The name of the method that completed (e.g., "hm_version").
     */
    void operation_result(const QString& resultJson, const QVariant& requestId, const QString& operation);

private:
    QThread m_workerThread;
    Worker *m_worker;
};

#endif // CONTROLLER_H