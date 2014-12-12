#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#include "json.h"

QScriptValue CreateValue(const QVariant& value, QScriptEngine& engine)
{
	if(value.type() == QVariant::Map)
	{
		QScriptValue obj = engine.newObject();

		QVariantMap map = value.toMap();
		QVariantMap::const_iterator it = map.begin();
		QVariantMap::const_iterator end = map.end();
		while(it != end)
		{
			obj.setProperty( it.key(), ::CreateValue(it.value(), engine) );
			++it;
		}

		return obj;
	}

	if(value.type() == QVariant::List)
	{
		QVariantList list = value.toList();
		QScriptValue array = engine.newArray(list.length());
		for(int i=0; i<list.count(); i++)
			array.setProperty(i, ::CreateValue(list.at(i),engine));

		return array;
	}

	switch(value.type())
	{
	case QVariant::String:
		return QScriptValue(value.toString());
	case QVariant::Int:
		return QScriptValue(value.toInt());
	case QVariant::UInt:
		return QScriptValue(value.toUInt());
	case QVariant::Bool:
		return QScriptValue(value.toBool());
	case QVariant::ByteArray:
		return QScriptValue(QLatin1String(value.toByteArray()));
	case QVariant::Double:
		return QScriptValue((qsreal)value.toDouble());
	default:
		break;
	}

	if(value.isNull())
		return QScriptValue(QScriptValue::NullValue);

	return engine.newVariant(value);
}

JSON& JSON::instance()
{
	static JSON theInstance;
	return theInstance;
}

struct JSONData
{
	QScriptEngine engine;
	QScriptValue parseFn;
	QScriptValue serializeFn;
};

JSON::JSON()
{
	d = new JSONData;

	const QString script = "function parse_json(string) { return JSON.parse(string); }\n"
						   "function serialize_json(object) { return JSON.stringify(object); }";
	QScriptValue result = d->engine.evaluate(script);

	d->parseFn = d->engine.globalObject().property("parse_json");
	d->serializeFn = d->engine.globalObject().property("serialize_json");
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

QString JSON::serialize(const QVariant& value) const
{
	QScriptValue arg = ::CreateValue(value, d->engine);
	QScriptValue result = d->serializeFn.call(QScriptValue(), QScriptValueList() << arg);
	QString resultString = result.toString();
	return resultString;
}
