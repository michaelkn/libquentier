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

#include "data/TagData.h"
#include <quentier/types/Tag.h>

namespace quentier {

QN_DEFINE_LOCAL_UID(Tag)
QN_DEFINE_DIRTY(Tag)
QN_DEFINE_LOCAL(Tag)
QN_DEFINE_FAVORITED(Tag)

Tag::Tag() :
    d(new TagData)
{}

Tag::Tag(const Tag & other) :
    d(other.d)
{}

Tag::Tag(Tag && other) :
    d(std::move(other.d))
{}

Tag & Tag::operator=(Tag && other)
{
    if (this != &other) {
        d = std::move(other.d);
    }

    return *this;
}

Tag::Tag(const qevercloud::Tag & other) :
    d(new TagData(other))
{}

Tag::Tag(qevercloud::Tag && other) :
    d(new TagData(std::move(other)))
{}

Tag & Tag::operator=(const Tag & other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

Tag::~Tag()
{}

bool Tag::operator==(const Tag & other) const
{
    if (isFavorited() != other.isFavorited()) {
        return false;
    }
    else if (isLocal() != other.isLocal()) {
        return false;
    }
    else if (isDirty() != other.isDirty()) {
        return false;
    }
    else if (d == other.d) {
        return true;
    }
    else {
        return (*d == *(other.d));
    }
}

bool Tag::operator!=(const Tag & other) const
{
    return !(*this == other);
}

const qevercloud::Tag & Tag::qevercloudTag() const
{
    return d->m_qecTag;
}

qevercloud::Tag & Tag::qevercloudTag()
{
    return d->m_qecTag;
}

void Tag::clear()
{
    d->clear();
}

bool Tag::validateName(const QString & name, ErrorString * pErrorDescription)
{
    if (name != name.trimmed())
    {
        if (pErrorDescription) {
            pErrorDescription->setBase(QT_TR_NOOP("Tag name cannot start or end with whitespace"));
            pErrorDescription->details() = name;
        }

        return false;
    }

    int len = name.length();
    if (len < qevercloud::EDAM_TAG_NAME_LEN_MIN)
    {
        if (pErrorDescription) {
            pErrorDescription->setBase(QT_TR_NOOP("Tag name's length is too small"));
            pErrorDescription->details() = name;
        }

        return false;
    }

    if (len > qevercloud::EDAM_TAG_NAME_LEN_MAX)
    {
        if (pErrorDescription) {
            pErrorDescription->setBase(QT_TR_NOOP("Tag name's length is too large"));
            pErrorDescription->details() = name;
        }

        return false;
    }

    return !name.contains(QStringLiteral(","));
}

bool Tag::hasGuid() const
{
    return d->m_qecTag.guid.isSet();
}

const QString & Tag::guid() const
{
    return d->m_qecTag.guid;
}

void Tag::setGuid(const QString & guid)
{
    if (!guid.isEmpty()) {
        d->m_qecTag.guid = guid;
    }
    else {
        d->m_qecTag.guid.clear();
    }
}

bool Tag::hasUpdateSequenceNumber() const
{
    return d->m_qecTag.updateSequenceNum.isSet();
}

qint32 Tag::updateSequenceNumber() const
{
    return d->m_qecTag.updateSequenceNum;
}

void Tag::setUpdateSequenceNumber(const qint32 usn)
{
    if (usn >= 0) {
        d->m_qecTag.updateSequenceNum = usn;
    }
    else {
        d->m_qecTag.updateSequenceNum.clear();
    }
}

bool Tag::checkParameters(ErrorString & errorDescription) const
{
    if (localUid().isEmpty() && !d->m_qecTag.guid.isSet()) {
        errorDescription.setBase(QT_TR_NOOP("Both tag's local and remote guids are empty"));
        return false;
    }

    return d->checkParameters(errorDescription);
}

bool Tag::hasName() const
{
    return d->m_qecTag.name.isSet();
}

const QString & Tag::name() const
{
    return d->m_qecTag.name;
}

void Tag::setName(const QString & name)
{
    if (!name.isEmpty()) {
        d->m_qecTag.name = name;
    }
    else {
        d->m_qecTag.name.clear();
    }
}

bool Tag::hasParentGuid() const
{
    return d->m_qecTag.parentGuid.isSet();
}

const QString & Tag::parentGuid() const
{
    return d->m_qecTag.parentGuid;
}

void Tag::setParentGuid(const QString & parentGuid)
{
    if (!parentGuid.isEmpty()) {
        d->m_qecTag.parentGuid = parentGuid;
    }
    else {
        d->m_qecTag.parentGuid.clear();
    }
}

bool Tag::hasParentLocalUid() const
{
    return d->m_parentLocalUid.isSet();
}

const QString & Tag::parentLocalUid() const
{
    return d->m_parentLocalUid;
}

void Tag::setParentLocalUid(const QString & parentLocalUid)
{
    if (!parentLocalUid.isEmpty()) {
        d->m_parentLocalUid = parentLocalUid;
    }
    else {
        d->m_parentLocalUid.clear();
    }
}

bool Tag::hasLinkedNotebookGuid() const
{
    return d->m_linkedNotebookGuid.isSet();
}

const QString & Tag::linkedNotebookGuid() const
{
    return d->m_linkedNotebookGuid;
}

void Tag::setLinkedNotebookGuid(const QString & linkedNotebookGuid)
{
    if (!linkedNotebookGuid.isEmpty()) {
        d->m_linkedNotebookGuid = linkedNotebookGuid;
    }
    else {
        d->m_linkedNotebookGuid.clear();
    }
}

QTextStream & Tag::print(QTextStream & strm) const
{
    strm << QStringLiteral("Tag { \n");

#define INSERT_DELIMITER \
    strm << QStringLiteral("; \n")

    const QString localUid_ = localUid();
    if (!localUid_.isEmpty()) {
        strm << QStringLiteral("localUid: ") << localUid_;
    }
    else {
        strm << QStringLiteral("localUid is not set");
    }
    INSERT_DELIMITER;

    if (d->m_parentLocalUid.isSet()) {
        strm << QStringLiteral("parent local uid: ") << d->m_parentLocalUid.ref();
    }
    else {
        strm << QStringLiteral("parent local uid is not set");
    }
    INSERT_DELIMITER;

    if (d->m_qecTag.guid.isSet()) {
        strm << QStringLiteral("guid: ") << d->m_qecTag.guid;
    }
    else {
        strm << QStringLiteral("guid is not set");
    }
    INSERT_DELIMITER;

    if (d->m_linkedNotebookGuid.isSet()) {
        strm << QStringLiteral("linked notebook guid: ") << d->m_linkedNotebookGuid;
    }
    else {
        strm << QStringLiteral("linked notebook guid is not set");
    }
    INSERT_DELIMITER;

    if (d->m_qecTag.name.isSet()) {
        strm << QStringLiteral("name: ") << d->m_qecTag.name;
    }
    else {
        strm << QStringLiteral("name is not set");
    }
    INSERT_DELIMITER;

    if (d->m_qecTag.parentGuid.isSet()) {
        strm << QStringLiteral("parentGuid: ") << d->m_qecTag.parentGuid;
    }
    else {
        strm << QStringLiteral("parentGuid is not set");
    }
    INSERT_DELIMITER;

    if (d->m_qecTag.updateSequenceNum.isSet()) {
        strm << QStringLiteral("updateSequenceNumber: ") << QString::number(d->m_qecTag.updateSequenceNum);
    }
    else {
        strm << QStringLiteral("updateSequenceNumber is not set");
    }
    INSERT_DELIMITER;

    strm << QStringLiteral("isDirty: ") << (isDirty() ? QStringLiteral("true") : QStringLiteral("false"));
    INSERT_DELIMITER;

    strm << QStringLiteral("isLocal: ") << (d->m_isLocal ? QStringLiteral("true") : QStringLiteral("false"));
    INSERT_DELIMITER;

    strm << QStringLiteral("isFavorited = ") << (isFavorited() ? QStringLiteral("true") : QStringLiteral("false"));
    INSERT_DELIMITER;

    strm << QStringLiteral("}; \n");
    return strm;
}

} // namespace quentier
