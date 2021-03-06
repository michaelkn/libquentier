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

#include "DecryptEncryptedTextDelegate.h"
#include "../NoteEditor_p.h"
#include "../NoteEditorPage.h"
#include "../dialogs/DecryptionDialog.h"
#include <quentier/utility/EncryptionManager.h>
#include <quentier/enml/ENMLConverter.h>
#include <quentier/enml/DecryptedTextManager.h>
#include <quentier/logging/QuentierLogger.h>

#ifndef QUENTIER_USE_QT_WEB_ENGINE
#include <QWebFrame>
#endif

namespace quentier {

#define CHECK_NOTE_EDITOR() \
    if (Q_UNLIKELY(m_pNoteEditor.isNull())) { \
        QNDEBUG(QStringLiteral("Note editor is null")); \
        return; \
    }

#define CHECK_ACCOUNT() \
    CHECK_NOTE_EDITOR() \
    if (Q_UNLIKELY(!m_pNoteEditor->accountPtr())) { \
        ErrorString error(QT_TR_NOOP("Can't decrypt the encrypted text: no account is set to the note editor")); \
        QNWARNING(error); \
        Q_EMIT notifyError(error); \
        return; \
    }

#define GET_PAGE() \
    CHECK_NOTE_EDITOR() \
    NoteEditorPage * page = qobject_cast<NoteEditorPage*>(m_pNoteEditor->page()); \
    if (Q_UNLIKELY(!page)) { \
        ErrorString error(QT_TR_NOOP("Can't decrypt the encrypted text: no note editor page")); \
        QNWARNING(error); \
        Q_EMIT notifyError(error); \
        return; \
    }

DecryptEncryptedTextDelegate::DecryptEncryptedTextDelegate(const QString & encryptedTextId, const QString & encryptedText,
                                                           const QString & cipher, const QString & length, const QString & hint,
                                                           NoteEditorPrivate * pNoteEditor,
                                                           QSharedPointer<EncryptionManager> encryptionManager,
                                                           QSharedPointer<DecryptedTextManager> decryptedTextManager) :
    m_encryptedTextId(encryptedTextId),
    m_encryptedText(encryptedText),
    m_cipher(cipher),
    m_length(0),
    m_hint(hint),
    m_decryptedText(),
    m_passphrase(),
    m_rememberForSession(false),
    m_decryptPermanently(false),
    m_pNoteEditor(pNoteEditor),
    m_encryptionManager(encryptionManager),
    m_decryptedTextManager(decryptedTextManager)
{
    if (length.isEmpty())
    {
        m_length = 128;
    }
    else
    {
        bool conversionResult = false;
        m_length = static_cast<size_t>(length.toInt(&conversionResult));
        if (Q_UNLIKELY(!conversionResult)) {
            m_length = 0;   // NOTE: postponing the error report until the attempt to start the delegate
        }
    }
}

void DecryptEncryptedTextDelegate::start()
{
    QNDEBUG(QStringLiteral("DecryptEncryptedTextDelegate::start"));

    CHECK_NOTE_EDITOR()

    if (Q_UNLIKELY(!m_length)) {
        ErrorString errorDescription(QT_TR_NOOP("Can't decrypt the encrypted text: can't convert "
                                                "the encryption key length from string to number"));
        QNWARNING(errorDescription);
        Q_EMIT notifyError(errorDescription);
        return;
    }

    if (m_pNoteEditor->isModified()) {
        QObject::connect(m_pNoteEditor.data(), QNSIGNAL(NoteEditorPrivate,convertedToNote,Note),
                         this, QNSLOT(DecryptEncryptedTextDelegate,onOriginalPageConvertedToNote,Note));
        m_pNoteEditor->convertToNote();
    }
    else {
        raiseDecryptionDialog();
    }
}

void DecryptEncryptedTextDelegate::onOriginalPageConvertedToNote(Note note)
{
    QNDEBUG(QStringLiteral("DecryptEncryptedTextDelegate::onOriginalPageConvertedToNote"));

    CHECK_NOTE_EDITOR()

    Q_UNUSED(note)

    QObject::disconnect(m_pNoteEditor.data(), QNSIGNAL(NoteEditorPrivate,convertedToNote,Note),
                        this, QNSLOT(DecryptEncryptedTextDelegate,onOriginalPageConvertedToNote,Note));

    raiseDecryptionDialog();
}

void DecryptEncryptedTextDelegate::raiseDecryptionDialog()
{
    QNDEBUG(QStringLiteral("DecryptEncryptedTextDelegate::raiseDecryptionDialog"));

    CHECK_ACCOUNT()

    if (m_cipher.isEmpty()) {
        m_cipher = QStringLiteral("AES");
    }

    QScopedPointer<DecryptionDialog> pDecryptionDialog(new DecryptionDialog(m_encryptedText, m_cipher, m_hint, m_length,
                                                                            *m_pNoteEditor->accountPtr(), m_encryptionManager,
                                                                            m_decryptedTextManager, m_pNoteEditor));
    pDecryptionDialog->setWindowModality(Qt::WindowModal);
    QObject::connect(pDecryptionDialog.data(), QNSIGNAL(DecryptionDialog,accepted,QString,size_t,QString,QString,QString,bool,bool),
                     this, QNSLOT(DecryptEncryptedTextDelegate,onEncryptedTextDecrypted,QString,size_t,QString,QString,QString,bool,bool));
    QNTRACE(QStringLiteral("Will exec decryption dialog now"));
    int res = pDecryptionDialog->exec();
    if (res == QDialog::Rejected) {
        Q_EMIT cancelled();
        return;
    }
}

void DecryptEncryptedTextDelegate::onEncryptedTextDecrypted(QString cipher, size_t keyLength, QString encryptedText,
                                                            QString passphrase, QString decryptedText, bool rememberForSession,
                                                            bool decryptPermanently)
{
    QNDEBUG(QStringLiteral("DecryptEncryptedTextDelegate::onEncryptedTextDecrypted: encrypted text = ") << encryptedText
            << QStringLiteral(", remember for session = ") << (rememberForSession ? QStringLiteral("true") : QStringLiteral("false"))
            << QStringLiteral(", decrypt permanently = ") << (decryptPermanently ? QStringLiteral("true") : QStringLiteral("false")));

    CHECK_NOTE_EDITOR()

    m_decryptedText = decryptedText;
    m_passphrase = passphrase;
    m_rememberForSession = rememberForSession;
    m_decryptPermanently = decryptPermanently;

    Q_UNUSED(cipher)
    Q_UNUSED(keyLength)

    QString decryptedTextHtml;
    if (!m_decryptPermanently) {
        decryptedTextHtml = ENMLConverter::decryptedTextHtml(m_decryptedText, m_encryptedText, m_hint,
                                                             m_cipher, m_length, m_pNoteEditor->GetFreeDecryptedTextId());
    }
    else {
        decryptedTextHtml = m_decryptedText;
    }

    ENMLConverter::escapeString(decryptedTextHtml);

    GET_PAGE()
    page->executeJavaScript(QStringLiteral("encryptDecryptManager.decryptEncryptedText('") + m_encryptedTextId + QStringLiteral("', '") +
                            decryptedTextHtml + QStringLiteral("');"), JsCallback(*this, &DecryptEncryptedTextDelegate::onDecryptionScriptFinished));
}

void DecryptEncryptedTextDelegate::onDecryptionScriptFinished(const QVariant & data)
{
    QNDEBUG(QStringLiteral("DecryptEncryptedTextDelegate::onDecryptionScriptFinished: ") << data);

    QMap<QString,QVariant> resultMap = data.toMap();

    auto statusIt = resultMap.find(QStringLiteral("status"));
    if (Q_UNLIKELY(statusIt == resultMap.end())) {
        ErrorString error(QT_TR_NOOP("Can't parse the result of text decryption script from JavaScript"));
        QNWARNING(error);
        Q_EMIT notifyError(error);
        return;
    }

    bool res = statusIt.value().toBool();
    if (!res)
    {
        ErrorString error;

        auto errorIt = resultMap.find(QStringLiteral("error"));
        if (Q_UNLIKELY(errorIt == resultMap.end())) {
            error.setBase(QT_TR_NOOP("Can't parse the error of text decryption from JavaScript"));
        }
        else {
            error.setBase(QT_TR_NOOP("Can't decrypt the encrypted text"));
            error.details() = errorIt.value().toString();
        }

        QNWARNING(error);
        Q_EMIT notifyError(error);
        return;
    }

    Q_EMIT finished(m_encryptedText, m_cipher, m_length, m_hint, m_decryptedText, m_passphrase,
                    m_rememberForSession, m_decryptPermanently);
}

} // namespace quentier
