#include "DOSBoxConfiguration.h"

#include <Utilities/StringUtilities.h>

CommentCollection::CommentCollection(const std::vector<std::string> & comments)
	: m_comments(comments)
	, m_commentCollectionModified(false) { }

CommentCollection::CommentCollection(std::vector<std::string> && comments)
	: m_comments(std::move(comments))
	, m_commentCollectionModified(false) { }

CommentCollection::CommentCollection(CommentCollection && comments) noexcept
	: m_comments(std::move(comments.m_comments))
	, m_commentCollectionModified(false) { }

CommentCollection::CommentCollection(const CommentCollection & comments)
	: m_comments(comments.m_comments)
	, m_commentCollectionModified(false) { }

CommentCollection & CommentCollection::operator = (CommentCollection && comments) noexcept {
	if(this != &comments) {
		clearComments();

		m_comments = std::move(comments.m_comments);

		for(size_t i = 0; i < m_comments.size(); i++) {
			commentAdded(*this, m_comments[i], i);
		}

		setCommentCollectionModified(true);
	}

	return *this;
}

CommentCollection & CommentCollection::operator = (const CommentCollection & comments) {
	clearComments();

	m_comments = comments.m_comments;

	for(size_t i = 0; i < m_comments.size(); i++) {
		commentAdded(*this, m_comments[i], i);
	}

	setCommentCollectionModified(true);

	return *this;
}

CommentCollection::~CommentCollection() { }

size_t CommentCollection::numberOfComments() const {
	return m_comments.size();
}

bool CommentCollection::hasComment(std::string_view comment) const {
	return firstIndexOfComment(comment) != std::numeric_limits<size_t>::max();
}

bool CommentCollection::containsComments(const std::vector<std::string> & comments) const {
	if(comments.empty() || m_comments.size() < comments.size()) {
		return false;
	}

	size_t firstCommentIndex = firstIndexOfComment(comments.front());

	if(firstCommentIndex == std::numeric_limits<size_t>::max() || firstCommentIndex + comments.size() - 1 >= m_comments.size()) {
		return false;
	}

	for(size_t i = 1; i < comments.size(); i++) {
		if(!Utilities::areStringsEqual(m_comments[firstCommentIndex + i], comments[i])) {
			return false;
		}
	}

	return true;
}

bool CommentCollection::containsComments(const CommentCollection & comments) const {
	return containsComments(comments.m_comments);
}

size_t CommentCollection::firstIndexOfComment(std::string_view comment) const {
	std::vector<std::string>::const_iterator commentIterator(std::find(m_comments.cbegin(), m_comments.cend(), comment));

	if(commentIterator == m_comments.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return commentIterator - m_comments.cbegin();
}

size_t CommentCollection::lastIndexOfComment(std::string_view comment) const {
	std::vector<std::string>::const_reverse_iterator commentIterator(std::find(m_comments.crbegin(), m_comments.crend(), comment));

	if(commentIterator == m_comments.crend()) {
		return std::numeric_limits<size_t>::max();
	}

	return m_comments.crend() - commentIterator - 1;
}

const std::string & CommentCollection::getComment(size_t index) const {
	if(index >= m_comments.size()) {
		return Utilities::emptyString;
	}

	return m_comments[index];
}

const std::vector<std::string> & CommentCollection::getComments() const {
	return m_comments;
}

void CommentCollection::addComment(std::string_view comment) {
	m_comments.push_back(std::string(comment));

	commentAdded(*this, m_comments.back(), m_comments.size() - 1);

	setCommentCollectionModified(true);
}

void CommentCollection::addComments(const std::vector<std::string> & comments) {
	for(const std::string & comment : comments) {
		addComment(comment);
	}
}

void CommentCollection::addComments(const CommentCollection & comments) {
	return addComments(comments.m_comments);
}

void CommentCollection::setComments(const CommentCollection & comments) {
	m_comments = comments.m_comments;
}

void CommentCollection::setComments(const std::vector<std::string> & comments) {
	m_comments = comments;
}

bool CommentCollection::replaceComment(size_t index, std::string_view newComment) {
	if(index >= m_comments.size()) {
		return false;
	}

	std::string oldComment(m_comments[index]);

	m_comments[index] = newComment;

	commentReplaced(*this, m_comments[index], index, oldComment);

	setCommentCollectionModified(true);

	return true;
}

bool CommentCollection::replaceFirstInstanceOfComment(std::string_view oldComment, std::string_view newComment) {
	return replaceComment(firstIndexOfComment(oldComment), newComment);
}

bool CommentCollection::replaceLastInstanceOfComment(std::string_view oldComment, std::string_view newComment) {
	return replaceComment(lastIndexOfComment(oldComment), newComment);
}

bool CommentCollection::insertComment(size_t index, std::string_view comment) {
	if(index >= m_comments.size()) {
		return false;
	}

	m_comments.insert(m_comments.cbegin() + index, std::string(comment));

	commentInserted(*this, m_comments[index], index);

	setCommentCollectionModified(true);

	return true;
}

bool CommentCollection::removeComment(size_t index) {
	if(index >= m_comments.size()) {
		return false;
	}

	std::string removedComment(m_comments[index]);

	m_comments.erase(m_comments.begin() + index);

	commentRemoved(*this, removedComment, index);

	setCommentCollectionModified(true);

	return true;
}

bool CommentCollection::removeFirstInstanceOfComment(std::string_view comment) {
	return removeComment(firstIndexOfComment(comment));
}

bool CommentCollection::removeLastInstanceOfComment(std::string_view comment) {
	return removeComment(lastIndexOfComment(comment));
}

void CommentCollection::clearComments() {
	if(m_comments.empty()) {
		return;
	}

	m_comments.clear();

	commentsCleared(*this);

	setCommentCollectionModified(true);
}

void CommentCollection::mergeWith(const CommentCollection & comments) {
	if(containsComments(comments)) {
		return;
	}

	addComments(comments);
}

bool CommentCollection::isCommentCollectionModified() const {
	return m_commentCollectionModified;
}

void CommentCollection::setCommentCollectionModified(bool value) {
	m_commentCollectionModified = value;

	commentCollectionModified(*this);
}

bool CommentCollection::operator == (const CommentCollection & comments) const {
	return m_comments == comments.m_comments;
}

bool CommentCollection::operator != (const CommentCollection & comments) const {
	return !operator == (comments);
}
