#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#include "json.h"

JSON& JSON::instance()
{
	static JSON theInstance;
	return theInstance;
}

struct JSONData
{
	QScriptEngine engine;
	QScriptValue parseFn;
};

JSON::JSON()
{
	d = new JSONData;

	const QString script = "function parse_json(string) { return JSON.parse(string); }\n";
	QScriptValue result = d->engine.evaluate(script);

	d->parseFn = d->engine.globalObject().property("parse_json");
}

JSON::~JSON()
{
	delete d;
}

QVariant JSON::parse(const QString& string) const
{
	QScriptValue result = d->parseFn.call(QScriptValue(), QScriptValueList() << QScriptValue(string));
	QVariant resultVariant = result.toVariant();
	return resultVariant;
}
