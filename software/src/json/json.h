#ifndef JSON_H
#define JSON_H

#include <QVariant>

struct JSONData;
class JSON
{
public:
	static JSON& instance();

	QVariant parse(const QString& string) const;
	QString serialize(const QVariant& value) const;

protected:
	JSON();
	~JSON();
private:
	JSONData* d;
};

#endif // JSON_H
