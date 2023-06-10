/****************************************************************************
**
** Copyright (C) 2022 Remy CARISIO.
**
** This file is part of the LogisDom project from Remy CARISIO.
** remy.carisio@orange.fr   http://logisdom.fr
** LogisDom is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.

** LogisDom is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.

** You should have received a copy of the GNU General Public License
** along with LogisDom.  If not, see <https://www.gnu.org/licenses/>
**
****************************************************************************/





#include <QtGui>

#include "highlighter.h"

TextBlockData::TextBlockData()
{
    // Nothing to do
}

QVector<ParenthesisInfo *> TextBlockData::parentheses()
{
    return m_parentheses;
}


void TextBlockData::insert(ParenthesisInfo *info)
{
    int i = 0;
    while (i < m_parentheses.size() &&
        info->position > m_parentheses.at(i)->position)
        ++i;

    m_parentheses.insert(i, info);
}


Highlighter::Highlighter(QTextDocument *document, QStringList keywordPatterns) : QSyntaxHighlighter(document)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
/*    QStringList keywordPatterns;
    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                    << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b";*/
    foreach (const QString &pattern, keywordPatterns)
    {
#if QT_VERSION < 0x060000
        rule.pattern = QRegExp(pattern);
#else
        rule.pattern = QRegularExpression(pattern);
#endif
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
#if QT_VERSION < 0x060000
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
#else
    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
#endif
    rule.format = classFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::gray);
#if QT_VERSION < 0x060000
    rule.pattern = QRegExp("'[^\n]*");
#else
    rule.pattern = QRegularExpression("'[^\n]*");
#endif
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::red);

    quotationFormat.setForeground(Qt::darkGreen);
#if QT_VERSION < 0x060000
    rule.pattern = QRegExp("\".*\"");
#else
    rule.pattern = QRegularExpression("\".*\"");
#endif
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
#if QT_VERSION < 0x060000
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
#else
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
#endif
    rule.format = functionFormat;
    highlightingRules.append(rule);
#if QT_VERSION < 0x060000
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
#else
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
#endif
}





void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
#if QT_VERSION < 0x060000
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
        setCurrentBlockState(0);
        int startIndex = 0;
        if (previousBlockState() != 1)
            startIndex = commentStartExpression.indexIn(text);

        while (startIndex >= 0)
        {
            int endIndex = commentEndExpression.indexIn(text, startIndex);
            int commentLength;
            if (endIndex == -1)
            {
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            }
            else
            {
                commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
            }
            setFormat(startIndex, commentLength, multiLineCommentFormat);
            startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
        }
#else
        QRegularExpression expression(rule.pattern);
        QRegularExpressionMatch match = expression.match(text);
        for (int i = 0; i <= match.lastCapturedIndex(); ++i) {
            qsizetype length = match.capturedLength(i);
            setFormat(match.capturedStart(), length, rule.format);
        }
        setCurrentBlockState(0);
        int startIndex = 0;
        QRegularExpressionMatch matchStartComment = commentStartExpression.match(text);

        if (previousBlockState() != 1)
            if (matchStartComment.hasMatch()) startIndex = match.capturedStart(0);

        /*while (startIndex >= 0)
        {
            int endIndex = commentEndExpression.indexIn(text, startIndex);
            int commentLength;
            if (endIndex == -1)
            {
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            }
            else
            {
                commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
            }
            setFormat(startIndex, commentLength, multiLineCommentFormat);
            startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
        }*/
#endif
    }

    TextBlockData *data = new TextBlockData;

    int leftPos = text.indexOf('(');
    while (leftPos != -1) {
        ParenthesisInfo *info = new ParenthesisInfo;
        info->character = '(';
        info->position = leftPos;

        data->insert(info);
        leftPos = text.indexOf('(', leftPos + 1);
    }

    int rightPos = text.indexOf(')');
    while (rightPos != -1) {
        ParenthesisInfo *info = new ParenthesisInfo;
        info->character = ')';
        info->position = rightPos;

        data->insert(info);

        rightPos = text.indexOf(')', rightPos +1);
    }

    setCurrentBlockUserData(data);
}

