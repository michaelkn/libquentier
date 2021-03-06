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

#ifndef LIB_QUENTIER_NOTE_EDITOR_UNDO_STACK_NOTE_EDITOR_CONTENT_EDIT_UNDO_COMMAND_H
#define LIB_QUENTIER_NOTE_EDITOR_UNDO_STACK_NOTE_EDITOR_CONTENT_EDIT_UNDO_COMMAND_H

#include "INoteEditorUndoCommand.h"
#include <quentier/types/Resource.h>
#include <QList>

namespace quentier {

class Q_DECL_HIDDEN NoteEditorContentEditUndoCommand: public INoteEditorUndoCommand
{
    Q_OBJECT
public:
    NoteEditorContentEditUndoCommand(NoteEditorPrivate & noteEditorPrivate,
                                     const QList<Resource> & resources,
                                     QUndoCommand * parent = Q_NULLPTR);
    NoteEditorContentEditUndoCommand(NoteEditorPrivate & noteEditorPrivate,
                                     const QList<Resource> & resources,
                                     const QString & text, QUndoCommand * parent = Q_NULLPTR);
    virtual ~NoteEditorContentEditUndoCommand();

    virtual void redoImpl() Q_DECL_OVERRIDE;
    virtual void undoImpl() Q_DECL_OVERRIDE;

private:
    void init();

private:
    /**
     * During the editing of the note's content the resources attached to the note
     * might be deleted via simply pressing backspace; the actual fact of the resource's
     * deletion only becomes clear when the HTML from note editor's page is converted
     * back into ENML and its analysis can reveal that certain resource
     * is no longer is a part of the note. Only at that time it is removed
     * from the list of note's resources.
     *
     * The undo stack must make it possible to restore the resources when undoing the changes
     * made to the note's content. For this reason each undo command for note content's edit
     * contains a list of note's resources which should be restored back to the note
     * on undoing the edit; most of the time it would be the same unchanged list;
     * thanks to Qt's copy-on-write facilities, the storage and copying of a list
     * with resources for this use case is actually very cheap
     *
     * Note that there's no need to worry about doing anything with note's resources
     * on redoing the edit. It is not possible to add a resource through editing
     * of note's content. If the edit action was actually removing the resource
     * from the note's content via backspace, that fact would be figured out elsewhere
     */
    QList<Resource>  m_resources;
};

} // namespace quentier

#endif // LIB_QUENTIER_NOTE_EDITOR_UNDO_STACK_NOTE_EDITOR_CONTENT_EDIT_UNDO_COMMAND_H
