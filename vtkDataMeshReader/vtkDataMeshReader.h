#ifndef VTKDATAMESHREADER_H
#define VTKDATAMESHREADER_H

#include <dtkCore/dtkAbstractDataReader.h>
#include <vtkDataMeshReaderPluginExport.h>

class vtkDataSetReader;

class VTKDATAMESHREADERPLUGIN_EXPORT vtkDataMeshReader: public dtkAbstractDataReader {
    Q_OBJECT

public:
    vtkDataMeshReader();
    virtual ~vtkDataMeshReader();

    virtual QStringList handled() const;

    static QStringList s_handled();

public slots:

    virtual bool canRead(const QString& path);
    virtual bool canRead(const QStringList& paths);

    virtual void readInformation(const QString& path);
    virtual void readInformation(const QStringList& paths);

    virtual bool read(const QString& path);
    virtual bool read(const QStringList& paths);

    virtual void setProgress(int value);

    virtual QString identifier()  const;
    virtual QString description() const;

    static bool registered();	

protected:

    vtkDataSetReader* reader;

private:

    static const char ID[];
};


dtkAbstractDataReader *createVtkDataMeshReader();


#endif
