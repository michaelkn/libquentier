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

#include "RemoveResourceDelegate.h"
#include "../NoteEditor_p.h"
#include <quentier/logging/QuentierLogger.h>

namespace quentier {

#define GET_PAGE() \
    NoteEditorPage * page = qobject_cast<NoteEditorPage*>(m_noteEditor.page()); \
    if (Q_UNLIKELY(!page)) { \
        ErrorString error(QT_TR_NOOP("Can't remove the attachment: no note editor page")); \
        QNWARNING(error); \
        Q_EMIT notifyError(error); \
        return; \
    }

RemoveResourceDelegate::RemoveResourceDelegate(const Resource & resourceToRemove, NoteEditorPrivate & noteEditor) :
    m_noteEditor(noteEditor),
    m_resource(resourceToRemove)
{}

void RemoveResourceDelegate::start()
{
    QNDEBUG(QStringLiteral("RemoveResourceDelegate::start"));

    if (m_noteEditor.isModified()) {
        QObject::connect(&m_noteEditor, QNSIGNAL(NoteEditorPrivate,convertedToNote,Note),
                         this, QNSLOT(RemoveResourceDelegate,onOriginalPageConvertedToNote,Note));
        m_noteEditor.convertToNote();
    }
    else {
        doStart();
    }
}

void RemoveResourceDelegate::onOriginalPageConvertedToNote(Note note)
{
    QNDEBUG(QStringLiteral("RemoveResourceDelegate::onOriginalPageConvertedToNote"));

    Q_UNUSED(note)

    QObject::disconnect(&m_noteEditor, QNSIGNAL(NoteEditorPrivate,convertedToNote,Note),
                        this, QNSLOT(RemoveResourceDelegate,onOriginalPageConvertedToNote,Note));

    doStart();
}

void RemoveResourceDelegate::doStart()
{
    QNDEBUG(QStringLiteral("RemoveResourceDelegate::doStart"));

    if (Q_UNLIKELY(!m_resource.hasDataHash())) {
        ErrorString error(QT_TR_NOOP("Can't remove the attachment: the data hash is missing"));
        QNWARNING(error);
        Q_EMIT notifyError(error);
        return;
    }

    QString javascript = QStringLiteral("resourceManager.removeResource('") +
                         QString::fromLocal8Bit(m_resource.dataHash().toHex()) +
                         QStringLiteral("');");

    GET_PAGE()
    page->executeJavaScript(javascript, JsCallback(*this, &RemoveResourceDelegate::onResourceReferenceRemovedFromNoteContent));
}

void RemoveResourceDelegate::onResourceReferenceRemovedFromNoteContent(const QVariant & data)
{
    QNDEBUG(QStringLiteral("RemoveResourceDelegate::onResourceReferenceRemovedFromNoteContent"));

    QMap<QString,QVariant> resultMap = data.toMap();

    auto statusIt = resultMap.find(QStringLiteral("status"));
    if (Q_UNLIKELY(statusIt == resultMap.end())) {
        ErrorString error(QT_TR_NOOP("Can't parse the result of attachment reference removal from JavaScript"));
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
            error.setBase(QT_TR_NOOP("Can't parse the error of attachment reference removal from JavaScript"));
        }
        else {
            error.setBase(QT_TR_NOOP("Can't remove the attachment reference from the note editor"));
            error.details() = errorIt.value().toString();
        }

        QNWARNING(error);
        Q_EMIT notifyError(error);
        return;
    }

    m_noteEditor.removeResourceFromNote(m_resource);

    Q_EMIT finished(m_resource);
}

} // namespace quentier
