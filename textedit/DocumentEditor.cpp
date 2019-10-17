#include "DocumentEditor.h"


DocumentEditor::DocumentEditor(Document doc, TextEdit* editor, User& user, QObject* parent) 
	: QObject(parent), _document(doc), _textedit(editor), _user(user)
{
	_textedit->setDocumentURI(doc.getURI().toString());
}

void DocumentEditor::openDocument()
{
	QVector<Symbol> document = _document.getContent();
	QList<TextBlockID> blocks = _document.getBlocksBetween(0, _document.length());

	for (int i = 0; i < document.length() - 1; i++){
		_textedit->newChar(document[i].getChar(), document[i].getFormat(), i);
	}

	foreach(TextBlockID id, blocks) {
		TextBlock& blk = _document.getBlock(id);
		_textedit->applyBlockFormat(_document.getBlockPosition(id), blk.getFormat());
	}
	
	_textedit->setCurrentFileName(_document.getName());
	_textedit->startCursorTimer();

	generateExtraSelection();
}


//From Server to Client
void DocumentEditor::addSymbol(Symbol s)
{
	int position = _document.insert(s);
	_textedit->newChar(s.getChar(), s.getFormat(), position, s.getAuthorId());
}

void DocumentEditor::removeSymbol(QVector<int> position)
{
	int pos = _document.remove(position);
	_textedit->removeChar(pos);
}

//From Client to Server
void DocumentEditor::deleteCharAtIndex(int position)
{
	QVector<qint32> fractionalPosition = _document.removeAtIndex(position);
	emit deleteChar(fractionalPosition);
}

void DocumentEditor::addCharAtIndex(QChar ch, QTextCharFormat fmt, int position)
{
	Symbol s;
	if (position == 0) {
		s = Symbol(ch, fmt, _user.getUserId(), _document.fractionalPosBegin());
	}
	else if (position == _document.length()) {
		s = Symbol(ch, fmt, _user.getUserId(), _document.fractionalPosEnd());
	}
	else {
		s = Symbol(ch, fmt, _user.getUserId(), _document.fractionalPosAtIndex(position));
	}

	_document.insert(s);
	emit insertChar(s);
}


//Generating extra selections for user
void DocumentEditor::generateExtraSelection()
{
	QPair<int, int> selectionDelimiters;
	QVector<Symbol> document = _document.getContent();

	qint32 userId = document.first().getAuthorId();
	selectionDelimiters.first = 0;
	selectionDelimiters.second = 0;

	for (int i = 0; i < document.length() - 1; i++) {
		if (document[i].getAuthorId() != userId) {
			_textedit->setExtraSelections(userId, selectionDelimiters);

			userId = document[i].getAuthorId();
			selectionDelimiters.first = i;
			selectionDelimiters.second = i;
		}
		selectionDelimiters.second++;
	}
	_textedit->setExtraSelections(userId, selectionDelimiters);
}

//Block format
void DocumentEditor::changeBlockFormat(int start, int end, QTextBlockFormat fmt)
{
	QList<TextBlockID> blocks = _document.getBlocksBetween(start, end);
	
	foreach(TextBlockID textBlock, blocks) {
		_document.formatBlock(textBlock, fmt);
		emit blockFormatChanged(textBlock, fmt);
	}
}

void DocumentEditor::applyBlockFormat(TextBlockID blockId, QTextBlockFormat fmt)
{
	int position = _document.formatBlock(blockId, fmt);
	_textedit->applyBlockFormat(position, fmt);
}


//Symbol format
void DocumentEditor::changeSymbolFormat(int position, QTextCharFormat fmt)
{
	Symbol s = _document[position];
	_document.formatSymbol(s._fPos, fmt);
	emit symbolFormatChanged(s._fPos, fmt);
}

void DocumentEditor::applySymbolFormat(QVector<qint32> position, QTextCharFormat fmt)
{
	int pos = _document.formatSymbol(position, fmt);
	_textedit->applyCharFormat(pos, fmt);
}



//List editing
void DocumentEditor::listEditBlock(TextBlockID blockId, TextListID listId, QTextListFormat fmt)
{
	// Early out if the local state is already aligned with the server's
	if (_document.getBlock(blockId).getListId() == listId)
		return;

	int blockPos = _document.getBlockPosition(blockId);

	if (!listId)
	{
		// Remove the block from its list
		_textedit->removeBlockFromList(blockPos);
	}
	else if (_document._lists.contains(listId))
	{
		// Add the block to an existing list
		_textedit->addBlockToList(_document.getListPosition(listId), blockPos);
	}
	else
	{
		// Create a new list including the specified block
		_textedit->createList(blockPos, fmt);
	}

	// Apply the remote change in the local document
	_document.editBlockList(blockId, listId, fmt);
}


// Called by textedit when creating a new list that will include the current block
void DocumentEditor::createList(int position, QTextListFormat fmt)
{
	// Create a new TextList
	TextListID newListId(_document._listCounter++, _user.getUserId());
	_document._lists.insert(newListId, TextList(newListId, fmt));

	TextList& list = _document.getList(newListId);
	TextBlock& block = _document.getBlock(_document.getBlockAt(position));

	_document.addBlockToList(block, list);

	// Notify other clients
	emit blockListChanged(block.getId(), list.getId(), fmt);
}


// Called by textedit when assigning a recently inserted block to a list
void DocumentEditor::assignBlockToList(int blockPosition, int listPosition)	
{
	// Get the specified block and the list
	TextBlockID blockId = _document.getBlockAt(blockPosition);
	TextListID listId = _document.getListAt(listPosition);

	if (listId)
	{
		TextBlock& block = _document.getBlock(blockId);
		TextList& list = _document.getList(listId);

		_document.addBlockToList(block, list);

		// Notify other clients
		emit blockListChanged(blockId, listId, list.getFormat());
	}
}


// Handle the press of the list button in the editor
void DocumentEditor::toggleList(int start, int end, QTextListFormat fmt)
{
	// Get all the blocks inside the user selection
	QList<TextBlockID> selectedBlocks = _document.getBlocksBetween(start, end);

	// Get all the lists involved in the operation (at least one block in the selection)
	QList<TextListID> involvedLists;
	foreach(TextBlockID blockId, selectedBlocks)
	{
		TextBlock& block = _document.getBlock(blockId);
		TextListID blockListId = block.getListId();
		if (blockListId && !involvedLists.contains(blockListId))
			involvedLists.append(blockListId);
	}


	foreach(TextListID listId, involvedLists)
	{
		TextList& oldList = _document.getList(listId);
		TextListID newListId;
		QList<TextBlockID> listBlocks = _document.getListBlocks(listId);
		bool selectionBegun = false;
		bool selectionEnded = false;

		foreach(TextBlockID blockId, listBlocks)
		{
			TextBlock& block = _document.getBlock(blockId);

			if (!selectionBegun)	// the block is before the selection
			{
				// Iterate on the list blocks until we encounter the first which is part of the selection
				if (selectedBlocks.contains(blockId))
				{
					selectionBegun = true;
					_document.removeBlockFromList(block, oldList);
				}
			}
			else if (selectionBegun && !selectionEnded)		// the block among those selected
			{
				_document.removeBlockFromList(block, oldList);	// selected blocks are removed from their previous list

				if (!selectedBlocks.contains(blockId))
				{
					selectionEnded = true;

					// Create a new list which will contain all the blocks of the old list which come
					// after the end of the user selection; the new list retains its previous format
					newListId = TextListID(_document._listCounter++, _user.getUserId());
					_document._lists.insert(newListId, TextList(newListId, oldList.getFormat()));

					TextList& newList = _document.getList(newListId);
					_document.removeBlockFromList(block, oldList);
					_document.addBlockToList(block, newList);

					// Apply changes to the editor and notify others
					_textedit->createList(_document.getBlockPosition(blockId), oldList.getFormat());
					emit blockListChanged(blockId, newListId, oldList.getFormat());
				}
			}
			else	// the block belongs to the list but is after the end of the selection
			{
				// Following blocks are added to the list that was born off the split
				TextList& newList = _document.getList(newListId);
				_document.removeBlockFromList(block, oldList);
				_document.addBlockToList(block, newList);

				// Apply changes to the editor and notify others
				_textedit->addBlockToList(_document.getBlockPosition(blockId), _document.getListPosition(newListId));
				emit blockListChanged(blockId, newListId, oldList.getFormat());
			}
		}
	}


	// If the selected blocks are being removed from their lists
	if (fmt.style() == QTextListFormat::ListStyleUndefined)
	{
		foreach(TextBlockID blockId, selectedBlocks)
		{
			// Send for each block a message of it being removed from the list
			emit blockListChanged(blockId, TextListID(nullptr), fmt);
		}
	}
	else	// If a new list format was applied to the blocks
	{
		// Create a new TextList that will contain all the selected blocks
		TextListID newListId(_document._listCounter++, _user.getUserId());
		_document._lists.insert(newListId, TextList(newListId, fmt));
		TextList& list = _document.getList(newListId);

		foreach(TextBlockID blockId, selectedBlocks)
		{
			TextBlock& block = _document.getBlock(blockId);
			_document.addBlockToList(block, list);

			if (list.isEmpty())
			{
				// The first block will take care of creating the list in the Qt editor as well
				_textedit->createList(_document.getBlockPosition(blockId), fmt);
			}
			else
			{
				// Following blocks will be appended to the list
				_textedit->addBlockToList(_document.getBlockPosition(blockId), _document.getListPosition(newListId));
			}

			// Send for each block the message for adding it to the new list
			emit blockListChanged(blockId, newListId, fmt);
		}
	}

}
