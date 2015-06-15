#include ""
class CrossTableEditFactory
: public QtAbstractEditorFactory<CrossTableManager>
{
	...
		private slots:
		void slotPropertyChanged(QtProperty *property,
				const QString &value);
	void slotFilterChanged(QtProperty *property,
			const QString &filter);
	void slotSetValue(const QString &value);
	void slotEditorDestroyed(QObject *object);
	private:
	QMap<QtProperty *, QList<FileEdit *> > createdEditors;
	QMap<FileEdit *, QtProperty *> editorToProperty;
};
