#ifndef _COMMENT_COLLECTION_H_
#define _COMMENT_COLLECTION_H_

#include <cstdint>
#include <string>
#include <vector>

class CommentCollection {
public:
	CommentCollection(const std::vector<std::string> & comments = {});
	CommentCollection(std::vector<std::string> && comments);
	CommentCollection(CommentCollection && comments) noexcept;
	CommentCollection(const CommentCollection & comments);
	CommentCollection & operator = (CommentCollection && comments) noexcept;
	CommentCollection & operator = (const CommentCollection & comments);
	virtual ~CommentCollection();

	size_t numberOfComments() const;
	bool hasComment(std::string_view comment) const;
	bool containsComments(const std::vector<std::string> & comments) const;
	bool containsComments(const CommentCollection & comments) const;
	size_t firstIndexOfComment(std::string_view comment) const;
	size_t lastIndexOfComment(std::string_view comment) const;
	const std::string & getComment(size_t index) const;
	const std::vector<std::string> & getComments() const;
	void addComment(std::string_view comment);
	void addComments(const std::vector<std::string> & comments);
	void addComments(const CommentCollection & comments);
	bool replaceComment(size_t index, std::string_view newComment);
	bool replaceFirstInstanceOfComment(std::string_view oldComment, std::string_view newComment);
	bool replaceLastInstanceOfComment(std::string_view oldComment, std::string_view newComment);
	bool insertComment(size_t index, std::string_view comment);
	bool removeComment(size_t index);
	bool removeFirstInstanceOfComment(std::string_view comment);
	bool removeLastInstanceOfComment(std::string_view comment);
	void clearComments();
	void mergeWith(const CommentCollection & comments);

	bool operator == (const CommentCollection & comments) const;
	bool operator != (const CommentCollection & comments) const;

protected:
	std::vector<std::string> m_comments;
};

#endif // _COMMENT_COLLECTION_H_
