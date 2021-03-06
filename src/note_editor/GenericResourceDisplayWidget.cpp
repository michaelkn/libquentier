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

#include "GenericResourceDisplayWidget.h"
#include "ui_GenericResourceDisplayWidget.h"
#include "NoteEditorSettingsName.h"
#include "ResourceFileStorageManager.h"
#include <quentier/utility/FileIOProcessorAsync.h>
#include <quentier/utility/QuentierCheckPtr.h>
#include <quentier/utility/Utility.h>
#include <quentier/utility/MessageBox.h>
#include <quentier/types/Resource.h>
#include <quentier/logging/QuentierLogger.h>
#include <quentier/utility/ApplicationSettings.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>

namespace quentier {

GenericResourceDisplayWidget::GenericResourceDisplayWidget(QWidget * parent) :
    QWidget(parent),
    m_pUI(new Ui::GenericResourceDisplayWidget),
    m_pResource(Q_NULLPTR),
    m_pResourceFileStorageManager(Q_NULLPTR),
    m_pFileIOProcessorAsync(Q_NULLPTR),
    m_preferredFileSuffixes(),
    m_filterString(),
    m_saveResourceToFileRequestId(),
    m_saveResourceToStorageRequestId(),
    m_account(),
    m_resourceHash(),
    m_savedResourceToStorage(false),
    m_pendingSaveResourceToStorage(false)
{
    m_pUI->setupUi(this);
}

GenericResourceDisplayWidget::~GenericResourceDisplayWidget()
{
    delete m_pUI;
    delete m_pResource;
}

void GenericResourceDisplayWidget::initialize(const QIcon & icon, const QString & name,
                                              const QString & size, const QStringList & preferredFileSuffixes,
                                              const QString & filterString,
                                              const Resource & resource, const Account & account,
                                              const ResourceFileStorageManager & resourceFileStorageManager,
                                              const FileIOProcessorAsync & fileIOProcessorAsync)
{
    QNDEBUG(QStringLiteral("GenericResourceDisplayWidget::initialize: name = ") << name << QStringLiteral(", size = ") << size);

    m_pResource = new Resource(resource);
    m_pResourceFileStorageManager = &resourceFileStorageManager;
    m_pFileIOProcessorAsync = &fileIOProcessorAsync;
    m_preferredFileSuffixes = preferredFileSuffixes;

    m_account = account;

    setupFilterString(filterString);

    updateResourceName(name);
    m_pUI->resourceDisplayNameLabel->setTextFormat(Qt::RichText);

    m_pUI->resourceSizeLabel->setText(QStringLiteral("<html><head/><body><p><span style=\" font-size:8pt;\">") + size +
                                      QStringLiteral("</span></p></body></head></html>"));
    m_pUI->resourceSizeLabel->setTextFormat(Qt::RichText);

    m_pUI->resourceIconLabel->setPixmap(icon.pixmap(QSize(16,16)));

    if (!QIcon::hasThemeIcon(QStringLiteral("document-open"))) {
        m_pUI->openResourceButton->setIcon(QIcon(QStringLiteral(":/generic_resource_icons/png/open_with.png")));
    }

    if (!QIcon::hasThemeIcon(QStringLiteral("document-save-as"))) {
        m_pUI->saveResourceButton->setIcon(QIcon(QStringLiteral(":/generic_resource_icons/png/save.png")));
    }

    QObject::connect(m_pUI->openResourceButton, QNSIGNAL(QPushButton,released),
                     this, QNSLOT(GenericResourceDisplayWidget,onOpenWithButtonPressed));
    QObject::connect(m_pUI->saveResourceButton, QNSIGNAL(QPushButton,released),
                     this, QNSLOT(GenericResourceDisplayWidget,onSaveAsButtonPressed));

    QObject::connect(m_pResourceFileStorageManager, QNSIGNAL(ResourceFileStorageManager,writeResourceToFileCompleted,QUuid,QByteArray,QString,int,ErrorString),
                     this, QNSLOT(GenericResourceDisplayWidget,onSaveResourceToStorageRequestProcessed,QUuid,QByteArray,QString,int,ErrorString));
    QObject::connect(this, QNSIGNAL(GenericResourceDisplayWidget,saveResourceToStorage,QString,QString,QByteArray,QByteArray,QString,QUuid,bool),
                     m_pResourceFileStorageManager, QNSLOT(ResourceFileStorageManager,onWriteResourceToFileRequest,QString,QString,QByteArray,QByteArray,QString,QUuid,bool));

    QObject::connect(m_pFileIOProcessorAsync, QNSIGNAL(FileIOProcessorAsync,writeFileRequestProcessed,bool,ErrorString,QUuid),
                     this, QNSLOT(GenericResourceDisplayWidget,onSaveResourceToFileRequestProcessed,bool,ErrorString,QUuid));
    QObject::connect(this, QNSIGNAL(GenericResourceDisplayWidget,saveResourceToFile,QString,QByteArray,QUuid,bool),
                     m_pFileIOProcessorAsync, QNSLOT(FileIOProcessorAsync,onWriteFileRequest,QString,QByteArray,QUuid,bool));

    if (!m_pResource->hasDataBody() && !m_pResource->hasAlternateDataBody()) {
        QNWARNING(QStringLiteral("Resource passed to GenericResourceDisplayWidget has no data: ") << *m_pResource);
        return;
    }

    const QByteArray & data = (m_pResource->hasDataBody()
                               ? m_pResource->dataBody()
                               : m_pResource->alternateDataBody());

    const QByteArray * dataHash = Q_NULLPTR;
    if (m_pResource->hasDataBody() && m_pResource->hasDataHash()) {
        dataHash = &(m_pResource->dataHash());
    }
    else if (m_pResource->hasAlternateDataBody() && m_pResource->hasAlternateDataHash()) {
        dataHash = &(m_pResource->alternateDataHash());
    }

    QByteArray localDataHash;
    if (!dataHash) {
        dataHash = &localDataHash;
    }

    // Write resource's data to file asynchronously so that it can further be opened in some application
    QString preferredFileSuffix = m_pResource->preferredFileSuffix();
    m_saveResourceToStorageRequestId = QUuid::createUuid();
    bool isImage = (m_pResource->hasMime()
                    ? m_pResource->mime().startsWith(QStringLiteral("image"))
                    : false);

    QNTRACE(QStringLiteral("Emitting the request to save the attachment to own file storage location, request id = ")
            << m_saveResourceToStorageRequestId << QStringLiteral(", resource local uid = ") << m_pResource->localUid());
    Q_EMIT saveResourceToStorage(m_pResource->noteLocalUid(), m_pResource->localUid(), data, *dataHash,
                                 preferredFileSuffix, m_saveResourceToStorageRequestId, isImage);
}

QString GenericResourceDisplayWidget::resourceLocalUid() const
{
    if (m_pResource) {
        return m_pResource->localUid();
    }

    return QString();
}

void GenericResourceDisplayWidget::updateResourceName(const QString & resourceName)
{
    m_pUI->resourceDisplayNameLabel->setText(QStringLiteral("<html><head/><body><p><span style=\" font-size:8pt;\">") +
                                             resourceName + QStringLiteral("</span></p></body></head></html>"));
}

void GenericResourceDisplayWidget::onOpenWithButtonPressed()
{
    if (m_savedResourceToStorage) {
        openResource();
        return;
    }

    setPendingMode(true);
}

void GenericResourceDisplayWidget::onSaveAsButtonPressed()
{
    QNDEBUG(QStringLiteral("GenericResourceDisplayWidget::onSaveAsButtonPressed"));

    QUENTIER_CHECK_PTR(m_pResource);

    if (!m_pResource->hasDataBody() && !m_pResource->hasAlternateDataBody()) {
        QNWARNING(QStringLiteral("Can't save resource: no data body or alternate data body within the resource"));
        internalErrorMessageBox(this, tr("resource has neither data body nor alternate data body"));
        return;
    }

    const QByteArray & data = (m_pResource->hasDataBody()
                               ? m_pResource->dataBody()
                               : m_pResource->alternateDataBody());

    QString preferredSuffix;
    QString preferredDirectory;

    if (!m_preferredFileSuffixes.isEmpty())
    {
        ApplicationSettings appSettings(m_account, NOTE_EDITOR_SETTINGS_NAME);
        QStringList childGroups = appSettings.childGroups();
        int attachmentsSaveLocGroupIndex = childGroups.indexOf(QStringLiteral("AttachmentSaveLocations"));
        if (attachmentsSaveLocGroupIndex >= 0)
        {
            QNTRACE(QStringLiteral("Found cached attachment save location group within application settings"));

            appSettings.beginGroup(QStringLiteral("AttachmentSaveLocations"));
            QStringList cachedFileSuffixes = appSettings.childKeys();
            const int numPreferredSuffixes = m_preferredFileSuffixes.size();
            for(int i = 0; i < numPreferredSuffixes; ++i)
            {
                preferredSuffix = m_preferredFileSuffixes[i];
                int indexInCache = cachedFileSuffixes.indexOf(preferredSuffix);
                if (indexInCache < 0) {
                    QNTRACE(QStringLiteral("Haven't found cached attachment save directory for file suffix ") << preferredSuffix);
                    continue;
                }

                QVariant dirValue = appSettings.value(preferredSuffix);
                if (dirValue.isNull() || !dirValue.isValid()) {
                    QNTRACE(QStringLiteral("Found inappropriate attachment save directory for file suffix ") << preferredSuffix);
                    continue;
                }

                QFileInfo dirInfo(dirValue.toString());
                if (!dirInfo.exists()) {
                    QNTRACE(QStringLiteral("Cached attachment save directory for file suffix ") << preferredSuffix
                            << QStringLiteral(" does not exist: ") << dirInfo.absolutePath());
                    continue;
                }
                else if (!dirInfo.isDir()) {
                    QNTRACE(QStringLiteral("Cached attachment save directory for file suffix ") << preferredSuffix
                            << QStringLiteral(" is not a directory: ") << dirInfo.absolutePath());
                    continue;
                }
                else if (!dirInfo.isWritable()) {
                    QNTRACE(QStringLiteral("Cached attachment save directory for file suffix ") << preferredSuffix
                            << QStringLiteral(" is not writable: ") << dirInfo.absolutePath());
                    continue;
                }

                preferredDirectory = dirInfo.absolutePath();
                break;
            }

            appSettings.endGroup();
        }
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as") + QStringLiteral("..."),
                                                    preferredDirectory, m_filterString);
    if (fileName.isEmpty()) {
        QNINFO(QStringLiteral("User cancelled writing to file"));
        return;
    }

    bool foundSuffix = false;
    const int numPreferredSuffixes = m_preferredFileSuffixes.size();
    for(int i = 0; i < numPreferredSuffixes; ++i)
    {
        const QString & currentSuffix = m_preferredFileSuffixes[i];
        if (fileName.endsWith(currentSuffix, Qt::CaseInsensitive)) {
            foundSuffix = true;
            break;
        }
    }

    if (!foundSuffix) {
        fileName += QStringLiteral(".") + preferredSuffix;
    }

    m_saveResourceToFileRequestId = QUuid::createUuid();
    Q_EMIT saveResourceToFile(fileName, data, m_saveResourceToFileRequestId, /* append = */ false);
    QNDEBUG(QStringLiteral("Sent request to save resource to file, request id = ") << m_saveResourceToFileRequestId);
}

void GenericResourceDisplayWidget::onSaveResourceToStorageRequestProcessed(QUuid requestId, QByteArray dataHash,
                                                                           QString fileStoragePath, int errorCode,
                                                                           ErrorString errorDescription)
{
    if (requestId == m_saveResourceToStorageRequestId)
    {
        if (errorCode == 0)
        {
            QNDEBUG(QStringLiteral("Successfully saved resource to storage, request id = ") << requestId
                    << QStringLiteral(", file storage path = ") << fileStoragePath);
            m_savedResourceToStorage = true;
            m_resourceHash = dataHash;
            if (m_pendingSaveResourceToStorage) {
                setPendingMode(false);
                openResource();
            }
        }
        else
        {
            QNWARNING(QStringLiteral("Could not save resource to storage: ") << errorDescription << QStringLiteral("; request id = ") << requestId);
            warningMessageBox(this, tr("Error saving the resource to hidden file"),
                              tr("Could not save the resource to hidden file "
                                 "(in order to make it possible to open it with some application)"),
                              tr("Error code") + QStringLiteral(" = ") + QString::number(errorCode) + QStringLiteral(": ") +
                              errorDescription.localizedString());
            if (m_pendingSaveResourceToStorage) {
                setPendingMode(false);
            }
        }
    }
    // otherwise it's not ours request reply, skip it
}

void GenericResourceDisplayWidget::onSaveResourceToFileRequestProcessed(bool success,
                                                                        ErrorString errorDescription,
                                                                        QUuid requestId)
{
    QNTRACE(QStringLiteral("GenericResourceDisplayWidget::onSaveResourceToFileRequestProcessed: success = ")
            << (success ? QStringLiteral("true") : QStringLiteral("false")) << QStringLiteral(", request id = ") << requestId);

    if (requestId == m_saveResourceToFileRequestId)
    {
        if (success) {
            QNDEBUG(QStringLiteral("Successfully saved resource to file, request id = ") << requestId);
            Q_EMIT savedResourceToFile();
        }
        else {
            QNWARNING(QStringLiteral("Could not save resource to file: ") << errorDescription << QStringLiteral("; request id = ") << requestId);
            warningMessageBox(this, tr("Error saving the resource to file"),
                              tr("Could not save the resource to file"),
                              errorDescription.localizedString());
        }
    }
}

void GenericResourceDisplayWidget::setPendingMode(const bool pendingMode)
{
    QNDEBUG(QStringLiteral("GenericResourceDisplayWidget::setPendingMode: pending mode = ")
            << (pendingMode ? QStringLiteral("true") : QStringLiteral("false")));

    m_pendingSaveResourceToStorage = pendingMode;
    if (pendingMode) {
        QApplication::setOverrideCursor(Qt::BusyCursor);
    }
    else {
        QApplication::restoreOverrideCursor();
    }
}

void GenericResourceDisplayWidget::openResource()
{
    QNDEBUG(QStringLiteral("GenericResourceDisplayWidget::openResource: hash = ") << m_resourceHash.toHex());
    Q_EMIT openResourceRequest(m_resourceHash);
}

void GenericResourceDisplayWidget::setupFilterString(const QString & defaultFilterString)
{
    QNDEBUG(QStringLiteral("GenericResourceDisplayWidget::setupFilterString: default = ") << defaultFilterString);

    m_filterString = defaultFilterString;

    if (Q_UNLIKELY(!m_pResource)) {
        return;
    }

    QString resourcePreferredSuffix = m_pResource->preferredFileSuffix();
    QString resourcePreferredFilterString;
    if (!resourcePreferredSuffix.isEmpty()) {
        resourcePreferredFilterString = QStringLiteral("(*.") + resourcePreferredSuffix + QStringLiteral(")");
    }

    QNTRACE(QStringLiteral("Resource preferred file suffix = ") << resourcePreferredSuffix
            << QStringLiteral(", resource preferred filter string = ") << resourcePreferredFilterString);

    bool shouldSkipResourcePreferredSuffix = false;
    if (!resourcePreferredSuffix.isEmpty() && !m_preferredFileSuffixes.contains(resourcePreferredSuffix))
    {
        const int numSuffixes = m_preferredFileSuffixes.size();
        for(int i = 0; i < numSuffixes; ++i)
        {
            const QString & currentSuffix = m_preferredFileSuffixes[i];
            if (resourcePreferredSuffix.contains(currentSuffix)) {
                shouldSkipResourcePreferredSuffix = true;
                break;
            }
        }

        if (!shouldSkipResourcePreferredSuffix) {
            m_preferredFileSuffixes.prepend(resourcePreferredSuffix);
        }
    }

    if (!shouldSkipResourcePreferredSuffix && !resourcePreferredFilterString.isEmpty()) {
        m_filterString = resourcePreferredFilterString + QStringLiteral(";;") + m_filterString;
        QNTRACE(QStringLiteral("m_filterString = ") << m_filterString);
    }
}

} // namespace quentier
