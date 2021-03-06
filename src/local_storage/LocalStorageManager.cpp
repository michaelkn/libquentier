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

#include <quentier/local_storage/LocalStorageManager.h>
#include "LocalStorageManager_p.h"

namespace quentier {

LocalStorageManager::LocalStorageManager(const Account & account,
                                         const bool startFromScratch, const bool overrideLock) :
    d_ptr(new LocalStorageManagerPrivate(account, startFromScratch, overrideLock))
{
    QObject::connect(d_ptr.data(), QNSIGNAL(LocalStorageManagerPrivate,upgradeProgress,double),
                     this, QNSIGNAL(LocalStorageManager,upgradeProgress,double));
}

LocalStorageManager::~LocalStorageManager()
{}

bool LocalStorageManager::addUser(const User & user, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->addUser(user, errorDescription);
}

bool LocalStorageManager::updateUser(const User & user, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->updateUser(user, errorDescription);
}

bool LocalStorageManager::findUser(User & user, ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->findUser(user, errorDescription);
}

bool LocalStorageManager::deleteUser(const User & user, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->deleteUser(user, errorDescription);
}

bool LocalStorageManager::expungeUser(const User & user, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->expungeUser(user, errorDescription);
}

int LocalStorageManager::notebookCount(ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->notebookCount(errorDescription);
}

void LocalStorageManager::switchUser(const Account & account,
                                     const bool startFromScratch, const bool overrideLock)
{
    Q_D(LocalStorageManager);
    d->switchUser(account, startFromScratch, overrideLock);
}

int LocalStorageManager::userCount(ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->userCount(errorDescription);
}

bool LocalStorageManager::addNotebook(Notebook & notebook, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->addNotebook(notebook, errorDescription);
}

bool LocalStorageManager::updateNotebook(Notebook & notebook, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->updateNotebook(notebook, errorDescription);
}

bool LocalStorageManager::findNotebook(Notebook & notebook, ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->findNotebook(notebook, errorDescription);
}

bool LocalStorageManager::findDefaultNotebook(Notebook & notebook, ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->findDefaultNotebook(notebook, errorDescription);
}

bool LocalStorageManager::findLastUsedNotebook(Notebook & notebook, ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->findLastUsedNotebook(notebook, errorDescription);
}

bool LocalStorageManager::findDefaultOrLastUsedNotebook(Notebook & notebook, ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->findDefaultOrLastUsedNotebook(notebook, errorDescription);
}

QList<Notebook> LocalStorageManager::listAllNotebooks(ErrorString & errorDescription,
                                                      const size_t limit, const size_t offset,
                                                      const ListNotebooksOrder::type order,
                                                      const OrderDirection::type orderDirection,
                                                      const QString & linkedNotebookGuid) const
{
    Q_D(const LocalStorageManager);
    return d->listAllNotebooks(errorDescription, limit, offset, order, orderDirection, linkedNotebookGuid);
}

QList<Notebook> LocalStorageManager::listNotebooks(const ListObjectsOptions flag, ErrorString & errorDescription,
                                                   const size_t limit, const size_t offset,
                                                   const ListNotebooksOrder::type order,
                                                   const OrderDirection::type orderDirection,
                                                   const QString & linkedNotebookGuid) const
{
    Q_D(const LocalStorageManager);
    return d->listNotebooks(flag, errorDescription, limit, offset, order, orderDirection, linkedNotebookGuid);
}

QList<SharedNotebook> LocalStorageManager::listAllSharedNotebooks(ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->listAllSharedNotebooks(errorDescription);
}

QList<SharedNotebook> LocalStorageManager::listSharedNotebooksPerNotebookGuid(const QString & notebookGuid,
                                                                                     ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->listSharedNotebooksPerNotebookGuid(notebookGuid, errorDescription);
}

bool LocalStorageManager::expungeNotebook(Notebook & notebook, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->expungeNotebook(notebook, errorDescription);
}

int LocalStorageManager::linkedNotebookCount(ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->linkedNotebookCount(errorDescription);
}

bool LocalStorageManager::addLinkedNotebook(const LinkedNotebook & linkedNotebook,
                                            ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->addLinkedNotebook(linkedNotebook, errorDescription);
}

bool LocalStorageManager::updateLinkedNotebook(const LinkedNotebook & linkedNotebook,
                                               ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->updateLinkedNotebook(linkedNotebook, errorDescription);
}

bool LocalStorageManager::findLinkedNotebook(LinkedNotebook & linkedNotebook, ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->findLinkedNotebook(linkedNotebook, errorDescription);
}

QList<LinkedNotebook> LocalStorageManager::listAllLinkedNotebooks(ErrorString & errorDescription, const size_t limit,
                                                                  const size_t offset, const ListLinkedNotebooksOrder::type order,
                                                                  const OrderDirection::type orderDirection) const
{
    Q_D(const LocalStorageManager);
    return d->listAllLinkedNotebooks(errorDescription, limit, offset, order, orderDirection);
}

QList<LinkedNotebook> LocalStorageManager::listLinkedNotebooks(const ListObjectsOptions flag, ErrorString & errorDescription, const size_t limit,
                                                               const size_t offset, const ListLinkedNotebooksOrder::type order,
                                                               const OrderDirection::type orderDirection) const
{
    Q_D(const LocalStorageManager);
    return d->listLinkedNotebooks(flag, errorDescription, limit, offset, order, orderDirection);
}

bool LocalStorageManager::expungeLinkedNotebook(const LinkedNotebook & linkedNotebook,
                                                ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->expungeLinkedNotebook(linkedNotebook, errorDescription);
}

int LocalStorageManager::noteCount(ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->noteCount(errorDescription);
}

int LocalStorageManager::noteCountPerNotebook(const Notebook & notebook, ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->noteCountPerNotebook(notebook, errorDescription);
}

int LocalStorageManager::noteCountPerTag(const Tag & tag, ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->noteCountPerTag(tag, errorDescription);
}

bool LocalStorageManager::noteCountsPerAllTags(QHash<QString, int> & noteCountsPerTagLocalUid, ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->noteCountsPerAllTags(noteCountsPerTagLocalUid, errorDescription);
}

bool LocalStorageManager::addNote(Note & note, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->addNote(note, errorDescription);
}

bool LocalStorageManager::updateNote(Note & note, const bool updateResources,
                                     const bool updateTags, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->updateNote(note, updateResources, updateTags, errorDescription);
}

bool LocalStorageManager::findNote(Note & note, ErrorString & errorDescription,
                                   const bool withResourceBinaryData) const
{
    Q_D(const LocalStorageManager);
    return d->findNote(note, errorDescription, withResourceBinaryData);
}

QList<Note> LocalStorageManager::listNotesPerNotebook(const Notebook & notebook,
                                                      ErrorString & errorDescription,
                                                      const bool withResourceBinaryData,
                                                      const LocalStorageManager::ListObjectsOptions & flag,
                                                      const size_t limit, const size_t offset,
                                                      const LocalStorageManager::ListNotesOrder::type & order,
                                                      const LocalStorageManager::OrderDirection::type & orderDirection) const
{
    Q_D(const LocalStorageManager);
    return d->listNotesPerNotebook(notebook, errorDescription, withResourceBinaryData,
                                   flag, limit, offset, order, orderDirection);
}

QList<Note> LocalStorageManager::listNotesPerTag(const Tag & tag, ErrorString & errorDescription,
                                                 const bool withResourceBinaryData,
                                                 const LocalStorageManager::ListObjectsOptions & flag,
                                                 const size_t limit, const size_t offset,
                                                 const LocalStorageManager::ListNotesOrder::type & order,
                                                 const LocalStorageManager::OrderDirection::type & orderDirection) const
{
    Q_D(const LocalStorageManager);
    return d->listNotesPerTag(tag, errorDescription, withResourceBinaryData, flag, limit, offset, order, orderDirection);
}

QList<Note> LocalStorageManager::listNotes(const ListObjectsOptions flag, ErrorString & errorDescription,
                                           const bool withResourceBinaryData, const size_t limit,
                                           const size_t offset, const ListNotesOrder::type order,
                                           const OrderDirection::type orderDirection,
                                           const QString & linkedNotebookGuid) const
{
    Q_D(const LocalStorageManager);
    return d->listNotes(flag, errorDescription, withResourceBinaryData, limit, offset, order, orderDirection, linkedNotebookGuid);
}

QStringList LocalStorageManager::findNoteLocalUidsWithSearchQuery(const NoteSearchQuery & noteSearchQuery,
                                                                  ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->findNoteLocalUidsWithSearchQuery(noteSearchQuery, errorDescription);
}

NoteList LocalStorageManager::findNotesWithSearchQuery(const NoteSearchQuery & noteSearchQuery,
                                                       ErrorString & errorDescription,
                                                       const bool withResourceBinaryData) const
{
    Q_D(const LocalStorageManager);
    return d->findNotesWithSearchQuery(noteSearchQuery, errorDescription, withResourceBinaryData);
}

bool LocalStorageManager::expungeNote(Note & note, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->expungeNote(note, errorDescription);
}

int LocalStorageManager::tagCount(ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->tagCount(errorDescription);
}

bool LocalStorageManager::addTag(Tag & tag, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->addTag(tag, errorDescription);
}

bool LocalStorageManager::updateTag(Tag & tag, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->updateTag(tag, errorDescription);
}

bool LocalStorageManager::findTag(Tag & tag, ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->findTag(tag, errorDescription);
}

QList<Tag> LocalStorageManager::listAllTagsPerNote(const Note & note, ErrorString & errorDescription,
                                                   const ListObjectsOptions & flag, const size_t limit,
                                                   const size_t offset, const ListTagsOrder::type & order,
                                                   const OrderDirection::type & orderDirection) const
{
    Q_D(const LocalStorageManager);
    return d->listAllTagsPerNote(note, errorDescription, flag, limit, offset, order, orderDirection);
}

QList<Tag> LocalStorageManager::listAllTags(ErrorString & errorDescription, const size_t limit,
                                            const size_t offset, const ListTagsOrder::type order,
                                            const OrderDirection::type orderDirection,
                                            const QString & linkedNotebookGuid) const
{
    Q_D(const LocalStorageManager);
    return d->listAllTags(errorDescription, limit, offset, order, orderDirection, linkedNotebookGuid);
}

QList<Tag> LocalStorageManager::listTags(const ListObjectsOptions flag, ErrorString & errorDescription,
                                         const size_t limit, const size_t offset,
                                         const ListTagsOrder::type & order,
                                         const OrderDirection::type orderDirection,
                                         const QString & linkedNotebookGuid) const
{
    Q_D(const LocalStorageManager);
    return d->listTags(flag, errorDescription, limit, offset, order, orderDirection, linkedNotebookGuid);
}

bool LocalStorageManager::expungeTag(Tag & tag, QStringList & expungedChildTagLocalUids, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->expungeTag(tag, expungedChildTagLocalUids, errorDescription);
}

bool LocalStorageManager::expungeNotelessTagsFromLinkedNotebooks(ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->expungeNotelessTagsFromLinkedNotebooks(errorDescription);
}

int LocalStorageManager::enResourceCount(ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->enResourceCount(errorDescription);
}

bool LocalStorageManager::addEnResource(Resource & resource, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->addEnResource(resource, errorDescription);
}

bool LocalStorageManager::updateEnResource(Resource & resource, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->updateEnResource(resource, errorDescription);
}

bool LocalStorageManager::findEnResource(Resource & resource, ErrorString & errorDescription,
                                         const bool withBinaryData) const
{
    Q_D(const LocalStorageManager);
    return d->findEnResource(resource, errorDescription, withBinaryData);
}

bool LocalStorageManager::expungeEnResource(Resource & resource, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->expungeEnResource(resource, errorDescription);
}

int LocalStorageManager::savedSearchCount(ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->savedSearchCount(errorDescription);
}

bool LocalStorageManager::addSavedSearch(SavedSearch & search, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->addSavedSearch(search, errorDescription);
}

bool LocalStorageManager::updateSavedSearch(SavedSearch & search, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->updateSavedSearch(search, errorDescription);
}

bool LocalStorageManager::findSavedSearch(SavedSearch & search, ErrorString & errorDescription) const
{
    Q_D(const LocalStorageManager);
    return d->findSavedSearch(search, errorDescription);
}

QList<SavedSearch> LocalStorageManager::listAllSavedSearches(ErrorString & errorDescription, const size_t limit, const size_t offset,
                                                             const ListSavedSearchesOrder::type order,
                                                             const OrderDirection::type orderDirection) const
{
    Q_D(const LocalStorageManager);
    return d->listAllSavedSearches(errorDescription, limit, offset, order, orderDirection);
}

QList<SavedSearch> LocalStorageManager::listSavedSearches(const ListObjectsOptions flag, ErrorString & errorDescription,
                                                          const size_t limit, const size_t offset,
                                                          const ListSavedSearchesOrder::type order,
                                                          const OrderDirection::type orderDirection) const
{
    Q_D(const LocalStorageManager);
    return d->listSavedSearches(flag, errorDescription, limit, offset, order, orderDirection);
}

bool LocalStorageManager::expungeSavedSearch(SavedSearch & search, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->expungeSavedSearch(search, errorDescription);
}

qint32 LocalStorageManager::accountHighUsn(const QString & linkedNotebookGuid, ErrorString & errorDescription)
{
    Q_D(LocalStorageManager);
    return d->accountHighUsn(linkedNotebookGuid, errorDescription);
}

} // namespace quentier
