#include <quentier/types/Account.h>
#include "data/AccountData.h"

namespace quentier {

Account::Account() :
    d(new AccountData)
{
    d->m_accountType = Account::Type::Local;
    d->m_userId = -1;
}

Account::Account(const QString & name, const Type::type type,
                 const qevercloud::UserID userId,
                 const EvernoteAccountType::type evernoteAccountType,
                 const QString & evernoteHost,
                 const QString & shardId) :
    d(new AccountData)
{
    d->m_name = name;
    d->m_accountType = type;
    d->m_userId = userId;
    d->m_evernoteHost = evernoteHost;
    d->m_shardId = shardId;
    d->switchEvernoteAccountType(evernoteAccountType);
}

Account::Account(const Account & other) :
    Printable(),
    d(other.d)
{}

Account & Account::operator=(const Account & other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

Account::~Account()
{}

bool Account::operator==(const Account & other) const
{
    if (d == other.d) {
        return true;
    }

    // NOTE: display name intentionally does not take part in the comparison
    if ((d->m_name == other.d->m_name) &&
        (d->m_accountType == other.d->m_accountType) &&
        (d->m_evernoteAccountType == other.d->m_evernoteAccountType) &&
        (d->m_userId == other.d->m_userId) &&
        (d->m_evernoteHost == other.d->m_evernoteHost) &&
        (d->m_shardId == other.d->m_shardId) &&
        (d->m_mailLimitDaily == other.d->m_mailLimitDaily) &&
        (d->m_noteSizeMax == other.d->m_noteSizeMax) &&
        (d->m_resourceSizeMax == other.d->m_resourceSizeMax) &&
        (d->m_linkedNotebookMax == other.d->m_linkedNotebookMax) &&
        (d->m_noteCountMax == other.d->m_noteCountMax) &&
        (d->m_notebookCountMax == other.d->m_notebookCountMax) &&
        (d->m_tagCountMax == other.d->m_tagCountMax) &&
        (d->m_noteTagCountMax == other.d->m_noteTagCountMax) &&
        (d->m_savedSearchCountMax == other.d->m_savedSearchCountMax) &&
        (d->m_noteResourceCountMax == other.d->m_noteResourceCountMax))
    {
        return true;
    }

    return false;
}

bool Account::operator!=(const Account & other) const
{
    return !(operator==(other));
}

bool Account::isEmpty() const
{
    if (d->m_accountType == Account::Type::Local) {
        return d->m_name.isEmpty();
    }
    else {
        return (d->m_userId < 0);
    }
}

void Account::setEvernoteAccountType(const EvernoteAccountType::type evernoteAccountType)
{
    d->switchEvernoteAccountType(evernoteAccountType);
}

void Account::setEvernoteHost(const QString & evernoteHost)
{
    d->m_evernoteHost = evernoteHost;
}

void Account::setShardId(const QString & shardId)
{
    d->m_shardId = shardId;
}

void Account::setEvernoteAccountLimits(const qevercloud::AccountLimits & limits)
{
    d->setEvernoteAccountLimits(limits);
}

QString Account::name() const
{
    return d->m_name;
}

void Account::setName(const QString & name)
{
    d->m_name = name;
}

QString Account::displayName() const
{
    return d->m_displayName;
}

void Account::setDisplayName(const QString & displayName)
{
    d->m_displayName = displayName;
}

qevercloud::UserID Account::id() const
{
    return d->m_userId;
}

Account::Type::type Account::type() const
{
    return d->m_accountType;
}

Account::EvernoteAccountType::type Account::evernoteAccountType() const
{
    return d->m_evernoteAccountType;
}

QString Account::evernoteHost() const
{
    return d->m_evernoteHost;
}

QString Account::shardId() const
{
    return d->m_shardId;
}

qint32 Account::mailLimitDaily() const
{
    return d->m_mailLimitDaily;
}

qint64 Account::noteSizeMax() const
{
    return d->m_noteSizeMax;
}

qint64 Account::resourceSizeMax() const
{
    return d->m_resourceSizeMax;
}

qint32 Account::linkedNotebookMax() const
{
    return d->m_linkedNotebookMax;
}

qint32 Account::noteCountMax() const
{
    return d->m_noteCountMax;
}

qint32 Account::notebookCountMax() const
{
    return d->m_notebookCountMax;
}

qint32 Account::tagCountMax() const
{
    return d->m_tagCountMax;
}

qint32 Account::noteTagCountMax() const
{
    return d->m_noteTagCountMax;
}

qint32 Account::savedSearchCountMax() const
{
    return d->m_savedSearchCountMax;
}

qint32 Account::noteResourceCountMax() const
{
    return d->m_noteResourceCountMax;
}

QTextStream & Account::print(QTextStream & strm) const
{
    strm << QStringLiteral("Account: {\n");
    strm << QStringLiteral("    name = ") << d->m_name << QStringLiteral(";\n");
    strm << QStringLiteral("    display name = ") << d->m_displayName << QStringLiteral(";\n");
    strm << QStringLiteral("    id = ") << d->m_userId << QStringLiteral(";\n");

    strm << QStringLiteral("    type = ");
    switch(d->m_accountType)
    {
    case Account::Type::Local:
        strm << QStringLiteral("Local");
        break;
    case Account::Type::Evernote:
        strm << QStringLiteral("Evernote");
        break;
    default:
        strm << QStringLiteral("Unknown");
        break;
    }
    strm << QStringLiteral(";\n");

    strm << QStringLiteral("    Evernote account type = ");
    switch(d->m_evernoteAccountType)
    {
    case Account::EvernoteAccountType::Free:
        strm << QStringLiteral("Free");
        break;
    case Account::EvernoteAccountType::Plus:
        strm << QStringLiteral("Plus");
        break;
    case Account::EvernoteAccountType::Premium:
        strm << QStringLiteral("Premium");
        break;
    case Account::EvernoteAccountType::Business:
        strm << QStringLiteral("Business");
        break;
    default:
        strm << QStringLiteral("Unknown");
        break;
    }
    strm << QStringLiteral(";\n");

    strm << QStringLiteral("    Evernote host = ") << d->m_evernoteHost << QStringLiteral(";\n");
    strm << QStringLiteral("    shard id = ") << d->m_shardId << QStringLiteral(";\n");
    strm << QStringLiteral("    mail limit daily = ") << d->m_mailLimitDaily << QStringLiteral(";\n");
    strm << QStringLiteral("    note size max = ") << d->m_noteSizeMax << QStringLiteral(";\n");
    strm << QStringLiteral("    resource size max = ") << d->m_resourceSizeMax << QStringLiteral(";\n");
    strm << QStringLiteral("    linked notebook max = ") << d->m_linkedNotebookMax << QStringLiteral(";\n");
    strm << QStringLiteral("    note count max = ") << d->m_noteCountMax << QStringLiteral(";\n");
    strm << QStringLiteral("    notebook count max = ") << d->m_notebookCountMax << QStringLiteral(";\n");
    strm << QStringLiteral("    tag count max = ") << d->m_tagCountMax << QStringLiteral(";\n");
    strm << QStringLiteral("    note tag count max = ") << d->m_noteTagCountMax << QStringLiteral(";\n");
    strm << QStringLiteral("    saved search count max = ") << d->m_savedSearchCountMax << QStringLiteral(";\n");
    strm << QStringLiteral("    note resource count max = ") << d->m_noteResourceCountMax << QStringLiteral(";\n");
    strm << QStringLiteral("};\n");

    return strm;
}

} // namespace quentier
