#ifndef DOCUMENTEDITOR_H
#define DOCUMENTEDITOR_H

#include <QObject>
#include <Document.h>
#include <User.h>
#include "TextEdit.h"


class DocumentEditor : public QObject
{
	Q_OBJECT

private:

	Document _document;
	User& _user;
	TextEdit* _textedit;

public:

	DocumentEditor(Document doc, TextEdit* editor, User& user, QObject* parent = nullptr);
	void openDocument();

public slots:

	// Char operations
	void addSymbol(Symbol s, bool isLast);
	void removeSymbol(QVector<int> position);
	void addCharAtIndex(QChar ch, QTextCharFormat fmt, int position, bool isLast);
	void deleteCharAtIndex(int position);
	void changeSymbolFormat(int position, QTextCharFormat fmt);
	void applySymbolFormat(QVector<qint32> position, QTextCharFormat fmt);

	void generateExtraSelection();		// Text highlighting update

	// Block operations
	void changeBlockAlignment(int start, int end, Qt::Alignment alignment);
	void changeBlockLineHeight(int start, int end, qreal height, int heightType);
	void changeBlockFormat(int start, int end, QTextBlockFormat fmt);
	void applyBlockFormat(TextBlockID blockId, QTextBlockFormat fmt);

	// List operations
	void listEditBlock(TextBlockID blockId, TextListID listId, QTextListFormat fmt);
	void createList(int position, QTextListFormat fmt);
	void assignBlockToList(int blockPosition, int listPosition);
	void removeBlockFromList(int blockPosition);
	void toggleList(int start, int end, QTextListFormat fmt);


signals:

	void deleteChar(QVector<qint32> fPos);
	void insertChar(Symbol s, bool isLast);
	void blockFormatChanged(TextBlockID blockId, QTextBlockFormat fmt);
	void symbolFormatChanged(QVector<qint32> position, QTextCharFormat fmt);
	void blockListChanged(TextBlockID blockId, TextListID listId, QTextListFormat fmt);
};


#endif //DOCUMENTEDITOR_H