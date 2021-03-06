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

#ifndef LIB_QUENTIER_ENML_ENML_CONVERTER_H
#define LIB_QUENTIER_ENML_ENML_CONVERTER_H

#include <quentier/utility/Printable.h>
#include <quentier/utility/Linkage.h>
#include <quentier/utility/Macros.h>
#include <quentier/types/ErrorString.h>
#include <quentier/types/Note.h>
#include <QSet>
#include <QString>
#include <QHash>
#include <QTextDocument>

namespace quentier {

QT_FORWARD_DECLARE_CLASS(Resource)
QT_FORWARD_DECLARE_CLASS(DecryptedTextManager)
QT_FORWARD_DECLARE_CLASS(ENMLConverterPrivate)

/**
 * @brief The ENMLConverter class encapsulates a set of methods
 * and helper data structures for performing the conversions between ENML
 * and other note content formats, namely HTML
 */
class QUENTIER_EXPORT ENMLConverter
{
public:
    ENMLConverter();
    virtual ~ENMLConverter();

    /**
     * @brief The SkipHtmlElementRule class describes the set of rules
     * for HTML -> ENML conversion about the HTML elements that should not
     * be actually converted to ENML due to their nature of being "helper"
     * elements for the display or functioning of something within the note editor's page.
     * The HTML -> ENML conversion would ignore tags and attributes forbidden by ENML
     * even without these rules conditionally preserving or skipping the contents and nested elements
     * of skipped elements
     */
    class QUENTIER_EXPORT SkipHtmlElementRule: public Printable
    {
    public:
        enum ComparisonRule {
            Equals = 0,
            StartsWith,
            EndsWith,
            Contains
        };

        SkipHtmlElementRule() :
            m_elementNameToSkip(),
            m_elementNameComparisonRule(Equals),
            m_elementNameCaseSensitivity(Qt::CaseSensitive),
            m_attributeNameToSkip(),
            m_attributeNameComparisonRule(Equals),
            m_attributeNameCaseSensitivity(Qt::CaseSensitive),
            m_attributeValueToSkip(),
            m_attributeValueComparisonRule(Equals),
            m_attributeValueCaseSensitivity(Qt::CaseSensitive),
            m_includeElementContents(false)
        {}

        virtual QTextStream & print(QTextStream & strm) const Q_DECL_OVERRIDE;

        QString             m_elementNameToSkip;
        ComparisonRule      m_elementNameComparisonRule;
        Qt::CaseSensitivity m_elementNameCaseSensitivity;

        QString             m_attributeNameToSkip;
        ComparisonRule      m_attributeNameComparisonRule;
        Qt::CaseSensitivity m_attributeNameCaseSensitivity;

        QString             m_attributeValueToSkip;
        ComparisonRule      m_attributeValueComparisonRule;
        Qt::CaseSensitivity m_attributeValueCaseSensitivity;

        bool                m_includeElementContents;
    };

    bool htmlToNoteContent(const QString & html, QString & noteContent,
                           DecryptedTextManager & decryptedTextManager,
                           ErrorString & errorDescription,
                           const QVector<SkipHtmlElementRule> & skipRules = QVector<SkipHtmlElementRule>()) const;

    /**
     * @brief cleanupExternalHtml method cleans up a piece of HTML coming from some external source: the cleanup includes
     * the removal (or replacement with equivalents/alternatives) of any tags and attributes not supported by the ENML
     * representation of note page's HTML
     *
     * @param inputHtml - the input HTML to be cleaned up
     * @param cleanedUpHtml - the result of the method's work
     * @param errorDescription - the textual description of the error if
     * conversion of input HTML into QTextDocument has failed
     *
     * @return true in case of successful conversion, false otherwise
     */
    bool cleanupExternalHtml(const QString & inputHtml, QString & cleanedUpHtml,
                             ErrorString & errorDescription) const;

    /**
     * Converts the passed in HTML into its simplified form acceptable by
     * QTextDocument (see http://doc.qt.io/qt-5/richtext-html-subset.html for
     * the list of elements supported by QTextDocument)
     *
     * @param html - the input HTML which needs to be converted to QTextDocument
     * @param doc - QTextDocument filled with the result of the method's work
     * @param errorDescription - the textual description of the error if
     * conversion of input HTML into QTextDocument has failed
     * @param skipRules - rules for skipping the particular elements
     *
     * @return true in case of successful conversion, false otherwise
     */
    bool htmlToQTextDocument(const QString & html, QTextDocument & doc, ErrorString & errorDescription,
                             const QVector<SkipHtmlElementRule> & skipRules = QVector<SkipHtmlElementRule>()) const;

    struct NoteContentToHtmlExtraData
    {
        quint64     m_numEnToDoNodes;
        quint64     m_numHyperlinkNodes;
        quint64     m_numEnCryptNodes;
        quint64     m_numEnDecryptedNodes;
    };

    bool noteContentToHtml(const QString & noteContent, QString & html, ErrorString & errorDescription,
                           DecryptedTextManager & decryptedTextManager, NoteContentToHtmlExtraData & extraData) const;

    bool validateEnml(const QString & enml, ErrorString & errorDescription) const;

    bool validateAndFixupEnml(QString & enml, ErrorString & errorDescription) const;

    static bool noteContentToPlainText(const QString & noteContent, QString & plainText,
                                       ErrorString & errorMessage);

    static bool noteContentToListOfWords(const QString & noteContent, QStringList & listOfWords,
                                         ErrorString & errorMessage, QString * plainText = Q_NULLPTR);

    static QStringList plainTextToListOfWords(const QString & plainText);

    static QString toDoCheckboxHtml(const bool checked, const quint64 idNumber);

    static QString encryptedTextHtml(const QString & encryptedText, const QString & hint,
                                     const QString & cipher, const size_t keyLength,
                                     const quint64 enCryptIndex);

    static QString decryptedTextHtml(const QString & decryptedText, const QString & encryptedText,
                                     const QString & hint, const QString & cipher,
                                     const size_t keyLength, const quint64 enDecryptedIndex);

    static QString resourceHtml(const Resource & resource, ErrorString & errorDescription);

    static void escapeString(QString & string, const bool simplify = true);

    /**
     * @brief The EnexExportTags struct is a C++98 style scoped enum which allows to specify whether export of note(s)
     * to ENEX should include the names of note's tags
     */
    struct EnexExportTags
    {
        enum type
        {
            Yes = 0,
            No
        };
    };

    /**
     * @brief exportNotesToEnex - exports either a single note or a set of notes into ENEX format
     *
     * @param notes - the notes to be exported into the enex format. The connection of particular notes to tags
     * is expected to follow from note's tag local uids. In other words, if some note has no tag local uids,
     * its corresponding fragment of ENEX won't contain tag names associated with the note
     * @param tagNamesByTagLocalUids - tag names for all tag local uids across all passed in notes. The lack of any tag
     * name for any tag local uid is considered an error and the overall export attempt fails
     * @param exportTagsOption - whether the export to ENEX should include the names of notes' tags
     * @param enex - the output of the method
     * @param errorDescription - the textual description of the error, if any
     * @param version - optional "version" tag for the ENEX. If not set, the corresponding ENEX tag is set to empty value
     *
     * @return true if the export completed successfully, false otherwise
     */
    bool exportNotesToEnex(const QVector<Note> & notes, const QHash<QString, QString> & tagNamesByTagLocalUids,
                           const EnexExportTags::type exportTagsOption, QString & enex, ErrorString & errorDescription,
                           const QString & version = QString()) const;

    /**
     * @brief importEnex - reads the content of input ENEX file and converts it into a set of notes
     * and tag names.
     *
     * @param enex - the input ENEX file contents
     * @param notes - notes read from the ENEX
     * @param tagNamesByNoteLocalUid - tag names per each read note; it is the responsibility of the method caller
     * to find the actual tags corresponding to these names and set the tag local uids and/or guids to the note
     * @param errorDescription - the textual descrition of the error if the ENEX file could not be read and converted
     * into a set of notes and tag names for them
     *
     * @return true of the ENEX file was read and converted into a set of notes and tag names successfully,
     * false otherwise
     */
    bool importEnex(const QString & enex, QVector<Note> & notes,
                    QHash<QString, QStringList> & tagNamesByNoteLocalUid,
                    ErrorString & errorDescription) const;

private:
    Q_DISABLE_COPY(ENMLConverter)

private:
    ENMLConverterPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(ENMLConverter)
};

} // namespace quentier

#endif // LIB_QUENTIER_ENML_ENML_CONVERTER_H
