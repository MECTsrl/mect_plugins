#include <QtAbstractPropertyManager>


class CrossTableManager : public QtAbstractPropertyManager
{
    //...
	QString value(const QtProperty *property) const;
	QString filter(const QtProperty *property) const;
public slots:
	void setValue(QtProperty *, const QString &);
	void setFilter(QtProperty *, const QString &);
signals:
	void valueChanged(QtProperty *, const QString &);
	void filterChanged(QtProperty *, const QString &);
protected:
	QString valueText(const QtProperty *property) const
	{ return value(property); }
	void initializeProperty(QtProperty *property)
	{ theValues[property] = Data(); }
	void uninitializeProperty(QtProperty *property)
	{ theValues.remove(property); }
private:
	struct Data {
		QString value;
		QString filter;
	};
	QMap<const QtProperty *, Data> theValues;
};
