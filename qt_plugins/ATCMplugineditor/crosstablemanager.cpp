#include "crosstablemanager.h"

QString CrossTableManager::value(const QtProperty *property)
	const
{
	if (!theValues.contains(property)) return "";
	return theValues[property].value;
}

void CrossTableManager::setValue(QtProperty *property, const QString &val)
{
	if (!theValues.contains(property)) return;
	Data data = theValues[property];
	if (data.value == val) return;
	data.value = val;
	theValues[property] = data;
	emit propertyChanged(property);
	emit valueChanged(property, data.value);
}

