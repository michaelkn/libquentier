/*
 * Copyright 2016 Dmitry Ivanov
 *
 * This file is part of libquentier
 *
 * libquentier is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * libquentier is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libquentier. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIB_QUENTIER_NOTE_EDITOR_RESOURCE_FILES_STORAGE_MANAGER_H
#define LIB_QUENTIER_NOTE_EDITOR_RESOURCE_FILES_STORAGE_MANAGER_H

#include <quentier/utility/Macros.h>
#include <quentier/utility/FileSystemWatcher.h>
#include <quentier/types/ErrorString.h>
#include <QObject>
#include <QUuid>
#include <QStringList>
#include <QHash>
#include <QScopedPointer>

QT_FORWARD_DECLARE_CLASS(QWidget)

namespace quentier {

QT_FORWARD_DECLARE_CLASS(Note)

/**
 * @brief The ResourceFileStorageManager class is intended to provide the service of
 * reading and writing the resource data from/to local files. The purpose of having
 * a separate class for that is to encapsulate the logics around the checks for resource
 * files actuality and also to make it possible to move all the resource file IO into a separate thread.
 */
class Q_DECL_HIDDEN ResourceFileStorageManager: public QObject
{
    Q_OBJECT
public:
    explicit ResourceFileStorageManager(const QString & nonImageResourceFileStorageFolderPath,
                                        const QString & imageResourceFileStorageFolderPath,
                                        QObject * parent = Q_NULLPTR);

    struct Errors
    {
        enum type
        {
            EmptyLocalUid = -1,
            EmptyRequestId = -2,
            EmptyData = -3,
            NoResourceFileStorageLocation = -4
        };
    };

Q_SIGNALS:
    void writeResourceToFileCompleted(QUuid requestId, QByteArray dataHash,
                                      QString fileStoragePath, int errorCode, ErrorString errorDescription);
    void readResourceFromFileCompleted(QUuid requestId, QByteArray data, QByteArray dataHash,
                                       int errorCode, ErrorString errorDescription);

    void resourceFileChanged(QString localUid, QString fileStoragePath);

    void diagnosticsCollected(QUuid requestId, QString diagnostics);

public Q_SLOTS:
    /**
     * @brief onWriteResourceToFileRequest - slot being called when the resource data needs to be written
     * to local file; the method would also check that the already existing file (if any) is actual.
     * If so, it would return successfully without doing any IO.
     * @param noteLocalUid - the local uid of the note to which the resource belongs
     * @param resourceLocalUid - the local uid of the resource for which the data is written to file
     * @param data - the resource data to be written to file
     * @param dataHash - the hash of the resource data; if it's empty, it would be calculated by the method itself
     * @param preferredFileSuffix - the preferred file suffix for the resource; if empty, the resource file is written withoug suffix
     * @param requestId - request identifier for writing the data to file
     * @param isImage - indicates whether the resource is the image which can be displayed inline in the note editor page
     */
    void onWriteResourceToFileRequest(QString noteLocalUid, QString resourceLocalUid, QByteArray data, QByteArray dataHash,
                                      QString preferredFileSuffix, QUuid requestId, bool isImage);

    /**
     * @brief onReadResourceFromFileRequest - slot being called when the resource data and hash need to be read
     * from local file
     * @param fileStoragePath - the path at which the resource is stored
     * @param resourceLocalUid - the local uid of the resource for which the data and hash should be read from file
     * @param requestId - request identifier for reading the resource data and hash from file
     */
    void onReadResourceFromFileRequest(QString fileStoragePath, QString resourceLocalUid, QUuid requestId);

    /**
     * @brief onOpenResourceRequest - slot being called when the resource file is requested to be opened
     * in any external program for viewing and/or editing; the resource file storage manager would watch
     * for the changes of this resource until the current note in the note editor is changed or until the file
     * is replaced with its own next version, for example
     */
    void onOpenResourceRequest(QString fileStoragePath);

    /**
     * @brief onCurrentNoteChanged - slot which should be called when the current note in the note editor is changed;
     * when the note is changed, this object stops watching for the changes of resource files belonging
     * to the previously edited note in the note editor; it is due to the performance cost and OS limitations
     * for the number of files which can be monitored for changes simultaneously by one process
     */
    void onCurrentNoteChanged(Note note);

    /**
     * @brief onRequestDiagnostics - slot which initiates the collection of diagnostics regarding the internal state of
     * ResourceFileStorageManager; intended primarily for troubleshooting purposes
     */
    void onRequestDiagnostics(QUuid requestId);

private Q_SLOTS:
    void onFileChanged(const QString & path);
    void onFileRemoved(const QString & path);

private:
    void createConnections();
    QByteArray calculateHash(const QByteArray & data) const;
    bool checkIfResourceFileExistsAndIsActual(const QString & noteLocalUid, const QString & resourceLocalUid,
                                              const QString & fileStoragePath, const QByteArray & dataHash) const;

    bool updateResourceHash(const QString & resourceLocalUid, const QByteArray & dataHash,
                            const QString & storageFolderPath, int & errorCode, ErrorString & errorDescription);
    void watchResourceFileForChanges(const QString & resourceLocalUid, const QString & fileStoragePath);
    void stopWatchingResourceFile(const QString & filePath);

    void removeStaleResourceFilesFromCurrentNote();

private:
    QString     m_nonImageResourceFileStorageLocation;
    QString     m_imageResourceFileStorageLocation;

    QScopedPointer<Note>                m_pCurrentNote;

    QHash<QString, QString>             m_resourceLocalUidByFilePath;
    FileSystemWatcher                   m_fileSystemWatcher;
};

} // namespace quentier

#endif // LIB_QUENTIER_NOTE_EDITOR_RESOURCE_FILES_STORAGE_MANAGER_H
