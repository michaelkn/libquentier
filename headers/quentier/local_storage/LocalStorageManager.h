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

#ifndef LIB_QUENTIER_LOCAL_STORAGE_LOCAL_STORAGE_MANAGER_H
#define LIB_QUENTIER_LOCAL_STORAGE_LOCAL_STORAGE_MANAGER_H

#include <quentier/types/Account.h>
#include <quentier/local_storage/Lists.h>
#include <quentier/local_storage/NoteSearchQuery.h>
#include <quentier/utility/Linkage.h>
#include <quentier/utility/Macros.h>
#include <quentier/types/ErrorString.h>
#include <QString>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QHash>
#include <cstdint>

namespace qevercloud {
QT_FORWARD_DECLARE_STRUCT(ResourceAttributes)
QT_FORWARD_DECLARE_STRUCT(NoteAttributes)
QT_FORWARD_DECLARE_STRUCT(UserAttributes)
QT_FORWARD_DECLARE_STRUCT(Accounting)
QT_FORWARD_DECLARE_STRUCT(PremiumInfo)
QT_FORWARD_DECLARE_STRUCT(BusinessUserInfo)
QT_FORWARD_DECLARE_STRUCT(SharedNotebook)
QT_FORWARD_DECLARE_STRUCT(NotebookRestrictions)
}

namespace quentier {

QT_FORWARD_DECLARE_CLASS(LocalStorageManagerPrivate)

class QUENTIER_EXPORT LocalStorageManager: public QObject
{
    Q_OBJECT
public:
    /**
     * @brief LocalStorageManager - constructor. Takes in the account for which the LocalStorageManager instance is created
     * plus some other parameters determining the startup behaviour
     *
     * @param account - the account for which the local storage is being created and initialized
     * @param startFromScratch - if set to true, the existing database file for this account (if any) would be purged
     * during the local storage manager construction (used in tests)
     * @param overrideLock - if set to true, the constructor would ignore the existing advisory lock (if any) put on the database file;
     * otherwise the presence of advisory lock on the database file would cause the constructor to throw @link DatabaseLockedException @endlink
     */
    LocalStorageManager(const Account & account, const bool startFromScratch, const bool overrideLock);

    virtual ~LocalStorageManager();

Q_SIGNALS:
    /**
     * @brief LocalStorageManager is capable of performing the automatic database upgrades if/when it is necessary
     *
     * As the database upgrade can be a lengthy operation, this signal is meant to provide some feedback on the progress
     * of the upgrade
     *
     * @param progress - the value from 0 to 1 denoting the database upgrade progress
     */
    void upgradeProgress(double progress);

public:
    /**
     * @brief The ListObjectsOption enum is the base enum for QFlags which allows to specify
     * the desired local storage elements in calls to methods listing them from the database
     *
     * For example, one can either list all available elements of certain type from local storage
     * or only elements marked as dirty (modified locally, not yet synchronized) or elements never synchronized
     * with the remote storage or elements which are synchronizable with the remote storage etc.
     */
    enum ListObjectsOption {
        ListAll                      = 0,
        ListDirty                    = 1,
        ListNonDirty                 = 2,
        ListElementsWithoutGuid      = 4,
        ListElementsWithGuid         = 8,
        ListLocal                    = 16,
        ListNonLocal                 = 32,
        ListFavoritedElements        = 64,
        ListNonFavoritedElements     = 128
    };
    Q_DECLARE_FLAGS(ListObjectsOptions, ListObjectsOption)

    /**
     * @brief switchUser - switches to another local storage database file associated with the passed in
     * account
     *
     * If optional "startFromScratch" parameter is set to true (it is false
     * by default), the database file would be erased and only then - opened. If optional "overrideLock" parameter
     * is set to true, the advisory lock set on the database file (if any) would be forcefully removed;
     * otherwise, if this parameter if set to false, the presence of advisory lock on the database file woud cause
     * the method to throw DatabaseLockedException
     *
     * @param account - the account to which the local storage is to be switched
     * @param startFromScratch - optional, false by default; if true and database file
     * for this user existed previously, it is erased before open
     * @param overrideLock - optional, false by default; if true and the database file does have the advisory lock put on it,
     * the lock would be forcefully removed; otherwise the presence of advisory lock on the database file
     * would cause the method to throw @link DatabaseLockedException @endlink
     */
    void switchUser(const Account & account, const bool startFromScratch = false,
                    const bool overrideLock = false);

    /**
     * @brief userCount - returns the number of non-deleted users currently stored in the local storage database
     * @param errorDescription - error description if the number of users could not be returned
     * @return either non-negative value with the number of users or -1 which means some error has occurred
     */
    int userCount(ErrorString & errorDescription) const;

    /**
     * @brief addUser - adds the passed in User object to the local storage database
     *
     * The table with Users is only involved in operations with notebooks which have "contact" field
     * set which in turn is used with business accounts
     *
     * @param user - the user to be added to the local storage database
     * @param errorDescription - error description if the user could not be added
     * @return true if the user was added successfully, false otherwise
     */
    bool addUser(const User & user, ErrorString & errorDescription);

    /**
     * @brief updateUser - updates the passed in User object in the local storage database
     *
     * The table with Users is only involved in operations with notebooks which have "contact" field
     * set which in turn is used with business accounts
     *
     * @param user - the user to be updated in the local storage database
     * @param errorDescription - error description if the user could not be updated
     * @return true if the user was updated successfully, false otherwise
     */
    bool updateUser(const User & user, ErrorString & errorDescription);

    /**
     * @brief findUser - attempts to find and fill the fields of the passed in User object which must have
     * "id" field set as this value is used as the identifier of User objects in the local storage database
     *
     * @param user - the user to be found. Must have "id" field set
     * @param errorDescription - error description if the user could not be found
     * @return true if the user was found successfully, false otherwise
     */
    bool findUser(User & user, ErrorString & errorDescription) const;

    /**
     * @brief deleteUser - marks the user as deleted in local storage
     * @param user - the user to be marked as deleted
     * @param errorDescription - error description if the user could not be marked as deleted
     * @return true if the user was marked as deleted successfully, false otherwise
     */
    bool deleteUser(const User & user, ErrorString & errorDescription);

    /**
     * @brief expungeUser - permanently deletes the user from the local storage database
     * @param user - the user to be expunged
     * @param errorDescription - error description if the user could not be expunged
     * @return true if the user was expunged successfully, false otherwise
     */
    bool expungeUser(const User & user, ErrorString & errorDescription);

    /**
     * @brief notebookCount returns the number of notebooks currently stored in the local storage database
     * @param errorDescription - error description if the number of notebooks could not be returned
     * @return either non-negative value with the number of notebooks or -1 which means some error has occurred
     */
    int notebookCount(ErrorString & errorDescription) const;

    /**
     * @brief addNotebook - adds the passed in Notebook to the local storage database
     *
     * If the notebook has "remote" Evernote service's guid set, it is identified by this guid
     * in the local storage database. Otherwise it is identified by the local uid
     *
     * @param notebook - the notebook to be added to the local storage database; the object is passed by reference
     * and may be changed as a result of the call (filled with autocompleted fields like local uid if it was empty
     * before the call)
     * @param errorDescription - error description if the notebook could not be added
     * @return true if the notebook was added successfully, false otherwise
     */
    bool addNotebook(Notebook & notebook, ErrorString & errorDescription);

    /**
     * @brief updateNotebook - updates the passed in Notebook in the local storage database
     *
     * If the notebook has "remote" Evernote service's guid set, it is identified by this guid
     * in the local storage database. If no notebook with such guid is found, the local uid is used
     * to identify the notebook in the local storage database. If the notebook has no guid, the local uid
     * is used to identify it in the local storage database.
     *
     * @param notebook - notebook to be updated in the local storage database; the object is passed by reference
     * and may be changed as a result of the call (filled with autocompleted fields like local uid if it was empty
     * before the call)
     * @param errorDescription - error description if the notebook could not be updated
     * @return true if the notebook was updated successfully, false otherwise
     */
    bool updateNotebook(Notebook & notebook, ErrorString & errorDescription);

    /**
     * @brief findNotebook - attempts to find and set all found fields of the passed in
     * Notebook object
     *
     * If "remote" Evernote service's guid for the notebook is set,
     * it is used to identify the notebook in the local storage database. Otherwise the notebook is
     * identified by its local uid. If it's empty, the search would attempt to find the notebook
     * by its name. If the name is also not set, the search would attempt to find the notebook by linked notebook guid
     * assuming that no more than one notebook corresponds to the linked notebook guid. If linked notebook guid is also
     * not set, the search would fail.
     *
     * Important! Due to the fact that the notebook name is only unique within
     * the users's own account as well as within each linked notebook, the
     * result of the search by name depends on the notebook's linked notebook
     * guid: if it is not set, the search by name would only search for the notebook
     * with the specified name within the user's own account. If it is set, the
     * search would only consider the linked notebook with the corresponding
     * guid.
     *
     * @param notebook - the notebook to be found. Must have either "remote" or local uid or name set
     * @param errorDescription - error description if the notebook could not be found
     * @return true if the notebook was found, false otherwise
     */
    bool findNotebook(Notebook & notebook, ErrorString & errorDescription) const;

    /**
     * @brief findDefaultNotebook - attempts to find the default notebook in the local storage database.
     * @param notebook - the default notebook to be found
     * @param errorDescription - error description if the default notebook could not be found
     * @return true if the default notebook was found, false otherwise
     */
    bool findDefaultNotebook(Notebook & notebook, ErrorString & errorDescription) const;

    /**
     * @brief findLastUsedNotebook - attempts to find the last used notebook in the local storage database.
     * @param notebook - the last used notebook to be found
     * @param errorDescription - error description if the last used notebook could not be found
     * @return true if the last used notebook was found, false otherwise
     */
    bool findLastUsedNotebook(Notebook & notebook, ErrorString & errorDescription) const;

    /**
     * @brief findDefaultOrLastUsedNotebook - attempts to find either the default or the last used notebook
     * in the local storage database
     * @param notebook - either the default or the last used notebook to be found
     * @param errorDescription - error description if the default or the last used notebook could not be found
     * @return true if the default or the last used notebook were found, false otherwise
     */
    bool findDefaultOrLastUsedNotebook(Notebook & notebook, ErrorString & errorDescription) const;

    /**
     * @brief The OrderDirection struct is a C++98 style scoped enum which specifies the direction of ordering
     * of the results for methods listing the objects from the local storage database
     */
    struct OrderDirection
    {
        enum type
        {
            Ascending = 0,
            Descending
        };
    };

    /**
     * @brief The ListNotebooksOrder struct is a C++98 style scoped enum which allows to specify the ordering
     * of the results for methods listing the notebooks from the local storage database
     */
    struct ListNotebooksOrder
    {
        enum type
        {
            ByUpdateSequenceNumber = 0,
            ByNotebookName,
            ByCreationTimestamp,
            ByModificationTimestamp,
            NoOrder
        };
    };

    /**
     * @brief listAllNotebooks - attempts to list all notebooks within the current account from the local storage
     * database
     * @param errorDescription - error description if all notebooks could not be listed;
     * if no error happens, this parameter is untouched
     * @param limit - the limit for the max number of notebooks in the result, zero by default which means no limit is set
     * @param offset - the number of notebooks to skip in the beginning of the result, zero by default
     * @param order - allows to specify a particular ordering of notebooks in the result, NoOrder by default
     * @param orderDirection - specifies the direction of ordering, by defauls ascending direction is used;
     * this parameter has no meaning if order is equal to NoOrder
     * @param linkedNotebookGuid - if it's null, the method would list the notebooks ignoring their belonging
     * to the current account or to some linked notebook; if it's empty, only the non-linked notebooks  would be listed;
     * otherwise, the only one notebook from the corresponding linked notebook would be listed
     * @return either the list of all notebooks within the account or empty list in cases of
     * error or no notebooks presence within the account
     */
    QList<Notebook> listAllNotebooks(ErrorString & errorDescription, const size_t limit = 0,
                                     const size_t offset = 0, const ListNotebooksOrder::type order = ListNotebooksOrder::NoOrder,
                                     const OrderDirection::type orderDirection = OrderDirection::Ascending,
                                     const QString & linkedNotebookGuid = QString()) const;

    /**
     * @brief listNotebooks - attempts to list notebooks within the account according to
     * the specified input flag
     * @param flag - input parameter used to set the filter for the desired notebooks to be listed
     * @param errorDescription - error description if notebooks within the account could not be listed;
     * if no error happens, this parameter is untouched
     * @param limit - the limit for the max number of notebooks in the result, zero by default which means no limit is set
     * @param offset - the number of notebooks to skip in the beginning of the result, zero by default
     * @param order - allows to specify a particular ordering of notebooks in the result, NoOrder by default
     * @param orderDirection - specifies the direction of ordering, by defauls ascending direction is used;
     * this parameter has no meaning if order is equal to NoOrder
     * @param linkedNotebookGuid - if it's null, the method would list the notebooks ignoring their belonging
     * to the current account or to some linked notebook; if it's empty, only the non-linked notebooks  would be listed;
     * otherwise, the only one notebook from the corresponding linked notebook would be listed
     * @return either the list of notebooks within the account conforming to the filter or empty list
     * in cases of error or no notebooks conforming to the filter exist within the account
     */
    QList<Notebook> listNotebooks(const ListObjectsOptions flag, ErrorString & errorDescription,
                                  const size_t limit = 0, const size_t offset = 0,
                                  const ListNotebooksOrder::type order = ListNotebooksOrder::NoOrder,
                                  const OrderDirection::type orderDirection = OrderDirection::Ascending,
                                  const QString & linkedNotebookGuid = QString()) const;

    /**
     * @brief listAllSharedNotebooks - attempts to list all shared notebooks within the account
     * @param errorDescription - error description if shared notebooks could not be listed;
     * if no error happens, this parameter is untouched
     * @return either the list of all shared notebooks within the account or empty list in cases of
     * error or no shared notebooks presence within the account
     */
    QList<SharedNotebook> listAllSharedNotebooks(ErrorString & errorDescription) const;

    /**
     * @brief listSharedNotebooksPerNotebookGuid - attempts to list all shared notebooks
     * per given notebook's remote guid (not local uid, it's important)
     * @param notebookGuid - remote Evernote service's guid of the notebook for which
     * the shared notebooks are requested
     * @param errorDescription - error description if shared notebooks per notebook guid
     * could not be listed; if no error happens, this parameter is untouched
     * @return either the list of shared notebooks per notebook guid or empty list
     * in case of error or no shared notebooks presence per given notebook guid
     */
    QList<SharedNotebook> listSharedNotebooksPerNotebookGuid(const QString & notebookGuid,
                                                             ErrorString & errorDescription) const;

    /**
     * @brief expungeNotebook - permanently deletes the specified notebook from the local storage database
     *
     * Evernote API doesn't allow to delete the notebooks from the remote storage, it can
     * only be done by the official desktop Evernote client or via its web client. So this method should be called
     * only during the synchronization with the remote storage, when some notebook is found to be
     * deleted via either the official desktop client or via the web client; also, this method can be called
     * for local notebooks not synchronized with Evernote at all
     *
     * @param notebook - the notebook to be expunged. Must have either "remote" or local uid set; the object is passed
     * by reference and may be changed as a result of the call (filled with local uid if it was empty before the call)
     * @param errorDescription - error description if the notebook could not be expunged
     * @return true if the notebook was expunged successfully, false otherwise
     */
    bool expungeNotebook(Notebook & notebook, ErrorString & errorDescription);

    /**
     * @brief linkedNotebookCount - returns the number of linked notebooks stored in the local storage database
     * @param errorDescription - error description if the number of linked notebooks count not be returned
     * @return either non-negative number of linked notebooks or -1 if some error has occured
     */
    int linkedNotebookCount(ErrorString & errorDescription) const;

    /**
     * @brief addLinkedNotebook - adds passed in LinkedNotebook to the local storage database;
     * LinkedNotebook must have "remote" Evernote service's guid set. It is not possible
     * to add a linked notebook in offline mode so it doesn't make sense for LinkedNotebook
     * objects to not have guid.
     * @param linkedNotebook - LinkedNotebook to be added to the local storage database
     * @param errorDescription - error description if linked notebook could not be added
     * @return true if linked notebook was added successfully, false otherwise
     */
    bool addLinkedNotebook(const LinkedNotebook & linkedNotebook, ErrorString & errorDescription);

    /**
     * @brief updateLinkedNotebook - updates passd in LinkedNotebook in the local storage database;
     * LinkedNotebook must have "remote" Evernote service's guid set.
     * @param linkedNotebook - LinkedNotebook to be updated in the local storage database
     * @param errorDescription - error description if linked notebook could not be updated
     * @return true if linked notebook was updated successfully, false otherwise
     */
    bool updateLinkedNotebook(const LinkedNotebook & linkedNotebook, ErrorString & errorDescription);

    /**
     * @brief findLinkedNotebook - attempts to find and set all found fields for passed in
     * by reference LinkedNotebook object. For LinkedNotebook local uid doesn't mean anything
     * because it can only be considered valid if it has "remote" Evernote service's guid set.
     * So this passed in LinkedNotebook object must have guid set to identify
     * the linked notebook in the local storage database.
     * @param linkedNotebook - linked notebook to be found. Must have "remote" guid set
     * @param errorDescription - error description if linked notebook could not be found
     * @return true if linked notebook was found, false otherwise
     */
    bool findLinkedNotebook(LinkedNotebook & linkedNotebook, ErrorString & errorDescription) const;

    /**
     * @brief The ListLinkedNotebooksOrder struct is a C++98-style scoped enum which allows to specify ordering
     * of the results of methods listing linked notebooks from local storage
     */
    struct ListLinkedNotebooksOrder
    {
        enum type
        {
            ByUpdateSequenceNumber = 0,
            ByShareName,
            ByUsername,
            NoOrder
        };
    };

    /**
     * @brief listAllLinkedNotebooks - attempts to list all linked notebooks within the account
     * @param errorDescription - error description if linked notebooks could not be listed,
     * otherwise this parameter is untouched
     * @param limit - limit for the max number of linked notebooks in the result, zero by default which means no limit is set
     * @param offset - number of linked notebooks to skip in the beginning of the result, zero by default
     * @param order - allows to specify particular ordering of linked notebooks in the result, NoOrder by default
     * @param orderDirection - specifies the direction of ordering, by defauls ascending direction is used;
     * this parameter has no meaning if order is equal to NoOrder
     * @return either list of all linked notebooks or empty list in case of error or
     * no linked notebooks presence within the account
     */
    QList<LinkedNotebook> listAllLinkedNotebooks(ErrorString & errorDescription, const size_t limit = 0, const size_t offset = 0,
                                                 const ListLinkedNotebooksOrder::type order = ListLinkedNotebooksOrder::NoOrder,
                                                 const OrderDirection::type orderDirection = OrderDirection::Ascending) const;

    /**
     * @brief listLinkedNotebooks - attempts to list linked notebooks within the account
     * according to the specified input flag
     * @param flag - input parameter used to set the filter for the desired linked notebooks to be listed
     * @param errorDescription - error description if linked notebooks within the account could not be listed;
     * if no error happens, this parameter is untouched
     * @param limit - limit for the max number of linked notebooks in the result, zero by default which means no limit is set
     * @param offset - number of linked notebooks to skip in the beginning of the result, zero by default
     * @param order - allows to specify particular ordering of linked notebooks in the result, NoOrder by default
     * @param orderDirection - specifies the direction of ordering, by defauls ascending direction is used;
     * this parameter has no meaning if order is equal to NoOrder
     * @return either list of linked notebooks within the account conforming to the filter or empty list
     * in cases of error or no linked notebooks conforming to the filter exist within the account
     */
    QList<LinkedNotebook> listLinkedNotebooks(const ListObjectsOptions flag, ErrorString & errorDescription,
                                              const size_t limit = 0, const size_t offset = 0,
                                              const ListLinkedNotebooksOrder::type order = ListLinkedNotebooksOrder::NoOrder,
                                              const OrderDirection::type orderDirection = OrderDirection::Ascending) const;

    /**
     * @brief expungeLinkedNotebook - permanently deletes specified linked notebook from local storage.
     * Evernote API doesn't allow to delete linked notebooks from remote storage, it can
     * only be done by official desktop client or web GUI. So this method should be called
     * only during the synchronization with remote database, when some linked notebook
     * is found to be deleted via either official desktop client or web GUI
     * @param linkedNotebook - linked notebook to be expunged. Must have "remote" guid set
     * @param errorDescription - error description if linked notebook could not be expunged
     * @return true if linked notebook was expunged successfully, false otherwise
     */
    bool expungeLinkedNotebook(const LinkedNotebook & linkedNotebook, ErrorString & errorDescription);

    /**
     * @brief noteCount returns the number of non-deleted notes currently stored in local storage database
     * @param errorDescription - error description if the number of notes could not be returned
     * @return either non-negative value with the number of notes or -1 which means some error occured
     */
    int noteCount(ErrorString & errorDescription) const;

    /**
     * @brief noteCountPerNotebook returns the number of non-deleted notes currently stored in local storage database per given notebook
     * @param notebook - notebook for which the number of notes is requested. If its guid is set, it is used to identify the notebook,
     * otherwise its local uid is used
     * @param errorDescription - error description if the number of notes per given notebook could not be returned
     * @return either non-negative value with the number of notes per given notebook or -1 which means some error occured
     */
    int noteCountPerNotebook(const Notebook & notebook, ErrorString & errorDescription) const;

    /**
     * @brief noteCountPerTag returns the number of non-deleted notes currently stored in local storage database labeled with given tag
     * @param tag - tag for which the number of notes labeled with it is requested. If its guid is set, it is used to identify the tag,
     * otherwise its local uid is used
     * @param errorDescription - error description if the number of notes per given tag could not be returned
     * @return either non-negative value with the number of notes per given tag or -1 which means some error occured
     */
    int noteCountPerTag(const Tag & tag, ErrorString & errorDescription) const;

    /**
     * @brief noteCountsPerAllTags returns the number of non-deleted notes currently stored in local storage database labeled
     * with each tag stored in the local storage database
     * @param noteCountsPerTagLocalUid - the result hash: note counts by tag local uids
     * @param errorDescription - error description if the number of notes per all tags could not be returned
     * @return true if note counts for all tags were computed successfully, false otherwise
     */
    bool noteCountsPerAllTags(QHash<QString, int> & noteCountsPerTagLocalUid, ErrorString & errorDescription) const;

    /**
     * @brief addNote - adds passed in Note to the local storage database.
     * @param note - note to be passed to local storage database; required to contain either "remote" notebook guid
     * or local notebook uid; may be changed as a result of the call, filled with autogenerated fields like local uid
     * if it was empty before the call; also tag guids are filled if the note passed in contained only tag local uids
     * and tag local uids are filled if the note passed in contained only tag guids
     * @param errorDescription - error description if note could not be added
     * @return true if note was added successfully, false otherwise
     */
    bool addNote(Note & note, ErrorString & errorDescription);

    /**
     * @brief updateNote - updates passed in Note in the local storage database
     *
     * If the note has "remote" Evernote service's guid set, it is identified by this guid
     * in the local storage database. If no note with such guid is found, the local uid is used
     * to identify the note in the local storage database. If the note has no guid, the local uid
     * is used to identify it in the local storage database.
     *
     * @param note - note to be updated in the local storage database; required to contain either "remote" notebook guid
     * or local notebook uid; may be changed as a result of the call, filled with fields like local uid or notebook guid or local uid
     * if any of these were empty before the call; also tag guids are filled if the note passed in contained only tag local uids
     * and tag local uids are filled if the note passed in contained only tag guids. Bear in mind that after the call the note
     * may not have the representative resources if "updateResources" input parameter was false as well as it may not
     * have the representative tags if "updateTags" input parameter was false
     * @param updateResources - flag indicating whether the note's resources should be updated
     * along with the note; if not, the existing resource information stored in the local storage is not touched
     * @param updateTags - flag indicating whether the note's tags should be updated along with the note;
     * if not, the existing tags to note linkage information is not touched
     * @param errorDescription - error description if note could not be updated
     * @return true if note was updated successfully, false otherwise
     */
    bool updateNote(Note & note, const bool updateResources,
                    const bool updateTags, ErrorString & errorDescription);

    /**
     * @brief findNote - attempts to find note in the local storage database
     * @param note - note to be found in the local storage database. Must have either
     * local or "remote" Evernote service's guid set
     * @param errorDescription - error description if note could not be found
     * @param withResourceBinaryData - optional boolean parameter defining whether found note
     * should be filled with all the contents of its attached resources. By default this parameter is true
     * which means the whole contents of all resources would be filled. If it's false,
     * dataBody, recognitionBody or alternateDataBody won't be present within the found note's
     * resources
     * @return true if note was found, false otherwise
     */
    bool findNote(Note & note, ErrorString & errorDescription,
                  const bool withResourceBinaryData = true) const;

    /**
     * @brief The ListNotesOrder struct is a C++98-style scoped enum which allows to specify the ordering
     * of the results of methods listing notes from local storage
     */
    struct ListNotesOrder
    {
        enum type
        {
            ByUpdateSequenceNumber = 0,
            ByTitle,
            ByCreationTimestamp,
            ByModificationTimestamp,
            ByDeletionTimestamp,
            ByAuthor,
            BySource,
            BySourceApplication,
            ByReminderTime,
            ByPlaceName,
            NoOrder
        };
    };

    /**
     * @brief listNotesPerNotebook - attempts to list notes per given notebook
     * @param notebook - notebook for which the list of notes is requested. If it has
     * the "remote" Evernote service's guid set, it would be used to identify the notebook
     * in the local storage database, otherwise its local uid would be used
     * @param errorDescription - error description in case notes could not be listed
     * @param withResourceBinaryData - optional boolean parameter defining whether found notes
     * should be filled with all the contents of their respective attached resources.
     * By default this parameter is true which means the whole contents of all resources
     * would be filled. If it's false, dataBody, recognitionBody or alternateDataBody
     * won't be present within each found note's resources
     * @param flag - input parameter used to set the filter for the desired notes to be listed
     * @param limit - limit for the max number of notes in the result, zero by default which means no limit is set
     * @param offset - number of notes to skip in the beginning of the result, zero by default
     * @param order - allows to specify particular ordering of notes in the result, NoOrder by default
     * @param orderDirection - specifies the direction of ordering, by default ascending direction is used;
     * @return either list of notes per notebook or empty list in case of error or
     * no notes presence in the given notebook
     */
    QList<Note> listNotesPerNotebook(const Notebook & notebook, ErrorString & errorDescription,
                                     const bool withResourceBinaryData = true,
                                     const ListObjectsOptions & flag = ListAll,
                                     const size_t limit = 0, const size_t offset = 0,
                                     const ListNotesOrder::type & order = ListNotesOrder::NoOrder,
                                     const OrderDirection::type & orderDirection = OrderDirection::Ascending) const;

    /**
     * @brief listNotesPerTag - attempts to list notes labeled with a given tag
     * @param tag - tag for which the list of notes labeled with it is requested. If it has
     * the "remote" Evernote service's guid set, it would be used to identify the tag
     * in the local storage database, otherwise its local uid would be used
     * @param errorDescription - error description in case notes could not be listed
     * @param withResourceBinaryData - optional boolean parameter defining whether found notes
     * should be filled with all the contents of their respective attached resources.
     * By default this parameter is true which means the whole contents of all resources
     * would be filled. If it's false, dataBody, recognitionBody or alternateDataBody
     * won't be present within each found note's resources
     * @param flag - input parameter used to set the filter for the desired notes to be listed
     * @param limit - limit for the max number of notes in the result, zero by default which means no limit is set
     * @param offset - number of notes to skip in the beginning of the result, zero by default
     * @param order - allows to specify particular ordering of notes in the result, NoOrder by default
     * @param orderDirection - specifies the direction of ordering, by default ascending direction is used;
     * @return either list of notes per tag or empty list in case of error or no notes labeled with the given tag presence
     */
    QList<Note> listNotesPerTag(const Tag & tag, ErrorString & errorDescription,
                                const bool withResourceBinaryData,
                                const LocalStorageManager::ListObjectsOptions & flag,
                                const size_t limit, const size_t offset,
                                const LocalStorageManager::ListNotesOrder::type & order,
                                const LocalStorageManager::OrderDirection::type & orderDirection) const;

    /**
     * @brief listNotes - attempts to list notes within the account according to the specified input flag
     * @param flag - input parameter used to set the filter for the desired notes to be listed
     * @param errorDescription - error description if notes within the account could not be listed;
     * if no error happens, this parameter is untouched
     * @param withResourceBinaryData - optional boolean parameter defining whether found notes
     * should be filled with all the contents of their respective attached resources.
     * By default this parameter is true which means the whole contents of all resources
     * would be filled. If it's false, dataBody, recognitionBody or alternateDataBody
     * won't be present within each found note's resources
     * @param limit - limit for the max number of notes in the result, zero by default which means no limit is set
     * @param offset - number of notes to skip in the beginning of the result, zero by default
     * @param order - allows to specify particular ordering of notes in the result, NoOrder by default
     * @param orderDirection - specifies the direction of ordering, by defauls ascending direction is used;
     * this parameter has no meaning if order is equal to NoOrder
     * @param linkedNotebookGuid - if it's null, notes from both user's own notebooks and linked notebooks would be listed;
     * if it's empty, only the notes from non-linked notebooks would be listed;
     * otherwise, only the notes from the specified linked notebook would be listed
     * @return either list of notes within the account conforming to the filter or empty list
     * in cases of error or no notes conforming to the filter exist within the account
     */
    QList<Note> listNotes(const ListObjectsOptions flag, ErrorString & errorDescription,
                          const bool withResourceBinaryData = true, const size_t limit = 0,
                          const size_t offset = 0, const ListNotesOrder::type order = ListNotesOrder::NoOrder,
                          const OrderDirection::type orderDirection = OrderDirection::Ascending,
                          const QString & linkedNotebookGuid = QString()) const;

    /**
     * @brief findNoteLocalUidsWithSearchQuery - attempt to find note local uids of notes
     * corresponding to the passed in NoteSearchQuery object.
     * @param noteSearchQuery - filled NoteSearchQuery object used to filter the notes
     * @param errorDescription - error description in case note local uids could not be listed
     */
    QStringList findNoteLocalUidsWithSearchQuery(const NoteSearchQuery & noteSearchQuery,
                                                 ErrorString & errorDescription) const;

    /**
     * @brief findNotesWithSearchQuery - attempt to find notes corresponding to the passed in
     * NoteSearchQuery object.
     * @param noteSearchQuery - filled NoteSearchQuery object used to filter the notes
     * @param errorDescription - error description in case notes could not be listed
     * @param withResourceBinaryData - optional boolean parameter defining whether found notes
     * should be filled with all the contents of their respective attached resources.
     * By default this parameter is true which means the whole contents of all resources
     * would be filled. If it's false, dataBody, recognitionBody or alternateDataBody
     * won't be present within each found note's resources
     * @return either list of notes per NoteSearchQuery or empty list in case of error or
     * no notes presence for the given NoteSearchQuery
     */
    NoteList findNotesWithSearchQuery(const NoteSearchQuery & noteSearchQuery,
                                      ErrorString & errorDescription,
                                      const bool withResourceBinaryData = true) const;

    /**
     * @brief expungeNote - permanently deletes note from local storage.
     * Evernote API doesn't allow to delete notes from remote storage, it can
     * only be done by official desktop client or web GUI. So this method should be called
     * only during the synchronization with remote database, when some note is found to be
     * deleted via either official desktop client or web GUI.
     * @param note - note to be expunged; may be changed as a result of the call, filled with fields like local uid
     * or notebook guid or local uid
     * @param errorDescription - error description if note could not be expunged
     * @return true if note was expunged successfully, false otherwise
     */
    bool expungeNote(Note & note, ErrorString & errorDescription);

    /**
     * @brief tagCount returns the number of non-deleted tags currently stored in local storage database
     * @param errorDescription - error description if the number of tags could not be returned
     * @return either non-negative value with the number of tags or -1 which means some error occured
     */
    int tagCount(ErrorString & errorDescription) const;

    /**
     * @brief addTag - adds passed in Tag to the local storage database. If tag has
     * "remote" Evernote service's guid set, it is identified in the database by this guid.
     * Otherwise it is identified by local uid.
     * @param tag - tag to be added to the local storage; may be changed as a result of the call,
     * filled with autogenerated fields like local uid if it was empty before the call
     * @param errorDescription - error description if Tag could not be added
     * @return true if Tag was added successfully, false otherwise
     */
    bool addTag(Tag & tag, ErrorString & errorDescription);

    /**
     * @brief updateTag - updates passed in Tag in the local storage database.
     *
     * If the tag has "remote" Evernote service's guid set, it is identified by this guid
     * in the local storage database. If no tag with such guid is found, the local uid is used
     * to identify the tag in the local storage database. If the tag has no guid, the local uid
     * is used to identify it in the local storage database.
     *
     * @param tag - Tag filled with values to be updated in the local storage database. Note that it can be changed
     * as a result of the call: automatically filled with local uid if it was empty before the call
     * @param errorDescription - error description if Tag could not be updated
     * @return true if Tag was updated successfully, false otherwise
     */
    bool updateTag(Tag & tag, ErrorString & errorDescription);

    /**
     * @brief findTag - attempts to find and fill the fields of passed in tag object.
     *
     * If "remote" Evernote service's guid for the tag is set, it would be used to identify
     * the tag in the local storage database. Otherwise the local uid would be used. If neither
     * guid nor local uid are set, tag's name would be used. If the name is also
     * not set, the search would fail.
     *
     * Important! Due to the fact that the tag name is only unique within
     * the users's own account as well as within each linked notebook, the
     * result of the search by name depends on the tag's linked notebook
     * guid: if it is not set, the search by name would only search for the tag
     * with the specified name within the user's own account. If it is set, the
     * search would only consider tags from a linked notebook with the corresponding
     * guid.
     *
     * @param tag - tag to be found in the local storage database; must have either guid, local uid or name set
     * @param errorDescription - error description in case tag could not be found
     * @return true if tag was found, false otherwise
     */
    bool findTag(Tag & tag, ErrorString & errorDescription) const;

    struct ListTagsOrder
    {
        enum type
        {
            ByUpdateSequenceNumber,
            ByName,
            NoOrder
        };
    };

    /**
     * @brief listAllTagsPerNote - lists all tags per given note
     * @param note - note for which the list of tags is requested. If it has "remote"
     * Evernote service's guid set, it is used to identify the note in the local storage database.
     * Otherwise its local uid is used for that.
     * @param errorDescription - error description if tags were not listed successfully.
     * In such case the returned list of tags would be empty and error description won't be empty.
     * However, if, for example, the list of tags is empty and error description is empty too,
     * it means the provided note does not have any tags assigned to it.
     * @param flag - input parameter used to set the filter for the desired tags to be listed
     * @param limit - limit for the max number of tags in the result, zero by default which means no limit is set
     * @param offset - number of tags to skip in the beginning of the result, zero by default
     * @param order - allows to specify particular ordering of tags in the result, NoOrder by default
     * @param orderDirection - specifies the direction of ordering, by default ascending direction is used;
     * @return the list of found tags per note
     */
    QList<Tag> listAllTagsPerNote(const Note & note, ErrorString & errorDescription,
                                  const ListObjectsOptions & flag = ListAll,
                                  const size_t limit = 0, const size_t offset = 0,
                                  const ListTagsOrder::type & order = ListTagsOrder::NoOrder,
                                  const OrderDirection::type & orderDirection = OrderDirection::Ascending) const;

    /**
     * @brief listAllTags - lists all tags within current user's account
     * @param errorDescription - error description if tags were not listed successfully.
     * In such case the returned list of tags would be empty and error description won't be empty.
     * However, if, for example, the list of tags is empty and error description is empty too,
     * it means the current account does not have any tags created.
     * @param limit - limit for the max number of tags in the result, zero by default which means no limit is set
     * @param offset - number of tags to skip in the beginning of the result, zero by default
     * @param order - allows to specify particular ordering of tags in the result, NoOrder by default
     * @param orderDirection - specifies the direction of ordering, by defauls ascending direction is used;
     * this parameter has no meaning if order is equal to NoOrder
     * @param linkedNotebookGuid - if it's null, the method would list tags ignoring their belonging to the current account
     * or to some linked notebook; if it's empty, only the tags from user's own account would be listed;
     * otherwise, only the tags corresponding to the certain linked notebook would be listed
     * @return the list of found tags within the account
     */
    QList<Tag> listAllTags(ErrorString & errorDescription, const size_t limit = 0,
                           const size_t offset = 0, const ListTagsOrder::type order = ListTagsOrder::NoOrder,
                           const OrderDirection::type orderDirection = OrderDirection::Ascending,
                           const QString & linkedNotebookGuid = QString()) const;

    /**
     * @brief listTags - attempts to list tags within the account according to the specified input flag
     * @param flag - input parameter used to set the filter for the desired tags to be listed
     * @param errorDescription - error description if notes within the account could not be listed;
     * if no error happens, this parameter is untouched
     * @param limit - limit for the max number of tags in the result, zero by default which means no limit is set
     * @param offset - number of tags to skip in the beginning of the result, zero by default
     * @param order - allows to specify particular ordering of tags in the result, NoOrder by default
     * @param orderDirection - specifies the direction of ordering, by defauls ascending direction is used;
     * this parameter has no meaning if order is equal to NoOrder
     * @param linkedNotebookGuid - if it's null, the method would list tags ignoring their belonging to the current account
     * or to some linked notebook; if it's empty, only the tags from user's own account would be listed;
     * otherwise, only the tags corresponding to the certain linked notebook would be listed
     * @return either list of tags within the account conforming to the filter or empty list
     * in cases of error or no tags conforming to the filter exist within the account
     */
    QList<Tag> listTags(const ListObjectsOptions flag, ErrorString & errorDescription,
                        const size_t limit = 0, const size_t offset = 0,
                        const ListTagsOrder::type & order = ListTagsOrder::NoOrder,
                        const OrderDirection::type orderDirection = OrderDirection::Ascending,
                        const QString & linkedNotebookGuid = QString()) const;

    /**
     * @brief expungeTag - permanently deletes tag from local storage.
     * Evernote API doesn't allow to delete tags from remote storage, it can
     * only be done by official desktop client or web GUI. So this method should be called
     * only during the synchronization with remote database, when some tag is found to be
     * deleted via either official desktop client or web GUI.
     * @param tag - tag to be expunged; may be changed as a result of the call,
     * automatically filled with local uid if it was empty before the call
     * @param expungedChildTagLocalUids - if the expunged tag was a parent of some other tags,
     * these were expunged as well; this parameter would contain the local uids of expunged child tags
     * @param errorDescription - error description if tag could not be expunged
     * @return true if tag was expunged successfully, false otherwise
     */
    bool expungeTag(Tag & tag, QStringList & expungedChildTagLocalUids, ErrorString & errorDescription);

    /**
     * @brief expungeNotelessTagsFromLinkedNotebooks - permanently deletes from local storage those tags
     * which belong to some linked notebook and are not linked with any notes in the local storage
     * @param errorDescription - error description if tag could not be expunged
     * @return true if relevant tags were expunged successfully, false otherwise
     */
    bool expungeNotelessTagsFromLinkedNotebooks(ErrorString & errorDescription);

    /**
     * @brief enResourceCount (the name is not Resource to prevent problems with Resource macro defined
     * on some versions of Windows) returns the number of resources currently stored in local storage database
     * @param errorDescription - error description if the number of resources could not be returned
     * @return either non-negative value with the number of resources or -1 which means some error occured
     */
    int enResourceCount(ErrorString & errorDescription) const;

    /**
     * @brief addEnResource - adds passed in resource to the local storage database
     * @param resource - resource to be added to the database, must have either note's local uid set
     * or note's "remote" Evernote service's guid set; may be changed as a result of the call,
     * filled with autogenerated fields like local uid if it was empty before the call
     * @param errorDescription - error description if resource could not be added
     * @return true if resource was added successfully, false otherwise
     */
    bool addEnResource(Resource & resource, ErrorString & errorDescription);

    /**
     * @brief updateEnResource - updates passed in resource in the local storage database.
     *
     * If the resource has "remote" Evernote service's guid set, it is identified by this guid
     * in the local storage database. If no resource with such guid is found, the local uid is used
     * to identify the resource in the local storage database. If the resource has no guid, the local uid
     * is used to identify it in the local storage database.
     *
     * @param resource - resource to be updated; may be changed as a result of the call,
     * automatically filled with local uid and note local uid and/or guid if these were empty before the call
     * @param errorDescription - error description if resource could not be updated
     * @return true if resource was updated successfully, false otherwise
     */
    bool updateEnResource(Resource & resource, ErrorString & errorDescription);

    /**
     * @brief findEnResource - attempts to find resource in the local storage database
     * @param resource - resource to be found in the local storage database. If it has
     * "remote" Evernote service's guid set, this guid is used to identify the resource
     * in the local storage database. Otherwise resource's local uid is used
     * @param errorDescription - error description if resource could not be found
     * @param withBinaryData - optional parameter defining whether found resource should have
     * dataBody recognitionBody, alternateDataBody filled with actual binary data.
     * By default this parameter is true.
     * @return true if resource was found successfully, false otherwise
     */
    bool findEnResource(Resource & resource, ErrorString & errorDescription, const bool withBinaryData = true) const;

    // NOTE: there is no 'deleteEnResource' method for a reason: resources are deleted automatically
    // in remote storage so there's no need to mark some resource as deleted for the synchronization procedure.

    /**
     * @brief expungeResource - permanently deletes resource from local storage completely,
     * without the possibility to restore it
     * @param resource - resource to be expunged; may be changed as a result of the call,
     * automatically filled with local uid and note local uid and/or guid if these were empty before the call
     * @param errorDescription - error description if resource could not be expunged
     * @return true if resource was expunged successfully, false otherwise
     */
    bool expungeEnResource(Resource & resource, ErrorString & errorDescription);

    /**
     * @brief savedSearchCount returns the number of saved seacrhes currently stored in local storage database
     * @param errorDescription - error description if the number of saved seacrhes could not be returned
     * @return either non-negative value with the number of saved seacrhes or -1 which means some error occured
     */
    int savedSearchCount(ErrorString & errorDescription) const;

    /**
     * @brief addSavedSearch - adds passed in SavedSearch to the local storage database;
     * If search has "remote" Evernote service's guid set, it is identified in the database
     * by this guid. Otherwise it is identified by local uid.
     * @param search - SavedSearch to be added to the local storage; may be changed as a result of the call,
     * filled with autogenerated fields like local uid if it was empty before the call
     * @param errorDescription - error description if SavedSearch could not be added
     * @return true if SavedSearch was added successfully, false otherwise
     */
    bool addSavedSearch(SavedSearch & search, ErrorString & errorDescription);

    /**
     * @brief updateSavedSearch - updates passed in SavedSearch in th local storage database.
     *
     * If search has "remote" Evernote service's guid set, it is identified in the database
     * by this guid. If no saved search with such guid is found, the local uid is used
     * to identify the saved search in the local storage database. If the saved search has no guid,
     * the local uid is used to identify it in the local storage database.
     *
     * @param search - SavedSearch filled with values to be updated in the local storage database;
     * may be changed as a result of the call filled local uid if it was empty before the call
     * @param errorDescription - error description if SavedSearch could not be updated
     * @return true if SavedSearch was updated successfully, false otherwise
     */
    bool updateSavedSearch(SavedSearch & search, ErrorString & errorDescription);

    /**
     * @brief findSavedSearch - attempts to find SavedSearch in the local storage database
     * @param search - SavedSearch to be found in the local storage database. If it
     * would have "remote" Evernote service's guid set, it would be used to identify
     * the search in the local storage database. Otherwise its local uid would be used.
     * @param errorDescription - error description if SavedSearch could not be found
     * @return true if SavedSearch was found, false otherwise
     */
    bool findSavedSearch(SavedSearch & search, ErrorString & errorDescription) const;

    /**
     * @brief The ListSavedSearchesOrder struct is a C++98-style scoped enum which allows to specify the ordering
     * of the results of methods listing saved searches from local storage
     */
    struct ListSavedSearchesOrder
    {
        enum type
        {
            ByUpdateSequenceNumber = 0,
            ByName,
            ByFormat,
            NoOrder
        };
    };

    /**
     * @brief listAllSavedSearches - lists all saved searches within the account
     * @param errorDescription - error description if all saved searches could not be listed;
     * otherwise this parameter is untouched
     * @param limit - limit for the max number of saved searches in the result, zero by default which means no limit is set
     * @param offset - number of saved searches to skip in the beginning of the result, zero by default
     * @param order - allows to specify particular ordering of saved searches in the result, NoOrder by default
     * @param orderDirection - specifies the direction of ordering, by defauls ascending direction is used;
     * this parameter has no meaning if order is equal to NoOrder
     * @return either the list of all saved searches within the account or empty list
     * in case of error or if there are no saved searches within the account
     */
    QList<SavedSearch> listAllSavedSearches(ErrorString & errorDescription, const size_t limit = 0, const size_t offset = 0,
                                            const ListSavedSearchesOrder::type order = ListSavedSearchesOrder::NoOrder,
                                            const OrderDirection::type orderDirection = OrderDirection::Ascending) const;

    /**
     * @brief listSavedSearches - attempts to list saved searches within the account
     * according to the specified input flag
     * @param flag - input parameter used to set the filter for the desired saved searches to be listed
     * @param errorDescription - error description if saved searches within the account could not be listed;
     * if no error happens, this parameter is untouched
     * @param limit - limit for the max number of saved searches in the result, zero by default which means no limit is set
     * @param offset - number of saved searches to skip in the beginning of the result, zero by default
     * @param order - allows to specify particular ordering of saved searches in the result, NoOrder by default
     * @param orderDirection - specifies the direction of ordering, by defauls ascending direction is used;
     * this parameter has no meaning if order is equal to NoOrder
     * @return either list of saved searches within the account conforming to the filter or empty list
     * in cases of error or no saved searches conforming to the filter exist within the account
     */
    QList<SavedSearch> listSavedSearches(const ListObjectsOptions flag, ErrorString & errorDescription, const size_t limit = 0, const size_t offset = 0,
                                         const ListSavedSearchesOrder::type order = ListSavedSearchesOrder::NoOrder,
                                         const OrderDirection::type orderDirection = OrderDirection::Ascending) const;

    // NOTE: there is no 'deleteSearch' method for a reason: saved searches are deleted automatically
    // in remote storage so there's no need to mark some saved search as deleted for synchronization procedure.

    /**
     * @brief expungeSavedSearch - permanently deletes saved search from local storage completely,
     * without the possibility to restore it
     * @param search - saved search to be expunged; may be changed as a result of the call filled local uid
     * if it was empty before the call
     * @param errorDescription - error description if saved search could not be expunged
     * @return true if saved search was expunged succesfully, false otherwise
     */
    bool expungeSavedSearch(SavedSearch & search, ErrorString & errorDescription);

    /**
     * @brief accountHighUsn - returns the highest update sequence number within the data elements
     * stored in the local storage database, either for user's own account or for some linked notebook
     * @param linkedNotebookGuid - the guid of the linked notebook for which the highest update sequence number is
     * requested; if null or empty, the highest update sequence number for user's own account is returned
     * @param errorDescription - error description if account's highest update sequence number could not be returned
     * @return either the highest update sequence number - a non-negative value - or a negative number
     * in case of error
     */
    qint32 accountHighUsn(const QString & linkedNotebookGuid, ErrorString & errorDescription);

private:
    LocalStorageManager() Q_DECL_EQ_DELETE;
    Q_DISABLE_COPY(LocalStorageManager)

    QScopedPointer<LocalStorageManagerPrivate>  d_ptr;
    Q_DECLARE_PRIVATE(LocalStorageManager)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(LocalStorageManager::ListObjectsOptions)

} // namespace quentier

#endif // LIB_QUENTIER_LOCAL_STORAGE_LOCAL_STORAGE_MANAGER_H
